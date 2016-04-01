/*
 * application.cpp
 *
 *  Created on: Mar 11, 2016
 *      Author: wallingt
 */
#include "OneWire/OneWire.h"

String searchStr;
OneWire onewire(D2);
String id = "ow_test";

int enumerate(String command) {
  int i = 0;
  uint8_t nextAddr[8];
  searchStr = "";
  int res = onewire.search(nextAddr);
  while(res == TRUE) {
    searchStr.concat(String::format("Sensor %d: ", i));
    for(int j=0; j<8; j++) {
        searchStr.concat(String::format("%02X", nextAddr[j]));
    }
    i++;
    res = onewire.search(nextAddr);
    if (res) { searchStr.concat("; "); };
  }
  return i;
}

void setup() {
    Particle.function("search", enumerate);
    Particle.variable("sensors", searchStr);
    Particle.variable("id", id);
}

void loop() {
    enumerate("");
}
