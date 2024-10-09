
// ****************************************************************************
//
//                                   XOSC
//                            crystal oscillator
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

#if USE_XOSC	// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)

#include "../inc/sdk_xosc.h"

// Enable crystal oscillator
void XoscEnable(void)
{
	// enable oscillator (this will not affect freq. range)
	RegSet(&xosc_hw->ctrl, 0xFAB << 12);

	// wait for XOSC to be stable
	XoscWait();
}

// Disable crystal oscillator (cannot be disabled if CPU uses it as a clock source)
void XoscDisable(void)
{
	// disable oscillator (this will not affect freq. range)
	RegSet(&xosc_hw->ctrl, 0xD1E << 12);

	// wait for unstable state of crystal oscillator
	while (XoscIsStable()) {}
}

// initialize crystal oscillator
void XoscInit(void)
{
	// set frequency range to 1..15 MHz
	*XOSC_CTRL = XOSC_RANGE_0;

	// set startup delay
	*XOSC_STARTUP = XOSC_STARTUP_DELAY;

	// update current frequency
	CurrentFreq[CLK_XOSC] = XOSC_MHZ*MHZ; // default XOR crystal oscillator is 12 MHz

	// enable oscillator
	XoscEnable();
}

#endif // USE_XOSC	// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)
