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
	bool newPinState = (currentState == HEATING || open) ? LOW : HIGH;
	Serial.printf("Door is %s, setting heat pin %d\r\n", (open ? "OPEN" : "CLOSED"), newPinState);
	digitalWrite(HEAT_PIN, newPinState);
	Particle.publish(open ? "doorOpened" : "doorClosed");
}

FridgeState FridgeActuator::changeState(FridgeState newState) {
	lastStateChangeTime = millis();
	char msg[30];
	snprintf(msg, 30, "from:%s, to:%s",	actuatorConfig.stateNames[currentState],
									actuatorConfig.stateNames[newState]);
	Serial.printf("Actuator changing state: %s\r\n", msg);
	Particle.publish("stateChange", msg);
	if (newState == currentState) {
		Serial.printf("Ignoring state change request to current state of %s.\r\n",
							actuatorConfig.stateNames[currentState]);
		return currentState;
	}
	currentState = newState;
	stateActive = false;
	uint minTime = 1000*actuatorConfig.stateTimes[currentState][0];
	uint maxTime = 1000*actuatorConfig.stateTimes[currentState][1];
	Serial.printf("Starting State min/max timers: %d / %d\r\n", minTime, maxTime);
	minStateTimer->changePeriod(minTime);
	minStateTimer->reset();
	if (maxTime > 0) {
		maxStateTimer->changePeriod(maxTime);
		maxStateTimer->reset();
	}
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
	debounceTimer = new Timer(40, 	 &FridgeActuator::doorOpen,				*this, true);
	minStateTimer = new Timer(INIT_TIME, &FridgeActuator::minStateTimeExceeded, *this, true);
	maxStateTimer = new Timer(300000, &FridgeActuator::maxStateTimeExceeded, *this, true);
	minStateTimer->start();

    attachInterrupt(DOOR_PIN, &FridgeActuator::doorISR, this, CHANGE);
    Serial.println ("Attached door pin interrupt! Firing it once to be sure :)");
    doorISR();
}

void FridgeActuator::minStateTimeExceeded() {
	Serial.println("Exceeded state min time - now allowing state changes");
	Particle.publish("stateMinTimePassed", actuatorConfig.stateNames[currentState]);
	stateActive = true;
}

void FridgeActuator::maxStateTimeExceeded() {
	// Return to idle if max state time is present and exceeded
	Serial.println("Exceeded state max time");
	Particle.publish("stateMaxTimePassed", actuatorConfig.stateNames[currentState]);
	changeState(IDLE);
}

FridgeState FridgeActuator::update(double tgt, double curr) {
	unsigned long now = millis();
	double timediff = (double)(now - lastStateChangeTime) / 1000.0 ;

	Serial.printf("%s min:%d max:%d diff:%.2f - ",
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

