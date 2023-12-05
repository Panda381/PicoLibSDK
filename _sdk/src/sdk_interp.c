
// ****************************************************************************
//
//                               Interpolator
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

#if USE_INTERP	// use interpolator (sdk_interp.c, sdk_interp.h)

#include "../inc/sdk_interp.h"

// claimed interpolator lanes
u8 InterpClaimed = 0;

// save interpolator state (for current CPU core)
void NOFLASH(InterpSave)(int interp, interp_hw_save_t* save)
{
	interp_hw_t* hw = InterpGetHw(interp);
	save->accum[0] = hw->accum[0];
	save->accum[1] = hw->accum[1];
	save->base[0] = hw->base[0];
	save->base[1] = hw->base[1];
	save->base[2] = hw->base[2];
	save->ctrl[0] = hw->ctrl[0];
	save->ctrl[1] = hw->ctrl[1];
}

// load interpolator state (for current CPU core)
void NOFLASH(InterpLoad)(int interp, const interp_hw_save_t* save)
{
	interp_hw_t* hw = InterpGetHw(interp);
	hw->accum[0] = save->accum[0];
	hw->accum[1] = save->accum[1];
	hw->base[0] = save->base[0];
	hw->base[1] = save->base[1];
	hw->base[2] = save->base[2];
	hw->ctrl[0] = save->ctrl[0];
	hw->ctrl[1] = save->ctrl[1];
}

// reset interpolator to default state
//  interp ... interpolator 0 or 1
void NOFLASH(InterpReset)(int interp)
{
	interp_hw_t* hw = InterpGetHw(interp);
	hw->accum[0] = 0;
	hw->accum[1] = 0;
	hw->base[0] = 0;
	hw->base[1] = 0;
	hw->base[2] = 0;
	hw->ctrl[0] = 31<<10;
	hw->ctrl[1] = 31<<10;
}

#endif // USE_INTERP	// use interpolator (sdk_interp.c, sdk_interp.h)
