// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

/*

To include PicoVGA8 into your project, add following lines to project files.

To config.h add:
#define USE_PICOVGA8		1		// use PicoVGA 8-bit display
#define USE_MINIVGA		0		// use mini-VGA display with simple frame buffer
#define USE_DRAW		0		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define USE_DRAW_STDIO		0		// use DRAW stdio (DrawPrint function)
#include "../../../_display/picovga8/_config.h"
#include "../../../config_def.h"	// default configuration

To include.h add:
#include "../../../includes.h"	// all includes
#include "../../../_display/picovga8/_include.h"

To Makefile add:
include ../../../_display/picovga8/_makefile.inc
include ../../../Makefile.inc

*/

#ifndef COLBITS
#define COLBITS 	8	// number of output color bits (4, 8, 15 or 16)
#endif

#ifndef LAYERS
#define LAYERS		4	// total layers 1..4 (1 base layer + 3 overlapped layers)
#endif

#ifndef SEGMAX
#define SEGMAX		8	// max. number of video segment per video strip
#endif

#ifndef STRIPMAX
#define STRIPMAX	8	// max. number of video strips
#endif

#ifndef MAXX
#define MAXX		640	// max. resolution in X direction (must be power of 4)
#endif

#ifndef MAXY
#define MAXY		480	// max. resolution in Y direction
#endif

#ifndef MAXLINE
#define MAXLINE		700	// max. number of scanlines (including sync and dark lines)
#endif

#ifndef VGA_GPIO_FIRST
#define VGA_GPIO_FIRST	2	// GPIO of first color pin
#endif

#ifndef VGA_GPIO_NUM
#define VGA_GPIO_NUM	8	// number of VGA color GPIOs, without HSYNC and VSYNC
#endif

#ifndef VGA_GPIO_HSYNC
#define VGA_GPIO_HSYNC	10	// VGA HSYNC GPIO
#endif

#ifndef VGA_GPIO_VSYNC
#define VGA_GPIO_VSYNC	11	// VGA VSYNC GPIO
#endif

#ifndef VGA_PIO
#define VGA_PIO		1	// VGA PIO
#endif

#ifndef VGA_SM0
#define VGA_SM0		0	// VGA state machine of base layer 0
#endif

#ifndef VGA_DMA
#define VGA_DMA		2	// VGA first DMA channel (can use up to 8 channels)
#endif

#define COLOR(r,g,b)	((u8)( ((r)&0xe0) | (((g)&0xe0)>>3) | (((b)&0xc0)>>6) ))
