// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// ======== PicoinoMini
#if USE_PICOINOMINI

#ifndef QVGA_GPIO_HSYNC
#define QVGA_GPIO_HSYNC	16	// VGA HSYNC GPIO (QVGA requires VSYNC = HSYNC+1)
#endif

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT	0		// UART stdio port 0 or 1
#endif

#ifndef UART_STDIO_TX
#define UART_STDIO_TX	0		// UART stdio TX GPIO pin
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX	1		// UART stdio RX GPIO pin
#endif

#define KEY_OUT_GPIO	22	// output to keyboard
// left pad - OUT is HIGH, IN is pull-down
#define KEY_UP_GPIO	20	// input UP from keyboard
#define KEY_LEFT_GPIO	19	// input LEFT from keyboard
#define KEY_RIGHT_GPIO	21	// input RIGHT from keyboard
#define KEY_DOWN_GPIO	18	// input DOWN from keyboard
// right pad - OUT is LOW, IN is pull-up
#define KEY_X_GPIO	20	// input X from keyboard
#define KEY_Y_GPIO	21	// input Y from keyboard
#define KEY_A_GPIO	18	// input A from keyboard
#define KEY_B_GPIO	19	// input B from keyboard

// PWM sound
#ifndef PWMSND_GPIO
#define PWMSND_GPIO	15	// PWM output GPIO pin (0..29)
#endif

// SD card
#ifndef SD_SPI
#define SD_SPI		1	// SD card SPI interface
#endif

#ifndef SD_RX
#define SD_RX		12	// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS		13	// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK		10	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		11	// SD card TX (MOSI output), to DATA_IN pin 2
#endif

// ======== Picoino 1.0
#else // USE_PICOINOMINI

#ifndef QVGA_GPIO_HSYNC
#define QVGA_GPIO_HSYNC	10	// VGA HSYNC GPIO (QVGA requires VSYNC = HSYNC+1)
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

// PWM sound
#ifndef PWMSND_GPIO
#define PWMSND_GPIO	0	// PWM output GPIO pin (0..29)
#endif

// SD card
#ifndef SD_SPI
#define SD_SPI		1	// SD card SPI interface
#endif

#ifndef SD_RX
#define SD_RX		12	// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS		13	// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK		14	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		15	// SD card TX (MOSI output), to DATA_IN pin 2
#endif


// === keyboard
//	GP16 ... keyboard serial data input/output
//	GP17 ... keyboard clock 1, L->H shift SIPO output register to COL1..COL8,
//			LOW load PISO input register, inverted LED3 (SHIFT)
//	GP18 ... keyboard clock 2, L->H shift PISO input register from ROW1..ROW7..DIP1..DIP2, inverted LED4
#define KEY_DATA_GPIO	16	// serial data input/output
#define KEY_CLK1_GPIO	17	// keyboard clock 1, output COL1..COL8
#define KEY_CLK2_GPIO	18	// keyboard clock 2, input ROW1..ROW7..DIP1..DIP2

#endif // USE_PICOINOMINI

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

#ifndef USE_QDRAW
#define USE_QDRAW	1	// use QDRAW drawing
#endif

#ifndef USE_QVGA
#define USE_QVGA	2	// use QVGA display 320x240/8, 1=use 1 frame buffer 76.8 KB, 2=use 2 frame buffers 153.6 KB (qvga.c, qvga.h)
#endif

#ifndef QVGA_PIO_OFF
#define QVGA_PIO_OFF	0	// offset of QVGA program in PIO memory
#endif

#ifndef QVGA_GPIO_FIRST
#define QVGA_GPIO_FIRST	2	// first VGA GPIO
#endif

#ifndef QVGA_GPIO_NUM
#define QVGA_GPIO_NUM	8	// number of VGA color GPIOs, without HSYNC and VSYNC
#endif

#ifndef QVGA_PIO
#define QVGA_PIO	0	// VGA PIO
#endif

#ifndef QVGA_SM
#define QVGA_SM		0	// VGA state machine of base layer 0
#endif

#ifndef QVGA_DMA
#define QVGA_DMA	8	// QVGA DMA base channel (QVGA requires 2 DMA channels)
#endif

#ifndef USE_DRAW_STDIO
#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)
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
#define SD_BAUDLOW	250000 //250000	// SD card low baud speed (to initialize connection)
#endif

#ifndef SD_BAUD
#define SD_BAUD		1500000 //2000000 //7000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier
#define BAT_DIODE_FV_INT 	311		// voltage drop in mV on diode
#define BAT_DIODE_FV 		0.311f		// voltage drop in mV on diode
