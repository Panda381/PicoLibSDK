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

// ============================================================================
//                            PicoPadHSTX
// ============================================================================

#if USE_PICOPADHSTX

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT		0		// UART stdio port 0 or 1
#endif

#define PICO_DEFAULT_UART UART_STDIO_PORT	// original-SDK setup

#ifndef UART_STDIO_TX
#define UART_STDIO_TX		26		// UART stdio TX GPIO pin (function mode UART or AUX is auto-selected)
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX		27		// UART stdio RX GPIO pin (function mode UART or AUX is auto-selected)
#endif

// LEDs
#define LED1_PIN 		25		// default internal LED pin, on Pico board

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier

// === Sound

#ifndef USE_PWMSND
#define USE_PWMSND		4		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#endif

#ifndef PWMSND_GPIO
#define PWMSND_GPIO		20		// PWM sound output GPIO pin (left or single channel)
#endif

#ifndef PWMSND_GPIO_R
#define PWMSND_GPIO_R		21		// PWM sound output GPIO pin (right channel; -1 = not used, only single channel is used)
#endif

// === keyboard

#ifndef SYSTICK_KEYSCAN
#define SYSTICK_KEYSCAN		1		// call KeyScan() function from SysTick system timer
#endif

// buttons GPIO
#define BTN_UP_PIN		0		// up
#define BTN_LEFT_PIN		22		// left
#define BTN_RIGHT_PIN		2		// right
#define BTN_DOWN_PIN		3		// down
#define BTN_X_PIN		8		// X
#define BTN_Y_PIN		9		// Y
#define BTN_A_PIN		10		// A
#define BTN_B_PIN		11		// B

// === SD card

#ifndef SD_SPI
#define SD_SPI			0		// SD card SPI interface
#endif

#ifndef SD_RX
#define SD_RX			4		// SD card RX (MISO input), to DATA_OUT pin 7
#endif

#ifndef SD_CS
#define SD_CS			5		// SD card CS, to CS pin 1
#endif

#ifndef SD_SCK
#define SD_SCK			6		// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX			7		// SD card TX (MOSI output), to DATA_IN pin 2
#endif

#ifndef USE_SD
#define USE_SD			1		// use SD card (lib_sd.c, lib_sd.h)
#endif

#ifndef USE_FAT
#define USE_FAT			1		// use FAT file system (lib_fat.c, lib_fat.h)
#endif

// SD card speed
#ifndef SD_BAUDLOW
#define SD_BAUDLOW		250000		// SD card low baud speed (to initialize connection)
#endif

#ifndef SD_BAUD
#define SD_BAUD			4000000		// SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
						//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

#ifndef SD_BAUDWRITE
#define SD_BAUDWRITE		1000000		// SD card baud speed of write (should be max. 7-12 Mbit/s; default standard bus speed
						//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

// === Display

#if !defined(USE_DISPHSTX) && !defined(USE_DISPHSTXMINI)
#define USE_DISPHSTXMINI	0		// 1=use HSTX Display Mini driver
#define USE_DISPHSTX		1		// 1=use HSTX Display driver
#endif

#if !USE_DISPHSTXMINI && !defined(USE_DISPHSTX)
#define USE_DISPHSTX		1		// 1=use HSTX Display driver
#endif

#if !USE_DISPHSTX && !defined(USE_DISPHSTXMINI)
#define USE_DISPHSTXMINI	1		// 1=use HSTX Display Mini driver
#endif

#ifndef DISPHSTX_DISP_SEL
#define DISPHSTX_DISP_SEL	1		// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#endif

#ifndef DISPHSTX_DVI_PINOUT
#define DISPHSTX_DVI_PINOUT	0		// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+
#endif

// === Display DispHSTX

#if USE_DISPHSTX

#ifndef USE_DISPHSTX_VMODE
#define USE_DISPHSTX_VMODE	1	// DispHSTX videomode (default 1)
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
#endif // USE_DISPHSTX_VMODE

