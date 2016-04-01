/*
 * pid_control.h
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#ifndef USER_APPLICATIONS_MYAPP_PID_CONTROL_H_
#define USER_APPLICATIONS_MYAPP_PID_CONTROL_H_
#include "application.h"
#include "config.h"

extern double int_state;

class PIDControl {
	public:
		// returns new actuator target
		double updatePID(double target, double current);

		PIDControl(double int_val) : iErr(int_val) {
			Particle.variable("pidStatus", pidStatusMsg);
		};

	private:
		double iErr;
		double avgTemp = 0;
		double pErr_last = 0;
		unsigned long lastMillis = 0;
		char pidStatusMsg[64];
};



#endif /* USER_APPLICATIONS_MYAPP_PID_CONTROL_H_ */
