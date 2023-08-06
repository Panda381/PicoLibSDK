
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Sample output:
// CPU temperature: 23.7°C

#include "../include.h"

int main()
{
	// ADC init
	ADC_Init();

	while(True)
	{
		// get temperature
		float temp = ADC_Temp();

		// print
		Print("CPU temperature: %.1f°C\n", temp);

		// 1 sec delay
		WaitMs(1000);
	}
}
