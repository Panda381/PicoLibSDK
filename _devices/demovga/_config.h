// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#ifndef VGA_GPIO_HSYNC
#define VGA_GPIO_HSYNC	16	// VGA HSYNC GPIO (VGA requires VSYNC = HSYNC+1)
#endif

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT	1		// UART stdio port 0 or 1
#endif

#ifndef UART_STDIO_TX
#define UART_STDIO_TX	20		// UART stdio TX GPIO pin
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX	21		// UART stdio RX GPIO pin
#endif

#define KEY_A_GPIO	22		// input key A
#define KEY_B_GPIO	26		// input key B
#define KEY_C_GPIO	27		// input key C

// PWM sound
#ifndef PWMSND_GPIO
#define PWMSND_GPIO	28	// PWM output GPIO pin (0..29)
#endif

// SD card
#ifndef SD_SPI
#define SD_SPI		0	// SD card SPI interface
#endif

#ifndef SD_RX
#define SD_RX		20	// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS		21	// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK		18	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		19	// SD card TX (MOSI output), to DATA_IN pin 2
#endif

// 126 MHz required by QVGA
#ifndef PLL_KHZ
#define PLL_KHZ		126000		// PLL system frequency in kHz (default 125000 kHz)
#endif

// === Display
#ifndef WIDTH
#define WIDTH	320		// display width
#endif

#ifndef HEIGHT
#define HEIGHT	240		// display height
#endif

#ifndef FRAMESIZE
#define FRAMESIZE (WIDTH*HEIGHT) // frame size in number of colors
#endif

#ifndef USE_DRAWTFT
#define USE_DRAWTFT	1	// use TFT or VGA drawing (lib_drawtft.c, lib_drawtft.h)
#endif

#ifndef USE_VGA
#define USE_VGA		1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
#endif

#ifndef VGA_PIO_OFF
#define VGA_PIO_OFF	0	// offset of QVGA program in PIO memory
#endif

#ifndef VGA_GPIO_FIRST
#define VGA_GPIO_FIRST	0	// first VGA GPIO
#endif

#ifndef VGA_GPIO_NUM
#define VGA_GPIO_NUM	16	// number of VGA color GPIOs, without HSYNC and VSYNC, including SCK if should skip it
#endif

//#define VGA_GPIO_SKIP	5	// VGA this GPIO skips; comment out this line if you don't want the VGA pin to skip

#ifndef VGA_MODE
#define VGA_MODE	0	// used VGA output mode: 0=RGB, 1=BGR (with conversion on output)
#endif

#ifndef VGA_PIO
#define VGA_PIO		0	// VGA PIO
#endif

#ifndef VGA_SM
#define VGA_SM		0	// VGA state machine of base layer 0
#endif

#ifndef VGA_DMA
#define VGA_DMA		8	// QVGA DMA base channel (QVGA requires 2 DMA channels)
#endif

#ifndef USE_DRAW_STDIO
#define USE_DRAW_STDIO	1	// use DRAW stdio (DrawPrint function)
#endif

#ifndef SYSTICK_KEYSCAN
#define SYSTICK_KEYSCAN	1	// call KeyScan() function from SysTick system timer
#endif

#ifndef USE_PWMSND
#define USE_PWMSND	4	// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#endif

// === SD card
#ifndef USE_SD
#define USE_SD		1	// use SD card (lib_sd.c, lib_sd.h)
#endif

#ifndef USE_FAT
#define USE_FAT		1	// use FAT file system (lib_fat.c, lib_fat.h)
#endif

// SD card speed
#ifndef SD_BAUDLOW
#define SD_BAUDLOW	250000 // SD card low baud speed (to initialize connection)
#endif

#ifndef SD_BAUD
#define SD_BAUD		12000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier
#define BAT_DIODE_FV_INT 	311		// voltage drop in mV on diode
#define BAT_DIODE_FV 		0.311f		// voltage drop in mV on diode
