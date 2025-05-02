
// ****************************************************************************
//                                 
//                        Project library configuration
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

#ifndef _CONFIG_H
#define _CONFIG_H

// *********
// At this place you can specify the switches and settings you want
// to change from the default configuration in config_def.h.
// *********

#define WIDTH		320		// screen width
#define HEIGHT		240		// screen height

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define USE_DISPHSTXMINI	0	// 1=use HSTX Display Mini driver
#define USE_DISPHSTX		1	// 1=use HSTX Display driver
//#define DISPHSTX_DISP_SEL	-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#define DISPHSTX_USE_DVI	1	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#define DISPHSTX_USE_VGA	1	// 1=use VGA support (VGA requires about 30 KB of RAM)

#define USE_DISPHSTXMINI_VMODE	1	// DispHstxMini videomode (default 1; data must be DISPHSTX_VGA_MASK masked in VGA case if only FrameBuf is used)
					//	1=320x240/16bit, FrameBuf+DispBuf, sys_clock=126 MHz
					//	2=320x240/16bit, FrameBuf+DispBuf, sys_clock=252 MHz
					//	3=320x240/16bit (borders 2x46 pixels), FrameBuf+DispBuf, sys_clock variable, clocked from USB 144 MHz
					//	4=320x240/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	5=320x144/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	6=360x240/16bit, only FrameBuf, sys_clock variable, clocked from USB 144 MHz

#define USE_DISPHSTX_VMODE	3	// DispHSTX videomode (default 1)
					//	0=custom
					//	1=320x240/16 FrameBuf+DispBuf, slow (sys_clock=126 MHz, detected as 640x480@60)
					//	2=320x240/16 FrameBuf+DispBuf, fast (sys_clock=252 MHz, detected as 640x480@60)
					//	3=320x240/16 only FrameBuf, slow (sys_clock=126 MHz, detected as 640x480@60)
					//	4=320x240/16 only FrameBuf, fast (sys_clock=252 MHz, detected as 640x480@60)
					//	5=400x300/16 (sys_clock=200 MHz, detected as 800x600@60)
					//	6=512x384/16 (sys_clock=324 MHz, detected as 1024x768@60Hz, sys_clock may not work on some Pico2s)
					//	7=532x400/16 (sys_clock=210 MHz, detected as 720x400@70, can be unreliable on some monitors)
					//	8=640x350/16 (sys_clock=252 MHz, detected as 640x350@70)
					//	9=640x480/8 slow (sys_clock=126 MHz, detected as 640x480@60)
					//	10=640x480/8 fast (sys_clock=252 MHz, detected as 640x480@60)
					//	11=800x600/6 (sys_clock=200 MHz, detected as 800x600@60)
					//	12=1024x768/4 (sys_clock=324 MHz, detected as 1024x768@60Hz, sys_clock may not work on some Pico2s)
#endif // USE_PICOPADHSTX

#define USE_FATALERROR	0		// use fatal error message 0=no, 1=display LCD message (sdk_fatal.c, sdk_fatal.h)

#define USE_ORIGSDK	0		// include interface of original-SDK
#define USE_DISP_DMA	0		// use DMA output do LCD display
#define USE_FRAMEBUF	0		// use default frame buffer
#define USE_SCREENSHOT	0		// use screen shots

