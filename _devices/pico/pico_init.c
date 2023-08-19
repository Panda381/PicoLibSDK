
// ****************************************************************************
//
//                              PicoPad Init
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
#include "../../_sdk/inc/sdk_gpio.h"
#include "pico_bat.h"

// Device init
void DeviceInit()
{
	// init battery measurement
	BatInit();
}

// Device terminate
void DeviceTerm()
{
	// terminate battery measurement
	BatTerm();
}

// set LED ON (inx = LED index LED?)
void LedOn(u8 inx)
{
	GPIO_Out1(LED_PIN);
}

// set LED OFF (inx = LED index LED?)
void LedOff(u8 inx)
{
	GPIO_Out0(LED_PIN);
}

// flip LED (inx = LED index LED?)
void LedFlip(u8 inx)
{
	GPIO_Flip(LED_PIN);
}

// set LED (inx = LED index LED?)
void LedSet(u8 inx, u8 val)
{
	if (val == 0) LedOff(inx); else LedOn(inx);
}
