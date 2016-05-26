/*
 * cloud_funcs.h - functions for cloud/API interface
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#ifndef USER_APPLICATIONS_MYAPP_CLOUD_FUNCS_H_
#define USER_APPLICATIONS_MYAPP_CLOUD_FUNCS_H_

#include "application.h"
#include "OneWire.h"
#include "config.h"
#include "ThingSpeak.h"

#define TS_CHANNEL 102415
#define TS_API_KEY "OU2ESK3M7JBHPB4H"

extern OneWire onewire;
extern double beerTemp;
extern double fridgeTemp;
extern bool setupDone;

void cloudInit();

int enumerate(String command);

int updateTemps(String command);
int assignSensors(String command);
int updateTarget(String command);
int publishStatus(double beerTemp, double fridgeTemp, double target, int state, double actuator);

#endif /* USER_APPLICATIONS_MYAPP_CLOUD_FUNCS_H_ */
