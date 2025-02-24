
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                             HSTX DVI driver
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
HDMI timings:

HSTX_clock = pixel_clock * 5 = 10 half-clock pulses ... one TMDS token is 10 bits
sys_clock = HSTX_clock, clkdiv = 1 ..... OR .... sys_clock = HSTX_clock * 2, clkdiv = 2 .... OR .... pll_clock = HSTX_clock

In all formats:
		- shifter_shift 2
		- shifter_repeat 5

formats 1..4 bits direct (hardware service, fast)
	single-pixel, width is multiply of u32 (32..8 pixels):
		- DMA transfer by u32 (= 32..8 pixels)
		- DMA number = width/(32..8)
		- swap bytes
		- expander_shift left by 1..4
		- expander_repeat 32..8
	single-pixel, width is not multiply of u32, but must be multiply of u8 (8..2 pixels):
		- DMA transfer by u8 (= 8..2 pixels)
		- DMA number = width/(8..2)
		- swap bytes
		- expander_shift left by 1..4
		- expander_repeat 8..2
	double-pixel: not supported

formats 8 bits direct (hardware service, fast)
	single-pixel, width is multiply of u32 (4 pixels):
		- DMA transfer by u32 (= 4 pixels)
		- DMA number = width/4
		- swap bytes
		- expander_shift left by 8
		- expander_repeat 4
	single-pixel, width is not multiply of u32:
		- DMA transfer by u8 (= 1 pixel)
		- DMA number = width
		- (swap bytes ... does not matter)
		- expander_shift left by 8
		- expander_repeat 1
	double-pixel:
		- DMA transfer by u8 (duplicated by the bus)
		- DMA number = width
		- (swap bytes ... does not matter)
		- expander_shift left by 8 (or can be 0)
		- expander_repeat 2 or more

formats 16 bits direct (hardware service, fast)
	single-pixel, width is multiply of u32 (2 pixels):
		- DMA transfer by u32 (= 2 pixels)
		- DMA number = width/2
		- expander_shift right (can be left) by 16
		- expander_repeat 2
	single-pixel, width is not multiply of u32:
		- DMA transfer by u16 (= 1 pixel)
		- DMA number = width
		- expander_shift right by 16 (or can be 0)
		- expander_repeat 1
	double-pixel:
		- DMA transfer by u16 (duplicated by the bus)
		- DMA number = width
		- expander_shift right by 16 (or can be 0)
		- expander_repeat 2 or more

formats 1..8 bits with 16-bit palette colors (software service, slower)
	single-pixel, width is multiply of u32 (2 pixels):
		- DMA transfer by u32 (= 2 pixels)
		- DMA number = width/2
		- expander_shift right (can be left) by 16
		- expander_repeat 2
	single-pixel, width is not multiply of u32:
		- DMA transfer by u16 (= 1 pixel)
		- DMA number = width
		- expander_shift right by 16 (or can be 0)
		- expander_repeat 1
	double-pixel:
		- DMA transfer by u16 (duplicated by the bus)
		- DMA number = width
		- expander_shift right by 16
		- expander_repeat 2 or more
*/

#if USE_DISPHSTX		// 1=use HSTX Display driver

#ifndef _DISPHSTX_DVI_H
#define _DISPHSTX_DVI_H

#ifdef __cplusplus
extern "C" {
#endif

#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

// flag - HSTX DVI mode is active
extern Bool DispHstxDviRunning;

// find nearest higher TMDS code DC balanced
u16 DispHstxTmdsDC(int val);

// convert color in RGB565 format to TMDS 30-bit word - DC balanced
u32 DispHstxRgbToTmdsDC(u16 col);

// convert color in RGB565 format to TMDS 30-bit word - even (lower value)
u32 DispHstxRgbToTmds1(u16 col);

// convert color in RGB565 format to TMDS 30-bit word - odd (higher value)
u32 DispHstxRgbToTmds2(u16 col);

// prepare videomode state descriptor before start HSTX DVI mode
void DispHstxDviPrepare(sDispHstxVModeState* v);

// initialize and start HSTX DVI mode (requires initialized DispHstxVMode descriptor)
// - DVI mode must be deactivated, system clock must be set
void DispHstxDviInit();

// exchange videomode state descriptor, without terminating HSTX DVI mode
// - HSTX DVI mode must be started using DispHstxDviInit()
// - videomode state descriptor must be prepared using DispHstxDviPrepare()
void DispHstxDviExchange(sDispHstxVModeState* v);

// terminate HSTX DVI mode
void DispHstxDviTerm();

// execute core 1 remote function
void DispHstxDviCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool DispHstxDviCore1Busy();

// wait if core 1 is busy (executing remote function)
void DispHstxDviCore1Wait();

// start DVI on core 1 from core 0 (must be paired with DispHstxDviStop())
void DispHstxDviStart();

// terminate DVI on core 1 from core 0 (must be paired with DispHstxDviStart())
void DispHstxDviStop();

#endif // DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_DVI_H

#endif // USE_DISPHSTX		// 1=use HSTX Display driver
