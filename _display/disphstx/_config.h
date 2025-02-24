
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                            Default configuration
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

// For most parameters, set 1 to enable the property or 0 to disable the property.
// If you change the configuration, do a full rebuild of the project
//   to make the changes take effect (run the d.* script first).

#if USE_DISPHSTX		// 1=use HSTX Display driver

#ifndef _DISPHSTX_CONFIG_H
#define _DISPHSTX_CONFIG_H

#ifndef DISPHSTX_PICOSDK
#define DISPHSTX_PICOSDK		0	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#endif

#ifndef DISPHSTX_USE_DVI
#define DISPHSTX_USE_DVI		1	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#endif

#ifndef DISPHSTX_USE_VGA
#define DISPHSTX_USE_VGA		1	// 1=use VGA support (VGA requires about 30 KB of RAM)
#endif

#ifndef DISPHSTX_DVI_PINOUT
#define DISPHSTX_DVI_PINOUT		0	// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+
#endif

#ifndef DISPHSTX_ARM_ASM
#define DISPHSTX_ARM_ASM		1	// 1 = use ARM assembler for render functions
#endif

#ifndef DISPHSTX_RISCV_ASM
#define DISPHSTX_RISCV_ASM		1	// 1 = use RISC-V assembler for render functions
#endif

#ifndef DISPHSTX_USEPLL
#define DISPHSTX_USEPLL			0	// 1 = use separate PLL_SYS for HSTX generator (sys_clk will use PLL_USB)
#endif

#ifndef DISPHSTX_CHECK_LOAD
#define DISPHSTX_CHECK_LOAD		0	// 1 = check CPU load during IRQ interrupt (get DispHstxTimeIn, DispHstxTimeOut)
#endif

#ifndef DISPHSTX_CHECK_LEDIRQ
#define DISPHSTX_CHECK_LEDIRQ		-1	// LED_PIN, >=0 (use GPIO pin) = use debug LED to indicate that we are alive, interval about 1 sec
#endif

#ifndef DISPHSTX_DISP_SEL
#define DISPHSTX_DISP_SEL		-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#endif

// DMA channels
#ifndef DISPHSTX_DMA_DATA
#define DISPHSTX_DMA_DATA		15	// DMA data channel (to copy data to HSTX interface)
#endif

#ifndef DISPHSTX_DMA_CMD
#define DISPHSTX_DMA_CMD		14	// DMA command channel (to control DMA data channel)
#endif

// limits
#ifndef DISPHSTX_WIDTHMAX
#define DISPHSTX_WIDTHMAX		1440	// max. supported width (specifies also size of the scanline render buffer)
#endif

#ifndef DISPHSTX_STRIP_MAX
#define DISPHSTX_STRIP_MAX		4	// max. number of videomode strips
#endif

#ifndef DISPHSTX_SLOT_MAX
#define DISPHSTX_SLOT_MAX		4	// max. number of videomode slots
#endif

// ===== DVI GPIO pins

#if DISPHSTX_DVI_PINOUT == 1	// DVI predefined pinout: 0=DVI breakout board, 1=order D2+..CLK-, 2=order CLK-..D2+

// order D2+..CLK-
#ifndef DISPHSTX_DVI_D2P
#define DISPHSTX_DVI_D2P		12	// GPIO pin to output D2+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D2M
#define DISPHSTX_DVI_D2M		13	// GPIO pin to output D2- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1P
#define DISPHSTX_DVI_D1P		14	// GPIO pin to output D1+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1M
#define DISPHSTX_DVI_D1M		15	// GPIO pin to output D1- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0P
#define DISPHSTX_DVI_D0P		16	// GPIO pin to output D0+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0M
#define DISPHSTX_DVI_D0M		17	// GPIO pin to output D0- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKP
#define DISPHSTX_DVI_CLKP		18	// GPIO pin to output CLK+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKM
#define DISPHSTX_DVI_CLKM		19	// GPIO pin to output CLK- (use 12..19)
#endif

#elif DISPHSTX_DVI_PINOUT == 2

