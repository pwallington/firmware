/*
 * actuator.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#include "actuator.h"
#include "config.h"

#define INIT_TIME 10 * 1000

ActuatorConfig actuatorConfig;

void FridgeActuator::doorISR () {
    Serial.printf("%8d		doorISR!	%d\r\n", millis(), digitalRead(DOOR_PIN));
    debounceTimer->resetFromISR();
}

// N.B. debounce delay must happen BEFORE calling this function
void FridgeActuator::doorOpen() {
	bool open = ! digitalRead(DOOR_PIN);
	Serial.printf("Door pin %s, setting heat pin %s\r\n",
					(open ? "HIGH" : "LOW"),
					(currentState == HEATING || !open) ? "LOW" : "HIGH");
	digitalWrite(HEAT_PIN, (currentState == HEATING || !open));
	Particle.publish(open ? "doorOpened" : "doorClosed");
}

FridgeState FridgeActuator::changeState(FridgeState newState) {
	lastStateChangeTime = millis();
	Serial.printf("Actuator changing from state %s to %s\r\n",
										actuatorConfig.stateNames[currentState],
										actuatorConfig.stateNames[newState]);
	Particle.publish("stateChange", String::format("%s --> %s",
											actuatorConfig.stateNames[currentState],
											actuatorConfig.stateNames[newState]));
	if (newState == currentState) {
		Serial.printf("Ignoring state change request to current state of %s.\r\n",
							actuatorConfig.stateNames[currentState]);
		return currentState;
	}
	currentState = newState;

	minStateTimer->changePeriod(1000*actuatorConfig.stateTimes[currentState][0]);
	minStateTimer->start();
	maxStateTimer->changePeriod(1000*actuatorConfig.stateTimes[currentState][1]);
	maxStateTimer->start();

	// Set heat state if door is closed (HIGH), otherwise just leave it for the door handler
	if (digitalRead(DOOR_PIN)) digitalWrite(HEAT_PIN, actuatorConfig.states[newState][0]);
	digitalWrite(COOL_PIN, actuatorConfig.states[newState][1]);

	return currentState;
}

FridgeActuator::FridgeActuator() : currentState(IDLE) {
    pinMode(HEAT_PIN, OUTPUT);
    pinMode(COOL_PIN, OUTPUT);
    digitalWrite(HEAT_PIN, HIGH);
    digitalWrite(COOL_PIN, HIGH);

    pinMode(DOOR_PIN, INPUT_PULLUP);

    // TODO: Deprecate lastStateChangeTime
	lastStateChangeTime = millis();

	// Initialise timers
	debounceTimer = new Timer(40, 		 &FridgeActuator::doorOpen,				*this, true);
	minStateTimer = new Timer(INIT_TIME, &FridgeActuator::minStateTimeExceeded, *this, true);
	maxStateTimer = new Timer(1000, 	 &FridgeActuator::maxStateTimeExceeded, *this, true);
	minStateTimer->start();

    attachInterrupt(DOOR_PIN, &FridgeActuator::doorISR, this, CHANGE);
    Serial.println ("Attached door pin interrupt! Firing it once to be sure :)");
    doorISR();
}

void FridgeActuator::minStateTimeExceeded() {
	Serial.println("Exceeded state min time - now allowing state changes");
	stateActive = true;
}

void FridgeActuator::maxStateTimeExceeded() {
	// Return to idle if max state time is present and exceeded
	Serial.println("Exceeded state max time");
	changeState(IDLE);
}

FridgeState FridgeActuator::update(double tgt, double curr) {
	unsigned long now = millis();
	double timediff = (double)(now - lastStateChangeTime) / 1000.0 ;

	Serial.printf("%s min:%.2f max:%.2f diff:%.2f - ",
			actuatorConfig.stateNames[currentState],
			actuatorConfig.stateTimes[currentState][0],
			actuatorConfig.stateTimes[currentState][1],
			timediff);

	if (stateActive == false) {
		// Waiting on state minimum time
		Serial.println("Waiting on state min time.");
		return currentState;
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

