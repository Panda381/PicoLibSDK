
// ****************************************************************************
//                                 
//                        Project library configuration
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

#ifndef _CONFIG_H
#define _CONFIG_H

// *********
// At this place you can specify the switches and settings you want
// to change from the default configuration in config_def.h.
// *********

#define DISPHSTX_USE_DVI	0	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#define DISPHSTX_USE_VGA	1	// 1=use VGA support (VGA requires about 30 KB of RAM)
#define WIDTH			800
#define HEIGHT			600
#define VMODEFNC		DispVMode800x600x3

#define USE_DISPHSTX		1	// 1=use HSTX Display driver
#define DISPHSTX_DVI_PINOUT	0	// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+
#define DISPHSTX_DISP_SEL	-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#define USE_DISPHSTX_VMODE	0	// DispHstx videomode (0=custom)
#define USE_VREG_LOCKED		1	// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
#define DISPHSTX_WIDTHMAX	WIDTH	// max. supported width (specifies also size of the scanline render buffer)

#if DISPHSTX_USE_DVI && DISPHSTX_USE_VGA
#error DVI & VGA !
#endif

#define USE_DRAW		0
#define USE_DRAWCAN		1

#define USE_DRAWCAN0		0
#define USE_DRAWCAN1		0
#define USE_DRAWCAN2		0
#define USE_DRAWCAN3		1
#define USE_DRAWCAN4		0
#define USE_DRAWCAN6		0
#define USE_DRAWCAN8		0
#define USE_DRAWCAN12		0
#define USE_DRAWCAN16		0

// DVI breakout board
#define DISPHSTX_DVI_D0P	12	// GPIO pin to output D0+ (use 12..19)
#define DISPHSTX_DVI_D0M	13	// GPIO pin to output D0- (use 12..19)
#define DISPHSTX_DVI_CLKP	14	// GPIO pin to output CLK+ (use 12..19)
#define DISPHSTX_DVI_CLKM	15	// GPIO pin to output CLK- (use 12..19)
#define DISPHSTX_DVI_D2P	16	// GPIO pin to output D2+ (use 12..19)
#define DISPHSTX_DVI_D2M	17	// GPIO pin to output D2- (use 12..19)
#define DISPHSTX_DVI_D1P	18	// GPIO pin to output D1+ (use 12..19)
#define DISPHSTX_DVI_D1M	19	// GPIO pin to output D1- (use 12..19)

// VGA output
#define DISPHSTX_VGA_B0		12	// GPIO pin to output B0 (use 12..19)
#define DISPHSTX_VGA_B1		13	// GPIO pin to output B1 (use 12..19)
#define DISPHSTX_VGA_G0		14	// GPIO pin to output G0 (use 12..19)
#define DISPHSTX_VGA_G1		15	// GPIO pin to output G1 (use 12..19)
#define DISPHSTX_VGA_R0		16	// GPIO pin to output R0 (use 12..19)
#define DISPHSTX_VGA_R1		17	// GPIO pin to output R1 (use 12..19)
#define DISPHSTX_VGA_HSYNC	18	// GPIO pin to output HSYNC (use 12..19)
#define DISPHSTX_VGA_VSYNC	19	// GPIO pin to output VSYNC (use 12..19)

#define DISPHSTX_STRIP_MAX	1	// max. number of videomode strips
#define DISPHSTX_SLOT_MAX	1	// max. number of videomode slots
#define DISPHSTX_USE_FORMAT_1			0	// 1=use format DISPHSTX_FORMAT_1, 1 bit per pixel, black & white format 0..1 (8 pixels per 1 byte; width must be aligned to 8 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_2			0	// 1=use format DISPHSTX_FORMAT_2, 2 bits per pixel, grayscale format 0..3 (4 pixels per 1 byte; width must be aligned to 4 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_3			1	// 1=use format DISPHSTX_FORMAT_3, 3 bits per pixel, format RGB111 (10 pixels per 32-bit word; width must be aligned to 10 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_4			0	// 1=use format DISPHSTX_FORMAT_4, 4 bits per pixel, format gray 16 levels (2 pixels per 1 byte; width must be aligned to 2 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_6			0	// 1=use format DISPHSTX_FORMAT_6, 6 bits per pixel, format RGB222 (5 pixels per 32-bit word; width must be aligned to 5 pixels; hdbl > 1 not supported)
#define DISPHSTX_USE_FORMAT_8			0	// 1=use format DISPHSTX_FORMAT_8, 8 bits per pixel, format RGB332 (1 pixel per 1 byte)
#define DISPHSTX_USE_FORMAT_12			0	// 1=use format DISPHSTX_FORMAT_12, 12 bits per pixel, format RGB444 (8 pixels per three 32-bit words)
#define DISPHSTX_USE_FORMAT_15			0	// 1=use format DISPHSTX_FORMAT_15, 15 bits per pixel, format RGB555 (1 pixel per 2 bytes; bit 15 is ignored)
#define DISPHSTX_USE_FORMAT_16			0	// 1=use format DISPHSTX_FORMAT_16, 16 bits per pixel, format RGB565 (1 pixel per 2 bytes)
#define DISPHSTX_USE_FORMAT_1_PAL		0	// 1=use format DISPHSTX_FORMAT_1_PAL, 1 bit per pixel, paletted colors (8 pixels per 1 byte)
#define DISPHSTX_USE_FORMAT_2_PAL		0	// 1=use format DISPHSTX_FORMAT_2_PAL, 2 bits per pixel, paletted colors (4 pixels per 1 byte)
#define DISPHSTX_USE_FORMAT_3_PAL		3	// 1=use format DISPHSTX_FORMAT_3_PAL, 3 bits per pixel, paletted colors (10 pixels per 32-bit word)
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

#define FONT			FontBold8x8	// default system font
#define FONTW			8		// width of system font
#define FONTH			8		// height of system font

//#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)
//#define USE_USB_STDIO		1		// use USB stdio (UsbPrint function)
//#define USE_UART_STDIO	1		// use UART stdio (UartPrint function)

#include CONFIG_DEF_H				// default configuration

#endif // _CONFIG_H
