// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// To use UART console, connect UART-USB converter to pins GPIO0 (TX output from Pico)
// and GPIO1 (RX input to Pico) and set COM port to 115200 Baud, 8 bits, no parity,
// 1 stop bit, no flow control.

#ifndef USE_FRAMEBUF
#if USE_VIDEO
#define USE_FRAMEBUF	0
#else
#define USE_FRAMEBUF	1		// use default display frame buffer
#endif
#endif

// ============================================================================
//                       Extern VGA/DVI display module
// ============================================================================

#if USE_EXTDISP

// === Display
#ifndef USE_DRAW
#define USE_DRAW	1	// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#endif

#ifndef USE_DRAW_STDIO
#define USE_DRAW_STDIO	1	// use DRAW stdio (DrawPrint function)
#endif

#ifndef VGA_GPIO_HSYNC
#define VGA_GPIO_HSYNC	26	// VGA HSYNC GPIO (VGA requires VSYNC = HSYNC+1)
#endif

//#define VGA_USECSYNC	1	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)

#ifndef WIDTH
#define WIDTH	320		// display width
#endif

#ifndef VGA_GPIO_FIRST
#define VGA_GPIO_FIRST	3	// first VGA GPIO
#endif

#ifndef VGA_GPIO_NUM
#define VGA_GPIO_NUM	16	// number of VGA color GPIOs, without HSYNC and VSYNC, including SCK if should skip it
#endif

//#define VGA_GPIO_SKIP	5	// VGA this GPIO skips; comment out this line if you don't want the VGA pin to skip

#ifndef VGA_PIO
#define VGA_PIO		1	// VGA PIO
#endif

#ifndef VGA_SM
#define VGA_SM		0	// VGA state machine
#endif

#ifndef VGA_DMA
#define VGA_DMA		8	// VGA first DMA channel (uses 2 channels)
#endif

#ifndef USE_MINIVGA
#define USE_MINIVGA		1		// use mini-VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer
#endif // USE_MINIVGA

#ifndef USE_DVI
#define USE_DVI			1		// use DVI (HDMI) display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer
#endif // USE_DVI

#ifndef DVI_GPIO_D2
#define DVI_GPIO_D2	0	// GPIO pin with DVI_D2+ (DVI requires DVI_D2- = DVI_D2+ + 1)
#endif

#ifndef DVI_GPIO_D1
#define DVI_GPIO_D1	2	// GPIO pin with DVI_D1+ (DVI requires DVI_D1- = DVI_D1+ + 1)
#endif

#ifndef DVI_GPIO_D0
#define DVI_GPIO_D0	8	// GPIO pin with DVI_D0+ (DVI requires DVI_D0- = DVI_D0+ + 1)
#endif

#ifndef DVI_GPIO_CLK
#define DVI_GPIO_CLK	14	// GPIO pin with DVI_CLK+ (DVI requires DVI_D2- = DVI_D2+ + 1) ... DVI_GPIO_CLK must be even! (uses PWM)
#endif

#ifndef DVI_PIO
#define DVI_PIO		1	// DVI PIO
#endif

#ifndef DVI_SM0
#define DVI_SM0		1	// DVI first state machine (uses 3 state machines)
#endif

#ifndef DVI_DMA
#define DVI_DMA		2	// DVI first DMA channel (uses 6 DMA channels)
#endif

#ifndef COLBITS
#define COLBITS 16		// number of output color bits (4, 8, 15 or 16)
#endif

// mini-VGA auto config
#include "../../_display/minivga/_config.h"

// DVI auto config
#include "../../_display/dvi/_config.h"

// ============================================================================
//                             Base Pico
// ============================================================================

#else // USE_EXTDISP

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT	0		// UART stdio port 0 or 1
#endif

#define PICO_DEFAULT_UART UART_STDIO_PORT // original-SDK setup

#ifndef UART_STDIO_TX
#define UART_STDIO_TX	0		// UART stdio TX GPIO pin
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX	1		// UART stdio RX GPIO pin
#endif

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier

#endif // USE_EXTDISP