// order CLK-..D2+
#ifndef DISPHSTX_DVI_D2P
#define DISPHSTX_DVI_D2P		19	// GPIO pin to output D2+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D2M
#define DISPHSTX_DVI_D2M		18	// GPIO pin to output D2- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1P
#define DISPHSTX_DVI_D1P		17	// GPIO pin to output D1+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1M
#define DISPHSTX_DVI_D1M		16	// GPIO pin to output D1- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0P
#define DISPHSTX_DVI_D0P		15	// GPIO pin to output D0+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0M
#define DISPHSTX_DVI_D0M		14	// GPIO pin to output D0- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKP
#define DISPHSTX_DVI_CLKP		13	// GPIO pin to output CLK+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKM
#define DISPHSTX_DVI_CLKM		12	// GPIO pin to output CLK- (use 12..19)
#endif

#else // DISPHSTX_DVI_PINOUT

// DVI breakout board
#ifndef DISPHSTX_DVI_D2P
#define DISPHSTX_DVI_D2P		16	// GPIO pin to output D2+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D2M
#define DISPHSTX_DVI_D2M		17	// GPIO pin to output D2- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1P
#define DISPHSTX_DVI_D1P		18	// GPIO pin to output D1+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D1M
#define DISPHSTX_DVI_D1M		19	// GPIO pin to output D1- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0P
#define DISPHSTX_DVI_D0P		12	// GPIO pin to output D0+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_D0M
#define DISPHSTX_DVI_D0M		13	// GPIO pin to output D0- (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKP
#define DISPHSTX_DVI_CLKP		14	// GPIO pin to output CLK+ (use 12..19)
#endif

#ifndef DISPHSTX_DVI_CLKM
#define DISPHSTX_DVI_CLKM		15	// GPIO pin to output CLK- (use 12..19)
#endif

#endif // DISPHSTX_DVI_PINOUT

// ===== VGA GPIO pins

#ifndef DISPHSTX_VGA_B0
#define DISPHSTX_VGA_B0			12	// GPIO pin to output B0 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_B1
#define DISPHSTX_VGA_B1			13	// GPIO pin to output B1 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_G0
#define DISPHSTX_VGA_G0			14	// GPIO pin to output G0 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_G1
#define DISPHSTX_VGA_G1			15	// GPIO pin to output G1 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_R0
#define DISPHSTX_VGA_R0			16	// GPIO pin to output R0 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_R1
#define DISPHSTX_VGA_R1			17	// GPIO pin to output R1 (use 12..19)
#endif

#ifndef DISPHSTX_VGA_HSYNC
#define DISPHSTX_VGA_HSYNC		18	// GPIO pin to output HSYNC (use 12..19)
#endif

#ifndef DISPHSTX_VGA_VSYNC
#define DISPHSTX_VGA_VSYNC		19	// GPIO pin to output VSYNC (use 12..19)
#endif

// ===== selection of supported formats (You can disable unused formats to save some RAM)

#ifndef DISPHSTX_USE_FORMAT_1
#define DISPHSTX_USE_FORMAT_1			1	// 1=use format DISPHSTX_FORMAT_1, 1 bit per pixel, black & white format 0..1 (8 pixels per 1 byte; width must be aligned to 8 pixels; hdbl > 1 not supported)
#endif

#ifndef DISPHSTX_USE_FORMAT_2
#define DISPHSTX_USE_FORMAT_2			1	// 1=use format DISPHSTX_FORMAT_2, 2 bits per pixel, grayscale format 0..3 (4 pixels per 1 byte; width must be aligned to 4 pixels; hdbl > 1 not supported)
#endif

#ifndef DISPHSTX_USE_FORMAT_3
#define DISPHSTX_USE_FORMAT_3			1	// 1=use format DISPHSTX_FORMAT_3, 3 bits per pixel, format RGB111 (10 pixels per 32-bit word; width must be aligned to 10 pixels; hdbl > 1 not supported)
#endif

#ifndef DISPHSTX_USE_FORMAT_4
#define DISPHSTX_USE_FORMAT_4			1	// 1=use format DISPHSTX_FORMAT_4, 4 bits per pixel, format gray 16 levels (2 pixels per 1 byte; width must be aligned to 2 pixels; hdbl > 1 not supported)
#endif

#ifndef DISPHSTX_USE_FORMAT_6
#define DISPHSTX_USE_FORMAT_6			1	// 1=use format DISPHSTX_FORMAT_6, 6 bits per pixel, format RGB222 (5 pixels per 32-bit word; width must be aligned to 5 pixels; hdbl > 1 not supported)
#endif

#ifndef DISPHSTX_USE_FORMAT_8
#define DISPHSTX_USE_FORMAT_8			1	// 1=use format DISPHSTX_FORMAT_8, 8 bits per pixel, format RGB332 (1 pixel per 1 byte)
#endif

