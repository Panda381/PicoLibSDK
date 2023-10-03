
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

#define USE_SCREENSHOT	0		// use screen shots

#define USE_DOUBLE	0		// use Double-floating point (sdk_double.c, sdk_double_asm.S, sdk_double.h)
#define USE_FIFO	0		// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)
//#define USE_FLOAT	1		// use Single-floating point (sdk_float.c, sdk_float_asm.S, sdk_float.h)
#define USE_I2C		0		// use I2C interface (sdk_i2c.c, sdk_i2c.h)
#define USE_INTERP	0		// use interpolator (sdk_interp.c, sdk_interp.h)
#define USE_MULTICORE	0		// use Multicore (sdk_multicore.c, sdk_multicore.h)
//#define USE_PWM		1		// use PWM (sdk_pwm.c, sdk_pwm.h)
#define USE_RTC		0		// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)
#define USE_UART	0		// use UART serial port (sdk_uart.c, sdk_uart.h)
//#define USE_WATCHDOG	1		// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)

//#define USE_CALENDAR	1		// use 32-bit calendar (lib_calendar.c, lib_calendar.h)
#define USE_CALENDAR64	0		// use 64-bit calendar (lib_calendar64.c, lib_calendar64.h)
#define USE_CANVAS	0		// use Canvas (lib_canvas.c, lib_canvas.h)
#define USE_COLOR	0		// use color vector (lib_color.c, lib_color.h)
#define USE_ESCPKT	0		// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)
//#define USE_MAT2D	1		// use 2D transformation matrix (lib_mat2d.c, lib_mat2d.h)
//#define USE_MALLOC	1		// use Memory Allocator (lib_malloc.c, lib_malloc.h)
//#define USE_PRINT	1		// use Formatted print (lib_print.c, lib_print.h)
//#define USE_PWMSND	1		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
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
