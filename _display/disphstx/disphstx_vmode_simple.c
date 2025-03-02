
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                            Video modes - simple
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

// Note: The following 2 switches are not yet defined in the PicoLibSDK at this point, so the global.h file is included.
#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#include "disphstx_picolibsk.h"
#else
#include "../../global.h"
#endif

#if USE_DISPHSTX		// 1=use HSTX Display driver

#include "disphstx.h"

// Alternate definitions so we don't have to condition each row separately. 
#if !DISPHSTX_USE_FORMAT_1_PAL
#define DISPHSTX_FORMAT_1_PAL	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_2_PAL
#define DISPHSTX_FORMAT_2_PAL	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_3_PAL
#define DISPHSTX_FORMAT_3_PAL	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_4_PAL
#define DISPHSTX_FORMAT_4_PAL	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_6_PAL
#define DISPHSTX_FORMAT_6_PAL	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_8
#define DISPHSTX_FORMAT_8	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_12
#define DISPHSTX_FORMAT_12	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_15
#define DISPHSTX_FORMAT_15	DISPHSTX_FORMAT_NONE
#endif

#if !DISPHSTX_USE_FORMAT_16
#define DISPHSTX_FORMAT_16	DISPHSTX_FORMAT_NONE
#endif

// Start simple display graphics videomodes, supported by the DrawCan drawing library
//  dispmode ... display mode DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA, set to DISPHSTX_DISPMODE_NONE or 0 to auto-detect display mode selection
//  buf ... pointer to frame buffer (aligned to 32-bit), or NULL to create new one
// Returns error code DISPHSTX_ERR_* (DISPHSTX_ERR_OK = 0 = all OK)

// 532x400/70.2Hz/31.5kHz (EGA 4:3), pixel clock 20.9 MHz, system clock 104.571 MHz ... detected as 720x400@70Hz on VGA
int DispVMode266x200x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_16); }	// 266x200/16-bit, sys_clock=105 MHz, buffer 106400 B
int DispVMode266x200x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_15); }	// 266x200/15-bit, sys_clock=105 MHz, buffer 106400 B
int DispVMode266x200x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_12); }	// 266x200/12-bit, sys_clock=105 MHz, buffer 81600 B
int DispVMode266x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_8); }		// 266x200/8-bit, sys_clock=105 MHz, buffer 53600 B
int DispVMode266x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 266x200/6-bit, sys_clock=105 MHz, buffer 43200 B
int DispVMode266x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 266x200/4-bit, sys_clock=105 MHz, buffer 27200 B
int DispVMode266x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 266x200/3-bit, sys_clock=105 MHz, buffer 21600 B
int DispVMode266x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 266x200/2-bit, sys_clock=105 MHz, buffer 13600 B
int DispVMode266x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 266x200/1-bit, sys_clock=105 MHz, buffer 7200 B

int DispVMode266x400x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_16); }	// 266x400/16-bit, sys_clock=105 MHz, buffer 212800 B
int DispVMode266x400x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_15); }	// 266x400/15-bit, sys_clock=105 MHz, buffer 212800 B
int DispVMode266x400x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_12); }	// 266x400/12-bit, sys_clock=105 MHz, buffer 163200 B
int DispVMode266x400x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_8); }		// 266x400/8-bit, sys_clock=105 MHz, buffer 107200 B
int DispVMode266x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 266x400/6-bit, sys_clock=105 MHz, buffer 86400 B
int DispVMode266x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 266x400/4-bit, sys_clock=105 MHz, buffer 54400 B
int DispVMode266x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 266x400/3-bit, sys_clock=105 MHz, buffer 43200 B
int DispVMode266x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 266x400/2-bit, sys_clock=105 MHz, buffer 27200 B
int DispVMode266x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 266x400/1-bit, sys_clock=105 MHz, buffer 14400 B

int DispVMode532x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 2, DISPHSTX_FORMAT_8); }		// 532x200/8-bit, sys_clock=105 MHz, buffer 106400 B
int DispVMode532x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 532x200/6-bit, sys_clock=105 MHz, buffer 85600 B
int DispVMode532x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 532x200/4-bit, sys_clock=105 MHz, buffer 53600 B
int DispVMode532x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 532x200/3-bit, sys_clock=105 MHz, buffer 43200 B
int DispVMode532x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 532x200/2-bit, sys_clock=105 MHz, buffer 27200 B
int DispVMode532x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 532x200/1-bit, sys_clock=105 MHz, buffer 13600 B

int DispVMode532x400x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 1, DISPHSTX_FORMAT_8); }		// 532x400/8-bit, sys_clock=105 MHz, buffer 212800 B
int DispVMode532x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 532x400/6-bit, sys_clock=105 MHz, buffer 171200 B
int DispVMode532x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 532x400/4-bit, sys_clock=105 MHz, buffer 107200 B
int DispVMode532x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 532x400/3-bit, sys_clock=105 MHz, buffer 86400 B
int DispVMode532x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 532x400/2-bit, sys_clock=105 MHz, buffer 54400 B
int DispVMode532x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 532x400/1-bit, sys_clock=105 MHz, buffer 27200 B

// 532x400/70.2Hz/31.5kHz (EGA 4:3), pixel clock 20.9 MHz, system clock 209.143 MHz ... detected as 720x400@70Hz on VGA
int DispVMode266x200x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_16); }	// 266x200/16-bit, sys_clock=210 MHz, buffer 106400 B
int DispVMode266x200x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_15); }	// 266x200/15-bit, sys_clock=210 MHz, buffer 106400 B
int DispVMode266x200x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_12); }	// 266x200/12-bit, sys_clock=210 MHz, buffer 81600 B
int DispVMode266x200x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_8); }	// 266x200/8-bit, sys_clock=210 MHz, buffer 53600 B
int DispVMode266x200x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 266x200/6-bit, sys_clock=210 MHz, buffer 43200 B
int DispVMode266x200x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 266x200/4-bit, sys_clock=210 MHz, buffer 27200 B
int DispVMode266x200x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 266x200/3-bit, sys_clock=210 MHz, buffer 21600 B
int DispVMode266x200x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 266x200/2-bit, sys_clock=210 MHz, buffer 13600 B
int DispVMode266x200x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 266x200/1-bit, sys_clock=210 MHz, buffer 7200 B

int DispVMode266x400x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_16); }	// 266x400/16-bit, sys_clock=210 MHz, buffer 212800 B
int DispVMode266x400x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_15); }	// 266x400/15-bit, sys_clock=210 MHz, buffer 212800 B
int DispVMode266x400x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_12); }	// 266x400/12-bit, sys_clock=210 MHz, buffer 163200 B
int DispVMode266x400x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_8); }	// 266x400/8-bit, sys_clock=210 MHz, buffer 107200 B
int DispVMode266x400x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 266x400/6-bit, sys_clock=210 MHz, buffer 86400 B
int DispVMode266x400x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 266x400/4-bit, sys_clock=210 MHz, buffer 54400 B
int DispVMode266x400x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 266x400/3-bit, sys_clock=210 MHz, buffer 43200 B
int DispVMode266x400x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 266x400/2-bit, sys_clock=210 MHz, buffer 27200 B
int DispVMode266x400x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 266x400/1-bit, sys_clock=210 MHz, buffer 14400 B

int DispVMode532x200x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_16); }	// 532x200/16-bit, sys_clock=210 MHz, buffer 212800 B
int DispVMode532x200x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_15); }	// 532x200/15-bit, sys_clock=210 MHz, buffer 212800 B
int DispVMode532x200x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_12); }	// 532x200/12-bit, sys_clock=210 MHz, buffer 160800 B
int DispVMode532x200x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_8); }	// 532x200/8-bit, sys_clock=210 MHz, buffer 106400 B
int DispVMode532x200x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 532x200/6-bit, sys_clock=210 MHz, buffer 85600 B
int DispVMode532x200x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 532x200/4-bit, sys_clock=210 MHz, buffer 53600 B
int DispVMode532x200x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 532x200/3-bit, sys_clock=210 MHz, buffer 43200 B
int DispVMode532x200x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 532x200/2-bit, sys_clock=210 MHz, buffer 27200 B
int DispVMode532x200x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 532x200/1-bit, sys_clock=210 MHz, buffer 13600 B

int DispVMode532x400x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_16); }	// 532x400/16-bit, sys_clock=210 MHz, buffer 425600 B
int DispVMode532x400x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_15); }	// 532x400/15-bit, sys_clock=210 MHz, buffer 425600 B
int DispVMode532x400x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_12); }	// 532x400/12-bit, sys_clock=210 MHz, buffer 321600 B
int DispVMode532x400x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_8); }	// 532x400/8-bit, sys_clock=210 MHz, buffer 212800 B
int DispVMode532x400x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 532x400/6-bit, sys_clock=210 MHz, buffer 171200 B
int DispVMode532x400x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 532x400/4-bit, sys_clock=210 MHz, buffer 107200 B
int DispVMode532x400x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 532x400/3-bit, sys_clock=210 MHz, buffer 86400 B
int DispVMode532x400x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 532x400/2-bit, sys_clock=210 MHz, buffer 54400 B
int DispVMode532x400x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_532x400_fast, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 532x400/1-bit, sys_clock=210 MHz, buffer 27200 B

#if DISPHSTX_WIDTHMAX >= 640	// max. supported width
// 640x350/70.2Hz/31.5kHz, pixel clock 25.2 MHz, system clock 126 MHz ... detected as 640x350@70Hz on VGA
int DispVMode320x175x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_16); }		// 320x175/16-bit, sys_clock=126 MHz, buffer 112000 B
int DispVMode320x175x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_15); }		// 320x175/15-bit, sys_clock=126 MHz, buffer 112000 B
int DispVMode320x175x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_12); }		// 320x175/12-bit, sys_clock=126 MHz, buffer 84000 B
int DispVMode320x175x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_8); }			// 320x175/8-bit, sys_clock=126 MHz, buffer 56000 B
int DispVMode320x175x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 320x175/6-bit, sys_clock=126 MHz, buffer 44800 B
int DispVMode320x175x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 320x175/4-bit, sys_clock=126 MHz, buffer 28000 B
int DispVMode320x175x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 320x175/3-bit, sys_clock=126 MHz, buffer 22400 B
int DispVMode320x175x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 320x175/2-bit, sys_clock=126 MHz, buffer 14000 B
int DispVMode320x175x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 320x175/1-bit, sys_clock=126 MHz, buffer 7000 B

int DispVMode320x350x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_16); }		// 320x350/16-bit, sys_clock=126 MHz, buffer 224000 B
int DispVMode320x350x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_15); }		// 320x350/15-bit, sys_clock=126 MHz, buffer 224000 B
int DispVMode320x350x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_12); }		// 320x350/12-bit, sys_clock=126 MHz, buffer 168000 B
int DispVMode320x350x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_8); }			// 320x350/8-bit, sys_clock=126 MHz, buffer 112000 B
int DispVMode320x350x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 320x350/6-bit, sys_clock=126 MHz, buffer 89600 B
int DispVMode320x350x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 320x350/4-bit, sys_clock=126 MHz, buffer 56000 B
int DispVMode320x350x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 320x350/3-bit, sys_clock=126 MHz, buffer 44800 B
int DispVMode320x350x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 320x350/2-bit, sys_clock=126 MHz, buffer 28000 B
int DispVMode320x350x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 320x350/1-bit, sys_clock=126 MHz, buffer 14000 B

