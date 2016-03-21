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
		double 	temp = 0;
		char	label[20];

		int 	matchAddr(uint8_t* addr);
		void	setLabel(const char* newLabel) {
			strncpy(label, newLabel, 19);
		}

		double getTemp() {
			unsigned long now = millis();
			uint8_t data[10];

			// Don't convert again if we're within the caching time
			if ((now - lastConvTime) > cacheTime) {
		 		setConvTime(now);
				startTempConv(addr);
		 		delay(800);
		 	}

		 	// Only re-read if there is no cached value
			if (temp != 0) {
			 	int ret = readTempData(addr, data);
				if (ret != 0) {
					return -255;
				}
				temp = tempCfromData(data);
			}
			return temp;
		}


		// Should be called when we issue a 'convert all' cmd
		void setConvTime(unsigned long millis) {
			lastConvTime = millis;
			temp = 0;
		}

		Sensor(uint8_t* newaddr) {
			memcpy(addr, newaddr, 8);
			lastConvTime = 0;
		}

	private:
		unsigned long lastConvTime;
		const unsigned long cacheTime = 5000;
};

#endif /* USER_APPLICATIONS_MYAPP_SENSOR_H_ */
