
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************
// Sample output:
// Vsys supply voltage: 4.95V

#include "../include.h"

// Measure Vsys supply voltage through GPIO29_ADC3 input,
// from 2:1 divider of resistors 200K + 100K.
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier

int main()
{
	// ADC init
	ADC_Init();

	// init battery GPIO
	ADC_PinInit(BAT_PIN);

	// workaround for battery measurement on Pico W (switch off wifi controller)
	GPIO_Out1(LED_PIN); 

	// switch to battery input
	ADC_Mux(BAT_ADC);

	while(True)
	{
		// get battery voltage
		float voltage = ADC_SingleU()*BAT_MUL;

		// print
		Print("Vsys supply voltage: %.2f\n", voltage);

		// 1 sec delay
		WaitMs(1000);
	}
}
