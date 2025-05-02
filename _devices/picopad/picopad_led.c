
// ****************************************************************************
//
//                              PicoPad LEDs
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

#include "../../_sdk/inc/sdk_gpio.h"
#include "picopad_led.h"

// GPIO with LED
// - Do not use 'const' for faster access
u8 LedGpioTab[LED_NUM] = {
#if LED_NUM > 1
	LED1_PIN,	// LED1 inverted, blue, on right
	LED2_PIN,	// default internal LED pin, on Pico board
#else
	LED_PIN,
#endif
};

// GPIO invert flag
// - Do not use 'const' for faster access
Bool LedGpioTabInv[LED_NUM] = {
#if LED_NUM > 1
	True,		// LED1 inverted, blue, on right
#endif
	False,		// default internal LED pin, on Pico board
};

// set LED ON (inx = LED index LED?)
void NOFLASH(LedOn)(u8 inx)
{
	if (inx >= LED_NUM) return;

#if USE_PICOPADVGA
	if (LedGpioTabInv[inx])
		GPIO_Out0(LedGpioTab[inx]);
	else
#endif
	GPIO_Out1(LedGpioTab[inx]);
}

// set LED OFF (inx = LED index LED?)
void NOFLASH(LedOff)(u8 inx)
{
	if (inx >= LED_NUM) return;

#if USE_PICOPADVGA
	if (LedGpioTabInv[inx])
		GPIO_Out1(LedGpioTab[inx]);
	else
#endif
	GPIO_Out0(LedGpioTab[inx]);
}

// flip LED (inx = LED index LED?)
void NOFLASH(LedFlip)(u8 inx)
{
	if (inx >= LED_NUM) return;
	GPIO_Flip(LedGpioTab[inx]);
}

// set LED (inx = LED index LED?)
void NOFLASH(LedSet)(u8 inx, u8 val)
{
	if (val == 0) LedOff(inx); else LedOn(inx);
}

// initialize LEDs
void LedInit()
{
	int i;
	for (i = 0; i < LED_NUM; i++)
	{
		GPIO_Init(LedGpioTab[i]);
		GPIO_DirOut(LedGpioTab[i]);
		if (LedGpioTabInv[i]) GPIO_OutOverInvert(LedGpioTab[i]);
		LedOff(i);
	}
}

// terminate LEDs
void LedTerm()
{
	int i;
	for (i = 0; i < LED_NUM; i++)
	{
		GPIO_Reset(LedGpioTab[i]);
	}
}

#endif // USE_PICOPAD