int DispVMode640x175x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 2, DISPHSTX_FORMAT_8); }			// 640x175/8-bit, sys_clock=126 MHz, buffer 112000 B
int DispVMode640x175x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 640x175/6-bit, sys_clock=126 MHz, buffer 89600 B
int DispVMode640x175x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 640x175/4-bit, sys_clock=126 MHz, buffer 56000 B
int DispVMode640x175x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 640x175/3-bit, sys_clock=126 MHz, buffer 44800 B
int DispVMode640x175x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 640x175/2-bit, sys_clock=126 MHz, buffer 28000 B
int DispVMode640x175x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 640x175/1-bit, sys_clock=126 MHz, buffer 14000 B

int DispVMode640x350x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 1, DISPHSTX_FORMAT_8); }			// 640x350/8-bit, sys_clock=126 MHz, buffer 224000 B
int DispVMode640x350x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x350/6-bit, sys_clock=126 MHz, buffer 179200 B
int DispVMode640x350x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x350/4-bit, sys_clock=126 MHz, buffer 112000 B
int DispVMode640x350x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x350/3-bit, sys_clock=126 MHz, buffer 89600 B
int DispVMode640x350x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x350/2-bit, sys_clock=126 MHz, buffer 56000 B
int DispVMode640x350x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x350/1-bit, sys_clock=126 MHz, buffer 28000 B

// 640x350/70.2Hz/31.5kHz, pixel clock 25.2 MHz, system clock 252 MHz ... detected as 640x350@70Hz on VGA
int DispVMode320x175x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_16); }	// 320x175/16-bit, sys_clock=252 MHz, buffer 112000 B
int DispVMode320x175x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_15); }	// 320x175/15-bit, sys_clock=252 MHz, buffer 112000 B
int DispVMode320x175x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_12); }	// 320x175/12-bit, sys_clock=252 MHz, buffer 84000 B
int DispVMode320x175x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_8); }	// 320x175/8-bit, sys_clock=252 MHz, buffer 56000 B
int DispVMode320x175x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 320x175/6-bit, sys_clock=252 MHz, buffer 44800 B
int DispVMode320x175x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 320x175/4-bit, sys_clock=252 MHz, buffer 28000 B
int DispVMode320x175x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 320x175/3-bit, sys_clock=252 MHz, buffer 22400 B
int DispVMode320x175x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 320x175/2-bit, sys_clock=252 MHz, buffer 14000 B
int DispVMode320x175x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 320x175/1-bit, sys_clock=252 MHz, buffer 7000 B

int DispVMode320x350x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_16); }	// 320x350/16-bit, sys_clock=252 MHz, buffer 224000 B
int DispVMode320x350x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_15); }	// 320x350/15-bit, sys_clock=252 MHz, buffer 224000 B
int DispVMode320x350x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_12); }	// 320x350/12-bit, sys_clock=252 MHz, buffer 168000 B
int DispVMode320x350x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_8); }	// 320x350/8-bit, sys_clock=252 MHz, buffer 112000 B
int DispVMode320x350x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 320x350/6-bit, sys_clock=252 MHz, buffer 89600 B
int DispVMode320x350x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 320x350/4-bit, sys_clock=252 MHz, buffer 56000 B
int DispVMode320x350x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 320x350/3-bit, sys_clock=252 MHz, buffer 44800 B
int DispVMode320x350x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 320x350/2-bit, sys_clock=252 MHz, buffer 28000 B
int DispVMode320x350x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 320x350/1-bit, sys_clock=252 MHz, buffer 14000 B

int DispVMode640x175x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_16); }	// 640x175/16-bit, sys_clock=252 MHz, buffer 224000 B
int DispVMode640x175x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_15); }	// 640x175/15-bit, sys_clock=252 MHz, buffer 224000 B
int DispVMode640x175x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_12); }	// 640x175/12-bit, sys_clock=252 MHz, buffer 168000 B
int DispVMode640x175x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_8); }	// 640x175/8-bit, sys_clock=252 MHz, buffer 112000 B
int DispVMode640x175x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 640x175/6-bit, sys_clock=252 MHz, buffer 89600 B
int DispVMode640x175x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 640x175/4-bit, sys_clock=252 MHz, buffer 56000 B
int DispVMode640x175x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 640x175/3-bit, sys_clock=252 MHz, buffer 44800 B
int DispVMode640x175x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 640x175/2-bit, sys_clock=252 MHz, buffer 28000 B
int DispVMode640x175x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 640x175/1-bit, sys_clock=252 MHz, buffer 14000 B

int DispVMode640x350x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_16); }	// 640x350/16-bit, sys_clock=252 MHz, buffer 448000 B
int DispVMode640x350x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_15); }	// 640x350/15-bit, sys_clock=252 MHz, buffer 448000 B
int DispVMode640x350x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_12); }	// 640x350/12-bit, sys_clock=252 MHz, buffer 336000 B
int DispVMode640x350x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_8); }	// 640x350/8-bit, sys_clock=252 MHz, buffer 224000 B
int DispVMode640x350x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 640x350/6-bit, sys_clock=252 MHz, buffer 179200 B
int DispVMode640x350x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 640x350/4-bit, sys_clock=252 MHz, buffer 112000 B
int DispVMode640x350x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 640x350/3-bit, sys_clock=252 MHz, buffer 89600 B
int DispVMode640x350x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 640x350/2-bit, sys_clock=252 MHz, buffer 56000 B
int DispVMode640x350x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x350_fast, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 640x350/1-bit, sys_clock=252 MHz, buffer 28000 B

// 640x400/70.2Hz/31.5kHz, pixel clock 25.2 MHz, system clock 126 MHz ... detected as 720x400@70Hz on VGA
int DispVMode320x200x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_16); }		// 320x200/16-bit, sys_clock=126 MHz, buffer 128000 B
int DispVMode320x200x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_15); }		// 320x200/15-bit, sys_clock=126 MHz, buffer 128000 B
int DispVMode320x200x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_12); }		// 320x200/12-bit, sys_clock=126 MHz, buffer 96000 B
int DispVMode320x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_8); }			// 320x200/8-bit, sys_clock=126 MHz, buffer 64000 B
int DispVMode320x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 320x200/6-bit, sys_clock=126 MHz, buffer 51200 B
int DispVMode320x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 320x200/4-bit, sys_clock=126 MHz, buffer 32000 B
int DispVMode320x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 320x200/3-bit, sys_clock=126 MHz, buffer 25600 B
int DispVMode320x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 320x200/2-bit, sys_clock=126 MHz, buffer 16000 B
int DispVMode320x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 320x200/1-bit, sys_clock=126 MHz, buffer 8000 B

int DispVMode320x400x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_16); }		// 320x400/16-bit, sys_clock=126 MHz, buffer 256000 B
int DispVMode320x400x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_15); }		// 320x400/15-bit, sys_clock=126 MHz, buffer 256000 B
int DispVMode320x400x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_12); }		// 320x400/12-bit, sys_clock=126 MHz, buffer 192000 B
int DispVMode320x400x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_8); }			// 320x400/8-bit, sys_clock=126 MHz, buffer 128000 B
int DispVMode320x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 320x400/6-bit, sys_clock=126 MHz, buffer 102400 B
int DispVMode320x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 320x400/4-bit, sys_clock=126 MHz, buffer 64000 B
int DispVMode320x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 320x400/3-bit, sys_clock=126 MHz, buffer 51200 B
int DispVMode320x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 320x400/2-bit, sys_clock=126 MHz, buffer 32000 B
int DispVMode320x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 320x400/1-bit, sys_clock=126 MHz, buffer 16000 B

int DispVMode640x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 2, DISPHSTX_FORMAT_8); }			// 640x200/8-bit, sys_clock=126 MHz, buffer 128000 B
int DispVMode640x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 640x200/6-bit, sys_clock=126 MHz, buffer 102400 B
int DispVMode640x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 640x200/4-bit, sys_clock=126 MHz, buffer 64000 B
int DispVMode640x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 640x200/3-bit, sys_clock=126 MHz, buffer 51200 B
int DispVMode640x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 640x200/2-bit, sys_clock=126 MHz, buffer 32000 B
int DispVMode640x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 640x200/1-bit, sys_clock=126 MHz, buffer 16000 B

int DispVMode640x400x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 1, DISPHSTX_FORMAT_8); }			// 640x400/8-bit, sys_clock=126 MHz, buffer 256000 B
int DispVMode640x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x400/6-bit, sys_clock=126 MHz, buffer 204800 B
int DispVMode640x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x400/4-bit, sys_clock=126 MHz, buffer 128000 B
int DispVMode640x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x400/3-bit, sys_clock=126 MHz, buffer 102400 B
int DispVMode640x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x400/2-bit, sys_clock=126 MHz, buffer 64000 B
int DispVMode640x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x400/1-bit, sys_clock=126 MHz, buffer 32000 B

// 640x400/70.2Hz/31.5kHz, pixel clock 25.2 MHz, system clock 252 MHz ... detected as 720x400@70Hz on VGA
int DispVMode320x200x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_16); }	// 320x200/16-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode320x200x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_15); }	// 320x200/15-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode320x200x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_12); }	// 320x200/12-bit, sys_clock=252 MHz, buffer 96000 B
int DispVMode320x200x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_8); }	// 320x200/8-bit, sys_clock=252 MHz, buffer 64000 B
int DispVMode320x200x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 320x200/6-bit, sys_clock=252 MHz, buffer 51200 B
int DispVMode320x200x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 320x200/4-bit, sys_clock=252 MHz, buffer 32000 B
int DispVMode320x200x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 320x200/3-bit, sys_clock=252 MHz, buffer 25600 B
int DispVMode320x200x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 320x200/2-bit, sys_clock=252 MHz, buffer 16000 B
int DispVMode320x200x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 320x200/1-bit, sys_clock=252 MHz, buffer 8000 B

int DispVMode320x400x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_16); }	// 320x400/16-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode320x400x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_15); }	// 320x400/15-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode320x400x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_12); }	// 320x400/12-bit, sys_clock=252 MHz, buffer 192000 B
int DispVMode320x400x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_8); }	// 320x400/8-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode320x400x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 320x400/6-bit, sys_clock=252 MHz, buffer 102400 B
int DispVMode320x400x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 320x400/4-bit, sys_clock=252 MHz, buffer 64000 B
int DispVMode320x400x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 320x400/3-bit, sys_clock=252 MHz, buffer 51200 B
int DispVMode320x400x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 320x400/2-bit, sys_clock=252 MHz, buffer 32000 B
int DispVMode320x400x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 320x400/1-bit, sys_clock=252 MHz, buffer 16000 B

