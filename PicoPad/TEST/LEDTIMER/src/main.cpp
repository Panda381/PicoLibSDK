
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// alarm callback
void AlarmCB(sAlarm* a)
{
	LedFlip(LED1);
	LedFlip(LED2);
}

int main()
{
	// register alarm
	sAlarm a;
	a.time = SysTime + 100; // time of first alarm in [ms]
	a.delta = 500;		// delta of next alarm in [ms]
	a.callback = AlarmCB;	// callback function
	AlarmReg(&a);		// register alarm

	while (True)
	{
		if (KeyGet() == KEY_Y) ResetToBootLoader();
	}
}
