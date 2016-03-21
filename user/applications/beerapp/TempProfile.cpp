/*
 * TempProfile.cpp
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */
#include "application.h"
#include "TempProfile.h"

TempProfileStep::TempProfileStep(char* stepStr, TempProfileStep* prev)  {
	int rampHrs = 0;
	int target = 0;
	int holdHrs = 0;
	nextStep = NULL;

	if (prev != NULL) {
		startTime = prev->startTime + prev->rampDuration + prev->holdDuration;
		prevStep = prev;
	}
	else {
		startTime = millis();
		prevStep = NULL;
	}
	if (2 < sscanf(stepStr, "%2d%2d%3d", &rampHrs, &target, &holdHrs)) {
		// Millis!
		rampDuration = rampHrs * MSEC_PER_HOUR;
		holdDuration = holdHrs * MSEC_PER_HOUR;
		targetTemp = target + 0.0;
		if (rampDuration > 0 && prevStep != NULL) {
			float tempDiff = targetTemp - prev->targetTemp;
			deltaInterval = tempDiff / (holdDuration * DELTA_STEPS);
		}
	}

}

TempProfile::TempProfile(char* profile, double startT) : startTemp(startT), last(NULL) {
	char * pch;
	steps = 0;

  pch = strtok (profile,";");
  while (pch != NULL) {
  	TempProfileStep* newStep = new TempProfileStep(pch, last);
  	if (newStep->targetTemp == 0) {
  		Serial.printf("Error handling profile step: %s\n", pch);
  		break;
  	}
  	if (steps == 0) {
  		start = newStep;
  		newStep->prevStep = NULL;
  		last = newStep;
  		current = start;
  	} else {
  		last->nextStep = newStep;
  		newStep->prevStep = last;
  		last = newStep;
  	}
		steps++;
  }

	return;
}