int DispVMode640x200x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_16); }	// 640x200/16-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode640x200x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_15); }	// 640x200/15-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode640x200x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_12); }	// 640x200/12-bit, sys_clock=252 MHz, buffer 192000 B
int DispVMode640x200x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_8); }	// 640x200/8-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode640x200x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 640x200/6-bit, sys_clock=252 MHz, buffer 102400 B
int DispVMode640x200x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 640x200/4-bit, sys_clock=252 MHz, buffer 64000 B
int DispVMode640x200x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 640x200/3-bit, sys_clock=252 MHz, buffer 51200 B
int DispVMode640x200x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 640x200/2-bit, sys_clock=252 MHz, buffer 32000 B
int DispVMode640x200x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 640x200/1-bit, sys_clock=252 MHz, buffer 16000 B

int DispVMode640x400x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_12); }	// 640x400/12-bit, sys_clock=252 MHz, buffer 384000 B
int DispVMode640x400x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_8); }	// 640x400/8-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode640x400x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 640x400/6-bit, sys_clock=252 MHz, buffer 204800 B
int DispVMode640x400x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 640x400/4-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode640x400x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 640x400/3-bit, sys_clock=252 MHz, buffer 102400 B
int DispVMode640x400x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 640x400/2-bit, sys_clock=252 MHz, buffer 64000 B
int DispVMode640x400x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x400_fast, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 640x400/1-bit, sys_clock=252 MHz, buffer 32000 B

// 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz, system clock 126 MHz ... detected as 640x480@60Hz on VGA
int DispVMode320x240x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_16); }		// 320x240/16-bit, sys_clock=126 MHz, buffer 153600 B
int DispVMode320x240x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_15); }		// 320x240/15-bit, sys_clock=126 MHz, buffer 153600 B
int DispVMode320x240x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_12); }		// 320x240/12-bit, sys_clock=126 MHz, buffer 115200 B
int DispVMode320x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_8); }			// 320x240/8-bit, sys_clock=126 MHz, buffer 76800 B
int DispVMode320x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 320x240/6-bit, sys_clock=126 MHz, buffer 61440 B
int DispVMode320x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 320x240/4-bit, sys_clock=126 MHz, buffer 38400 B
int DispVMode320x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 320x240/3-bit, sys_clock=126 MHz, buffer 30720 B
int DispVMode320x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 320x240/2-bit, sys_clock=126 MHz, buffer 19200 B
int DispVMode320x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 320x240/1-bit, sys_clock=126 MHz, buffer 9600 B

int DispVMode320x480x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_16); }		// 320x480/16-bit, sys_clock=126 MHz, buffer 307200 B
int DispVMode320x480x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_15); }		// 320x480/15-bit, sys_clock=126 MHz, buffer 307200 B
int DispVMode320x480x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_12); }		// 320x480/12-bit, sys_clock=126 MHz, buffer 230400 B
int DispVMode320x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_8); }			// 320x480/8-bit, sys_clock=126 MHz, buffer 153600 B
int DispVMode320x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 320x480/6-bit, sys_clock=126 MHz, buffer 122880 B
int DispVMode320x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 320x480/4-bit, sys_clock=126 MHz, buffer 76800 B
int DispVMode320x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 320x480/3-bit, sys_clock=126 MHz, buffer 61440 B
int DispVMode320x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 320x480/2-bit, sys_clock=126 MHz, buffer 38400 B
int DispVMode320x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 320x480/1-bit, sys_clock=126 MHz, buffer 19200 B

int DispVMode640x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 2, DISPHSTX_FORMAT_8); }			// 640x240/8-bit, sys_clock=126 MHz, buffer 153600 B
int DispVMode640x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 640x240/6-bit, sys_clock=126 MHz, buffer 122880 B
int DispVMode640x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 640x240/4-bit, sys_clock=126 MHz, buffer 76800 B
int DispVMode640x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 640x240/3-bit, sys_clock=126 MHz, buffer 61440 B
int DispVMode640x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 640x240/2-bit, sys_clock=126 MHz, buffer 38400 B
int DispVMode640x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 640x240/1-bit, sys_clock=126 MHz, buffer 19200 B

int DispVMode640x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 1, DISPHSTX_FORMAT_8); }			// 640x480/8-bit, sys_clock=126 MHz, buffer 307200 B
int DispVMode640x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x480/6-bit, sys_clock=126 MHz, buffer 245760 B
int DispVMode640x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x480/4-bit, sys_clock=126 MHz, buffer 153600 B
int DispVMode640x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x480/3-bit, sys_clock=126 MHz, buffer 122880 B
int DispVMode640x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x480/2-bit, sys_clock=126 MHz, buffer 76800 B
int DispVMode640x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x480/1-bit, sys_clock=126 MHz, buffer 38400 B

// 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz, system clock 252 MHz ... detected as 640x480@60Hz on VGA
int DispVMode320x240x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_16); }	// 320x240/16-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode320x240x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_15); }	// 320x240/15-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode320x240x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_12); }	// 320x240/12-bit, sys_clock=252 MHz, buffer 115200 B
int DispVMode320x240x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_8); }	// 320x240/8-bit, sys_clock=252 MHz, buffer 76800 B
int DispVMode320x240x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 320x240/6-bit, sys_clock=252 MHz, buffer 61440 B
int DispVMode320x240x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 320x240/4-bit, sys_clock=252 MHz, buffer 38400 B
int DispVMode320x240x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 320x240/3-bit, sys_clock=252 MHz, buffer 30720 B
int DispVMode320x240x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 320x240/2-bit, sys_clock=252 MHz, buffer 19200 B
int DispVMode320x240x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 320x240/1-bit, sys_clock=252 MHz, buffer 9600 B

int DispVMode320x480x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_16); }	// 320x480/16-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode320x480x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_15); }	// 320x480/15-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode320x480x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_12); }	// 320x480/12-bit, sys_clock=252 MHz, buffer 230400 B
int DispVMode320x480x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_8); }	// 320x480/8-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode320x480x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 320x480/6-bit, sys_clock=252 MHz, buffer 122880 B
int DispVMode320x480x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 320x480/4-bit, sys_clock=252 MHz, buffer 76800 B
int DispVMode320x480x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 320x480/3-bit, sys_clock=252 MHz, buffer 61440 B
int DispVMode320x480x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 320x480/2-bit, sys_clock=252 MHz, buffer 38400 B
int DispVMode320x480x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 320x480/1-bit, sys_clock=252 MHz, buffer 19200 B

int DispVMode640x240x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_16); }	// 640x240/16-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode640x240x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_15); }	// 640x240/15-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode640x240x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_12); }	// 640x240/12-bit, sys_clock=252 MHz, buffer 230400 B
int DispVMode640x240x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_8); }	// 640x240/8-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode640x240x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 640x240/6-bit, sys_clock=252 MHz, buffer 122880 B
int DispVMode640x240x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 640x240/4-bit, sys_clock=252 MHz, buffer 76800 B
int DispVMode640x240x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 640x240/3-bit, sys_clock=252 MHz, buffer 61440 B
int DispVMode640x240x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 640x240/2-bit, sys_clock=252 MHz, buffer 38400 B
int DispVMode640x240x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 640x240/1-bit, sys_clock=252 MHz, buffer 19200 B

int DispVMode640x480x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 1, DISPHSTX_FORMAT_8); }	// 640x480/8-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode640x480x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 640x480/6-bit, sys_clock=252 MHz, buffer 245760 B
int DispVMode640x480x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 640x480/4-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode640x480x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 640x480/3-bit, sys_clock=252 MHz, buffer 122880 B
int DispVMode640x480x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 640x480/2-bit, sys_clock=252 MHz, buffer 76800 B
int DispVMode640x480x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x480_fast, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 640x480/1-bit, sys_clock=252 MHz, buffer 38400 B

// 640x720/60Hz/45kHz (half-HD), pixel clock 37.2 MHz, system clock 186 MHz ... detected as 1280x720@60Hz on VGA
int DispVMode320x360x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_16); }		// 320x360/16-bit, sys_clock=186 MHz, buffer 230400 B
int DispVMode320x360x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_15); }		// 320x360/15-bit, sys_clock=186 MHz, buffer 230400 B
int DispVMode320x360x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_12); }		// 320x360/12-bit, sys_clock=186 MHz, buffer 172800 B
int DispVMode320x360x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_8); }			// 320x360/8-bit, sys_clock=186 MHz, buffer 115200 B 
int DispVMode320x360x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 320x360/6-bit, sys_clock=186 MHz, buffer 92160 B  
int DispVMode320x360x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 320x360/4-bit, sys_clock=186 MHz, buffer 57600 B  
int DispVMode320x360x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 320x360/3-bit, sys_clock=186 MHz, buffer 46080 B  
int DispVMode320x360x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 320x360/2-bit, sys_clock=186 MHz, buffer 28800 B  
int DispVMode320x360x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 320x360/1-bit, sys_clock=186 MHz, buffer 14400 B  

int DispVMode320x720x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_12); }		// 320x720/12-bit, sys_clock=186 MHz, buffer 345600 B
int DispVMode320x720x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_8); }			// 320x720/8-bit, sys_clock=186 MHz, buffer 230400 B
int DispVMode320x720x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 320x720/6-bit, sys_clock=186 MHz, buffer 184320 B
int DispVMode320x720x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 320x720/4-bit, sys_clock=186 MHz, buffer 115200 B
int DispVMode320x720x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 320x720/3-bit, sys_clock=186 MHz, buffer 92160 B
int DispVMode320x720x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 320x720/2-bit, sys_clock=186 MHz, buffer 57600 B
int DispVMode320x720x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 320x720/1-bit, sys_clock=186 MHz, buffer 28800 B

int DispVMode640x360x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 2, DISPHSTX_FORMAT_8); }			// 640x360/8-bit, sys_clock=186 MHz, buffer 230400 B
int DispVMode640x360x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 640x360/6-bit, sys_clock=186 MHz, buffer 184320 B
int DispVMode640x360x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 640x360/4-bit, sys_clock=186 MHz, buffer 115200 B
int DispVMode640x360x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 640x360/3-bit, sys_clock=186 MHz, buffer 92160 B
int DispVMode640x360x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 640x360/2-bit, sys_clock=186 MHz, buffer 57600 B
int DispVMode640x360x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 640x360/1-bit, sys_clock=186 MHz, buffer 28800 B

int DispVMode640x720x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x720/6-bit, sys_clock=186 MHz, buffer 368640 B
int DispVMode640x720x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x720/4-bit, sys_clock=186 MHz, buffer 230400 B
int DispVMode640x720x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x720/3-bit, sys_clock=186 MHz, buffer 184320 B
int DispVMode640x720x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x720/2-bit, sys_clock=186 MHz, buffer 115200 B
int DispVMode640x720x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_640x720, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x720/1-bit, sys_clock=186 MHz, buffer 57600 B
#endif // DISPHSTX_WIDTHMAX >= 640

