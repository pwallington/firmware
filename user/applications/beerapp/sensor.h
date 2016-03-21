/*
 * sensor.h
 *
 *  Created on: Mar 9, 2016
 *      Author: wallingt
 */

#include "beerlib.h"

#ifndef USER_APPLICATIONS_MYAPP_SENSOR_H_
#define USER_APPLICATIONS_MYAPP_SENSOR_H_


class Sensor {
	public:
		uint8_t addr[8];
		char	label[20];

		Sensor(uint8_t* newaddr) {
			memcpy(addr, newaddr, 8);
		}

	private:
};

#endif /* USER_APPLICATIONS_MYAPP_SENSOR_H_ */
