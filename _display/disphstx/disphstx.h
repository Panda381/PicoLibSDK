
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico/Pico2 and RP2040/RP2350
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

/*
Picopad pinout (8x R=270 ohm):

GPIO12 ... D2+
GPIO13 ... D2-
GPIO14 ... D1+
GPIO15 ... D1-
GPIO16 ... D0+
GPIO17 ... D0-
GPIO18 ... CLK+
GPIO19 ... CLK-

DVI breakout board pinout (8x R=270 ohm):

GPIO12 ... D0+
GPIO13 ... D0-
GPIO14 ... CLK+
GPIO15 ... CLK-
GPIO16 ... D2+
GPIO17 ... D2-
GPIO18 ... D1+
GPIO19 ... D1-

VGA output:

GPIO12 ... B0, R=800 (800) ohm
GPIO13 ... B1, R=400 (400) ohm
GPIO14 ... G0, R=800 (800) ohm
GPIO15 ... G1, R=400 (400) ohm
GPIO16 ... R0, R=800 (800) ohm
GPIO17 ... R1, R=400 (400) ohm
GPIO18 ... HSYNC, R=47 ohm
GPIO19 ... VSYNC, R=47 ohm
... and 3x capatitor 220pF on VGA RGB outputs
*/

#if USE_DISPHSTX		// 1=use HSTX Display driver

#ifndef _DISPHSTX_H
#define _DISPHSTX_H

#include "_config.h"

// SDK interface
#if DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library

// PicoSDK interface, if the PicoLibSDK library is used
#include "disphstx_picolibsk.h"
#include "../../_lib/_include.h"
#include "../../_font/_include.h"

#else // DISPHSTX_PICOSDK

// PicoLibSDK library
#include "../../_sdk/inc/sdk_clocks.h"
#include "../../_sdk/inc/sdk_dma.h"
#include "../../_sdk/inc/sdk_flash.h"
#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_hstx.h"
#include "../../_sdk/inc/sdk_irq.h"
#include "../../_sdk/inc/sdk_multicore.h"
#include "../../_sdk/inc/sdk_pll.h"
#include "../../_sdk/inc/sdk_reset.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_powman.h"
#include "../../_lib/inc/lib_malloc.h"
#include "../../_lib/inc/lib_pwmsnd.h"
#include "../../_lib/inc/lib_drawcan.h"
#include "../../_lib/inc/lib_drawcan1.h"
#include "../../_lib/inc/lib_drawcan2.h"
#include "../../_lib/inc/lib_drawcan3.h"
#include "../../_lib/inc/lib_drawcan4.h"
#include "../../_lib/inc/lib_drawcan6.h"
#include "../../_lib/inc/lib_drawcan8.h"
#include "../../_lib/inc/lib_drawcan12.h"
#include "../../_lib/inc/lib_drawcan16.h"
//#include "../../_lib/inc/lib_print.h"

#endif // DISPHSTX_PICOSDK

#include "../../_font/_include.h"

#include "disphstx_vmode_time.h"
#include "disphstx_vmode_format.h"
#include "disphstx_vmode.h"
#include "disphstx_vmode_simple.h"
#include "disphstx_dvi_render.h"
#include "disphstx_dvi.h"
#include "disphstx_vga_render.h"
#include "disphstx_vga.h"

#endif // _DISPHSTX_H

#endif // USE_DISPHSTX