#if DISPHSTX_WIDTHMAX >= 720	// max. supported width
// 720x400/70.1Hz/31.5kHz, pixel clock 28.32 MHz, system clock 141.6 MHz ... detected as 720x400@70Hz on VGA
int DispVMode360x200x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_16); }		// 360x200/16-bit, sys_clock=141.6 MHz, buffer 144000 B
int DispVMode360x200x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_15); }		// 360x200/15-bit, sys_clock=141.6 MHz, buffer 144000 B
int DispVMode360x200x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_12); }		// 360x200/12-bit, sys_clock=141.6 MHz, buffer 108000 B
int DispVMode360x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_8); }			// 360x200/8-bit, sys_clock=141.6 MHz, buffer 72000 B
int DispVMode360x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 360x200/6-bit, sys_clock=141.6 MHz, buffer 57600 B
int DispVMode360x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 360x200/4-bit, sys_clock=141.6 MHz, buffer 36000 B
int DispVMode360x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 360x200/3-bit, sys_clock=141.6 MHz, buffer 28800 B
int DispVMode360x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 360x200/2-bit, sys_clock=141.6 MHz, buffer 18400 B
int DispVMode360x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 360x200/1-bit, sys_clock=141.6 MHz, buffer 9600 B

int DispVMode360x400x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_16); }		// 360x400/16-bit, sys_clock=141.6 MHz, buffer 288000 B
int DispVMode360x400x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_15); }		// 360x400/15-bit, sys_clock=141.6 MHz, buffer 288000 B
int DispVMode360x400x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_12); }		// 360x400/12-bit, sys_clock=141.6 MHz, buffer 216000 B
int DispVMode360x400x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_8); }			// 360x400/8-bit, sys_clock=141.6 MHz, buffer 144000 B
int DispVMode360x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 360x400/6-bit, sys_clock=141.6 MHz, buffer 115200 B
int DispVMode360x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 360x400/4-bit, sys_clock=141.6 MHz, buffer 72000 B
int DispVMode360x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 360x400/3-bit, sys_clock=141.6 MHz, buffer 57600 B
int DispVMode360x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 360x400/2-bit, sys_clock=141.6 MHz, buffer 36800 B
int DispVMode360x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 360x400/1-bit, sys_clock=141.6 MHz, buffer 19200 B

int DispVMode720x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 2, DISPHSTX_FORMAT_8); }			// 720x200/8-bit, sys_clock=141.6 MHz, buffer 144000 B
int DispVMode720x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 720x200/6-bit, sys_clock=141.6 MHz, buffer 115200 B
int DispVMode720x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 720x200/4-bit, sys_clock=141.6 MHz, buffer 72000 B
int DispVMode720x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 720x200/3-bit, sys_clock=141.6 MHz, buffer 57600 B
int DispVMode720x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 720x200/2-bit, sys_clock=141.6 MHz, buffer 36000 B
int DispVMode720x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 720x200/1-bit, sys_clock=141.6 MHz, buffer 18400 B

int DispVMode720x400x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 1, DISPHSTX_FORMAT_8); }			// 720x400/8-bit, sys_clock=141.6 MHz, buffer 288000 B
int DispVMode720x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 720x400/6-bit, sys_clock=141.6 MHz, buffer 230400 B
int DispVMode720x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 720x400/4-bit, sys_clock=141.6 MHz, buffer 144000 B
int DispVMode720x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 720x400/3-bit, sys_clock=141.6 MHz, buffer 115200 B
int DispVMode720x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 720x400/2-bit, sys_clock=141.6 MHz, buffer 72000 B
int DispVMode720x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 720x400/1-bit, sys_clock=141.6 MHz, buffer 36800 B

// 720x400/70.1Hz/31.5kHz, pixel clock 28.32 MHz, system clock 283.2 MHz ... detected as 720x400@70Hz on VGA
int DispVMode360x200x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_16); }	// 360x200/16-bit, sys_clock=283.2 MHz, buffer 144000 B
int DispVMode360x200x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_15); }	// 360x200/15-bit, sys_clock=283.2 MHz, buffer 144000 B
int DispVMode360x200x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_12); }	// 360x200/12-bit, sys_clock=283.2 MHz, buffer 108000 B
int DispVMode360x200x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_8); }	// 360x200/8-bit, sys_clock=283.2 MHz, buffer 72000 B
int DispVMode360x200x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 360x200/6-bit, sys_clock=283.2 MHz, buffer 57600 B
int DispVMode360x200x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 360x200/4-bit, sys_clock=283.2 MHz, buffer 36000 B
int DispVMode360x200x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 360x200/3-bit, sys_clock=283.2 MHz, buffer 28800 B
int DispVMode360x200x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 360x200/2-bit, sys_clock=283.2 MHz, buffer 18400 B
int DispVMode360x200x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 360x200/1-bit, sys_clock=283.2 MHz, buffer 9600 B

int DispVMode360x400x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_16); }	// 360x400/16-bit, sys_clock=283.2 MHz, buffer 288000 B
int DispVMode360x400x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_15); }	// 360x400/15-bit, sys_clock=283.2 MHz, buffer 288000 B
int DispVMode360x400x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_12); }	// 360x400/12-bit, sys_clock=283.2 MHz, buffer 216000 B
int DispVMode360x400x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_8); }	// 360x400/8-bit, sys_clock=283.2 MHz, buffer 144000 B
int DispVMode360x400x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 360x400/6-bit, sys_clock=283.2 MHz, buffer 115200 B
int DispVMode360x400x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 360x400/4-bit, sys_clock=283.2 MHz, buffer 72000 B
int DispVMode360x400x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 360x400/3-bit, sys_clock=283.2 MHz, buffer 57600 B
int DispVMode360x400x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 360x400/2-bit, sys_clock=283.2 MHz, buffer 36800 B
int DispVMode360x400x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 360x400/1-bit, sys_clock=283.2 MHz, buffer 19200 B

int DispVMode720x200x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_16); }	// 720x200/16-bit, sys_clock=283.2 MHz, buffer 288000 B
int DispVMode720x200x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_15); }	// 720x200/15-bit, sys_clock=283.2 MHz, buffer 288000 B
int DispVMode720x200x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_12); }	// 720x200/12-bit, sys_clock=283.2 MHz, buffer 216000 B
int DispVMode720x200x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_8); }	// 720x200/8-bit, sys_clock=283.2 MHz, buffer 144000 B
int DispVMode720x200x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 720x200/6-bit, sys_clock=283.2 MHz, buffer 115200 B
int DispVMode720x200x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 720x200/4-bit, sys_clock=283.2 MHz, buffer 72000 B
int DispVMode720x200x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 720x200/3-bit, sys_clock=283.2 MHz, buffer 57600 B
int DispVMode720x200x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 720x200/2-bit, sys_clock=283.2 MHz, buffer 36000 B
int DispVMode720x200x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 720x200/1-bit, sys_clock=283.2 MHz, buffer 18400 B

int DispVMode720x400x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 1, DISPHSTX_FORMAT_8); }	// 720x400/8-bit, sys_clock=283.2 MHz, buffer 288000 B
int DispVMode720x400x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 720x400/6-bit, sys_clock=283.2 MHz, buffer 230400 B
int DispVMode720x400x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 720x400/4-bit, sys_clock=283.2 MHz, buffer 144000 B
int DispVMode720x400x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 720x400/3-bit, sys_clock=283.2 MHz, buffer 115200 B
int DispVMode720x400x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 720x400/2-bit, sys_clock=283.2 MHz, buffer 72000 B
int DispVMode720x400x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x400_fast, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 720x400/1-bit, sys_clock=283.2 MHz, buffer 36800 B

// 720x480/60Hz/31.5kHz (VGA 3:2, SD video NTSC), pixel clock 28.32 MHz, system clock 141.6 MHz ... detected as 640x480@60Hz on VGA
int DispVMode360x240x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_16); }		// 360x240/16-bit, sys_clock=141.6 MHz, buffer 172800 B
int DispVMode360x240x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_15); }		// 360x240/15-bit, sys_clock=141.6 MHz, buffer 172800 B
int DispVMode360x240x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_12); }		// 360x240/12-bit, sys_clock=141.6 MHz, buffer 129600 B
int DispVMode360x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_8); }			// 360x240/8-bit, sys_clock=141.6 MHz, buffer 86400 B
int DispVMode360x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 360x240/6-bit, sys_clock=141.6 MHz, buffer 69120 B
int DispVMode360x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 360x240/4-bit, sys_clock=141.6 MHz, buffer 43200 B
int DispVMode360x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 360x240/3-bit, sys_clock=141.6 MHz, buffer 34560 B
int DispVMode360x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 360x240/2-bit, sys_clock=141.6 MHz, buffer 22080 B
int DispVMode360x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 360x240/1-bit, sys_clock=141.6 MHz, buffer 11520 B

int DispVMode360x480x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_16); }		// 360x480/16-bit, sys_clock=141.6 MHz, buffer 345600 B
int DispVMode360x480x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_15); }		// 360x480/15-bit, sys_clock=141.6 MHz, buffer 345600 B
int DispVMode360x480x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_12); }		// 360x480/12-bit, sys_clock=141.6 MHz, buffer 259200 B
int DispVMode360x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_8); }			// 360x480/8-bit, sys_clock=141.6 MHz, buffer 172800 B
int DispVMode360x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 360x480/6-bit, sys_clock=141.6 MHz, buffer 138240 B
int DispVMode360x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 360x480/4-bit, sys_clock=141.6 MHz, buffer 86400 B
int DispVMode360x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 360x480/3-bit, sys_clock=141.6 MHz, buffer 69120 B
int DispVMode360x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 360x480/2-bit, sys_clock=141.6 MHz, buffer 44160 B
int DispVMode360x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 360x480/1-bit, sys_clock=141.6 MHz, buffer 23040 B

int DispVMode720x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 2, DISPHSTX_FORMAT_8); }			// 720x240/8-bit, sys_clock=141.6 MHz, buffer 172800 B
int DispVMode720x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 720x240/6-bit, sys_clock=141.6 MHz, buffer 138240 B
int DispVMode720x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 720x240/4-bit, sys_clock=141.6 MHz, buffer 86400 B
int DispVMode720x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 720x240/3-bit, sys_clock=141.6 MHz, buffer 69120 B
int DispVMode720x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 720x240/2-bit, sys_clock=141.6 MHz, buffer 43200 B
int DispVMode720x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 720x240/1-bit, sys_clock=141.6 MHz, buffer 22080 B

int DispVMode720x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 1, DISPHSTX_FORMAT_8); }			// 720x480/8-bit, sys_clock=141.6 MHz, buffer 345600 B
int DispVMode720x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 720x480/6-bit, sys_clock=141.6 MHz, buffer 276480 B
int DispVMode720x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 720x480/4-bit, sys_clock=141.6 MHz, buffer 172800 B
int DispVMode720x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 720x480/3-bit, sys_clock=141.6 MHz, buffer 138240 B
int DispVMode720x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 720x480/2-bit, sys_clock=141.6 MHz, buffer 86400 B
int DispVMode720x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 720x480/1-bit, sys_clock=141.6 MHz, buffer 44160 B

