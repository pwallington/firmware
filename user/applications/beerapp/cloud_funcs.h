/*
 * cloud_funcs.h - functions for cloud/API interface
 *
 *  Created on: Mar 8, 2016
 *      Author: wallingt
 */

#ifndef USER_APPLICATIONS_MYAPP_CLOUD_FUNCS_H_
#define USER_APPLICATIONS_MYAPP_CLOUD_FUNCS_H_

#include "application.h"
#include "OneWire/OneWire.h"
#include "config.h"

extern OneWire onewire;
extern double beerTemp;
extern double fridgeTemp;

void cloudInit();

int enumerate(String command);

int updateTemps(String command);
int assignSensor(String command);
int updateTarget(String command);
int publishStatus(double beerTemp, double fridgeTemp, int state);

#endif /* USER_APPLICATIONS_MYAPP_CLOUD_FUNCS_H_ */