#if (USE_DISPHSTX_VMODE == 1) || (USE_DISPHSTX_VMODE == 3) // 320x240/16 slow

#define WIDTH		320			// display width
#define HEIGHT		240			// display height
#define DEFAULT_VMODE_FNC DispVMode320x240x16	// videomode function

#elif (USE_DISPHSTX_VMODE == 2) || (USE_DISPHSTX_VMODE == 4) // 320x240/16 fast

#define WIDTH		320			// display width
#define HEIGHT		240			// display height
#define DEFAULT_VMODE_FNC DispVMode320x240x16_Fast // videomode function

#elif USE_DISPHSTX_VMODE == 5	// 400x300/16

#define WIDTH		400			// display width
#define HEIGHT		300			// display height
#define DEFAULT_VMODE_FNC DispVMode400x300x16	// videomode function

#elif USE_DISPHSTX_VMODE == 6	// 512x384/16

#define WIDTH		512			// display width
#define HEIGHT		384			// display height
#define DEFAULT_VMODE_FNC DispVMode512x384x16	// videomode function

#elif USE_DISPHSTX_VMODE == 7	// 532x400/16

#define WIDTH		532			// display width
#define HEIGHT		400			// display height
#define DEFAULT_VMODE_FNC DispVMode532x400x16_Fast // videomode function

#elif USE_DISPHSTX_VMODE == 8	// 640x350/16

#define WIDTH		640			// display width
#define HEIGHT		350			// display height
#define DEFAULT_VMODE_FNC DispVMode640x350x16_Fast // videomode function

#elif USE_DISPHSTX_VMODE == 9	// 640x480/8 slow

#define WIDTH		640			// display width
#define HEIGHT		480			// display height
#define DEFAULT_VMODE_FNC DispVMode640x480x8	// videomode function

#elif USE_DISPHSTX_VMODE == 10	// 640x480/8 fast

#define WIDTH		640			// display width
#define HEIGHT		480			// display height
#define DEFAULT_VMODE_FNC DispVMode640x480x8_Fast // videomode function

#elif USE_DISPHSTX_VMODE == 11	// 800x600/6

#define WIDTH		800			// display width
#define HEIGHT		600			// display height
#define DEFAULT_VMODE_FNC DispVMode800x600x6	// videomode function

#elif USE_DISPHSTX_VMODE == 12	// 1024x768/4

#define WIDTH		1024			// display width
#define HEIGHT		768			// display height
#define DEFAULT_VMODE_FNC DispVMode1024x768x4	// videomode function

#endif // USE_DISPHSTX_VMODE

#if !defined(USE_DRAWCAN) && !defined(USE_DRAW)
#if (USE_DISPHSTX_VMODE >= 1) && (USE_DISPHSTX_VMODE <= 8) // 16-bit mode
#define USE_DRAW		1		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define USE_DRAWCAN		0		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)
#define COLOR(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
#else
#define USE_DRAW		0		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define USE_DRAWCAN		1		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)
#endif // USE_DISPHSTX_VMODE
#endif // USE_DRAWCAN

#if USE_DRAW
#ifndef USE_DRAW_STDIO
#if !USE_USB_STDIO
#define USE_DRAW_STDIO		1		// use DRAW stdio (DrawPrint function)
#endif
#endif // USE_DRAW_STDIO
#endif // USE_DRAW

// colors
#if USE_DISPHSTX_VMODE
#if USE_DISPHSTX_VMODE <= 8	// 16-bit color

#define COLBITS		16		// number of output color bits
#define COLTYPE		u16		// type of color
#define FRAMETYPE	u16		// type of frame entry
#define DEFAULT_PDRAWCAN pDrawCan16	// pointer to draw canvas
#ifndef USE_DRAWCAN16
#define USE_DRAWCAN16	1		// 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
#endif

#elif USE_DISPHSTX_VMODE <= 10	// 8-bit color

