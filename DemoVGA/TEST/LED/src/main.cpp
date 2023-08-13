
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

int main()
{
	while (True)
	{
		LedOn(LED1);
		WaitMs(250);
		if (KeyGet() == BTN_C) ResetToBootLoader();

		LedOff(LED1);
		WaitMs(250);
		if (KeyGet() == BTN_C) ResetToBootLoader();
	}
}
