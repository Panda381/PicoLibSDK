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

#include "picovga8_vmode.h"
#include "picovga8_layer.h"
#include "picovga8_screen.h"
#include "picovga8_util.h"
#include "picovga8_pal.h"
#include "picovga8.h"
