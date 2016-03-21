#include <cloud_funcs.h>
#include "sensor.h"
#include "config.h"

// Cloud variables
double beerTemp;
double fridgeTemp;
String beerAddrStr;
String fridgeAddrStr;
String tempsStr;
Sensor* sensors[5];
String sensorStr;
String appname = "beermentor";

void cloudInit() {
    Particle.function("assign", assignSensor);
    Particle.function("update", updateTemps);
    Particle.function("updateTarget", updateTarget);
    Particle.function("search", enumerate);

    Particle.variable("sensors", sensorStr);
    Particle.variable("beerAddr", beerAddrStr);
    Particle.variable("fridgeAddr", fridgeAddrStr);
    Particle.variable("temps", tempsStr);
    Particle.variable("id", appname);
    Particle.variable("target", targetTemp);
}

/* Enumerate the sensors so they can be assigned to roles */
int enumerate(String command) {
	for (int i = 0; i < 5; ++i) {
		if (sensors[i]) {
			delete sensors[i];
			sensors[i] = NULL;
		}
	}
	int i = 0;
	uint8_t nextAddr[8];
	sensorStr = "";
	int res = onewire.search(nextAddr);
	while(res == TRUE) {
		Serial.print("Found a sensor!");
		Serial.println(i);

		Sensor* newSensor = new Sensor(nextAddr);
	    sensorStr.concat(String::format("Sensor %d: ", i));
		for(int j=0; j<8; j++) {
			sensorStr.concat(String::format("%02X", nextAddr[j]));
		}
		sensors[i++] = newSensor;
		res = onewire.search(nextAddr);
		if (res) { sensorStr.concat("; "); };
	}
	Serial.println(sensorStr);
	return i;
}

/* Assign a sensor (by index) to a role */
int assignSensor(String command) {
    Serial.println(command);
    //convert ASCII to integer
    unsigned int sensorNumber = command.charAt(0) - '0';
    //Sanity check to see if the pin numbers are within limits
    if (sensorNumber < 0 || sensorNumber > arraySize(sensors)) return -1;

    uint8_t* dest;
    String name;
    if(command.substring(2,6) == "beer") {
        dest = beerAddr;
        name = "beer";
        beerAddrStr = "";
        for(int j=0; j<8; j++) {
            beerAddrStr.concat(String::format("%02X", sensors[sensorNumber]->addr[j]));
        }
    }
    else if(command.substring(2,8) == "fridge") {
        dest = fridgeAddr;
        name = "fridge";
        fridgeAddrStr = "";
        for(int j=0; j<8; j++) {
            fridgeAddrStr.concat(String::format("%02X", sensors[sensorNumber]->addr[j]));
        }
    }
    else return -2;

    memcpy(dest, sensors[sensorNumber]->addr, 8);
    return 0;
}

/* Get temperature readings, and format them for the cloud variable */
int updateTemps(String command){
    if (beerAddr[0] != 0x28 || fridgeAddr[0] != 0x28) {
    	Serial.println("Sensors not set up yet...");
        return -2;
    }
    tempsStr = "";
    int ret = getTemps(beerTemp, fridgeTemp);
    if (ret != 0) {
        return -1;
    }
    tempsStr = String::format("beer:%.2f fridge:%.2f", beerTemp, fridgeTemp);
    Serial.println(tempsStr);
    return 0;
}

int updateTarget(String command) {
	double tgt = 0;
	tgt = strtod(command, NULL);
	if (tgt == 0) return 2;
	targetTemp = constrain(tgt, pidConfig.targetMin, pidConfig.targetMax);
	Serial.printf("New target temp: %.2f", tgt);
	if (targetTemp != tgt) {
		Serial.printf(" constrained to %.2f\r\n", targetTemp);
		return 1;
	}
	Serial.println();
	return 0;
}

int publishStatus(double beerTemp, double fridgeTemp, int state) {
	return Particle.publish("status", String::format("{ beerTemp: %.2f, fridgeTemp: %.2f, state: %s }",
		beerTemp, fridgeTemp, actuatorConfig.stateNames[state]), 60, PRIVATE);
}
