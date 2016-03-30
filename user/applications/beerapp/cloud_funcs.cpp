#include <cloud_funcs.h>
#include "sensor.h"
#include "config.h"

// Cloud variables
double beerTemp;
double fridgeTemp;
char beerAddrStr[20] = {0,};
char fridgeAddrStr[20] = {0,};

char tempsStr[25];
Sensor* sensors[5];
unsigned int sensorCount = 0;
char sensorStr[84];
char appname[] = "beermentor_pub";

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

	sprintf(sensorStr, "");
	// Sensor 0: 28B3956E06000071; Sensor 1: 28B3956E06000071
	int res = onewire.search(nextAddr);
	while(res == TRUE) {
		Serial.print("Found a sensor!");
		Serial.println(i);

		Sensor* newSensor = new Sensor(nextAddr);
		sprintf(sensorStr+strlen(sensorStr), "Sensor %d: ", i);
		for(int j=0; j<8; j++) {
			sprintf(sensorStr+strlen(sensorStr), "%02X", nextAddr[j]);
		}
		sensors[i++] = newSensor;
		res = onewire.search(nextAddr);
		if (res) sprintf(sensorStr+strlen(sensorStr), "; ");
	}
	Serial.println(sensorStr);
	sensorCount = i;
	return sensorCount;
}

/* Assign a sensor (by index) to a role */
int assignSensor(String command) {
    Serial.print("Assign sensors: ");
    Serial.println(command);

    // Tokenize the string
    char cmd[64] = {0,};
    strncpy(cmd, command.c_str(), 63);
    char* tok = strtok(cmd, " ");


    while (tok) {
    	uint8_t* tgtAddr;
    	char* 	 tgtStr;
    	const char* name;

		//convert ASCII to integer
		unsigned int sensorNumber = tok[0] - '0';

		//Sanity check to see if the pin numbers are within limits
		if (sensorNumber < 0 || sensorNumber >= sensorCount) {
			return -1;
		}

		if(tok[2] == 'b') {
			tgtAddr = beerAddr;
			tgtStr  = beerAddrStr;
			name = "beer";
		}
		else if(tok[2] == 'f') {
			tgtAddr = fridgeAddr;
			tgtStr  = fridgeAddrStr;
			name = "fridge";
		}
		else return -2;

		memcpy(tgtAddr, sensors[sensorNumber]->addr, 8);
		sprintf(tgtStr, "%02X-%02X%02X%02X %02X%02X%02X%02X",
					sensors[sensorNumber]->addr[0],
					sensors[sensorNumber]->addr[1],
					sensors[sensorNumber]->addr[2],
					sensors[sensorNumber]->addr[3],
					sensors[sensorNumber]->addr[4],
					sensors[sensorNumber]->addr[5],
					sensors[sensorNumber]->addr[6],
					sensors[sensorNumber]->addr[7]);

		Serial.printf("Assigned sensor %d to \"%s\" (%s)\r\n", sensorNumber, name, tgtStr);
		tok = strtok(NULL, " ");
    }
    return 0;
}

/* Get temperature readings, and format them for the cloud variable */
int updateTemps(String command) {
    if (beerAddr[0] != 0x28 || fridgeAddr[0] != 0x28) {
    	Serial.println("Sensors not set up yet...");
        return -2;
    }

    int ret = getTemps(beerTemp, fridgeTemp);
    if (ret != 0) {
        return -1;
    }
    snprintf(tempsStr, 24, "beer:%.2f fridge:%.2f", beerTemp, fridgeTemp);
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
	char msg[54];
	snprintf(msg, 54, "{ beerTemp: %.2f, fridgeTemp: %.2f, state: %s }",
			beerTemp, fridgeTemp, actuatorConfig.stateNames[state]);
	Particle.publish("status", msg);
	return 0;
}