// 720x480/60Hz/31.5kHz (VGA 3:2, SD video NTSC), pixel clock 28.32 MHz, system clock 283.2 MHz ... detected as 640x480@60Hz on VGA
int DispVMode360x240x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_16); }	// 360x240/16-bit, sys_clock=283.2 MHz, buffer 172800 B
int DispVMode360x240x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_15); }	// 360x240/15-bit, sys_clock=283.2 MHz, buffer 172800 B
int DispVMode360x240x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_12); }	// 360x240/12-bit, sys_clock=283.2 MHz, buffer 129600 B
int DispVMode360x240x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_8); }	// 360x240/8-bit, sys_clock=283.2 MHz, buffer 86400 B
int DispVMode360x240x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_6_PAL); }	// 360x240/6-bit, sys_clock=283.2 MHz, buffer 69120 B
int DispVMode360x240x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_4_PAL); }	// 360x240/4-bit, sys_clock=283.2 MHz, buffer 43200 B
int DispVMode360x240x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_3_PAL); }	// 360x240/3-bit, sys_clock=283.2 MHz, buffer 34560 B
int DispVMode360x240x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_2_PAL); }	// 360x240/2-bit, sys_clock=283.2 MHz, buffer 22080 B
int DispVMode360x240x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 2, DISPHSTX_FORMAT_1_PAL); }	// 360x240/1-bit, sys_clock=283.2 MHz, buffer 11520 B

int DispVMode360x480x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_16); }	// 360x480/16-bit, sys_clock=283.2 MHz, buffer 345600 B
int DispVMode360x480x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_15); }	// 360x480/15-bit, sys_clock=283.2 MHz, buffer 345600 B
int DispVMode360x480x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_12); }	// 360x480/12-bit, sys_clock=283.2 MHz, buffer 259200 B
int DispVMode360x480x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_8); }	// 360x480/8-bit, sys_clock=283.2 MHz, buffer 172800 B
int DispVMode360x480x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_6_PAL); }	// 360x480/6-bit, sys_clock=283.2 MHz, buffer 138240 B
int DispVMode360x480x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_4_PAL); }	// 360x480/4-bit, sys_clock=283.2 MHz, buffer 86400 B
int DispVMode360x480x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_3_PAL); }	// 360x480/3-bit, sys_clock=283.2 MHz, buffer 69120 B
int DispVMode360x480x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_2_PAL); }	// 360x480/2-bit, sys_clock=283.2 MHz, buffer 44160 B
int DispVMode360x480x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 2, 1, DISPHSTX_FORMAT_1_PAL); }	// 360x480/1-bit, sys_clock=283.2 MHz, buffer 23040 B

int DispVMode720x240x16_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_16); }	// 720x240/16-bit, sys_clock=283.2 MHz, buffer 345600 B
int DispVMode720x240x15_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_15); }	// 720x240/15-bit, sys_clock=283.2 MHz, buffer 345600 B
int DispVMode720x240x12_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_12); }	// 720x240/12-bit, sys_clock=283.2 MHz, buffer 259200 B
int DispVMode720x240x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_8); }	// 720x240/8-bit, sys_clock=283.2 MHz, buffer 172800 B
int DispVMode720x240x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_6_PAL); }	// 720x240/6-bit, sys_clock=283.2 MHz, buffer 138240 B
int DispVMode720x240x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_4_PAL); }	// 720x240/4-bit, sys_clock=283.2 MHz, buffer 86400 B 
int DispVMode720x240x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_3_PAL); }	// 720x240/3-bit, sys_clock=283.2 MHz, buffer 69120 B 
int DispVMode720x240x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_2_PAL); }	// 720x240/2-bit, sys_clock=283.2 MHz, buffer 43200 B 
int DispVMode720x240x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 2, DISPHSTX_FORMAT_1_PAL); }	// 720x240/1-bit, sys_clock=283.2 MHz, buffer 22080 B 

int DispVMode720x480x8_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 1, DISPHSTX_FORMAT_8); }	// 720x480/8-bit, sys_clock=283.2 MHz, buffer 345600 B
int DispVMode720x480x6_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 1, DISPHSTX_FORMAT_6_PAL); }	// 720x480/6-bit, sys_clock=283.2 MHz, buffer 276480 B
int DispVMode720x480x4_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 1, DISPHSTX_FORMAT_4_PAL); }	// 720x480/4-bit, sys_clock=283.2 MHz, buffer 172800 B
int DispVMode720x480x3_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 1, DISPHSTX_FORMAT_3_PAL); }	// 720x480/3-bit, sys_clock=283.2 MHz, buffer 138240 B
int DispVMode720x480x2_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 1, DISPHSTX_FORMAT_2_PAL); }	// 720x480/2-bit, sys_clock=283.2 MHz, buffer 86400 B 
int DispVMode720x480x1_Fast(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_720x480_fast, 1, 1, DISPHSTX_FORMAT_1_PAL); }	// 720x480/1-bit, sys_clock=283.2 MHz, buffer 44160 B 
#endif // DISPHSTX_WIDTHMAX >= 720

#if DISPHSTX_WIDTHMAX >= 800	// max. supported width
// 800x480/60Hz/31.5kHz (MAME 5:3), pixel clock 31.5 MHz, system clock 157.333 MHz ... detected as 640x480@60Hz on VGA
int DispVMode400x240x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_16); }		// 400x240/16-bit, sys_clock=160 MHz, buffer 192000 B
int DispVMode400x240x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_15); }		// 400x240/15-bit, sys_clock=160 MHz, buffer 192000 B
int DispVMode400x240x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_12); }		// 400x240/12-bit, sys_clock=160 MHz, buffer 144000 B
int DispVMode400x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_8); }			// 400x240/8-bit, sys_clock=160 MHz, buffer 96000 B
int DispVMode400x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 400x240/6-bit, sys_clock=160 MHz, buffer 76800 B
int DispVMode400x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 400x240/4-bit, sys_clock=160 MHz, buffer 48000 B
int DispVMode400x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 400x240/3-bit, sys_clock=160 MHz, buffer 38400 B
int DispVMode400x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 400x240/2-bit, sys_clock=160 MHz, buffer 24000 B
int DispVMode400x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 400x240/1-bit, sys_clock=160 MHz, buffer 12480 B

int DispVMode400x480x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_16); }		// 400x480/16-bit, sys_clock=160 MHz, buffer 384000 B
int DispVMode400x480x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_15); }		// 400x480/15-bit, sys_clock=160 MHz, buffer 384000 B
int DispVMode400x480x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_12); }		// 400x480/12-bit, sys_clock=160 MHz, buffer 288000 B
int DispVMode400x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_8); }			// 400x480/8-bit, sys_clock=160 MHz, buffer 192000 B
int DispVMode400x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 400x480/6-bit, sys_clock=160 MHz, buffer 153600 B
int DispVMode400x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 400x480/4-bit, sys_clock=160 MHz, buffer 96000 B
int DispVMode400x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 400x480/3-bit, sys_clock=160 MHz, buffer 76800 B
int DispVMode400x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 400x480/2-bit, sys_clock=160 MHz, buffer 48000 B
int DispVMode400x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 400x480/1-bit, sys_clock=160 MHz, buffer 24960 B

int DispVMode800x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 2, DISPHSTX_FORMAT_8); }			// 800x240/8-bit, sys_clock=160 MHz, buffer 192000 B
int DispVMode800x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 800x240/6-bit, sys_clock=160 MHz, buffer 153600 B
int DispVMode800x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 800x240/4-bit, sys_clock=160 MHz, buffer 96000 B
int DispVMode800x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 800x240/3-bit, sys_clock=160 MHz, buffer 76800 B
int DispVMode800x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 800x240/2-bit, sys_clock=160 MHz, buffer 48000 B
int DispVMode800x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 800x240/1-bit, sys_clock=160 MHz, buffer 24000 B

int DispVMode800x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 1, DISPHSTX_FORMAT_8); }			// 800x480/8-bit, sys_clock=160 MHz, buffer 384000 B
int DispVMode800x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 800x480/6-bit, sys_clock=160 MHz, buffer 307200 B
int DispVMode800x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 800x480/4-bit, sys_clock=160 MHz, buffer 192000 B
int DispVMode800x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 800x480/3-bit, sys_clock=160 MHz, buffer 153600 B
int DispVMode800x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 800x480/2-bit, sys_clock=160 MHz, buffer 96000 B
int DispVMode800x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x480, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 800x480/1-bit, sys_clock=160 MHz, buffer 48000 B

// 800x600/60.3Hz/37.9kHz (SVGA 4:3, VESA DMT ID 09h), pixel clock 40 MHz, system clock 200 MHz ... detected as 800x600@60Hz on VGA
int DispVMode400x300x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_16); }		// 400x300/16-bit, sys_clock=200 MHz, buffer 240000 B
int DispVMode400x300x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_15); }		// 400x300/15-bit, sys_clock=200 MHz, buffer 240000 B
int DispVMode400x300x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_12); }		// 400x300/12-bit, sys_clock=200 MHz, buffer 180000 B
int DispVMode400x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_8); }			// 400x300/8-bit, sys_clock=200 MHz, buffer 120000 B
int DispVMode400x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 400x300/6-bit, sys_clock=200 MHz, buffer 96000 B
int DispVMode400x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 400x300/4-bit, sys_clock=200 MHz, buffer 60000 B
int DispVMode400x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 400x300/3-bit, sys_clock=200 MHz, buffer 48000 B
int DispVMode400x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 400x300/2-bit, sys_clock=200 MHz, buffer 30000 B
int DispVMode400x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 400x300/1-bit, sys_clock=200 MHz, buffer 15600 B

int DispVMode400x600x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_12); }		// 400x600/12-bit, sys_clock=200 MHz, buffer 360000 B
int DispVMode400x600x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_8); }			// 400x600/8-bit, sys_clock=200 MHz, buffer 240000 B
int DispVMode400x600x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 400x600/6-bit, sys_clock=200 MHz, buffer 192000 B
int DispVMode400x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 400x600/4-bit, sys_clock=200 MHz, buffer 120000 B
int DispVMode400x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 400x600/3-bit, sys_clock=200 MHz, buffer 96000 B
int DispVMode400x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 400x600/2-bit, sys_clock=200 MHz, buffer 60000 B
int DispVMode400x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 400x600/1-bit, sys_clock=200 MHz, buffer 31200 B

int DispVMode800x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 2, DISPHSTX_FORMAT_8); }			// 800x300/8-bit, sys_clock=200 MHz, buffer 240000 B
int DispVMode800x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 800x300/6-bit, sys_clock=200 MHz, buffer 192000 B
int DispVMode800x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 800x300/4-bit, sys_clock=200 MHz, buffer 120000 B
int DispVMode800x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 800x300/3-bit, sys_clock=200 MHz, buffer 96000 B
int DispVMode800x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 800x300/2-bit, sys_clock=200 MHz, buffer 60000 B
int DispVMode800x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 800x300/1-bit, sys_clock=200 MHz, buffer 31200 B

int DispVMode800x600x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 800x600/6-bit, sys_clock=200 MHz, buffer 384000 B
int DispVMode800x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 800x600/4-bit, sys_clock=200 MHz, buffer 240000 B
int DispVMode800x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 800x600/3-bit, sys_clock=200 MHz, buffer 192000 B
int DispVMode800x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 800x600/2-bit, sys_clock=200 MHz, buffer 120000 B
int DispVMode800x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_800x600, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 800x600/1-bit, sys_clock=200 MHz, buffer 60000 B
#endif // DISPHSTX_WIDTHMAX >= 800