#define COLBITS		8		// number of output color bits
#define COLTYPE		u8		// type of color
#define FRAMETYPE	u8		// type of frame entry
#define DEFAULT_PDRAWCAN pDrawCan8	// pointer to draw canvas
#ifndef USE_DRAWCAN8
#define USE_DRAWCAN8	1		// 1=use DrawCan8 8-bit functions, if use drawing canvas
#endif

#elif USE_DISPHSTX_VMODE == 11	// 6-bit color

#define COLBITS		6		// number of output color bits
#define COLTYPE		u8		// type of color
#define FRAMETYPE	u8		// type of frame entry
#define DEFAULT_PDRAWCAN pDrawCan6	// pointer to draw canvas
#ifndef USE_DRAWCAN6
#define USE_DRAWCAN6	1		// 1=use DrawCan6 6-bit functions, if use drawing canvas
#endif

#else // USE_DISPHSTX_VMODE		// 4-bit colo

#define COLBITS		4		// number of output color bits
#define COLTYPE		u8		// type of color
#define FRAMETYPE	u8		// type of frame entry
#define DEFAULT_PDRAWCAN pDrawCan4	// pointer to draw canvas
#ifndef USE_DRAWCAN4
#define USE_DRAWCAN4	1		// 1=use DrawCan4 4-bit functions, if use drawing canvas
#endif

#endif // USE_DISPHSTX_VMODE

#define WIDTHLEN	WIDTH		// length of one line in pixels
#define FRAMESIZE 	(WIDTHLEN*HEIGHT) // frame size in number of pixels
#define	DISP_STRIP_NUM	1		// number of back strips

#if (USE_DISPHSTX_VMODE == 1) || (USE_DISPHSTX_VMODE == 2)
#define USE_DISPHSTX_DISPBUF	1	// 1 = use DispBuf + FrameBuf
#else
#define USE_DISPHSTX_DISPBUF	0	// 0 = use only FrameBuf
#endif

#endif // USE_DISPHSTX_VMODE

#endif // USE_DISPHSTX

// === Display - DrawCan library (used with DispHSTX)

#ifndef USE_DRAWCAN0
#define USE_DRAWCAN0	1		// 1=use DrawCan common functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN1
#define USE_DRAWCAN1	0		// 1=use DrawCan1 1-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN2
#define USE_DRAWCAN2	0		// 1=use DrawCan2 2-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN3
#define USE_DRAWCAN3	0		// 1=use DrawCan3 3-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN4
#define USE_DRAWCAN4	0		// 1=use DrawCan4 4-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN6
#define USE_DRAWCAN6	0		// 1=use DrawCan6 6-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN8
#define USE_DRAWCAN8	0		// 1=use DrawCan8 8-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN12
#define USE_DRAWCAN12	0		// 1=use DrawCan12 12-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN16
#define USE_DRAWCAN16	0		// 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
#endif

// === Display DispHSTXMini

#if USE_DISPHSTXMINI		// 1=use HSTX Display Mini driver

#ifndef USE_DISPHSTXMINI_VMODE
#define USE_DISPHSTXMINI_VMODE 1	// DispHstxMini videomode (default 1; data must be DISPHSTX_VGA_MASK masked in VGA case with only FrameBuf)
					//	1=320x240/16bit, FrameBuf+DispBuf, sys_clock=126 MHz
					//	2=320x240/16bit, FrameBuf+DispBuf, sys_clock=252 MHz
					//	3=320x240/16bit (borders 2x46 pixels), FrameBuf+DispBuf, sys_clock variable, clocked from USB 144 MHz
					//	4=320x240/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	5=320x144/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	6=360x240/16bit, only FrameBuf, sys_clock variable, clocked from USB 144 MHz
#endif // USE_DISPHSTXMINI_VMODE

#if (USE_DISPHSTXMINI_VMODE < 1) || (USE_DISPHSTXMINI_VMODE > 6)
#error Invalid USE_DISPHSTXMINI_VMODE
#endif

#if USE_DISPHSTXMINI_VMODE == 5 // 320x144
#define HEIGHT		144		// display height
#else
#define HEIGHT		240		// display height
#endif

