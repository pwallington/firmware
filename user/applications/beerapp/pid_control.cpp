/*
 * pid_control.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#include "pid_control.h"

double avgTemp = 0;
double pErr_last = 0;
unsigned long lastMillis = 0;

double updatePID(double target, double current) {
	double pErr = 0;
	double dErr = 0;
	double actTgtTmp = 0;
	double actTarget = 0;

	unsigned long now = millis();
    Serial.println("In PID Update"); //delay(500);
	if (lastMillis == 0) {
		avgTemp = current;
		pErr = target - current;
	    Serial.println("Fist PID Update!"); //delay(500);
	} else {
		// Make sure we can't get a div/0?
		double timeDiff = (double)(now - lastMillis)/1000.0;
		timeDiff = 0.1 > timeDiff ? 0.1 : timeDiff;
		Serial.print("Timediff = ");
		Serial.println(timeDiff); //delay(500);

		pErr_last = pErr;
		avgTemp *= (pidConfig.smoothing - 1) / pidConfig.smoothing;
		avgTemp += current / pidConfig.smoothing;
		pErr = target - avgTemp;
		iErr += ((pErr + pErr_last)/2.0) * timeDiff / 60.0;
		iErr = constrain(iErr, -pidConfig.intMax, pidConfig.intMax);

		// This needs some smoothing somehow
		dErr = (pErr - pErr_last) * 60.0 / timeDiff;
	}
	lastMillis = now;
	Serial.printf("calculating target temp (%d)\r\n", lastMillis);//delay(1000);

	actTgtTmp = target + (pidConfig.kP * pErr) + (pidConfig.kI * iErr) + (pidConfig.kD * dErr);

	Serial.print("Actuator target:"); Serial.println(actTgtTmp);//delay(500);

	actTarget = constrain(actTgtTmp, pidConfig.targetMin, pidConfig.targetMax);
	Serial.print("Constrained target:"); Serial.println(actTarget);//delay(500);

	// back-calculate integrator state in case we constrained the target temp
	if (pidConfig.kI && actTarget != actTgtTmp) {
		Serial.println("Doing back-calculation"); //delay(500);
		double backCalc = (actTarget - target - (pidConfig.kP * pErr) - (pidConfig.kD * dErr)) / pidConfig.kI;
//		iErr = constrain(backCalc, -pidConfig.intMax, pidConfig.intMax);
//		Particle.publish("backCalc", String::format("%f", backCalc));
		Serial.print("BackCalc:"); Serial.println(backCalc); //delay(500);
	}

//	Somehow this causes a hard fault?!
//	snprintf(pidStatusMsg, 63, "T:t:%.2f,c:%.2f,s:%.2f P:p:%.2f,i:%.2f,d:%.2f T:o:%.2f",
//					   			target, current, avgTemp, pErr,  iErr,   dErr,   actTarget);
//	Serial.println(pidStatusMsg);
//	Particle.publish("pidStatus", pidStatusMsg);

	Serial.println("PID Update finished");// delay(500);
	return actTarget;
}


