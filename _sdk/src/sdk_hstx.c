
// ****************************************************************************
//
//                    HSTX High-speed serial transmit (only RP2350)
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

#if USE_HSTX && !RP2040		// use HSTX (sdk_hstx.c, sdk_hstx.h)

#include "../sdk_addressmap.h"		// Register address offsets
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_hstx.h"

#define HSTX_GPIO_FIRST	12	// first GPIO pin with HSTX (bit 0)
#define HSTX_GPIO_LAST	19	// last GPIO pin with HSTX (bit 7)
#define HSTX_GPIO_NUM	8	// number of GPIO pins with HSTX

// configure one pin to use HSTX (pin = 12..19)
void HSTX_InitGPIO(int pin)
{
	// reset pin
	GPIO_Reset(pin);

	// output enable
	GPIO_OutEnable(pin);

	// input disable
	GPIO_InDisable(pin);

	// set direction to output
	GPIO_DirOut(pin);

	// no pulls
	GPIO_NoPull(pin);

	// fast slew rate
	GPIO_Fast(pin);

	// set GPIO function
	GPIO_FncRaw(pin, GPIO_FNC_HSTX);

	// pad isolation disable
	GPIO_IsolationDisable(pin);
}

// configure pins GPIO12..19 to use HSTX
// To use pin mask in range (first..last), use function RangeMask.
void HSTX_InitGPIOMask(u32 mask)
{
	int pin;
	for (pin = HSTX_GPIO_FIRST; pin <= HSTX_GPIO_LAST; pin++)
	{
		if ((mask & BIT(pin)) != 0) HSTX_InitGPIO(pin);
	}
}

#endif // USE_HSTX
