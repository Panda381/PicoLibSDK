
// ****************************************************************************
//
//                              PicoPad Battery
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "../../global.h"	// globals

#if USE_PICOPAD

#include "../../_sdk/inc/sdk_adc.h"
#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_lib/inc/lib_config.h"
#include "picopad_bat.h"

// init battery measurement
void BatInit()
{
	// ADC init
	ADC_Init();

	// init battery GPIO
	ADC_PinInit(BAT_PIN);
}

// get battery voltage in V
float GetBat()
{
	// workaround for battery measurement on Pico W
	GPIO_Fnc(LED_PIN, GPIO_FNC_SIO);
	GPIO_DirOut(LED_PIN); 
	GPIO_Out1(LED_PIN); 
	WaitUs(20);

	// switch to battery input
	ADC_Mux(BAT_ADC);

	// get battery voltage
#if USE_PICOPADVGA || USE_PICOPADHSTX
	float voltage = ADC_SingleU()*BAT_MUL;
#else // USE_PICOPADVGA || USE_PICOPADHSTX

#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	float voltage = ADC_SingleU()*BAT_MUL+ConfigGetBatDiode();
#else
	float voltage = ADC_SingleU()*BAT_MUL+BAT_DIODE_FV;
#endif

#endif // USE_PICOPADVGA || USE_PICOPADHSTX
	GPIO_Out0(LED_PIN); 
	return voltage;
}

// get battery voltage, integer in mV
int GetBatInt()
{
	// workaround for battery measurement on Pico W
	GPIO_Fnc(LED_PIN, GPIO_FNC_SIO);
	GPIO_DirOut(LED_PIN); 
	GPIO_Out1(LED_PIN); 
	WaitUs(20);

	// switch to battery input
	ADC_Mux(BAT_ADC);

	// get battery voltage
#if USE_PICOPADVGA || USE_PICOPADHSTX
	int voltage = ADC_SingleUint()*BAT_MUL;
#else // USE_PICOPADVGA || USE_PICOPADHSTX

#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	int voltage = ADC_SingleUint()*BAT_MUL+ConfigGetBatDiodeInt();
#else
	int voltage = ADC_SingleUint()*BAT_MUL+BAT_DIODE_FV_INT;
#endif

#endif // USE_PICOPADVGA || USE_PICOPADHSTX
	GPIO_Out0(LED_PIN); 
	return voltage;
}

// terminate battery measurement
void BatTerm()
{
	// terminate battery GPIO
	ADC_PinTerm(BAT_PIN);

	// disable ADC
	ADC_Disable();
}

#endif // USE_PICOPAD