#if USE_DISPHSTXMINI_VMODE == 6 // 360x240
#define WIDTH		360		// display width
#else
#define WIDTH		320		// display width
#endif

#define COLBITS		16		// number of output color bits
#define COLTYPE		u16		// type of color
#define FRAMETYPE	u16		// type of frame entry
#define WIDTHLEN	WIDTH		// length of one line in pixels
#define FRAMESIZE 	(WIDTHLEN*HEIGHT) // frame size in number of pixels
#define	DISP_STRIP_NUM	1		// number of back strips

#if USE_DISPHSTXMINI_VMODE <= 3
#define USE_DISPHSTX_DISPBUF	1	// 1 = use DispBuf + FrameBuf
#else
#define USE_DISPHSTX_DISPBUF	0	// 0 = use only FrameBuf
#endif

#if !USE_DRAWCAN && !defined(USE_DRAW)
#define USE_DRAW		1		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#endif // USE_DRAWCAN

#if USE_DRAW		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define pDrawBuf	FrameBuf	// current draw buffer
#define DispStripInx	0		// current index of back buffer strip
#define DispMinY  	0		// minimal Y; base of back buffer strip
#define DispMaxY 	HEIGHT		// maximal Y + 1; end of back buffer strip
#define COLOR(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
#ifndef USE_DRAW_STDIO
#if !USE_USB_STDIO
#define USE_DRAW_STDIO		1		// use DRAW stdio (DrawPrint function)
#endif
#endif // USE_DRAW_STDIO
#endif // USE_DRAW

#endif // USE_DISPHSTXMINI

// ============================================================================
//                            PicoPadVGA
// ============================================================================

#elif USE_PICOPADVGA

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT	0		// UART stdio port 0 or 1
#endif

#define PICO_DEFAULT_UART UART_STDIO_PORT // original-SDK setup

#ifndef UART_STDIO_TX
#define UART_STDIO_TX	7		// UART stdio TX GPIO pin
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX	1		// UART stdio RX GPIO pin
#endif

// PWM sound
#ifndef PWMSND_GPIO
#define PWMSND_GPIO	0	// PWM output GPIO pin (0..29)
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
#define SD_SCK		22	// SD card SCK, to SCLK pin 5
#endif

#ifndef SD_TX
#define SD_TX		19	// SD card TX (MOSI output), to DATA_IN pin 2
#endif

// === Display
#ifndef USE_DRAW
#define USE_DRAW	1	// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#endif

#ifndef USE_DRAW_STDIO
#if !USE_USB_STDIO
#define USE_DRAW_STDIO	1	// use DRAW stdio (DrawPrint function)
#endif
#endif

#ifndef SYSTICK_KEYSCAN
#define SYSTICK_KEYSCAN	1	// call KeyScan() function from SysTick system timer
#endif

#ifndef USE_PWMSND
#define USE_PWMSND	4	// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#endif

// LEDs
#define LED1_PIN	21	// LED1 - SD select
#define LED2_PIN 	25	// default internal LED pin, on Pico board

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
#define SD_BAUD		4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

#ifndef SD_BAUDWRITE
#define SD_BAUDWRITE	1000000 // SD card baud speed of write (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

// Battery
#define BAT_PIN		29		// input from battery
#define BAT_ADC		ADC_MUX_GPIO29	// ADC input
#define BAT_MUL		3		// voltage multiplier

// === Display

#ifndef VGA_GPIO_HSYNC
#define VGA_GPIO_HSYNC	17	// VGA HSYNC GPIO (VGA requires VSYNC = HSYNC+1)
#endif

//#define VGA_USECSYNC	1	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)

#ifndef WIDTH
#define WIDTH	320		// display width
#endif

#ifndef VGA_GPIO_FIRST
#define VGA_GPIO_FIRST	1	// first VGA GPIO
#endif

#ifndef VGA_GPIO_NUM
#define VGA_GPIO_NUM	16	// number of VGA color GPIOs, without HSYNC and VSYNC, including SCK if should skip it
#endif

