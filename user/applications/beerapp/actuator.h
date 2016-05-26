/*
 * actuator.h
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#ifndef USER_APPLICATIONS_MYAPP_ACTUATOR_H_
#define USER_APPLICATIONS_MYAPP_ACTUATOR_H_

#include "application.h"
#include "config.h"



//TODO: Implement wait state logic
enum FridgeState {
	HEATING = 0,
	COOLING = 1,
	IDLE 	= 2,
	BOOT	= 3
};

class FridgeActuator {
	public:
		bool getHeatState();
		FridgeState changeState(FridgeState newState);

		void doorOpen();

		// Send new target & current temp to fridge, it decides the action for itself.
		FridgeState update(double tgt, double curr);
		FridgeActuator();

	private:
		unsigned long lastStateChangeTime = 0;
		FridgeState currentState;

		void doorISR();
		Timer* debounceTimer;

		Timer* minStateTimer;
		Timer* maxStateTimer;
		void minStateTimeExceeded();
		void maxStateTimeExceeded();
		bool stateActive = false;
};

// ISR handling for door state changes - debounce logic is in main loop()
void doorISR ();
extern FridgeActuator* fridgeActuator;


#endif /* USER_APPLICATIONS_MYAPP_ACTUATOR_H_ */
