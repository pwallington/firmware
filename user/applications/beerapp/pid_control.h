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

double updatePID(double target, double current);
extern double  iErr;



#endif /* USER_APPLICATIONS_MYAPP_PID_CONTROL_H_ */
