
// ****************************************************************************
//
//    HDMI and VGA mini display driver for Pico2 RP2350 over HSTX interface
//                            Default configuration
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

// For most parameters, set 1 to enable the property or 0 to disable the property.
// If you change the configuration, do a full rebuild of the project
//   to make the changes take effect (run the d.* script first).

#ifndef _DISPHSTXMINI_CONFIG_H
#define _DISPHSTXMINI_CONFIG_H

#if USE_DISPHSTXMINI		// 1=use HSTX Display Mini driver

// See Picopad's _config.h (section DISPHSTXMINI) for more configuratin switches:

//#define USE_DISPHSTXMINI_VMODE 1	// DispHstxMini videomode (default 1; data must be DISPHSTX_VGA_MASK masked in VGA case with only FrameBuf)
					//	1=320x240/16bit, FrameBuf+DispBuf, sys_clock=126 MHz
					//	2=320x240/16bit, FrameBuf+DispBuf, sys_clock=252 MHz
					//	3=320x240/16bit (borders 2x46 pixels), FrameBuf+DispBuf, sys_clock variable, clocked from USB 144 MHz
					//	4=320x240/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	5=320x144/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	6=360x240/16bit, only FrameBuf, sys_clock variable, clocked from USB 144 MHz
//#define WIDTH		320		// display width
//#define HEIGHT	240		// display height
//#define COLBITS	16		// number of output color bits
//#define COLTYPE	u16		// type of color
//#define FRAMETYPE	u16		// type of frame entry
//#define WIDTHLEN	WIDTH		// length of one line in pixels
//#define FRAMESIZE 	(WIDTHLEN*HEIGHT) // frame size in number of pixels
//#define DISP_STRIP_NUM 1		// number of back strips
//#define USE_DISPHSTX_DISPBUF	1	// 1 = use DispBuf + FrameBuf
//#define USE_DRAW	1		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
//#define DISPHSTX_DISP_SEL	1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
//#define DISPHSTX_DVI_PINOUT	0	// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+

#define DISPHSTX_VGA_VSYNCBIT	0	// VSYNC bit in VGA sample (instead of blue lowest bit)
#define DISPHSTX_VGA_HSYNCBIT	5	// HSYNC bit in VGA sample (instead of green lowest bit)
#define DISPHSTX_VGA_VSYNCVAL	BIT(DISPHSTX_VGA_VSYNCBIT) // VSYNC bit value in VGA sample (instead of blue lowest bit)
#define DISPHSTX_VGA_HSYNCVAL	BIT(DISPHSTX_VGA_HSYNCBIT) // HSYNC bit value in VGA sample (instead of green lowest bit)
#define DISPHSTX_VGA_MASK (~(DISPHSTX_VGA_HSYNCVAL|DISPHSTX_VGA_VSYNCVAL)) // mask to clear SYNC bits in VGA sample

#ifndef DISPHSTX_USE_DVI
#define DISPHSTX_USE_DVI		1	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#endif

#ifndef DISPHSTX_USE_VGA
#define DISPHSTX_USE_VGA		1	// 1=use VGA support (VGA requires about 30 KB of RAM)
#endif

#ifndef DISPHSTX_DVI_PINOUT
#define DISPHSTX_DVI_PINOUT		0	// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+
#endif

#ifndef DISPHSTX_DISP_SEL
#define DISPHSTX_DISP_SEL		-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#endif

// DMA channels
#ifndef DISPHSTX_DMA_DATA
#define DISPHSTX_DMA_DATA		15	// DMA data channel (to copy data to HSTX interface)
#endif

#ifndef DISPHSTX_DMA_CMD
#define DISPHSTX_DMA_CMD		14	// DMA command channel (to control DMA data channel)
#endif

// ===== DVI GPIO pins

#if DISPHSTX_DVI_PINOUT == 1	// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+

// order D2+..CLK-
#ifndef DISPHSTX_DVI_D2P
#define DISPHSTX_DVI_D2P		12	// GPIO pin to output D2+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D2M
#define DISPHSTX_DVI_D2M		13	// GPIO pin to output D2- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1P
#define DISPHSTX_DVI_D1P		14	// GPIO pin to output D1+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1M
#define DISPHSTX_DVI_D1M		15	// GPIO pin to output D1- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0P
#define DISPHSTX_DVI_D0P		16	// GPIO pin to output D0+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0M
#define DISPHSTX_DVI_D0M		17	// GPIO pin to output D0- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKP
#define DISPHSTX_DVI_CLKP		18	// GPIO pin to output CLK+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKM
#define DISPHSTX_DVI_CLKM		19	// GPIO pin to output CLK- (use 12..19)
#endif

#elif DISPHSTX_DVI_PINOUT == 2

// order CLK-..D2+
#ifndef DISPHSTX_DVI_D2P
#define DISPHSTX_DVI_D2P		19	// GPIO pin to output D2+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D2M
#define DISPHSTX_DVI_D2M		18	// GPIO pin to output D2- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1P
#define DISPHSTX_DVI_D1P		17	// GPIO pin to output D1+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1M
#define DISPHSTX_DVI_D1M		16	// GPIO pin to output D1- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0P
#define DISPHSTX_DVI_D0P		15	// GPIO pin to output D0+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0M
#define DISPHSTX_DVI_D0M		14	// GPIO pin to output D0- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKP
#define DISPHSTX_DVI_CLKP		13	// GPIO pin to output CLK+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKM
#define DISPHSTX_DVI_CLKM		12	// GPIO pin to output CLK- (use 12..19)
#endif

#else // DISPHSTX_DVI_PINOUT

// DVI breakout board
#ifndef DISPHSTX_DVI_D2P
#define DISPHSTX_DVI_D2P		16	// GPIO pin to output D2+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D2M
#define DISPHSTX_DVI_D2M		17	// GPIO pin to output D2- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1P
#define DISPHSTX_DVI_D1P		18	// GPIO pin to output D1+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1M
#define DISPHSTX_DVI_D1M		19	// GPIO pin to output D1- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0P
#define DISPHSTX_DVI_D0P		12	// GPIO pin to output D0+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0M
#define DISPHSTX_DVI_D0M		13	// GPIO pin to output D0- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKP
#define DISPHSTX_DVI_CLKP		14	// GPIO pin to output CLK+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKM
#define DISPHSTX_DVI_CLKM		15	// GPIO pin to output CLK- (use 12..19)
#endif

#endif // DISPHSTX_DVI_PINOUT

// ===== VGA GPIO pins

#ifndef DISPHSTX_VGA_B0
#define DISPHSTX_VGA_B0			12	// GPIO pin to output B0 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_B1
#define DISPHSTX_VGA_B1			13	// GPIO pin to output B1 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_G0
#define DISPHSTX_VGA_G0			14	// GPIO pin to output G0 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_G1
#define DISPHSTX_VGA_G1			15	// GPIO pin to output G1 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_R0
#define DISPHSTX_VGA_R0			16	// GPIO pin to output R0 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_R1
#define DISPHSTX_VGA_R1			17	// GPIO pin to output R1 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_HSYNC
#define DISPHSTX_VGA_HSYNC		18	// GPIO pin to output HSYNC (use 12..19)
#endif

#ifndef DISPHSTX_VGA_VSYNC
#define DISPHSTX_VGA_VSYNC		19	// GPIO pin to output VSYNC (use 12..19)
#endif

#endif // USE_DISPHSTXMINI

#endif // _DISPHSTXMINI_CONFIG_H