#if DISPHSTX_WIDTHMAX >= 848	// max. supported width
// 848x480/60Hz/31.4kHz (DMT SVGA 16:9), pixel clock 33.33 MHz, system clock 166.666 MHz ... detected as 640x480@60Hz on VGA
int DispVMode424x240x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_16); }		// 424x240/16-bit, sys_clock=166.666 MHz, buffer 203520 B
int DispVMode424x240x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_15); }		// 424x240/15-bit, sys_clock=166.666 MHz, buffer 203520 B
int DispVMode424x240x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_12); }		// 424x240/12-bit, sys_clock=166.666 MHz, buffer 152640 B
int DispVMode424x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_8); }			// 424x240/8-bit, sys_clock=166.666 MHz, buffer 101760 B
int DispVMode424x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 424x240/6-bit, sys_clock=166.666 MHz, buffer 81600 B
int DispVMode424x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 424x240/4-bit, sys_clock=166.666 MHz, buffer 50880 B
int DispVMode424x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 424x240/3-bit, sys_clock=166.666 MHz, buffer 41280 B
int DispVMode424x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 424x240/2-bit, sys_clock=166.666 MHz, buffer 25920 B
int DispVMode424x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 424x240/1-bit, sys_clock=166.666 MHz, buffer 13440 B

int DispVMode424x480x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_16); }		// 424x480/16-bit, sys_clock=166.666 MHz, buffer 407040 B
int DispVMode424x480x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_15); }		// 424x480/15-bit, sys_clock=166.666 MHz, buffer 407040 B
int DispVMode424x480x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_12); }		// 424x480/12-bit, sys_clock=166.666 MHz, buffer 305280 B
int DispVMode424x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_8); }			// 424x480/8-bit, sys_clock=166.666 MHz, buffer 203520 B
int DispVMode424x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 424x480/6-bit, sys_clock=166.666 MHz, buffer 163200 B
int DispVMode424x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 424x480/4-bit, sys_clock=166.666 MHz, buffer 101760 B
int DispVMode424x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 424x480/3-bit, sys_clock=166.666 MHz, buffer 82560 B
int DispVMode424x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 424x480/2-bit, sys_clock=166.666 MHz, buffer 51840 B
int DispVMode424x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 424x480/1-bit, sys_clock=166.666 MHz, buffer 26880 B

int DispVMode848x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 2, DISPHSTX_FORMAT_8); }			// 848x240/8-bit, sys_clock=166.666 MHz, buffer 203520 B
int DispVMode848x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 848x240/6-bit, sys_clock=166.666 MHz, buffer 163200 B
int DispVMode848x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 848x240/4-bit, sys_clock=166.666 MHz, buffer 101760 B
int DispVMode848x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 848x240/3-bit, sys_clock=166.666 MHz, buffer 81600 B
int DispVMode848x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 848x240/2-bit, sys_clock=166.666 MHz, buffer 50880 B
int DispVMode848x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 848x240/1-bit, sys_clock=166.666 MHz, buffer 25920 B

int DispVMode848x480x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 1, DISPHSTX_FORMAT_8); }			// 848x480/8-bit, sys_clock=166.666 MHz, buffer 407040 B
int DispVMode848x480x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 848x480/6-bit, sys_clock=166.666 MHz, buffer 326400 B
int DispVMode848x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 848x480/4-bit, sys_clock=166.666 MHz, buffer 203520 B
int DispVMode848x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 848x480/3-bit, sys_clock=166.666 MHz, buffer 163200 B
int DispVMode848x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 848x480/2-bit, sys_clock=166.666 MHz, buffer 101760 B
int DispVMode848x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_848x480, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 848x480/1-bit, sys_clock=166.666 MHz, buffer 51840 B
#endif // DISPHSTX_WIDTHMAX >= 848

#if DISPHSTX_WIDTHMAX >= 960	// max. supported width
// 960x720/60Hz/45kHz (VESA 4:3), pixel clock 55.7 MHz, system clock 278.4 MHz ... detected as 1280x720@60Hz on VGA
int DispVMode480x360x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_16); }		// 480x360/16-bit, sys_clock=278.4 MHz, buffer 345600 B
int DispVMode480x360x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_15); }		// 480x360/15-bit, sys_clock=278.4 MHz, buffer 345600 B
int DispVMode480x360x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_12); }		// 480x360/12-bit, sys_clock=278.4 MHz, buffer 259200 B
int DispVMode480x360x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_8); }			// 480x360/8-bit, sys_clock=278.4 MHz, buffer 172800 B
int DispVMode480x360x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 480x360/6-bit, sys_clock=278.4 MHz, buffer 138240 B
int DispVMode480x360x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 480x360/4-bit, sys_clock=278.4 MHz, buffer 86400 B
int DispVMode480x360x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 480x360/3-bit, sys_clock=278.4 MHz, buffer 69120 B
int DispVMode480x360x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 480x360/2-bit, sys_clock=278.4 MHz, buffer 43200 B
int DispVMode480x360x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 480x360/1-bit, sys_clock=278.4 MHz, buffer 21600 B

int DispVMode480x720x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 1, DISPHSTX_FORMAT_8); }			// 480x720/8-bit, sys_clock=278.4 MHz, buffer 345600 B
int DispVMode480x720x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 480x720/6-bit, sys_clock=278.4 MHz, buffer 276480 B
int DispVMode480x720x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 480x720/4-bit, sys_clock=278.4 MHz, buffer 172800 B
int DispVMode480x720x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 480x720/3-bit, sys_clock=278.4 MHz, buffer 138240 B
int DispVMode480x720x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 480x720/2-bit, sys_clock=278.4 MHz, buffer 86400 B
int DispVMode480x720x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 480x720/1-bit, sys_clock=278.4 MHz, buffer 43200 B

int DispVMode960x360x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 2, DISPHSTX_FORMAT_8); }			// 960x360/8-bit, sys_clock=278.4 MHz, buffer 345600 B
int DispVMode960x360x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 960x360/6-bit, sys_clock=278.4 MHz, buffer 276480 B
int DispVMode960x360x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 960x360/4-bit, sys_clock=278.4 MHz, buffer 172800 B
int DispVMode960x360x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 960x360/3-bit, sys_clock=278.4 MHz, buffer 138240 B
int DispVMode960x360x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 960x360/2-bit, sys_clock=278.4 MHz, buffer 86400 B
int DispVMode960x360x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 960x360/1-bit, sys_clock=278.4 MHz, buffer 43200 B

int DispVMode960x720x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 960x720/4-bit, sys_clock=278.4 MHz, buffer 345600 B
int DispVMode960x720x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 960x720/3-bit, sys_clock=278.4 MHz, buffer 276480 B
int DispVMode960x720x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 960x720/2-bit, sys_clock=278.4 MHz, buffer 172800 B
int DispVMode960x720x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_960x720, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 960x720/1-bit, sys_clock=278.4 MHz, buffer 86400 B
#endif // DISPHSTX_WIDTHMAX >= 960

#if DISPHSTX_WIDTHMAX >= 1024	// max. supported width
// vmodetime_1024x768: 1024x768/59.9Hz/48.3kHz (XGA 4:3, VESA DMT ID 10h), pixel clock 64.8 MHz, system clock 324 MHz ... detected as 1024x768@60Hz on VGA
int DispVMode512x384x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_16); }		// 512x384/16-bit, sys_clock=324 MHz, buffer 393216 B
int DispVMode512x384x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_15); }		// 512x384/15-bit, sys_clock=324 MHz, buffer 393216 B
int DispVMode512x384x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_12); }		// 512x384/12-bit, sys_clock=324 MHz, buffer 294912 B
int DispVMode512x384x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_8); }		// 512x384/8-bit, sys_clock=324 MHz, buffer 196608 B
int DispVMode512x384x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 512x384/6-bit, sys_clock=324 MHz, buffer 158208 B
int DispVMode512x384x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 512x384/4-bit, sys_clock=324 MHz, buffer 98304 B
int DispVMode512x384x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 512x384/3-bit, sys_clock=324 MHz, buffer 79872 B
int DispVMode512x384x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 512x384/2-bit, sys_clock=324 MHz, buffer 49152 B
int DispVMode512x384x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 512x384/1-bit, sys_clock=324 MHz, buffer 24576 B

int DispVMode512x768x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 1, DISPHSTX_FORMAT_8); }		// 512x768/8-bit, sys_clock=324 MHz, buffer 393216 B
int DispVMode512x768x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 512x768/6-bit, sys_clock=324 MHz, buffer 316416 B
int DispVMode512x768x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 512x768/4-bit, sys_clock=324 MHz, buffer 196608 B
int DispVMode512x768x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 512x768/3-bit, sys_clock=324 MHz, buffer 159744 B
int DispVMode512x768x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 512x768/2-bit, sys_clock=324 MHz, buffer 98304 B
int DispVMode512x768x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 512x768/1-bit, sys_clock=324 MHz, buffer 49152 B

int DispVMode1024x384x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 2, DISPHSTX_FORMAT_8); }		// 1024x384/8-bit, sys_clock=324 MHz, buffer 393216 B
int DispVMode1024x384x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1024x384/6-bit, sys_clock=324 MHz, buffer 314880 B
int DispVMode1024x384x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1024x384/4-bit, sys_clock=324 MHz, buffer 196608 B
int DispVMode1024x384x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1024x384/3-bit, sys_clock=324 MHz, buffer 158208 B
int DispVMode1024x384x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1024x384/2-bit, sys_clock=324 MHz, buffer 98304 B
int DispVMode1024x384x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1024x384/1-bit, sys_clock=324 MHz, buffer 49152 B

int DispVMode1024x768x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1024x768/4-bit, sys_clock=324 MHz, buffer 393216 B
int DispVMode1024x768x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1024x768/3-bit, sys_clock=324 MHz, buffer 316416 B
int DispVMode1024x768x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1024x768/2-bit, sys_clock=324 MHz, buffer 196608 B
int DispVMode1024x768x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1024x768, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1024x768/1-bit, sys_clock=324 MHz, buffer 98304 B
#endif // DISPHSTX_WIDTHMAX >= 1024

#if DISPHSTX_WIDTHMAX >= 1056	// max. supported width
// 1056x600/60Hz/37.9kHz (near 16:9), pixel clock 52.8 MHz, system clock 264 MHz ... detected as 800x600@60Hz on VGA
int DispVMode528x300x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_16); }		// 528x300/16-bit, sys_clock=264 MHz, buffer 316800 B
int DispVMode528x300x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_15); }		// 528x300/15-bit, sys_clock=264 MHz, buffer 316800 B
int DispVMode528x300x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_12); }		// 528x300/12-bit, sys_clock=264 MHz, buffer 237600 B
int DispVMode528x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_8); }		// 528x300/8-bit, sys_clock=264 MHz, buffer 158400 B
int DispVMode528x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 528x300/6-bit, sys_clock=264 MHz, buffer 127200 B
int DispVMode528x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 528x300/4-bit, sys_clock=264 MHz, buffer 79200 B
int DispVMode528x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 528x300/3-bit, sys_clock=264 MHz, buffer 63600 B
int DispVMode528x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 528x300/2-bit, sys_clock=264 MHz, buffer 39600 B
int DispVMode528x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 528x300/1-bit, sys_clock=264 MHz, buffer 20400 B

