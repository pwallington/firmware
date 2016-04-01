/*
 * config.h
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#ifndef USER_APPLICATIONS_MYAPP_CONFIG_H_
#define USER_APPLICATIONS_MYAPP_CONFIG_H_

#define HEAT_PIN D7
#define COOL_PIN D6
#define DOOR_PIN D1


enum ControlMode {
	INACTIVE,
	BEER_CONTROL,
	FRIDGE_CONTROL,
};

struct PIDConfig {
	const double kP =  4.0;
	const double kI =  0.2;
	const double kD = 10.0;

	const double smoothing = 20;
	const double intMax = 50;

	// Actuator limits for safety.
	const double targetMin = 20;
	const double targetMax = 85;
};

struct ActuatorConfig {
	// Hysteresis values
	// Eventually peak detection will account for these automatically...
	const double coolThreshold = 1.5;
	const double heatThreshold = 1;
	const double coolOvershoot = 0.8;
	const double heatOvershoot = 0.4;

	// Actuator state time bounds:   		min, max N.B. trimmed to fire before next timer interval.
	const unsigned int stateTimes[3][2] = {{ 57,   0},  // HEATING
										  { 177, 598},  // COOLING
										  { 297,   0}}; // IDLE

	// Actuator pin states:  		 heat  cool
	const uint8_t states[3][2] = {{  LOW, HIGH },  // HEATING
								  { HIGH,  LOW },  // COOLING
								  { HIGH, HIGH }}; // IDLE

	const char stateNames[4][5] = { "HEAT", "COOL", "IDLE", "BOOT" };
};

extern double targetTemp;
extern PIDConfig pidConfig;
extern ActuatorConfig actuatorConfig;

#endif /* USER_APPLICATIONS_MYAPP_CONFIG_H_ */