#ifndef VGA_KEYSCAN_GPIO
#define VGA_KEYSCAN_GPIO 6	// scan keyboard
#endif

//#define VGA_GPIO_SKIP	6	// VGA this GPIO skips; comment out this line if you don't want the VGA pin to skip

#ifndef VGA_PIO
#define VGA_PIO		1	// VGA PIO
#endif

#ifndef VGA_SM
#define VGA_SM		0	// VGA state machine
#endif

#ifndef USE_MINIVGA
#if USE_FRAMEBUF
#define USE_MINIVGA		3		// use mini-VGA display with simple frame buffer:
#else
#define USE_MINIVGA		1
#endif // USE_FRAMEBUF
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer
#endif // USE_MINIVGA

#ifndef COLBITS
#define COLBITS 16		// number of output color bits (4, 8, 15 or 16)
#endif

#ifndef VGA_DMA
#define VGA_DMA		8	// VGA first DMA channel (use 2 channels)
#endif

// mini-VGA auto config
#include "../../_display/minivga/_config.h"

#else // USE_PICOPADVGA

// ============================================================================
//                       PicoPad08, PicoPad10, PicoPad20
// ============================================================================

// === Display

#ifndef DISP_ROT
#define DISP_ROT	1		// display rotation of LCD: 0 Portrait, 1 Landscape, 2 Inverted Portrait, 3 Inverted Landscape
#endif

#ifndef WIDTH
#if (DISP_ROT == 0) || (DISP_ROT == 2)
#define WIDTH		240		// display width
#else
#define WIDTH		320		// display width
#endif
#endif

#ifndef HEIGHT
#if (DISP_ROT == 0) || (DISP_ROT == 2)
#define HEIGHT		320		// display height
#else
#define HEIGHT		240		// display height
#endif
#endif

#ifndef COLBITS
#define COLBITS		16		// number of output color bits (4, 8, 15 or 16)
#endif

#define COLTYPE		u16		// type of color: u8, u16 or u32
#define FRAMETYPE	u16		// type of frame entry: u8 or u16
#define WIDTHLEN	WIDTH		// length of one line of one plane, in number of frame elements
#define FRAMESIZE 	(WIDTHLEN*HEIGHT) // frame size in number of colors
#define	DISP_STRIP_NUM	1		// number of back strips

#ifndef USE_ST7789
#define USE_ST7789	1		// use ST7789 TFT display (st7789.c, st7789.h)
#endif

#ifndef USE_DRAW
#define USE_DRAW	1	// use drawing to frame buffer (lib_draw.c, lib_draw.h)
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

#ifndef DISP_MOSI_PIN
#define DISP_MOSI_PIN	19		// master out TX MOSI pin
#endif

#ifndef DISP_RES_PIN
#define DISP_RES_PIN	20		// reset pin
#endif

#ifndef DISP_CS_PIN
#define DISP_CS_PIN	21		// chip selection pin
#endif

#ifndef USE_DRAW_STDIO
#if !USE_USB_STDIO
#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)
#endif
#endif

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

#ifndef USE_FATALERROR
#define USE_FATALERROR	1		// use fatal error message 0=no, 1=display LCD message (sdk_fatal.c, sdk_fatal.h)
#endif

// ============================================================================
//                            PicoPad08
// ============================================================================

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
#define SD_BAUD		4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

#ifndef SD_BAUDWRITE
#define SD_BAUDWRITE	1000000 // SD card baud speed of write (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif


#endif // USE_PICOPAD08

// ============================================================================
//                          PicoPad10 and PicoPad20
// ============================================================================

// ======== PicoPad version 1.0 and 2.0
#if USE_PICOPAD10 || USE_PICOPAD20

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
#define SD_BAUD		4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif

#ifndef SD_BAUDWRITE
#define SD_BAUDWRITE	1000000 // SD card baud speed of write (should be max. 7-12 Mbit/s; default standard bus speed
				//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)
#endif


#endif // USE_PICOPAD10 || USE_PICOPAD20

#endif // USE_PICOPADVGA
