/*
 * actuator.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#include "actuator.h"
#include "config.h"

ActuatorConfig actuatorConfig;

// ISR flag for door light
volatile bool doorChanged;

void doorISR () {
    doorChanged = true;
    Serial.println("doorISR!");
}


// N.B. debounce delay must happen BEFORE calling this function
void FridgeActuator::doorOpen() {
	doorIsOpen = (digitalRead(DOOR_PIN)) ? false : true;
	Serial.printf("Setting door pin %s\r\n",(currentState == HEATING || doorIsOpen) ? "LOW" : "HIGH");
	digitalWrite(HEAT_PIN, (currentState == HEATING || doorIsOpen) ? LOW : HIGH);
}

FridgeState FridgeActuator::changeState(FridgeState newState) {
	lastStateChangeTime = millis();
	if (newState == currentState) {
		Serial.printf("Ignoring state change request to current state of %s.\r\n",
							actuatorConfig.stateNames[currentState]);
		return currentState;
	}
	Serial.printf("Actuator changing from state %s to %s\r\n",
										actuatorConfig.stateNames[currentState],
										actuatorConfig.stateNames[newState]);

	currentState = newState;

	if (!doorIsOpen) digitalWrite(HEAT_PIN, actuatorConfig.states[newState][0]);
	digitalWrite(COOL_PIN, actuatorConfig.states[newState][1]);

	return currentState;
}

FridgeActuator::FridgeActuator() : currentState(IDLE) {
    pinMode(HEAT_PIN, OUTPUT);
    pinMode(COOL_PIN, OUTPUT);
    digitalWrite(HEAT_PIN, HIGH);
    digitalWrite(COOL_PIN, HIGH);

    pinMode(DOOR_PIN, INPUT_PULLUP);

	lastStateChangeTime = millis();
	doorIsOpen = (digitalRead(DOOR_PIN)) ? true : false;

    attachInterrupt(DOOR_PIN, doorISR, CHANGE);
    Serial.println ("Attached door pin interrupt! Firing it once to be sure :)");
    doorISR();
}


FridgeState FridgeActuator::update(double tgt, double curr) {
	unsigned long now = millis();
	double timediff = (double)(now - lastStateChangeTime) / 1000.0 ;

	Serial.printf("%s min:%.2f max:%.2f diff:%.2f - ",
			actuatorConfig.stateNames[currentState],
			actuatorConfig.stateTimes[currentState][0],
			actuatorConfig.stateTimes[currentState][1],
			timediff);

	if (actuatorConfig.stateTimes[currentState][1] &&
			timediff <  actuatorConfig.stateTimes[currentState][0]) {
		// Waiting on state minimum time
		Serial.println("Waiting on state min time.");
		return currentState;
	}
	if (actuatorConfig.stateTimes[currentState][1] &&
			timediff > actuatorConfig.stateTimes[currentState][1]) {
		// Return to idle if max state time is present and exceeded
		Serial.println("Exceeded state max time");
		return changeState(IDLE);
	}

	double diff = curr - tgt;
	Serial.printf("Considering state change.\r\n Delta:%.2f-%.2f = %.2f  ", curr, tgt, diff);
	FridgeState newState = currentState;
	switch (currentState) {
	case HEATING:
		// Heating, curr is less than tgt, diff is -ve
		if (-diff < actuatorConfig.heatOvershoot) {
			Serial.println("Diff below Heating Overshoot --> IDLE");
			newState = IDLE;
		}
		break;
	case COOLING:
		// Cooling, curr is above tgt, diff is +ve
		if (diff < actuatorConfig.coolOvershoot) {
			Serial.println("Diff below Cooling Overshoot --> IDLE");
			newState = IDLE;
		}
		break;
	case IDLE:
		if (-diff > actuatorConfig.heatThreshold) {
			// Curr below tgt, diff is -ve, --> HEAT
			Serial.println("Diff exceeds Heating Threshold --> HEAT");
			newState = HEATING;
		} else if (diff > actuatorConfig.coolThreshold) {
			// Curr above tgt, diff is +ve, --> COOL
			Serial.println("Diff exceeds Cooling Threshold --> COOL");
			newState = COOLING;
		}
		break;
	}
	if (newState != currentState) {
		Serial.println("Triggering state change");
		return (changeState(newState));
	}
	Serial.println("");
	return (currentState);
}

bool FridgeActuator::getHeatState() {
	return ((currentState == HEATING) ? true : false);
}

