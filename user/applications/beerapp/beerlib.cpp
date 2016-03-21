#include "beerlib.h"


OneWire onewire(D2);


double getBeerTemp() {
  return getTempByAddr(beerAddr);
}

double getFridgeTemp() {
  return getTempByAddr(fridgeAddr);
}

int getTemps(double &beer, double &fridge) {
  int ret = 0;
  byte f_data[9], b_data[9];

  startTempConvAll();

  // maybe 750ms is enough, maybe not, so use 800 instead
  delay(800);     

  ret = readTempData(beerAddr, b_data);
  if (ret != 0) {
	Serial.println("Bad Beer temp read");
    return -255;
  }
  ret = readTempData(fridgeAddr, f_data);
  if (ret != 0) {
	Serial.println("Bad Fridge temp read");
    return -255;
  }

  beer =   32.0 + 1.8 * tempCfromData(b_data);
  fridge = 32.0 + 1.8 * tempCfromData(f_data);
  return 0;
}

void startTempConv(uint8_t addr[]) {
  // 0x44 = Convert Temperature
  onewire.reset();
  onewire.select(addr);
  // N.B. not using parasite power!
  onewire.write(0x44);
}

void startTempConvAll() {
  onewire.reset();
  // Skip command to address all sensors
  onewire.skip();
  // 0x44 = Convert Temperature
  // N.B. not using parasite power!
  onewire.write(0x44);
}

int readTempData(uint8_t addr[], uint8_t data[]) {
  // 0xBE = Read Scratchpad
  onewire.reset();
  onewire.select(addr);
  onewire.write(0xBE);
  onewire.read_bytes(data, 9);

  uint8_t crc = OneWire::crc8(data, 8);
  if (crc != data[8]) {
	  Serial.print("Read data failed CRC! ");
	  for (int i = 0; i<= 8; i++) {
		  Serial.printf(" %02X", data[i]);
	  }
	  Serial.printf(" vs %02X\r\n", crc);
      return -1;
  }
  return 0;
}


double getTempByAddr(uint8_t addr[]) {
  byte data[9];
  // the first ROM byte indicates which chip
  if (addr[0] != 0x28) {
    Serial.printf("Device is not a DS18B20 device! (Family: %02X) \r\n", addr[0]);
    return -255;
  }

  startTempConv(addr);
  delay(800);     
  int ret = readTempData(addr, data);
  if (ret != 0) {
    return -255;
  }
  return tempCfromData(data);
}

double tempCfromData(uint8_t data[]) {
  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  return (double)raw / 16.0;
}

