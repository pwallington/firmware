struct EepromObj_v1 {
	uint8_t eepromVersion;
	uint8_t beerAddr[8];
	uint8_t fridgeAddr[8];
	double targetTemp;
	char tempProfile[128];
//	PIDConfig pidConfig;
//	ActuatorConfig fridgeConfig;
};

/****************************************/
/*	These functions lack (a) class.		*/
/*		Someone should give them a home	*/
/*			or maybe just love			*/
/****************************************/

int readEeprom() {
	uint8_t _eepromVersion;
	uint8_t _beerAddr[8];
	uint8_t _fridgeAddr[8];
	double _targetTemp;
	char _tempProfile[128];

	EEPROM.get(0, _eepromVersion);
	if (_eepromVersion == 1) {
		EEPROM.get(offsetof(EepromObj_v1, beerAddr), _beerAddr);
		EEPROM.get(offsetof(EepromObj_v1, fridgeAddr), _fridgeAddr);
		EEPROM.get(offsetof(EepromObj_v1, targetTemp), _targetTemp);
		EEPROM.get(offsetof(EepromObj_v1, tempProfile), _tempProfile);
		return 0;
	}
	return -1;
}

