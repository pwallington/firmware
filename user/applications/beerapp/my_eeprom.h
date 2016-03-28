#define EEPROM_VER 1
struct EepromObj {
	uint8_t eepromVersion;
	uint8_t beerAddr[8];
	uint8_t fridgeAddr[8];
	double targetTemp;
};

/****************************************/
/*	These functions lack (a) class.		*/
/*		Someone should give them a home	*/
/*			or maybe just love			*/
/****************************************/

int storeEeprom() {
	if (beerAddr[0] != 0x28 || fridgeAddr[0] != 0x28) {
		Serial.println("Sensors not yet assigned - not storing EEPROM config");
		return 1;
	}
	EepromObj storeObj;
	storeObj.eepromVersion = EEPROM_VER;
	memcpy(storeObj.beerAddr, beerAddr, 8);
	memcpy(storeObj.fridgeAddr, fridgeAddr, 8);
	storeObj.targetTemp = targetTemp;
	EEPROM.put(0,storeObj);
	return 0;
}

int readEeprom() {
	EepromObj obj;

	EEPROM.get(0, obj);
	if (obj.eepromVersion != EEPROM_VER) {
		Serial.println("Eeprom version not matched!");
		return 1;
	}

	if (obj.beerAddr[0] != 0x28) {
		Serial.println("Beer address family not matched!");
	} else {
		memcpy(beerAddr, obj.beerAddr, 8);
	}

	if (obj.fridgeAddr[0] != 0x28) {
		Serial.println("Fridge address family not matched!");
	} else {
		memcpy(fridgeAddr, obj.fridgeAddr, 8);
	}

	targetTemp = constrain(obj.targetTemp, pidConfig.targetMin, pidConfig.targetMax);
	return 0;
}

