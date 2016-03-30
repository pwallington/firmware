#include "Particle.h"
STARTUP(System.enableFeature(FEATURE_RETAINED_MEMORY));

/* Includes ------------------------------------------------------------------*/

#include "stdarg.h"
#include "beerlib.h"
#include "OneWire/OneWire.h"
#include "cloud_funcs.h"
#include "actuator.h"
#include "my_eeprom.h"
#include "pid_control.h"



#define INTERVAL 15000
void timerRoutine();
Timer controlTimer(INTERVAL, timerRoutine);
Timer retryTimer(5000, timerRoutine, true);


// Global Objects
PIDControl* pidController;
FridgeActuator* fridgeActuator;
retained double targetTemp = 75.0;
retained uint8_t beerAddr[8] = {0,};
retained uint8_t fridgeAddr[8] = {0,};
retained double  int_state = 0;


void setup() {
    Serial.begin(57600);
    Time.zone(-6);
    Particle.syncTime();
    cloudInit();

    Serial.print("Starting beermentor at ");
    Serial.println(Time.timeStr());

    // Read EEPROM data
    // readEeprom();

    enumerate("");
    pidController = new PIDControl();
    fridgeActuator = new FridgeActuator();
    delay(500);
    controlTimer.start();

}

void timerRoutine() {
	Serial.printf("In Timer routine. Free mem: %d\r\n", System.freeMemory());

    //TODO: check loop conditions?
	// Update for cloud API
    if (updateTemps("")) {
    	Serial.println("Bad response from updateTemps()");

    	if (beerAddr[0] && fridgeAddr[0]) retryTimer.start();
    	return;
    }

    if (!beerTemp || !fridgeTemp) {
        Serial.println("Error reading temperatures!");
        return;
    }

//    double fridgeTarget = targetTemp;
    double fridgeTarget = pidController->updatePID(targetTemp, beerTemp);
    FridgeState ret = fridgeActuator->update(fridgeTarget, fridgeTemp);

    publishStatus(beerTemp, fridgeTemp, ret);

}
