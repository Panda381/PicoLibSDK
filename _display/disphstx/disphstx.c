
// ****************************************************************************
//
//                                 Displays
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

#if USE_DISPHSTX

#include "disphstx_dvi.c"
#include "disphstx_dvi_render.c"
#include "disphstx_vga.c"
#include "disphstx_vga_render.c"
#include "disphstx_vmode.c"
#include "disphstx_vmode_simple.c"
#include "disphstx_vmode_format.c"
#include "disphstx_vmode_time.c"

#if DISPHSTX_PICOSDK
#include "disphstx_picolibsk.c"
#endif // DISPHSTX_PICOSDK

#endif // USE_DISPHSTX
