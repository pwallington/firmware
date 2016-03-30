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

retained extern double int_state;

class PIDControl {
	public:
		// returns new actuator target
		double updatePID(double target, double current);

	private:
		double iErr = int_state;
		double avgTemp = 0;
		double pErr_last = 0;
		unsigned long lastMillis = 0;
};



#endif /* USER_APPLICATIONS_MYAPP_PID_CONTROL_H_ */
