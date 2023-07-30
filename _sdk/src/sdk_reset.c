
// ****************************************************************************
//
//                             Reset and power-on
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

#include "../inc/sdk_reset.h"

// stop resetting periphery specified by the mask RESET_* and wait
void ResetPeripheryOffWaitMask(u32 mask)
{
	ResetPeripheryOffMask(mask);  // stop resetting periphery
	while (!ResetPeripheryDoneMask(mask)) {} // wait periphery to be done
}

// hard reset periphery (and wait to be accessed again)
//  Takes 0.5 us.
void ResetPeripheryMask(u32 mask)
{
	ResetPeripheryOnMask(mask);  // start resetting periphery
	ResetPeripheryOffWaitMask(mask); // stop resetting periphery and wait
}