int DispVMode528x600x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 1, DISPHSTX_FORMAT_8); }		// 528x600/8-bit, sys_clock=264 MHz, buffer 316800 B
int DispVMode528x600x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 528x600/6-bit, sys_clock=264 MHz, buffer 254400 B
int DispVMode528x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 528x600/4-bit, sys_clock=264 MHz, buffer 158400 B
int DispVMode528x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 528x600/3-bit, sys_clock=264 MHz, buffer 127200 B
int DispVMode528x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 528x600/2-bit, sys_clock=264 MHz, buffer 79200 B
int DispVMode528x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 528x600/1-bit, sys_clock=264 MHz, buffer 40800 B

int DispVMode1056x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 2, DISPHSTX_FORMAT_8); }		// 1056x300/8-bit, sys_clock=264 MHz, buffer 316800 B
int DispVMode1056x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1056x300/6-bit, sys_clock=264 MHz, buffer 254400 B
int DispVMode1056x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1056x300/4-bit, sys_clock=264 MHz, buffer 158400 B
int DispVMode1056x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1056x300/3-bit, sys_clock=264 MHz, buffer 127200 B
int DispVMode1056x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1056x300/2-bit, sys_clock=264 MHz, buffer 79200 B
int DispVMode1056x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1056x300/1-bit, sys_clock=264 MHz, buffer 39600 B

int DispVMode1056x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1056x600/4-bit, sys_clock=264 MHz, buffer 316800 B
int DispVMode1056x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1056x600/3-bit, sys_clock=264 MHz, buffer 254400 B
int DispVMode1056x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1056x600/2-bit, sys_clock=264 MHz, buffer 158400 B
int DispVMode1056x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1056x600, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1056x600/1-bit, sys_clock=264 MHz, buffer 79200 B
#endif // DISPHSTX_WIDTHMAX >= 1056

#if DISPHSTX_WIDTHMAX >= 1152	// max. supported width
// vmodetime_1152x600: 1152x600/60Hz/37.9kHz, pixel clock 57.6 MHz, system clock 288 MHz ... detected as 800x600@60Hz on VGA
int DispVMode576x300x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_16); }		// 576x300/16-bit, sys_clock=288 MHz, buffer 345600 B
int DispVMode576x300x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_15); }		// 576x300/15-bit, sys_clock=288 MHz, buffer 345600 B
int DispVMode576x300x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_12); }		// 576x300/12-bit, sys_clock=288 MHz, buffer 259200 B
int DispVMode576x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_8); }		// 576x300/8-bit, sys_clock=288 MHz, buffer 172800 B
int DispVMode576x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 576x300/6-bit, sys_clock=288 MHz, buffer 139200 B
int DispVMode576x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 576x300/4-bit, sys_clock=288 MHz, buffer 86400 B
int DispVMode576x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 576x300/3-bit, sys_clock=288 MHz, buffer 69600 B
int DispVMode576x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 576x300/2-bit, sys_clock=288 MHz, buffer 43200 B
int DispVMode576x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 576x300/1-bit, sys_clock=288 MHz, buffer 21600 B

int DispVMode576x600x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 1, DISPHSTX_FORMAT_8); }		// 576x600/8-bit, sys_clock=288 MHz, buffer 345600 B
int DispVMode576x600x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 576x600/6-bit, sys_clock=288 MHz, buffer 278400 B
int DispVMode576x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 576x600/4-bit, sys_clock=288 MHz, buffer 172800 B
int DispVMode576x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 576x600/3-bit, sys_clock=288 MHz, buffer 139200 B
int DispVMode576x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 576x600/2-bit, sys_clock=288 MHz, buffer 86400 B
int DispVMode576x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 576x600/1-bit, sys_clock=288 MHz, buffer 43200 B

int DispVMode1152x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 2, DISPHSTX_FORMAT_8); }		// 1152x300/8-bit, sys_clock=288 MHz, buffer 345600 B
int DispVMode1152x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1152x300/6-bit, sys_clock=288 MHz, buffer 277200 B
int DispVMode1152x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1152x300/4-bit, sys_clock=288 MHz, buffer 172800 B
int DispVMode1152x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1152x300/3-bit, sys_clock=288 MHz, buffer 139200 B
int DispVMode1152x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1152x300/2-bit, sys_clock=288 MHz, buffer 86400 B
int DispVMode1152x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1152x300/1-bit, sys_clock=288 MHz, buffer 43200 B

int DispVMode1152x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1152x600/4-bit, sys_clock=288 MHz, buffer 345600 B
int DispVMode1152x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1152x600/3-bit, sys_clock=288 MHz, buffer 278400 B
int DispVMode1152x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1152x600/2-bit, sys_clock=288 MHz, buffer 172800 B
int DispVMode1152x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1152x600, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1152x600/1-bit, sys_clock=288 MHz, buffer 86400 B
#endif // DISPHSTX_WIDTHMAX >= 1152

#if DISPHSTX_WIDTHMAX >= 1280	// max. supported width
// 1280x400/70Hz/31.5kHz, pixel clock 50.4 MHz, system clock 252 MHz ... detected as 720x400@70Hz on VGA
int DispVMode1280x200x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 2, DISPHSTX_FORMAT_8); }		// 1280x200/8-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode1280x200x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1280x200/6-bit, sys_clock=252 MHz, buffer 204800 B
int DispVMode1280x200x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1280x200/4-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode1280x200x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1280x200/3-bit, sys_clock=252 MHz, buffer 102400 B
int DispVMode1280x200x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1280x200/2-bit, sys_clock=252 MHz, buffer 64000 B
int DispVMode1280x200x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1280x200/1-bit, sys_clock=252 MHz, buffer 32000 B

int DispVMode1280x400x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 1, DISPHSTX_FORMAT_6_PAL); }		// 1280x400/6-bit, sys_clock=252 MHz, buffer 409600 B
int DispVMode1280x400x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1280x400/4-bit, sys_clock=252 MHz, buffer 256000 B
int DispVMode1280x400x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1280x400/3-bit, sys_clock=252 MHz, buffer 204800 B
int DispVMode1280x400x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1280x400/2-bit, sys_clock=252 MHz, buffer 128000 B
int DispVMode1280x400x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x400, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1280x400/1-bit, sys_clock=252 MHz, buffer 64000 B

// 1280x480/60Hz/31.5kHz (8:3), pixel clock 50.4 MHz, system clock 252 MHz ... detected as 640x480@60Hz on VGA
int DispVMode1280x240x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 2, DISPHSTX_FORMAT_8); }		// 1280x240/8-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode1280x240x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1280x240/6-bit, sys_clock=252 MHz, buffer 245760 B
int DispVMode1280x240x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1280x240/4-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode1280x240x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1280x240/3-bit, sys_clock=252 MHz, buffer 122880 B
int DispVMode1280x240x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1280x240/2-bit, sys_clock=252 MHz, buffer 76800 B
int DispVMode1280x240x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1280x240/1-bit, sys_clock=252 MHz, buffer 38400 B

int DispVMode1280x480x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1280x480/4-bit, sys_clock=252 MHz, buffer 307200 B
int DispVMode1280x480x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1280x480/3-bit, sys_clock=252 MHz, buffer 245760 B
int DispVMode1280x480x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1280x480/2-bit, sys_clock=252 MHz, buffer 153600 B
int DispVMode1280x480x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x480, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1280x480/1-bit, sys_clock=252 MHz, buffer 76800 B

// 1280x600/56Hz/35.2kHz, pixel clock 57.6 MHz, system clock 288 MHz ... detected as 800x600@56Hz on VGA
int DispVMode640x300x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_16); }		// 640x300/16-bit, sys_clock=288 MHz, buffer 384000 B
int DispVMode640x300x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_15); }		// 640x300/15-bit, sys_clock=288 MHz, buffer 384000 B
int DispVMode640x300x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_12); }		// 640x300/12-bit, sys_clock=288 MHz, buffer 288000 B
int DispVMode640x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_8); }		// 640x300/8-bit, sys_clock=288 MHz, buffer 192000 B
int DispVMode640x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 640x300/6-bit, sys_clock=288 MHz, buffer 153600 B
int DispVMode640x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 640x300/4-bit, sys_clock=288 MHz, buffer 96000 B
int DispVMode640x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 640x300/3-bit, sys_clock=288 MHz, buffer 76800 B
int DispVMode640x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 640x300/2-bit, sys_clock=288 MHz, buffer 48000 B
int DispVMode640x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 640x300/1-bit, sys_clock=288 MHz, buffer 24000 B

int DispVMode640x600x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 1, DISPHSTX_FORMAT_8); }		// 640x600/8-bit, sys_clock=288 MHz, buffer 384000 B
int DispVMode640x600x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x600/6-bit, sys_clock=288 MHz, buffer 307200 B
int DispVMode640x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x600/4-bit, sys_clock=288 MHz, buffer 192000 B
int DispVMode640x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x600/3-bit, sys_clock=288 MHz, buffer 153600 B
int DispVMode640x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x600/2-bit, sys_clock=288 MHz, buffer 96000 B
int DispVMode640x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x600/1-bit, sys_clock=288 MHz, buffer 48000 B

int DispVMode1280x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 2, DISPHSTX_FORMAT_8); }		// 1280x300/8-bit, sys_clock=288 MHz, buffer 384000 B
int DispVMode1280x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1280x300/6-bit, sys_clock=288 MHz, buffer 307200 B
int DispVMode1280x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1280x300/4-bit, sys_clock=288 MHz, buffer 192000 B
int DispVMode1280x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1280x300/3-bit, sys_clock=288 MHz, buffer 153600 B
int DispVMode1280x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1280x300/2-bit, sys_clock=288 MHz, buffer 96000 B
int DispVMode1280x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1280x300/1-bit, sys_clock=288 MHz, buffer 48000 B

int DispVMode1280x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1280x600/4-bit, sys_clock=288 MHz, buffer 384000 B
int DispVMode1280x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1280x600/3-bit, sys_clock=288 MHz, buffer 307200 B
int DispVMode1280x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1280x600/2-bit, sys_clock=288 MHz, buffer 192000 B
int DispVMode1280x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x600, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1280x600/1-bit, sys_clock=288 MHz, buffer 96000 B

// 1280x720/60.1Hz/45kHz (16:9, VESA DMT ID 55h), pixel clock 74.4 MHz, system clock 372 MHz ... detected as 1280x720@60Hz on VGA
int DispVMode640x360x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 2, 2, DISPHSTX_FORMAT_12); }		// 640x360/12-bit, sys_clock=372 MHz, buffer 345600 B

int DispVMode1280x360x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1280x360/6-bit, sys_clock=372 MHz, buffer 368640 B
int DispVMode1280x360x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1280x360/4-bit, sys_clock=372 MHz, buffer 230400 B
int DispVMode1280x360x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1280x360/3-bit, sys_clock=372 MHz, buffer 184320 B
int DispVMode1280x360x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1280x360/2-bit, sys_clock=372 MHz, buffer 115200 B
int DispVMode1280x360x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1280x360/1-bit, sys_clock=372 MHz, buffer 57600 B

