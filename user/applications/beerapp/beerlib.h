#include "OneWire/OneWire.h"
#ifndef BEERLIB
#define BEERLIB

extern OneWire onewire;
retained extern uint8_t beerAddr[8];
retained extern uint8_t fridgeAddr[8];


double getTempByAddr(uint8_t addr[]);

double tempCfromData(uint8_t data[]);

// Get individual temperature readings
double getBeerTemp();
double getFridgeTemp();

// Get both temperatures in parallel
int getTemps(double &beer, double &fridge);

// Used internally
void startTempConv(uint8_t addr[]);
void startTempConvAll();
int readTempData(uint8_t addr[], uint8_t data[]);

#endif
