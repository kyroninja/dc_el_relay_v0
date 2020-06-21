int sampleZoneCurrents()
{
	int i;

	for (i = 1; i <= *(numberOfZones); i++)
	{
		zoneCurrent[i] = customMapper(analogRead(zoneInputs[i]), zoneTransducer[i]);
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



int getMeasuredCurrent(void)
{
	return zoneCurrent[initialZone];
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

void getZoneCurrents()
{
	int i = 1;

	for (i; i <= *numberOfZones; i++)
	{
		zoneCurrent[i] = analogRead(zoneInputs[i]);
	}
}

void setRelayOutputs(void)
{
	int i;

	for (i = 1; i <= *numberOfZones; i++)
	{
		pinMode(relaysOutput[i], OUTPUT);
	}
}

void compareZoneCurrents(void)
{
	int i;

	for (i = 1; i <= *numberOfZones; i++)
	{
		if ((zoneCurrent[i] > zoneThresholdP[i]) || (zoneCurrent[i]) < zoneThresholdN[i])
		{
			digitalWrite(relaysOutput[i], HIGH);
		}
		else
		{
			digitalWrite(relaysOutput[i], LOW);
		}
	}
}

int customMapper(int value, int transducer)
{
	switch (transducer)
	{
	case 1: //hass a
		return map(value, 0, 1023, -25, 25);
		break;
	case 2: //hass b
		return map(value, 0, 1023, -50, 50);
		break;
	case 3: //hass c
		return map(value, 0, 1023, -75, 75);
		break;
	case 4: //hass d
		return map(value, 0, 1023, -100, 100);
		break;
	case 5: //hass e
		return map(value, 0, 1023, -125, 125);
		break;
	case 6: //hass f
		return map(value, 0, 1023, -150, 150);
		break;
	case 7: //hass g
		return map(value, 0, 1023, -250, 250);
		break;
	default:
		return map(value, 0, 1023, -25, 25);
	}
}