/*
 Name:		dc_el_relay_v0.ino
 Created:	6/16/2020 5:06:25 PM
 Author:	kyron
*/



// the setup function runs once when you press reset or power the board
#include <EEPROMVar.h>
#include <EEPROMex.h>
#include <Wire.h>
#include <OneButton.h>
#include <LiquidMenu.h>
#include <RTClib.h>
#include <MD_REncoder.h>
#include <LiquidCrystal.h>


//rotary encoder pins
#define encoder_button_pin  10
#define encoder_a_pin 2
#define encoder_b_pin 3

MD_REncoder rotEnc = MD_REncoder(encoder_a_pin, encoder_b_pin);


// LCD object
// liquid crystal needs (rs, e, dat4, dat5, dat6, dat7)
const int rs = 4, en = 5, d4 = 6, d5 = 7, d6 = 8, d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//onebutton
OneButton button(encoder_button_pin, true);

//RTC
RTC_DS3231 rtc;

#define datelength 20

//format for date and time from rtc
const char* formatarray = "YYYY-MM-DD hh:mm:ss";

//store the time and date here, don't know if it is null terminate
char datearray[20];

//keep a copy for later
char storedate[20];

//liquid menu stuff
unsigned long updatePeriod = 500;
unsigned long updateLastMs = 0;

// There are two types of functions for this example, one that increases some
// value and one that decreases it.
enum FunctionTypes
{
	increase = 1,
	decrease = 2,
	click = 3,
};

uint8_t rFocus[8] = 
{
	0b00000,
	0b00000,
	0b00100,
	0b01110,
	0b00100,
	0b00000,
	0b00000,
	0b00000
};

//max defines
#define MAX_ZONES 8
#define MAX_TRANS 8

//number of zones
int numberOfZones[1] = { 1 };

//transducer current inputs
const byte zoneA = A0;
const byte zoneB = A1;
const byte zoneC = A2;
const byte zoneD = A3;


//transducer settings
int zoneTransducer[MAX_ZONES] = { 0, 1, 1, 1, 1, 1, 1, 1};

//transducer names
char* transducerNames[MAX_TRANS] = { "UNDEF", "HASS-A", "HASS-B", "HASS-C", "HASS-D", "HASS-E", "HASS-F", "HASS-G" };

//zone defines arrary
int zoneList[MAX_ZONES] = { 0, 1, 2, 3, 4, 5, 6, 7}; //Zone A, B and Busbar

//zone measured currents
int zoneCurrent[MAX_ZONES] = { 0, 0, 0, 0, 0, 0, 0, 0 }; //Zone A, B and Busbar

//zone analog inputs
int zoneInputs[MAX_ZONES] = { 0, zoneA, zoneB, zoneC, zoneD, 0, 0, 0 }; //Zone A, B and Busbar

//zone threshold positive
int zoneThresholdP[MAX_ZONES] = { 0, 0, 0, 0, 0, 0, 0, 0 }; //Zone A, B and Busbar

//zone threshold negatives
int zoneThresholdN[MAX_ZONES] = { 0, 0, 0, 0, 0, 0, 0, 0 }; //Zone A, B and Busbar

//default zone
int initialZone = 1;

//saved state
bool isSaved = false;

//eeprom address
#define EEPROM_A 0x50
#define EEPROM_B 0x51
#define EEPROM_C 0x52
#define MAX_BYTES 8

//eeprom position
int zoneEepromPositive = 0x00;
int zoneEepromNegative = 0x20;
int zoneMeasuredCurrents = 0x40;
int zoneTransducerSettings = 0x50;
int numberOfZonesPosition = 0x60;
int checkIfReady = 0x80;
long zoneEepromPosition = 0;

int sampleZoneCurrents()
{
	int i;

	for (i = 0; i < *(numberOfZones); i++)
	{
		zoneCurrent[i] = analogRead(zoneInputs[i]);
	}
}

void copyArray(const char* src, char* dst, int len)
{
	while (len--)
	{
		*(dst + len) = *(src + len);
	}
}

void forwardZone(void)
{
	int* x = &initialZone;
	if (*x < *numberOfZones)
	{
		++(*x);
	}
}

void backwardZone(void)
{
	int* x = &initialZone;
	if (*x > 1)
	{
		--(*x);
	}
}

int getPosCurrent(void)
{
	return zoneThresholdP[initialZone];
}

