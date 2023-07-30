// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// === Display
#ifndef WIDTH
#define WIDTH	320			// display width
#endif

#ifndef HEIGHT
#define HEIGHT	240			// display height
#endif

#ifndef FRAMESIZE
#define FRAMESIZE (WIDTH*HEIGHT) 	// frame size in number of colors
#endif

#ifndef USE_ST7789
#define USE_ST7789	1		// use ST7789 TFT display (st7789.c, st7789.h)
#endif

#ifndef USE_DRAWTFT
#define USE_DRAWTFT	1		// use TFT drawing (lib_drawtft.c, lib_drawtft.h)
#endif

#ifndef DISP_SPI
#define DISP_SPI	0		// SPI used for display
#endif

#ifndef DISP_SPI_BAUD
#define DISP_SPI_BAUD	24000000	// SPI baudrate (max. CLK_PERI/2 = 24 MHz, absolute max. 62.5 MHz)
#endif

#ifndef DISP_BLK_PIN
#define DISP_BLK_PIN	16		// backlight pin
#endif

#ifndef DISP_DC_PIN
#define DISP_DC_PIN	17		// data/command pin
#endif

#ifndef DISP_SCK_PIN
#define DISP_SCK_PIN	18		// serial clock pin
#endif

#ifndef DIDP_MOSI_PIN
#define DIDP_MOSI_PIN	19		// master out TX MOSI pin
#endif

#ifndef DISP_RES_PIN
#define DISP_RES_PIN	20		// reset pin
#endif

#ifndef DISP_CS_PIN
#define DISP_CS_PIN	21		// chip selection pin
#endif

// PWM sound
#ifndef PWMSND_GPIO
#define PWMSND_GPIO	15		// PWM output GPIO pin (0..29)
#endif

#ifndef USE_PWMSND
#define USE_PWMSND	4		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#endif

// === SD card
#ifndef USE_SD
#define USE_SD		1		// use SD card (lib_sd.c, lib_sd.h)
#endif

#ifndef USE_FAT
#define USE_FAT		1		// use FAT file system (lib_fat.c, lib_fat.h)
#endif

#ifndef SD_SPI
#define SD_SPI		1		// SD card SPI interface
#endif

#ifndef SYSTICK_KEYSCAN
#define SYSTICK_KEYSCAN	1	// call KeyScan() function from SysTick system timer
#endif

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier
#define BAT_DIODE_FV_INT 	311		// voltage drop in mV on diode
#define BAT_DIODE_FV 		0.311f		// voltage drop in mV on diode

// ======== PicoPad beta version 0.8
#if USE_PICOPAD08

// SD card
#ifndef SD_RX
#define SD_RX		28		// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS		0		// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK		26	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		27	// SD card TX (MOSI output), to DATA_IN pin 2
#endif

// buttons GPIO
#define BTN_UP_PIN	12	// up
#define BTN_LEFT_PIN	13	// left
#define BTN_RIGHT_PIN	10	// right
#define BTN_DOWN_PIN	11	// down
#define BTN_X_PIN	9	// X
#define BTN_Y_PIN	8	// Y
#define BTN_A_PIN	7	// A
#define BTN_B_PIN	6	// B

// LEDs
#define LED1_PIN	5	// LED1 inverted, blue, on right
#define LED2_PIN 	25	// default internal LED pin, on Pico board

// SD card speed
#ifndef SD_BAUDLOW
#define SD_BAUDLOW	250000	// SD card low baud speed (to initialize connection)
#endif

#ifndef SD_BAUD
#define SD_BAUD		2000000 //7000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif


#endif

// ======== PicoPad version 1.0
#if USE_PICOPAD10

// SD card
#ifndef SD_RX
#define SD_RX		12		// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS		13		// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK		10	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		11	// SD card TX (MOSI output), to DATA_IN pin 2
#endif

// buttons GPIO
#define BTN_UP_PIN	4	// up
#define BTN_LEFT_PIN	3	// left
#define BTN_RIGHT_PIN	2	// right
#define BTN_DOWN_PIN	5	// down
#define BTN_X_PIN	9	// X
#define BTN_Y_PIN	8	// Y
#define BTN_A_PIN	7	// A
#define BTN_B_PIN	6	// B

// LEDs
#define LED1_PIN	22	// LED1 inverted, blue, on right
#define LED2_PIN 	25	// default internal LED pin, on Pico board

// SD card speed
#ifndef SD_BAUDLOW
#define SD_BAUDLOW	250000	// SD card low baud speed (to initialize connection)
#endif

#ifndef SD_BAUD
#define SD_BAUD		2000000 //7000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

#endif
