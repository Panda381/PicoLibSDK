
// ****************************************************************************
//
//                            XIP flash control
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
#include "../inc/sdk_xip.h"

#if RP2040
// flush flash cache and wait to complete
void FlashCacheFlushWait()
{
	// flush flash cache
	xip_ctrl_hw->flush = 1;

	// Read is blocked until flush completion
	(void)xip_ctrl_hw->flush;

	// Enable the cache
	RegSet(&xip_ctrl_hw->ctrl, B0);
}
#endif