int DispVMode1280x720x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1280x720/3-bit, sys_clock=372 MHz, buffer 368640 B
int DispVMode1280x720x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1280x720/2-bit, sys_clock=372 MHz, buffer 230400 B
int DispVMode1280x720x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x720, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1280x720/1-bit, sys_clock=372 MHz, buffer 115200 B

// 1280x768/59.6Hz/47.6kHz (VESA DMT ID 17h), pixel clock 79.2 MHz, system clock 396 MHz ... detected as 1280x768@60Hz on VGA
int DispVMode640x384x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_12); }		// 640x384/12-bit, sys_clock=396 MHz, buffer 368640 B
int DispVMode640x384x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_8); }		// 640x384/8-bit, sys_clock=396 MHz, buffer 245760 B
int DispVMode640x384x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 640x384/6-bit, sys_clock=396 MHz, buffer 196608 B
int DispVMode640x384x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 640x384/4-bit, sys_clock=396 MHz, buffer 122880 B
int DispVMode640x384x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 640x384/3-bit, sys_clock=396 MHz, buffer 98304 B
int DispVMode640x384x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 640x384/2-bit, sys_clock=396 MHz, buffer 61440 B
int DispVMode640x384x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 640x384/1-bit, sys_clock=396 MHz, buffer 30720 B

int DispVMode640x768x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x768/6-bit, sys_clock=396 MHz, buffer 393216 B
int DispVMode640x768x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x768/4-bit, sys_clock=396 MHz, buffer 245760 B
int DispVMode640x768x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x768/3-bit, sys_clock=396 MHz, buffer 196600 B
int DispVMode640x768x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x768/2-bit, sys_clock=396 MHz, buffer 122880 B
int DispVMode640x768x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x768/1-bit, sys_clock=396 MHz, buffer 61440 B

int DispVMode1280x384x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1280x384/6-bit, sys_clock=396 MHz, buffer 393216 B
int DispVMode1280x384x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1280x384/4-bit, sys_clock=396 MHz, buffer 245760 B
int DispVMode1280x384x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1280x384/3-bit, sys_clock=396 MHz, buffer 196608 B
int DispVMode1280x384x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1280x384/2-bit, sys_clock=396 MHz, buffer 122880 B
int DispVMode1280x384x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1280x384/1-bit, sys_clock=396 MHz, buffer 61440 B

int DispVMode1280x768x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1280x768/3-bit, sys_clock=396 MHz, buffer 393216 B
int DispVMode1280x768x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1280x768/2-bit, sys_clock=396 MHz, buffer 245760 B
int DispVMode1280x768x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x768, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1280x768/1-bit, sys_clock=396 MHz, buffer 122880 B

#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
// This video mode requires DISPHSTX_DISP_SEL switch to be enabled.
// 1280x800/59.6Hz/49.5kHz (VESA DMT ID 1Ch), pixel clock 83.2 MHz, system clock 416 MHz ... detected as 1280x800@60Hz on VGA
int DispVMode640x400x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 2, 2, DISPHSTX_FORMAT_12); }		// 640x400/12-bit, sys_clock=416 MHz, buffer 384000 B

int DispVMode640x800x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 640x800/6-bit, sys_clock=416 MHz, buffer 409600 B
int DispVMode640x800x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 640x800/4-bit, sys_clock=416 MHz, buffer 256000 B
int DispVMode640x800x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 640x800/3-bit, sys_clock=416 MHz, buffer 204800 B
int DispVMode640x800x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 640x800/2-bit, sys_clock=416 MHz, buffer 128000 B
int DispVMode640x800x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 640x800/1-bit, sys_clock=416 MHz, buffer 64000 B

int DispVMode1280x800x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1280x800/3-bit, sys_clock=416 MHz, buffer 409600 B
int DispVMode1280x800x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1280x800/2-bit, sys_clock=416 MHz, buffer 256000 B
int DispVMode1280x800x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1280x800, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1280x800/1-bit, sys_clock=416 MHz, buffer 128000 B
#endif // USE_VREG_LOCKED
#endif // DISPHSTX_WIDTHMAX >= 1280

#if DISPHSTX_WIDTHMAX >= 1360	// max. supported width
#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
// This video mode requires DISPHSTX_DISP_SEL switch to be enabled.
// 1360x768/60.1Hz/47.8kHz (VESA DMT ID 27h), pixel clock 85.6 MHz, system clock 428 MHz ... detected as 1024x768@60Hz on VGA
int DispVMode680x384x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_12); }		// 680x384/12-bit, sys_clock=428 MHz, buffer 391680 B
int DispVMode680x384x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_8); }		// 680x384/8-bit, sys_clock=428 MHz, buffer 261120 B
int DispVMode680x384x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 680x384/6-bit, sys_clock=428 MHz, buffer 208896 B
int DispVMode680x384x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 680x384/4-bit, sys_clock=428 MHz, buffer 130560 B
int DispVMode680x384x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 680x384/3-bit, sys_clock=428 MHz, buffer 104448 B
int DispVMode680x384x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 680x384/2-bit, sys_clock=428 MHz, buffer 66048 B
int DispVMode680x384x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 680x384/1-bit, sys_clock=428 MHz, buffer 33792 B

int DispVMode680x768x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 680x768/6-bit, sys_clock=428 MHz, buffer 417792 B
int DispVMode680x768x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 680x768/4-bit, sys_clock=428 MHz, buffer 261120 B
int DispVMode680x768x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 680x768/3-bit, sys_clock=428 MHz, buffer 208896 B
int DispVMode680x768x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 680x768/2-bit, sys_clock=428 MHz, buffer 132096 B
int DispVMode680x768x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 680x768/1-bit, sys_clock=428 MHz, buffer 67584 B

int DispVMode1360x384x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1360x384/6-bit, sys_clock=428 MHz, buffer 417792 B
int DispVMode1360x384x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1360x384/4-bit, sys_clock=428 MHz, buffer 261120 B
int DispVMode1360x384x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1360x384/3-bit, sys_clock=428 MHz, buffer 208896 B
int DispVMode1360x384x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1360x384/2-bit, sys_clock=428 MHz, buffer 130560 B
int DispVMode1360x384x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1360x384/1-bit, sys_clock=428 MHz, buffer 66048 B

int DispVMode1360x768x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1360x768/3-bit, sys_clock=428 MHz, buffer 417792 B
int DispVMode1360x768x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1360x768/2-bit, sys_clock=428 MHz, buffer 261120 B
int DispVMode1360x768x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1360x768, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1360x768/1-bit, sys_clock=428 MHz, buffer 132096 B
#endif // USE_VREG_LOCKED
#endif // DISPHSTX_WIDTHMAX >= 1360

#if DISPHSTX_WIDTHMAX >= 1440	// max. supported width
// 1440x600/56Hz/35kHz, pixel clock 64 MHz, system clock 320 MHz ... detected as 800x600@56Hz on VGA
int DispVMode720x300x16(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_16); }		// 720x300/16-bit, sys_clock=320 MHz, buffer 432000 B
int DispVMode720x300x15(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_15); }		// 720x300/15-bit, sys_clock=320 MHz, buffer 432000 B
int DispVMode720x300x12(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_12); }		// 720x300/12-bit, sys_clock=320 MHz, buffer 324000 B
int DispVMode720x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_8); }		// 720x300/8-bit, sys_clock=320 MHz, buffer 216000 B
int DispVMode720x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_6_PAL); }		// 720x300/6-bit, sys_clock=320 MHz, buffer 172800 B
int DispVMode720x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_4_PAL); }		// 720x300/4-bit, sys_clock=320 MHz, buffer 108000 B
int DispVMode720x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_3_PAL); }		// 720x300/3-bit, sys_clock=320 MHz, buffer 86400 B
int DispVMode720x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_2_PAL); }		// 720x300/2-bit, sys_clock=320 MHz, buffer 54000 B
int DispVMode720x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 2, DISPHSTX_FORMAT_1_PAL); }		// 720x300/1-bit, sys_clock=320 MHz, buffer 27600 B

int DispVMode720x600x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 1, DISPHSTX_FORMAT_8); }		// 720x600/8-bit, sys_clock=320 MHz, buffer 432000 B
int DispVMode720x600x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 1, DISPHSTX_FORMAT_6_PAL); }		// 720x600/6-bit, sys_clock=320 MHz, buffer 345600 B
int DispVMode720x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 1, DISPHSTX_FORMAT_4_PAL); }		// 720x600/4-bit, sys_clock=320 MHz, buffer 216000 B
int DispVMode720x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 1, DISPHSTX_FORMAT_3_PAL); }		// 720x600/3-bit, sys_clock=320 MHz, buffer 172800 B
int DispVMode720x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 1, DISPHSTX_FORMAT_2_PAL); }		// 720x600/2-bit, sys_clock=320 MHz, buffer 108000 B
int DispVMode720x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 2, 1, DISPHSTX_FORMAT_1_PAL); }		// 720x600/1-bit, sys_clock=320 MHz, buffer 55200 B

int DispVMode1440x300x8(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 2, DISPHSTX_FORMAT_8); }		// 1440x300/8-bit, sys_clock=320 MHz, buffer 432000 B
int DispVMode1440x300x6(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 2, DISPHSTX_FORMAT_6_PAL); }		// 1440x300/6-bit, sys_clock=320 MHz, buffer 345600 B
int DispVMode1440x300x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 2, DISPHSTX_FORMAT_4_PAL); }		// 1440x300/4-bit, sys_clock=320 MHz, buffer 216000 B
int DispVMode1440x300x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 2, DISPHSTX_FORMAT_3_PAL); }		// 1440x300/3-bit, sys_clock=320 MHz, buffer 172800 B
int DispVMode1440x300x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 2, DISPHSTX_FORMAT_2_PAL); }		// 1440x300/2-bit, sys_clock=320 MHz, buffer 108000 B
int DispVMode1440x300x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 2, DISPHSTX_FORMAT_1_PAL); }		// 1440x300/1-bit, sys_clock=320 MHz, buffer 54000 B

int DispVMode1440x600x4(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 1, DISPHSTX_FORMAT_4_PAL); }		// 1440x600/4-bit, sys_clock=320 MHz, buffer 432000 B
int DispVMode1440x600x3(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 1, DISPHSTX_FORMAT_3_PAL); }		// 1440x600/3-bit, sys_clock=320 MHz, buffer 345600 B
int DispVMode1440x600x2(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 1, DISPHSTX_FORMAT_2_PAL); }		// 1440x600/2-bit, sys_clock=320 MHz, buffer 216000 B
int DispVMode1440x600x1(int dispmode, void* buf) { return DispHstxVModeStartSimple(dispmode, buf, vmodetime_1440x600, 1, 1, DISPHSTX_FORMAT_1_PAL); }		// 1440x600/1-bit, sys_clock=320 MHz, buffer 108000 B
#endif // DISPHSTX_WIDTHMAX >= 1440

#endif // USE_DISPHSTX		// 1=use HSTX Display driver
