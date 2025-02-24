
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

#if USE_PICO

#include "../../_display/minivga/minivga.h" // VGA display
#include "../../_display/dvi/dvi.h" // DVI display
#include "../../_sdk/inc/sdk_gpio.h"
#include "pico_init.h"
#include "pico_bat.h"

// Device init
void DeviceInit()
{
#if !USE_DVI && !USE_DVIVGA
	// init battery measurement
	BatInit();
#endif
}

// Device terminate
void DeviceTerm()
{
#if !USE_DVI && !USE_DVIVGA
	// terminate battery measurement
	BatTerm();
#endif
}

// set LED ON (inx = LED index LED?)
void NOFLASH(LedOn)(u8 inx)
{
	if (inx == LED1) GPIO_Out1(LED_PIN);
}

// set LED OFF (inx = LED index LED?)
void NOFLASH(LedOff)(u8 inx)
{
	if (inx == LED1) GPIO_Out0(LED_PIN);
}

// flip LED (inx = LED index LED?)
void NOFLASH(LedFlip)(u8 inx)
{
	if (inx == LED1) GPIO_Flip(LED_PIN);
}

// set LED (inx = LED index LED?)
void NOFLASH(LedSet)(u8 inx, u8 val)
{
	if (val == 0) LedOff(inx); else LedOn(inx);
}

#endif // USE_PICO
