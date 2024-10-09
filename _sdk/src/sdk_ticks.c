
// ****************************************************************************
//
//                            Tick generator (only RP2350)
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

#if !RP2040

#include "../inc/sdk_ticks.h"

// start one tick generator
//  tick ... index of tick generator TICK_*
//  cycles ... clock division (source is CLK_REF)
void TickStart(int tick, u32 cycles)
{
	// stop generator
	TickStop(tick);

	// setup cycles
	ticks_hw->ticks[tick].cycles = cycles;

	// start generator
	ticks_hw->ticks[tick].ctrl = B0;
}

// start all tick generators (cycles = frequency of CLK_REF in MHz)
void TickStartAll(u32 cycles)
{
	int tick;
	for (tick = 0; tick < TICK_COUNT; tick++) TickStart(tick, cycles);
}

#endif // !RP2040
