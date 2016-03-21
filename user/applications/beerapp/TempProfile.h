/*
 * TempProfile.h
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#ifndef USER_APPLICATIONS_MYAPP_TEMPPROFILE_H_
#define USER_APPLICATIONS_MYAPP_TEMPPROFILE_H_

#define MSEC_PER_HOUR 3600000
#define DELTA_STEPS 0.01


class TempProfileStep {
	public:
		double targetTemp;
		unsigned long startTime;
		unsigned long rampDuration;
		unsigned long holdDuration;
		unsigned long deltaInterval;
		TempProfileStep* nextStep;
		TempProfileStep* prevStep;
		TempProfileStep(char* stepStr, TempProfileStep* prev);
};

class TempProfile {
	public:
		double startTemp;
		TempProfileStep* start;
		TempProfileStep* current;
		TempProfileStep* last;
		int steps;
		TempProfile(char* profileStr, double startT);
};




#endif /* USER_APPLICATIONS_MYAPP_TEMPPROFILE_H_ */
