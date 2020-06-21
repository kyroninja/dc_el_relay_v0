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