#ifndef DISPHSTX_USE_FORMAT_12
#define DISPHSTX_USE_FORMAT_12			1	// 1=use format DISPHSTX_FORMAT_12, 12 bits per pixel, format RGB444 (8 pixels per three 32-bit words)
#endif

#ifndef DISPHSTX_USE_FORMAT_15
#define DISPHSTX_USE_FORMAT_15			1	// 1=use format DISPHSTX_FORMAT_15, 15 bits per pixel, format RGB555 (1 pixel per 2 bytes; bit 15 is ignored)
#endif

#ifndef DISPHSTX_USE_FORMAT_16
#define DISPHSTX_USE_FORMAT_16			1	// 1=use format DISPHSTX_FORMAT_16, 16 bits per pixel, format RGB565 (1 pixel per 2 bytes)
#endif

#ifndef DISPHSTX_USE_FORMAT_1_PAL
#define DISPHSTX_USE_FORMAT_1_PAL		1	// 1=use format DISPHSTX_FORMAT_1_PAL, 1 bit per pixel, paletted colors (8 pixels per 1 byte)
#endif

#ifndef DISPHSTX_USE_FORMAT_2_PAL
#define DISPHSTX_USE_FORMAT_2_PAL		1	// 1=use format DISPHSTX_FORMAT_2_PAL, 2 bits per pixel, paletted colors (4 pixels per 1 byte)
#endif

#ifndef DISPHSTX_USE_FORMAT_3_PAL
#define DISPHSTX_USE_FORMAT_3_PAL		1	// 1=use format DISPHSTX_FORMAT_3_PAL, 3 bits per pixel, paletted colors (10 pixels per 32-bit word)
#endif

#ifndef DISPHSTX_USE_FORMAT_4_PAL
#define DISPHSTX_USE_FORMAT_4_PAL		1	// 1=use format DISPHSTX_FORMAT_4_PAL, 4 bits per pixel, paletted colors (2 pixels per 1 byte)
#endif

#ifndef DISPHSTX_USE_FORMAT_6_PAL
#define DISPHSTX_USE_FORMAT_6_PAL		1	// 1=use format DISPHSTX_FORMAT_6_PAL, 6 bits per pixel, paletted colors (5 pixels per 32-bit word)
#endif

#ifndef DISPHSTX_USE_FORMAT_8_PAL
#define DISPHSTX_USE_FORMAT_8_PAL		1	// 1=use format DISPHSTX_FORMAT_8_PAL, 8 bits per pixel, paletted colors (1 pixel per 1 byte)
#endif

#ifndef DISPHSTX_USE_FORMAT_COL
#define DISPHSTX_USE_FORMAT_COL			1	// 1=use format DISPHSTX_FORMAT_COL, simple color
#endif

#ifndef DISPHSTX_USE_FORMAT_MTEXT
#define DISPHSTX_USE_FORMAT_MTEXT		1	// 1=use format DISPHSTX_FORMAT_MTEXT, monocolor text, font width 8 pixels, colors of every row given as 2 palette colors RGB565 (BG,FG)
#endif

#ifndef DISPHSTX_USE_FORMAT_ATEXT
#define DISPHSTX_USE_FORMAT_ATEXT		1	// 1=use format DISPHSTX_FORMAT_ATEXT, attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit attribute low=FG high=BG index of color
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE4_8
#define DISPHSTX_USE_FORMAT_TILE4_8		1	// 1=use format DISPHSTX_FORMAT_TILE4_8, tiles 4x4 pixels, 1-byte index, 8-bit format RGB332
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE8_8
#define DISPHSTX_USE_FORMAT_TILE8_8		1	// 1=use format DISPHSTX_FORMAT_TILE8_8, tiles 8x8 pixels, 1-byte index, 8-bit format RGB332
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE16_8
#define DISPHSTX_USE_FORMAT_TILE16_8		1	// 1=use format DISPHSTX_FORMAT_TILE16_8, tiles 16x16 pixels, 1-byte index, 8-bit format RGB332
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE32_8
#define DISPHSTX_USE_FORMAT_TILE32_8		1	// 1=use format DISPHSTX_FORMAT_TILE32_8, tiles 32x32 pixels, 1-byte index, 8-bit format RGB332
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE4_8_PAL
#define DISPHSTX_USE_FORMAT_TILE4_8_PAL		1	// 1=use format DISPHSTX_FORMAT_TILE4_8_PAL, tiles 4x4 pixels, 1-byte index, 8-bit paletted colors
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE8_8_PAL
#define DISPHSTX_USE_FORMAT_TILE8_8_PAL		1	// 1=use format DISPHSTX_FORMAT_TILE8_8_PAL, tiles 8x8 pixels, 1-byte index, 8-bit paletted colors
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE16_8_PAL
#define DISPHSTX_USE_FORMAT_TILE16_8_PAL	1	// 1=use format DISPHSTX_FORMAT_TILE16_8_PAL, tiles 16x16 pixels, 1-byte index, 8-bit paletted colors
#endif

