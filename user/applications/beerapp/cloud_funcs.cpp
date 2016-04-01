#include <cloud_funcs.h>
#include "sensor.h"
#include "config.h"

// Cloud variables
double beerTemp;
double fridgeTemp;
char statusStr[65];
char tempsStr[25];
Sensor* sensors[5];
unsigned int sensorCount = 0;
char searchStr[84];
char sensorStr[84];

void makeSensorStr() {
	// Initialise beer and fridge address strings in case they're already stored
	char* tgt = sensorStr;
	tgt[0] = 0;
	if (beerAddr[0] == 0x28) {
		tgt += sprintf(tgt, "beer: %02X-%02X%02X%02X-%02X%02X%02X%02X",beerAddr[0],
				beerAddr[1], beerAddr[2], beerAddr[3],
				beerAddr[4], beerAddr[5], beerAddr[6], beerAddr[7]);
	}
	if 	(tgt != sensorStr) {
		tgt += sprintf(tgt, ", ");
	}
	if (fridgeAddr[0] == 0x28) {
		tgt += sprintf(tgt,	"fridge: %02X-%02X%02X%02X-%02X%02X%02X%02X", fridgeAddr[0],
				fridgeAddr[1], fridgeAddr[2], fridgeAddr[3],
				fridgeAddr[4], fridgeAddr[5], fridgeAddr[6], fridgeAddr[7]);
	}
}

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

	searchStr[0] = 0;
	// Target format:
	// Sensor 0: 28B3956E06000071; Sensor 1: 28B3956E06000071
	int res = onewire.search(nextAddr);
	while(res == TRUE) {
		Serial.print("Found a sensor!");
		Serial.println(i);

		Sensor* newSensor = new Sensor(nextAddr);
		sprintf(searchStr+strlen(searchStr), "Sensor %d: ", i);
		for(int j=0; j<8; j++) {
			sprintf(searchStr+strlen(searchStr), "%02X", nextAddr[j]);
		}
		sensors[i++] = newSensor;
		res = onewire.search(nextAddr);
		if (res) sprintf(searchStr+strlen(searchStr), "; ");
	}
	Serial.println(searchStr);
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
    	const char* name;

		//convert ASCII to integer
		unsigned int sensorNumber = tok[0] - '0';

		//Sanity check to see if the pin numbers are within limits
		if (sensorNumber < 0 || sensorNumber >= sensorCount) {
			return -1;
		}

		if(tok[2] == 'b') {
			tgtAddr = beerAddr;
			name = "beer";
		}
		else if(tok[2] == 'f') {
			tgtAddr = fridgeAddr;
			name = "fridge";
		}
		else return -2;

		memcpy(tgtAddr, sensors[sensorNumber]->addr, 8);
		Serial.printf("Assigned sensor %d to \"%s\"\r\n", sensorNumber, name);
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
