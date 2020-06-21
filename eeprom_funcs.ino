void writeEepromInt(void)
{
	writeDataEepromInternal(zoneEepromPositive, zoneThresholdP, MAX_ZONES);
	writeDataEepromInternal(zoneEepromNegative, zoneThresholdN, MAX_ZONES);
}

void readEepromInt(void)
{
	readDataEepromInternal(zoneEepromPositive, zoneThresholdP, MAX_ZONES);
	readDataEepromInternal(zoneEepromNegative, zoneThresholdN, MAX_ZONES);
}

void writeEepromTrans(void)
{
	writeDataEepromInternal(zoneTransducerSettings, zoneTransducer, MAX_ZONES);
}

void readEepromTrans(void)
{
	readDataEepromInternal(zoneTransducerSettings, zoneTransducer, MAX_ZONES);
}

void writeEepromZoneTot(void)
{
	writeDataEepromInternal(numberOfZonesPosition, numberOfZones, 1);
}

void readEepromZoneTot(void)
{
	readDataEepromInternal(numberOfZonesPosition, numberOfZones, 1);
}

void writeDataEeprom(int deviceAddress, long eepromAddress, char* data, int dataSize)
{
	//64Kbit = 8 Kbyte which equals to 250 entries @ 32 bytes per entry

	Wire.beginTransmission(deviceAddress);

	Wire.write((int)(eepromAddress >> 8)); //MSB of address
	Wire.write((int)(eepromAddress & 0xFF)); //LSB of address

	for (int i = 0; i < dataSize; i++)
	{
		Wire.write(data[i]);
	}
	Wire.endTransmission(); //end transmission

	long* x = &zoneEepromPosition;
	*x += dataSize;
}

//write data to eeprom internal
void writeDataEepromInternal(int eepromAddress, int* arr, int length)
{
	while (!EEPROM.isReady()) //is eeprom ready for write
	{
		;
	}
	EEPROM.writeBlock(eepromAddress, arr, length); //forgot that when you pass an array, only its ptr is passed
	//length is not passed

}

void readDataEepromInternal(int eepromAddress, int* arr, int length)
{
	while (!EEPROM.isReady()) //is eeprom ready for write
	{
		;
	}
	EEPROM.readBlock(eepromAddress, arr, length);//forgot that when you pass an array, only its ptr is passed
	//length is not passed
}

void toggleSaveThr(void)
{
	if (isSaved == false)
	{
		writeEepromInt();
		isSaved = true;
	}
}


void toggleSaveTrans(void)
{
	if (isSaved == false)
	{
		writeEepromTrans();
		isSaved = true;
	}
}

void toggleSaveZoneTrans(void)
{
	if (isSaved == false)
	{
		writeEepromZoneTot();
		writeEepromTrans();
		isSaved = true;
	}
}

void checkIfInitialized()
{
	while (!EEPROM.isReady()) //is eeprom ready for write
	{
		;
	}
	if (EEPROM.read(checkIfReady) != 'R')
	{
		delay(100);

		//write
		writeEepromInt();

		//read transducer
		writeEepromTrans();

		//read number of zones
		writeEepromZoneTot();

		delay(100);

		EEPROM.write(checkIfReady, 'R');
	}
}