#if USE_PICOPADHSTX && USE_DISPHSTX
#define DISPHSTX_WIDTHMAX	640	// max. supported width (specifies also size of the scanline render buffer)
#define DISPHSTX_STRIP_MAX	1	// max. number of videomode strips
#define DISPHSTX_SLOT_MAX	1	// max. number of videomode slots
#define DISPHSTX_USE_FORMAT_1			0	// 1=use format DISPHSTX_FORMAT_1, 1 bit per pixel, black & white format 0..1 (8 pixels per 1 byte; width must be aligned to 8 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_2			0	// 1=use format DISPHSTX_FORMAT_2, 2 bits per pixel, grayscale format 0..3 (4 pixels per 1 byte; width must be aligned to 4 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_3			0	// 1=use format DISPHSTX_FORMAT_3, 3 bits per pixel, format RGB111 (10 pixels per 32-bit word; width must be aligned to 10 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_4			0	// 1=use format DISPHSTX_FORMAT_4, 4 bits per pixel, format gray 16 levels (2 pixels per 1 byte; width must be aligned to 2 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_6			0	// 1=use format DISPHSTX_FORMAT_6, 6 bits per pixel, format RGB222 (5 pixels per 32-bit word; width must be aligned to 5 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_8			0	// 1=use format DISPHSTX_FORMAT_8, 8 bits per pixel, format RGB332 (1 pixel per 1 byte)
#define DISPHSTX_USE_FORMAT_12			0	// 1=use format DISPHSTX_FORMAT_12, 12 bits per pixel, format RGB444 (8 pixels per three 32-bit words)
#define DISPHSTX_USE_FORMAT_15			0	// 1=use format DISPHSTX_FORMAT_15, 15 bits per pixel, format RGB555 (1 pixel per 2 bytes; bit 15 is ignored)
#define DISPHSTX_USE_FORMAT_16			1	// 1=use format DISPHSTX_FORMAT_16, 16 bits per pixel, format RGB565 (1 pixel per 2 bytes)
#define DISPHSTX_USE_FORMAT_1_PAL		0	// 1=use format DISPHSTX_FORMAT_1_PAL, 1 bit per pixel, paletted colors (8 pixels per 1 byte)
#define DISPHSTX_USE_FORMAT_2_PAL		0	// 1=use format DISPHSTX_FORMAT_2_PAL, 2 bits per pixel, paletted colors (4 pixels per 1 byte)
#define DISPHSTX_USE_FORMAT_3_PAL		0	// 1=use format DISPHSTX_FORMAT_3_PAL, 3 bits per pixel, paletted colors (10 pixels per 32-bit word)
#define DISPHSTX_USE_FORMAT_4_PAL		0	// 1=use format DISPHSTX_FORMAT_4_PAL, 4 bits per pixel, paletted colors (2 pixels per 1 byte)
#define DISPHSTX_USE_FORMAT_6_PAL		0	// 1=use format DISPHSTX_FORMAT_6_PAL, 6 bits per pixel, paletted colors (5 pixels per 32-bit word)
#define DISPHSTX_USE_FORMAT_8_PAL		0	// 1=use format DISPHSTX_FORMAT_8_PAL, 8 bits per pixel, paletted colors (1 pixel per 1 byte)
#define DISPHSTX_USE_FORMAT_COL			0	// 1=use format DISPHSTX_FORMAT_COL, simple color
#define DISPHSTX_USE_FORMAT_MTEXT		0	// 1=use format DISPHSTX_FORMAT_MTEXT, monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG)
#define DISPHSTX_USE_FORMAT_ATEXT		0	// 1=use format DISPHSTX_FORMAT_ATEXT, attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color
#define DISPHSTX_USE_FORMAT_TILE4_8		0	// 1=use format DISPHSTX_FORMAT_TILE4_8, tiles 4x4 pixels, 1-byte index, 8-bit format RGB332
#define DISPHSTX_USE_FORMAT_TILE8_8		0	// 1=use format DISPHSTX_FORMAT_TILE8_8, tiles 8x8 pixels, 1-byte index, 8-bit format RGB332
#define DISPHSTX_USE_FORMAT_TILE16_8		0	// 1=use format DISPHSTX_FORMAT_TILE16_8, tiles 16x16 pixels, 1-byte index, 8-bit format RGB332
#define DISPHSTX_USE_FORMAT_TILE32_8		0	// 1=use format DISPHSTX_FORMAT_TILE32_8, tiles 32x32 pixels, 1-byte index, 8-bit format RGB332
#define DISPHSTX_USE_FORMAT_TILE4_8_PAL		0	// 1=use format DISPHSTX_FORMAT_TILE4_8_PAL, tiles 4x4 pixels, 1-byte index, 8-bit paletted colors
#define DISPHSTX_USE_FORMAT_TILE8_8_PAL		0	// 1=use format DISPHSTX_FORMAT_TILE8_8_PAL, tiles 8x8 pixels, 1-byte index, 8-bit paletted colors
#define DISPHSTX_USE_FORMAT_TILE16_8_PAL	0	// 1=use format DISPHSTX_FORMAT_TILE16_8_PAL, tiles 16x16 pixels, 1-byte index, 8-bit paletted colors
#define DISPHSTX_USE_FORMAT_TILE32_8_PAL	0	// 1=use format DISPHSTX_FORMAT_TILE32_8_PAL, tiles 32x32 pixels, 1-byte index, 8-bit paletted colors
#define DISPHSTX_USE_FORMAT_HSTX_15		0	// 1=use format DISPHSTX_FORMAT_HSTX_15, HSTX RLE compression, 15 bits per pixel, format RGB555
#define DISPHSTX_USE_FORMAT_HSTX_16		0	// 1=use format DISPHSTX_FORMAT_HSTX_16, HSTX RLE compression, 16 bits per pixel, format RGB565
#define DISPHSTX_USE_FORMAT_PAT_8		0	// 1=use format DISPHSTX_FORMAT_PAT_8, repeated pattern, 8-bit format RGB332, width derived from pitch, height from fonth
#define DISPHSTX_USE_FORMAT_PAT_8_PAL		0	// 1=use format DISPHSTX_FORMAT_PAT_8_PAL, repeated pattern, 8-bit format, paletted colors, width derived from pitch, height from fonth
#define DISPHSTX_USE_FORMAT_RLE8		0	// 1=use format DISPHSTX_FORMAT_RLE8, RLE compression, 8-bit format RGB332 (font=line offsets)
#define DISPHSTX_USE_FORMAT_RLE8_PAL		0	// 1=use format DISPHSTX_FORMAT_RLE8_PAL, RLE compression, 8-bit format, paletted colors (font=line offsets)
#define DISPHSTX_USE_FORMAT_ATTR1_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR1_PAL, attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes
#define DISPHSTX_USE_FORMAT_ATTR2_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR2_PAL, attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes
#define DISPHSTX_USE_FORMAT_ATTR3_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR3_PAL, attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes
#define DISPHSTX_USE_FORMAT_ATTR4_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR4_PAL, attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes
#define DISPHSTX_USE_FORMAT_ATTR5_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR5_PAL, attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes
#define DISPHSTX_USE_FORMAT_ATTR6_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR6_PAL, attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes
#define DISPHSTX_USE_FORMAT_ATTR7_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR7_PAL, attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes
#define DISPHSTX_USE_FORMAT_ATTR8_PAL		0	// 1=use format DISPHSTX_FORMAT_ATTR8_PAL, attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes
#endif // USE_PICOPADHSTX

