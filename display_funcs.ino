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
char* getStatus(void)
{
	return "OK";
}

char* getDate(void)
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