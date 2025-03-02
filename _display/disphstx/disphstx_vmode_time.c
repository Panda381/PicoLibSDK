
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                            Video modes - Time list
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

// list of predefined videomode timings
// https://www.improwis.com/tables/video.webt
const sDispHstxVModeTime DispHstxVModeTimeList[vmodetime_num] = {
	// vmodetime_532x400: 532x400/70.2Hz/31.5kHz (EGA 4:3), pixel clock 20.9 MHz, system clock 104.571 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		532,		// u16	hactive;// horizontal active pixels (must be even number)
		12,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		80,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		40,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		664,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		104571,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_532x400_fast: 532x400/70.2Hz/31.5kHz (EGA 4:3), pixel clock 20.9 MHz, system clock 209.143 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		2,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		532,		// u16	hactive;// horizontal active pixels (must be even number)
		12,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		80,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		40,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		664,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		209143,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

#if DISPHSTX_WIDTHMAX >= 640		// max. supported width
	// vmodetime_640x350: 640x350/70.2Hz/31.5kHz (EGA), pixel clock 25.2 MHz, system clock 126 MHz ... detected as 640x350@70Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		16,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		96,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		48,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		800,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		350,		// u16	vactive;// vertical active scanlines
		37,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		60,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		126000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_640x350_fast: 640x350/70.2Hz/31.5kHz (EGA), pixel clock 25.2 MHz, system clock 252 MHz ... detected as 640x350@70Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		2,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		16,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		96,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		48,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		800,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		350,		// u16	vactive;// vertical active scanlines
		37,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		60,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		252000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_640x400: 640x400/70.2Hz/31.5kHz, pixel clock 25.2 MHz, system clock 126 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		16,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		96,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		48,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		800,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		126000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_640x400_fast: 640x400/70.2Hz/31.5kHz, pixel clock 25.2 MHz, system clock 252 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		2,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		16,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		96,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		48,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		800,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		252000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_640x480: 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz, system clock 126 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		16,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		96,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		48,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		800,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		126000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_640x480_fast: 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz, system clock 252 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		2,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		16,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		96,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		48,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		800,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		252000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_640x720: 640x720/60Hz/45kHz (half-HD), pixel clock 37.2 MHz, system clock 186 MHz ... detected as 1280x720@60Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		640,		// u16	hactive;// horizontal active pixels (must be even number)
		56,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		20,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		110,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		826,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		720,		// u16	vactive;// vertical active scanlines
		5,		// u16	vfront;	// vertical front porch in scanlines
		5,		// u16	vsync;	// VSYNC height in scanlines
		20,		// u16	vback;	// vertical back porch in scanlines
		750,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		186000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 720		// max. supported width
	// vmodetime_720x400: 720x400/70.1Hz/31.5kHz, pixel clock 28.32 MHz, system clock 141.6 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		720,		// u16	hactive;// horizontal active pixels (must be even number)
		18,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		108,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		54,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		900,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		141600,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_720x400_fast: 720x400/70.1Hz/31.5kHz, pixel clock 28.32 MHz, system clock 283.2 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		2,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		720,		// u16	hactive;// horizontal active pixels (must be even number)
		18,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		108,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		54,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		900,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		283200,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_720x480: 720x480/60Hz/31.5kHz, pixel clock 28.32 MHz, system clock 141.6 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		720,		// u16	hactive;// horizontal active pixels (must be even number)
		18,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		108,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		54,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		900,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		141600,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_720x480_fast: 720x480/60Hz/31.5kHz, pixel clock 28.32 MHz, system clock 283.2 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		2,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		720,		// u16	hactive;// horizontal active pixels (must be even number)
		18,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		108,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		54,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		900,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		283200,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 800		// max. supported width
	// vmodetime_800x480: 800x480/60Hz/31.5kHz, pixel clock 31.5 MHz, system clock 157.333 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		800,		// u16	hactive;// horizontal active pixels (must be even number)
		20,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		120,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		60, 		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1000,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		157333,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_800x600: 800x600/60.3Hz/37.9kHz (SVGA 4:3, VESA DMT ID 09h), pixel clock 40 MHz, system clock 200 MHz ... detected as 800x600@60Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		800,		// u16	hactive;// horizontal active pixels (must be even number)
		40,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		128,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		88,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1056,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		600,		// u16	vactive;// vertical active scanlines
		1,		// u16	vfront;	// vertical front porch in scanlines
		4,		// u16	vsync;	// VSYNC height in scanlines
		23,		// u16	vback;	// vertical back porch in scanlines
		628,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		200000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 848		// max. supported width
	// vmodetime_848x480: 848x480/60Hz/31.4kHz (DMT SVGA 16:9), pixel clock 33.33 MHz, system clock 166.666 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		848,		// u16	hactive;// horizontal active pixels (must be even number)
		22,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		124,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		64,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1058,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		166666,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 960		// max. supported width
	// vmodetime_960x720: 960x720/60Hz/45kHz (VESA 4:3), pixel clock 55.7 MHz, system clock 278.4 MHz ... detected as 1280x720@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		960,		// u16	hactive;// horizontal active pixels (must be even number)
		82,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		30,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		166,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1238, 		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		720,		// u16	vactive;// vertical active scanlines
		5, 		// u16	vfront;	// vertical front porch in scanlines
		5,		// u16	vsync;	// VSYNC height in scanlines
		20, 		// u16	vback;	// vertical back porch in scanlines
		750,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		278400,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 1024	// max. supported width
	// vmodetime_1024x768: 1024x768/59.9Hz/48.3kHz (XGA 4:3, VESA DMT ID 10h), pixel clock 64.8 MHz, system clock 324 MHz ... detected as 1024x768@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1024,		// u16	hactive;// horizontal active pixels (must be even number)
		24,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		136,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		158, // 160	// u16	hback;	// horizontal back porch in pixels (must be even number)
		1342, // 1344	// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		768,		// u16	vactive;// vertical active scanlines
		3,		// u16	vfront;	// vertical front porch in scanlines
		6,		// u16	vsync;	// VSYNC height in scanlines
		29,		// u16	vback;	// vertical back porch in scanlines
		806,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		324000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 1056		// max. supported width
	// vmodetime_1056x600: 1056x600/60Hz/37.9kHz (near 16:9), pixel clock 52.8 MHz, system clock 264 MHz ... detected as 800x600@60Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1056,		// u16	hactive;// horizontal active pixels (must be even number)
		52,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		170,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		116,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1394,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		600,		// u16	vactive;// vertical active scanlines
		1,		// u16	vfront;	// vertical front porch in scanlines
		4,		// u16	vsync;	// VSYNC height in scanlines
		23,		// u16	vback;	// vertical back porch in scanlines
		628,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		264000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 1152		// max. supported width
	// vmodetime_1152x600: 1152x600/60Hz/37.9kHz, pixel clock 57.6 MHz, system clock 288 MHz ... detected as 800x600@60Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1152,		// u16	hactive;// horizontal active pixels (must be even number)
		58,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		184,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		126,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1520,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		600,		// u16	vactive;// vertical active scanlines
		1,		// u16	vfront;	// vertical front porch in scanlines
		4,		// u16	vsync;	// VSYNC height in scanlines
		23,		// u16	vback;	// vertical back porch in scanlines
		628,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		288000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

#if DISPHSTX_WIDTHMAX >= 1280	// max. supported width
	// vmodetime_1280x400: 1280x400/70Hz/31.5kHz, pixel clock 50.4 MHz, system clock 252 MHz ... detected as 720x400@70Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1280,		// u16	hactive;// horizontal active pixels (must be even number)
		32,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		192,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		96,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1600,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		400,		// u16	vactive;// vertical active scanlines
		12,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		35,		// u16	vback;	// vertical back porch in scanlines
		449,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		252000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_1280x480: 1280x480/60Hz/31.5kHz (8:3), pixel clock 50.4 MHz, system clock 252 MHz ... detected as 640x480@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1280,		// u16	hactive;// horizontal active pixels (must be even number)
		32,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		192,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		96,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1600,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		480,		// u16	vactive;// vertical active scanlines
		10,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		33,		// u16	vback;	// vertical back porch in scanlines
		525,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		252000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_1280x600: 1280x600/56Hz/35.2kHz, pixel clock 57.6 MHz, system clock 288 MHz ... detected as 800x600@56Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1280,		// u16	hactive;// horizontal active pixels (must be even number)
		38,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		116,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		204,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1638,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		600,		// u16	vactive;// vertical active scanlines
		1,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		22,		// u16	vback;	// vertical back porch in scanlines
		625,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		288000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_1280x720: 1280x720/60.1Hz/45kHz (16:9, VESA DMT ID 55h), pixel clock 74.4 MHz, system clock 372 MHz ... detected as 1280x720@60Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1280,		// u16	hactive;// horizontal active pixels (must be even number)
		110,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		40,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		220,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1650,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		720,		// u16	vactive;// vertical active scanlines
		5,		// u16	vfront;	// vertical front porch in scanlines
		5,		// u16	vsync;	// VSYNC height in scanlines
		20,		// u16	vback;	// vertical back porch in scanlines
		750,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		372000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

	// vmodetime_1280x768: 1280x768/59.6Hz/47.6kHz (VESA DMT ID 17h), pixel clock 79.2 MHz, system clock 396 MHz ... detected as 1280x768@60Hz on VGA
	{
		False,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1280,		// u16	hactive;// horizontal active pixels (must be even number)
		64,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		128,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		192,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1664,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		768,		// u16	vactive;// vertical active scanlines
		3,		// u16	vfront;	// vertical front porch in scanlines
		7,		// u16	vsync;	// VSYNC height in scanlines
		20,		// u16	vback;	// vertical back porch in scanlines
		798,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		396000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},

#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
	// vmodetime_1280x800: 1280x800/59.6Hz/49.5kHz (VESA DMT ID 1Ch), pixel clock 83.2 MHz, system clock 416 MHz ... detected as 1280x800@60Hz on VGA
	// This video mode requires DISPHSTX_DISP_SEL switch to be enabled.
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1280,		// u16	hactive;// horizontal active pixels (must be even number)
		72,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		128,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		200,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1680,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		800,		// u16	vactive;// vertical active scanlines
		3,		// u16	vfront;	// vertical front porch in scanlines
		6,		// u16	vsync;	// VSYNC height in scanlines
		22,		// u16	vback;	// vertical back porch in scanlines
		831,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		416000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif // USE_VREG_LOCKED

#endif

#if DISPHSTX_WIDTHMAX >= 1360	// max. supported width
#if USE_VREG_LOCKED		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
	// vmodetime_1360x768: 1360x768/60.1Hz/47.8kHz (VESA DMT ID 27h), pixel clock 85.6 MHz, system clock 428 MHz ... detected as 1024x768@60Hz on VGA
	// This video mode requires DISPHSTX_DISP_SEL switch to be enabled.
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		False,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1360,		// u16	hactive;// horizontal active pixels (must be even number)
		64,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		112,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		256,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1792,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		768,		// u16	vactive;// vertical active scanlines
		3,		// u16	vfront;	// vertical front porch in scanlines
		6,		// u16	vsync;	// VSYNC height in scanlines
		18,		// u16	vback;	// vertical back porch in scanlines
		795,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		428000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif // USE_VREG_LOCKED
#endif

#if DISPHSTX_WIDTHMAX >= 1440		// max. supported width
	// vmodetime_1440x600: 1440x600/56Hz/35kHz, pixel clock 64 MHz, system clock 320 MHz ... detected as 800x600@56Hz on VGA
	{
		True,		// u8	hpol;	// HSYNC polarity 0=negative, 1=positive
		True,		// u8	vpol;	// VSYNC polarity 0=negative, 1=positive
		1,		// u8	clkdiv;	// system clock divider (1 or 2)
		0,		// u8	res;	// ... reserved (align)

		// horizontal timings
		1440,		// u16	hactive;// horizontal active pixels (must be even number)
		40,		// u16	hfront;	// horizontal front porch in pixels (must be even number)
		124,		// u16	hsync;	// HSYNC width in pixels (must be even number)
		224,		// u16	hback;	// horizontal back porch in pixels (must be even number)
		1828,		// u16	htotal;	// horizontal total width in pixels

		// vertical timings
		600,		// u16	vactive;// vertical active scanlines
		1,		// u16	vfront;	// vertical front porch in scanlines
		2,		// u16	vsync;	// VSYNC height in scanlines
		22,		// u16	vback;	// vertical back porch in scanlines
		625,		// u16	vtotal;	// vertical total height in scanlines

		// clock (shifter: 5 clock cycles per pixel, expander: 1 loop per shifter, divider: 1 or 2)
		320000,		// u32	sysclk;	// system clock in kHz (= 5x or 10x video pixel clock)
	},
#endif

};

#endif // USE_DISPHSTX
