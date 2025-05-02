
// ****************************************************************************
//
//                              PicoPad Init
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

#if USE_PICOPAD

#include "../../_lib/inc/lib_pwmsnd.h"

#if USE_PICOPADHSTX
#include "../../_display/disphstxmini/disphstxmini.h"
#include "../../_display/disphstx/disphstx.h"
#else // USE_PICOPADHSTX
#include "../../_display/minivga/minivga.h"
#include "../../_display/st7789/st7789.h"
#endif // USE_PICOPADHSTX
#include "../../_lib/inc/lib_fat.h"
#include "../../_lib/inc/lib_sd.h"
#include "picopad_bat.h"
#include "picopad_key.h"
#include "picopad_led.h"
#include "picopad_init.h"	// initialize
#include "picopad_ss.h"		// screen shot

#if USE_DISPHSTX && USE_DISPHSTX_VMODE	// 1=use HSTX Display driver

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
FRAMETYPE ALIGNED DispBuf[FRAMESIZE];	// display buffer
#endif

#if USE_FRAMEBUF && USE_DISPHSTX_VMODE	// use videomode 0=custom, 1=320x240/16 BackBuf, ...
FRAMETYPE ALIGNED FrameBuf[FRAMESIZE];	// frame buffer
#endif // USE_FRAMEBUF

// last system time of auto update
u32 DispAutoUpdateLast;

#endif // USE_DISPHSTX && USE_DISPHSTX_VMODE

// DEBUG indicator - copy it into program checkpoint
// { GPIO_Init(LED1_PIN); GPIO_DirOut(LED1_PIN); while (1) { GPIO_Flip(LED1_PIN); io32 i; for (i = 1000000; i > 0; i--); } }

// Device init
void DeviceInit()
{
#if USE_PICOPADHSTX
	// initialize LEDs
	LedInit();
#else // USE_PICOPADHSTX
#if USE_MINIVGA					// use mini-VGA display with simple frame buffer
	// start VGA on CPU 1 (must be paired with VgaStop)
	VgaStart();
#else // USE_MINIVGA
	// initialize LEDs
	LedInit();

	// initialize display
	DispInit(DISP_ROT);
#endif // USE_MINIVGA
#endif // !USE_PICOPADHSTX

	// initilize keys
	KeyInit();

	// init battery measurement
	BatInit();

#if USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
	// initialize PWM sound output (must be re-initialized after changing CLK_SYS system clock)
	PWMSndInit();
#endif

#if USE_SD		// use SD card (lib_sd.c, lib_sd.h)
	// initialize SD card interface (must be re-initialized after changing system clock)
	SDInit();
#endif

#if USE_PICOPADHSTX

#if USE_DISPHSTX		// 1=use HSTX Display driver
#if USE_DISPHSTX_VMODE	// use videomode 0=custom, 1=320x240/16 BackBuf, 2=320x240/16, 3=640x480/8 BackBuf, 4=640x480/8

	// initialize videomode
#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
	int res = DEFAULT_VMODE_FNC(0, DispBuf);
#else // USE_DISPHSTX_DISPBUF
	int res = DEFAULT_VMODE_FNC(0, FrameBuf);
#endif // USE_DISPHSTX_DISPBUF
	while (res != DISPHSTX_ERR_OK) { GPIO_Flip(LED_PIN); WaitMs(100); }

#if USE_DRAWCAN && USE_DRAWCAN0

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
	pDrawCan->buf = (u8*)FrameBuf;
	pDrawCan->frontbuf = (u8*)DispBuf;
#endif // USE_DISPHSTX_DISPBUF

	DEFAULT_PDRAWCAN = pDrawCan;
#endif // USE_DRAWCAN && USE_DRAWCAN0

#endif // USE_DISPHSTX_VMODE
#endif // USE_DISPHSTX

#if USE_DISPHSTXMINI		// 1=use HSTX Display Mini driver
	DispHstxInit(0);
#endif // USE_DISPHSTXMINI

#endif // USE_PICOPADHSTX
}

// Device terminate
void DeviceTerm()
{
#if USE_PICOPADHSTX

	// terminate DispHSTX driver
	DispHstxAllTerm();

	// terminate LEDs
	LedTerm();
#else // USE_PICOPADHSTX

#if USE_MINIVGA					// use mini-VGA display with simple frame buffer
	// terminate VGA on CPU 1 (must be paired with VgaStart)
	VgaStop();
#else // USE_MINIVGA
	// terminate LEDs
	LedTerm();

	// terminate display
	DispTerm();
#endif // USE_MINIVGA
#endif // USE_PICOPADHSTX

	// terminate keys
	KeyTerm();

	// terminate battery measurement
	BatTerm();

#if USE_PWMSND	// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
	// terminate PWM sound output
	PWMSndTerm();
#endif

#if USE_SD		// use SD card (lib_sd.c, lib_sd.h)
	// terminate SD card interface
	SDTerm();
#endif
}

#if USE_PICOPADHSTX
#if USE_DISPHSTX		// 1=use HSTX Display driver
#if USE_DRAW		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
// load back buffer from frame buffer
void DispLoad()
{
	DMA_MemCopy(FrameBuf, DispBuf, sizeof(DispBuf));
}

// update - send dirty window to display
void DispUpdate()
{
	DMA_MemCopy(DispBuf, FrameBuf, sizeof(DispBuf));
}

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms)
{
	// interval in [us]
	u32 us = ms*1000;

	// check interval from last update
	if ((u32)(Time() - DispAutoUpdateLast) >= us)
	{
		// update display
		DispUpdate();

		// start measure new time interval of running program
		DispAutoUpdateLast = Time();
	}
}

#endif // USE_DISPHSTX_DISPBUF
#endif // USE_DRAW
#endif // USE_DISPHSTX
#endif // USE_PICOPADHSTX

#if USE_DISPHSTX && USE_DISPHSTX_VMODE	// 1=use HSTX Display driver

// LCD simulation: 320x240 pixels, 16-bit colors
u16* DispHstxDst = FrameBuf;

// LOW level control: start sending image data to display window (DispSendImg() must follow)
//   Only window on full display is supported (parameters are ignored)
void DispStartImg(u16 x1, u16 x2, u16 y1, u16 y2)
{
#if USE_EMUSCREENSHOT		// use emulator screen shots
	if (DoEmuScreenShot)	// request to do emulator screenshot
	{
		DoEmuScreenShot = False;

		// open screenshot
		OpenScreenShot();
	}
#endif

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
	DispHstxDst = DispBuf;
#else
	DispHstxDst = FrameBuf;
#endif
}

// LOW level control: send one word of image data to display (follows after DispStartImg())
void DispSendImg2(u16 data)
{
#if USE_EMUSCREENSHOT		// use emulator screen shots
	// write data to screenshot file
	WriteScreenShot(&data, 2);
#endif

	*DispHstxDst++ = data;
}

// LOW level control: stop sending image data (follows after DispStartImg() and DispSendImg())
void DispStopImg()
{
#if USE_EMUSCREENSHOT		// use emulator screen shots
	// close screenshot
	CloseScreenShot();
#endif
}

#endif // USE_DISPHSTX && USE_DISPHSTX_VMODE	// 1=use HSTX Display driver

#endif // USE_PICOPAD
