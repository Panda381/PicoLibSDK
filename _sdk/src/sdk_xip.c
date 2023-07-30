
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

// flush flash cache and wait to complete
void FlashCacheFlushWait()
{
	// flush flash cache
	*XIP_FLUSH = 1;

	// Read blocks until flush completion
	(void)*XIP_FLUSH;

	// Enable the cache
	RegSet(XIP_CTRL, B0);
}

