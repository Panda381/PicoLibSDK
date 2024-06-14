// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#ifndef USE_FRAMEBUF
#if USE_VIDEO
#define USE_FRAMEBUF	0
#else
#define USE_FRAMEBUF	1		// use default display frame buffer
#endif
#endif

// ======== Picotron

// PWM sound
#ifndef PWMSND_GPIO
#define PWMSND_GPIO	8	// PWM output GPIO pin (0..29)
#endif

#ifndef USE_PWMSND
#define USE_PWMSND	4	// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#endif

// SD card
#ifndef SD_SPI
#define SD_SPI		0	// SD card SPI interface
#endif

#ifndef SD_RX
#define SD_RX		16	// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS		17	// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK		18	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		19	// SD card TX (MOSI output), to DATA_IN pin 2
#endif

#ifndef USE_SD
#define USE_SD		1	// use SD card (lib_sd.c, lib_sd.h)
#endif

#ifndef USE_FAT
#define USE_FAT		1	// use FAT file system (lib_fat.c, lib_fat.h)
#endif

#ifndef SD_BAUDLOW
#define SD_BAUDLOW	250000 // SD card low baud speed (to initialize connection)
#endif

#ifndef SD_BAUD
#define SD_BAUD		4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
#endif

#ifndef SD_BAUDWRITE
#define SD_BAUDWRITE	1000000 // SD card baud speed of write (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier

// keyboard
#define KEY_ROW1_GPIO	12
#define KEY_ROW2_GPIO	27
#define KEY_ROW3_GPIO	9
#define KEY_ROW4_GPIO	28
#define KEY_ROW5_GPIO	10

#define KEY_COL1_GPIO	11
#define KEY_COL2_GPIO	13
#define KEY_COL3_GPIO	14
#define KEY_COL4_GPIO	15
#define KEY_COL5_GPIO	21
#define KEY_COL6_GPIO	20
#define KEY_COL7_GPIO	22
#define KEY_COL8_GPIO	26

#ifndef SYSTICK_KEYSCAN
#define SYSTICK_KEYSCAN	1	// call KeyScan() function from SysTick system timer
#endif

// display
#ifndef VGA_GPIO_HSYNC
#define VGA_GPIO_HSYNC	4	// VGA HSYNC GPIO (VGA requires VSYNC = HSYNC+1)
#endif

//#define VGA_USECSYNC	1	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)

#ifndef USE_DRAW
#define USE_DRAW	1	// use DRAW drawing
#endif

#ifndef USE_DRAW_STDIO
#if !USE_USB_STDIO
#define USE_DRAW_STDIO	1	// use DRAW stdio (DrawPrint function)
#endif
#endif

#ifndef USE_MINIVGA
#if USE_FRAMEBUF
#define USE_MINIVGA		2		// use mini-VGA display with simple frame buffer:
#else
#define USE_MINIVGA		1
#endif // USE_FRAMEBUF
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer
#endif // USE_MINIVGA

#ifndef WIDTH
#define WIDTH	320		// display width
#endif

#ifndef COLBITS
#define COLBITS 4		// number of output color bits (4, 8, 15 or 16)
#endif

#ifndef VGA_GPIO_FIRST
#define VGA_GPIO_FIRST	0	// GPIO of first color pin
#endif

#ifndef VGA_GPIO_NUM
#define VGA_GPIO_NUM	4	// number of VGA color GPIOs, without HSYNC and VSYNC
#endif

#ifndef VGA_PIO
#define VGA_PIO		1	// VGA PIO
#endif

#ifndef VGA_SM
#define VGA_SM		0	// VGA state machine of base layer 0
#endif

#ifndef VGA_DMA
#define VGA_DMA		8	// VGA first DMA channel (use 2 channels)
#endif

// mini-VGA auto config
#include "../../_display/minivga/_config.h"