// SDK modules
#define USE_ORIGSDK	0		// include interface of original-SDK
#define USE_DOUBLE	0		// use Double-floating point 1=in RAM, 2=in Flash (sdk_double.c, sdk_double_asm.S, sdk_double.h)
#define USE_FATALERROR	0		// use fatal error message 0=no, 1=display LCD message (sdk_fatal.c, sdk_fatal.h)
#define USE_FIFO	1		// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)
#define USE_FLOAT	2		// use Single-floating point 1=in RAM, 2=in Flash (sdk_float.c, sdk_float_asm.S, sdk_float.h)
#define USE_I2C		0		// use I2C interface (sdk_i2c.c, sdk_i2c.h)
#define USE_INTERP	0		// use interpolator (sdk_interp.c, sdk_interp.h)
#define USE_MULTICORE	1		// use Multicore (sdk_multicore.c, sdk_multicore.h)
//#define USE_PWM		1		// use PWM (sdk_pwm.c, sdk_pwm.h)
#define USE_RTC		0		// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)
#define USE_STACKCHECK	0		// use Stack check (sdk_cpu.c, sdk_cpu.h)
#define USE_UART	0		// use UART serial port (sdk_uart.c, sdk_uart.h)
#define USE_USB		0		// use USB (sdk_usb_*.c, sdk_usb_*.h) (only if not using TinyUSB library)
//#define USE_WATCHDOG	1		// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)

// LIB
//#define USE_CALENDAR	1		// use 32-bit calendar (lib_calendar.c, lib_calendar.h)
#define USE_CALENDAR64	0		// use 64-bit calendar (lib_calendar64.c, lib_calendar64.h)
#define USE_CANVAS	0		// use Canvas (lib_canvas.c, lib_canvas.h)
#define USE_COLOR	0		// use color vector (lib_color.c, lib_color.h)
#define USE_ESCPKT	0		// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)
//#define USE_MAT2D	1		// use 2D transformation matrix (lib_mat2d.c, lib_mat2d.h)
//#define USE_MALLOC	1		// use Memory Allocator (lib_malloc.c, lib_malloc.h)
//#define USE_PRINT	1		// use Formatted print (lib_print.c, lib_print.h)
#define USE_PWMSND	0		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#define USE_RAND	0		// use Random number generator (lib_rand.c, lib_rand.h)
#define USE_RECT	0		// use Rectangle (lib_rect.c, lib_rect.h)
#define USE_RING	0		// use Ring buffer (lib_ring.c, lib_ring.h)
#define USE_RINGRX	0		// use Ring buffer with DMA receiver (lib_ringrx.c, lib_ringrx.h)
#define USE_RINGTX	0		// use Ring buffer with DMA transmitter (lib_ringtx.c, lib_ringtx.h)
#define USE_TEXTLIST	0		// use List of text strings (lib_textlist.c, lib_textlist.h)
#define USE_TPRINT	0		// use Text Print (lib_tprint.c, lib_tprint.h)
#define USE_TREE	0		// use Tree list (lib_tree.c, lib_tree.h)

#define FONT		FontBold8x16	// default system font
#define FONTW		8		// width of system font
#define FONTH		16		// height of system font

#include "../../../config_def.h"	// default configuration

#endif // _CONFIG_H