int getNegCurrent(void)
{
	return zoneThresholdN[initialZone];
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

int getMeasuredCurrent(void)
{
	return zoneCurrent[initialZone];
}

char* getStatus(void)
{
	return "OK";
}

void getZoneCurrents()
{
	int i = 1;

	for (i; i < MAX_ZONES; i++)
	{
		zoneCurrent[i] = analogRead(zoneInputs[i]);
	}
}
char* getSaveState(void)
{
	if (isSaved == false) //isSaved = 0
	{
		return "Save?";
	}
	else //isSaved = 1
	{
		return "Saved!";
	}
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

//get transducer name
char* getTransducerName()
{
	int x = zoneTransducer[initialZone];

	return transducerNames[x];
}

int getMaxZones(void)
{
	return *(numberOfZones);
}

//liquid menu defines

//infomation screen
//Col, Row, Text, Variable
LiquidScreen infomationScreen;
LiquidLine infomationLineA(0, 0, "DC Prot Relay");
LiquidLine infomationLineB(0, 2, "Date");
LiquidLine infomationLineC(0, 3, getDate);

//zone screens
LiquidScreen zoneScreen;
LiquidLine zoneInfomationA(0, 0, "Zone Currents");
LiquidLine zoneInfomationB(0, 1, "Zone: ", initialZone);
LiquidLine zoneInfomationC(0, 2, "Pos: ", getPosCurrent, " A");
LiquidLine zoneInfomationD(0, 3, "Neg: ", getNegCurrent, " A");
LiquidLine zoneInfomationE(0, 4, getSaveState);

//measure zones
LiquidScreen measureScreen;
LiquidLine zoneMeasureA(0, 0, "Measured Current");
LiquidLine zoneMeasureB(0, 1, "Zone: ", initialZone);
LiquidLine zoneMeasureC(0, 2, "Current: ", getMeasuredCurrent);
LiquidLine zoneMeasureD(0, 3, "Status: ", getStatus);

//transducer settings
LiquidScreen settingsScreen;
LiquidLine settingsA(0, 0, "Settings");
LiquidLine settingsB(0, 1, "Zone: ", initialZone);
LiquidLine settingsC(0, 2, "Transducer: ", getTransducerName);
LiquidLine settingsD(0, 3, "No of Zones: ", getMaxZones);
LiquidLine settingsE(0, 4, getSaveState);

LiquidMenu menu(lcd);

//single click of rotary encoder
void doubleClick()
{
	menu.next_screen();
	isSaved = false;
}

//double click of rotary encoder
void singleClick()
{
	menu.switch_focus();
}

//button held down
void longPress()
{
	menu.call_function(click);
}

char *getDate(void)
{
	//get current date and time
	DateTime now = rtc.now();

	//fix date, src to dst
	copyArray(formatarray, datearray, datelength);

	//convert rtc date to string
	now.toString(datearray);

	//keep a copy for later
	copyArray(datearray, storedate, datelength);

	return datearray;
}

void rotaryCheck(void)
{
	button.tick();

	// read rotary encoder position
	int rotEncPos = rotEnc.read();

	if (rotEncPos == DIR_CW)
	{
		menu.call_function(increase);
	}
	else if (rotEncPos == DIR_CCW)
	{
		menu.call_function(decrease);
	}
}

//increase and decrease zone threshold
void increasePositiveZone(void)
{
	if (zoneThresholdP[initialZone] >= 0)
	{
		zoneThresholdP[initialZone]++;
	}
}

void decreasePositiveZone(void)
{
	if (zoneThresholdP[initialZone] > 0)
	{
		zoneThresholdP[initialZone]--;
	}
}

void increaseNegativeZone(void)
{
	if (zoneThresholdN[initialZone] <= 0)
	{
		zoneThresholdN[initialZone]--;
	}
}

void decreaseNegativeZone(void)
{
	if (zoneThresholdN[initialZone] != 0)
	{
		zoneThresholdN[initialZone]++;
	}
}

void increaseNoOfZones(void)
{
	if (*numberOfZones >= 0 && *numberOfZones < MAX_ZONES)
	{
		(*numberOfZones)++;
	}
}

void decreaseNoOfZones(void)
{
	if (*numberOfZones > 0 && *numberOfZones != 1)
	{
		(*numberOfZones)--;
	}
}

void nextTransducer(void)
{
	if (zoneTransducer[initialZone] > 0 && zoneTransducer[initialZone] < MAX_TRANS - 1)
	{
		zoneTransducer[initialZone]++;
	}
}

void prevTransducer(void)
{
	if (zoneTransducer[initialZone] > 1 && zoneTransducer[initialZone] < MAX_TRANS)
	{
		zoneTransducer[initialZone]--;
	}
}

//write data to eeprom internal
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

// Used for attaching something to the lines, to make them focusable.
void blankFunction() 
{
	return;
}

int customMapper(int value, int transducer)
{
	switch (transducer)
	{
	case 0: //hass
		return map(value, 0, 1023, -150, 150);
		break;

	default:
		return map(value, 0, 1023, -25, 25);
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

void setup() 
{


	//info screen
	infomationScreen.add_line(infomationLineA);
	infomationScreen.add_line(infomationLineB);
	infomationScreen.add_line(infomationLineC);

	//zone screen
	zoneScreen.add_line(zoneInfomationA);
	zoneScreen.add_line(zoneInfomationB);
	zoneScreen.add_line(zoneInfomationC);
	zoneScreen.add_line(zoneInfomationD);
	zoneScreen.add_line(zoneInfomationE);

	//measure screen
	measureScreen.add_line(zoneMeasureA);
	measureScreen.add_line(zoneMeasureB);
	measureScreen.add_line(zoneMeasureC);
	measureScreen.add_line(zoneMeasureD);

	//settings screen
	settingsScreen.add_line(settingsA);
	settingsScreen.add_line(settingsB);
	settingsScreen.add_line(settingsC);
	settingsScreen.add_line(settingsD);
	settingsScreen.add_line(settingsE);

	zoneScreen.set_displayLineCount(4);

	settingsScreen.set_displayLineCount(4);

	//write temportaty data to internal eeprom
	//writeDataEepromInternal(EEPROM_A, zoneThresholdP, MAX_ZONES);

	//check if initialized
	checkIfInitialized();

	//read data from eeprom
	readEepromInt();

	//read transducer
	readEepromTrans();

	//read number of zones
	readEepromZoneTot();

	//rotary encoder button
	pinMode(encoder_button_pin, INPUT_PULLUP);
	//rotary encoder enable
	rotEnc.begin();

	//lcd enable
	lcd.begin(20, 4);

	//enable rtc - check if available
	if (!rtc.begin())
	{
		lcd.setCursor(0, 0);
		lcd.print("Couldn't find RTC");
	}

	//reset time to script compile time if rtc lost power
	if (rtc.lostPower())
	{
		lcd.print("RTC lost power");
		rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
	}

	//pin outputs for relays
	pinMode(11, OUTPUT);
	pinMode(12, OUTPUT);
	pinMode(13, OUTPUT);

	//liquidmenu functions
	
	//increase and decrease
	zoneInfomationB.attach_function(increase, forwardZone);
	zoneInfomationB.attach_function(decrease, backwardZone);

	//increase and decrease
	zoneInfomationC.attach_function(increase, increasePositiveZone);
	zoneInfomationC.attach_function(decrease, decreasePositiveZone);
	zoneInfomationD.attach_function(increase, increaseNegativeZone);
	zoneInfomationD.attach_function(decrease, decreaseNegativeZone);
	zoneInfomationE.attach_function(click, toggleSaveThr);

	//measure stuff
	zoneMeasureB.attach_function(increase, forwardZone);
	zoneMeasureB.attach_function(decrease, backwardZone);

	//settings stuff
	settingsB.attach_function(increase, forwardZone);
	settingsB.attach_function(decrease, backwardZone);
	settingsC.attach_function(increase, nextTransducer);
	settingsC.attach_function(decrease, prevTransducer);
	settingsD.attach_function(increase, increaseNoOfZones);
	settingsD.attach_function(decrease, decreaseNoOfZones);
	settingsE.attach_function(click, toggleSaveZoneTrans);

	//liquid menu stuff
	menu.add_screen(infomationScreen);
	menu.add_screen(zoneScreen);
	menu.add_screen(measureScreen);
	menu.add_screen(settingsScreen);
	menu.set_focusSymbol(Position::RIGHT, rFocus);

	//button clicks handler
	button.setDebounceTicks(10); //ms to debounce
	button.attachClick(singleClick);
	button.attachDoubleClick(doubleClick);
	button.attachLongPressStart(longPress);
}

// the loop function runs over and over again until power down or reset
void loop() 
{
	getZoneCurrents();

	rotaryCheck();

	if (zoneCurrent[1] > 600)
	{
		digitalWrite(11, HIGH);
	}
	else
	{
		digitalWrite(11, LOW);
	}

	if (millis() - updateLastMs > updatePeriod) 
	{
		updateLastMs = millis();

		menu.update();
	}
}