#ifndef DISPHSTX_USE_FORMAT_TILE32_8_PAL
#define DISPHSTX_USE_FORMAT_TILE32_8_PAL	1	// 1=use format DISPHSTX_FORMAT_TILE32_8_PAL, tiles 32x32 pixels, 1-byte index, 8-bit paletted colors
#endif

#ifndef DISPHSTX_USE_FORMAT_HSTX_15
#define DISPHSTX_USE_FORMAT_HSTX_15		1	// 1=use format DISPHSTX_FORMAT_HSTX_15, HSTX RLE compression, 15 bits per pixel, format RGB555
#endif

#ifndef DISPHSTX_USE_FORMAT_HSTX_16
#define DISPHSTX_USE_FORMAT_HSTX_16		1	// 1=use format DISPHSTX_FORMAT_HSTX_16, HSTX RLE compression, 16 bits per pixel, format RGB565
#endif

#ifndef DISPHSTX_USE_FORMAT_PAT_8
#define DISPHSTX_USE_FORMAT_PAT_8		1	// 1=use format DISPHSTX_FORMAT_PAT_8, repeated pattern, 8-bit format RGB332, width derived from pitch, height from fonth
#endif

#ifndef DISPHSTX_USE_FORMAT_PAT_8_PAL
#define DISPHSTX_USE_FORMAT_PAT_8_PAL		1	// 1=use format DISPHSTX_FORMAT_PAT_8_PAL, repeated pattern, 8-bit format, paletted colors, width derived from pitch, height from fonth
#endif

#ifndef DISPHSTX_USE_FORMAT_RLE8
#define DISPHSTX_USE_FORMAT_RLE8		1	// 1=use format DISPHSTX_FORMAT_RLE8, RLE compression, 8-bit format RGB332 (font=line offsets)
#endif

#ifndef DISPHSTX_USE_FORMAT_RLE8_PAL
#define DISPHSTX_USE_FORMAT_RLE8_PAL		1	// 1=use format DISPHSTX_FORMAT_RLE8_PAL, RLE compression, 8-bit format, paletted colors (font=line offsets)
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR1_PAL
#define DISPHSTX_USE_FORMAT_ATTR1_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR1_PAL, attribute compression 1 (to 28%), 4-bit paletted colors, cell 8x8, 2 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR2_PAL
#define DISPHSTX_USE_FORMAT_ATTR2_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR2_PAL, attribute compression 2 (to 37%), 4-bit paletted colors, cell 4x4, 2 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR3_PAL
#define DISPHSTX_USE_FORMAT_ATTR3_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR3_PAL, attribute compression 3 (to 56%), 4-bit paletted colors, cell 8x8, 4 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR4_PAL
#define DISPHSTX_USE_FORMAT_ATTR4_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR4_PAL, attribute compression 4 (to 75%), 4-bit paletted colors, cell 4x4, 4 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR5_PAL
#define DISPHSTX_USE_FORMAT_ATTR5_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR5_PAL, attribute compression 5 (to 15%), 8-bit paletted colors, cell 8x8, 2 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR6_PAL
#define DISPHSTX_USE_FORMAT_ATTR6_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR6_PAL, attribute compression 6 (to 25%), 8-bit paletted colors, cell 4x4, 2 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR7_PAL
#define DISPHSTX_USE_FORMAT_ATTR7_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR7_PAL, attribute compression 7 (to 31%), 8-bit paletted colors, cell 8x8, 4 attributes
#endif

#ifndef DISPHSTX_USE_FORMAT_ATTR8_PAL
#define DISPHSTX_USE_FORMAT_ATTR8_PAL		1	// 1=use format DISPHSTX_FORMAT_ATTR8_PAL, attribute compression 8 (to 50%), 8-bit paletted colors, cell 4x4, 4 attributes
#endif

#endif // _DISPHSTX_CONFIG_H

#endif // USE_DISPHSTX
