/*
 * pid_control.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#include "pid_control.h"

PIDConfig pidConfig;

double PIDControl::updatePID(double target, double current) {
	double pErr = 0;
	double dErr = 0;
	unsigned long now = millis();

	if (lastMillis == 0) {
		avgTemp = current;
		pErr = target - current;
		iErr = 0;
		dErr = 0;
	} else {
		// Make sure we can't get a div/0?
		double timeDiff = (double)(now - lastMillis)/1000.0;
		timeDiff = 0.1 > timeDiff ? 0.1 : timeDiff;

		pErr_last = pErr;
		avgTemp *= (pidConfig.smoothing - 1) / pidConfig.smoothing;
		avgTemp += current / pidConfig.smoothing;
		pErr = target - avgTemp;
		iErr += (pErr + pErr_last)/2 * timeDiff;
		iErr = constrain(iErr, -pidConfig.intMax, pidConfig.intMax);

		// This needs some smoothing somehow
		dErr = (pErr - pErr_last) / timeDiff;
	}
	lastMillis = now;
	double actTgtTmp = target + (pidConfig.kP * pErr)
								+ (pidConfig.kI * iErr)
								+ (pidConfig.kD * dErr);

	double actTarget = constrain(actTgtTmp, pidConfig.targetMin, pidConfig.targetMax);

	// back-calculate integrator state in case we constrained the target temp
	if (pidConfig.kI && actTarget != actTgtTmp) {
		double backCalc = (actTarget - (target + (pidConfig.kP * pErr)
								 	 + (pidConfig.kD * dErr)) / pidConfig.kI);
		iErr = constrain(backCalc, -pidConfig.intMax, pidConfig.intMax);
	}


	snprintf(pidStatusMsg, 64, "T:t:%.2f,c:%.2f,s:%.2f P:p:%.2f,i:%.2f,d:%.2f T:o:%.2f",
					   target, current, avgTemp, pErr,  iErr,   dErr,   actTarget);
	Serial.println(pidStatusMsg);
	Particle.publish("pidStatus", pidStatusMsg);
	int_state = iErr;
	return actTarget;
}


