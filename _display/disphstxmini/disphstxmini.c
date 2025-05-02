
// ****************************************************************************
//
//    HDMI and VGA Mini display driver for Pico2 RP2350 over HSTX interface
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

// Videomode 640x480/60Hz/31.5kHz (VGA 4:3, VESA DMT ID 04h), pixel clock 25.2 MHz

// Variable system clock 640x480 (320x240) - use PLL_USB generator on 3*48=144 MHz:
//   vertical refresh: 60 Hz
//   horizontal refresh: 31.46875 kHz
//   pixel clock: 25.175 MHz ... real 144/5 = 28.8 MHz
// Horizontal timing (line):
//   Horizontal sync pulse: negative
//   Visible area: 640 pixels, 25.42205 us ... real 22.22222 us
//     ... additional borders 2*1.593481 us = 2*46 pixels
//   Front porch: 16 pixels, 0.635551 us ... real 0.635551+1.593481=2.229032 us, 64 pixels, 2.222222 us
//   Sync pulse: 96 pixels, 3.813307 us ... real 110 pixels, 3.819444 us
//   Back porch: 48 pixels, 1.906653 us ... real 1.906653+1.593481=3.500134 us, 101 pixels, 3.506944 us
//   Whole line: 800 pixels, 31.77756 us ... real 915 pixels, 31.77083 us
// Vertical timing (frame):
//   Vertical sync poles: negative
//   Visible area: 480 lines, 15.25323 ms
//   Front porch: 10 lines, 0.3177756 ms
//   Sync pulse: 2 lines, 0.06355511 ms
//   Back porch: 33 lines, 1.048659 ms
//   Whole frame: 525 lines, 16.68322 ms

// Variable system clock 720x480 (360x240) - use PLL_USB generator on 3*48=144 MHz:
//   vertical refresh: 60 Hz
//   horizontal refresh: 31.47541 kHz
//   pixel clock: 28.8 MHz
// Horizontal timing (line):
//   Horizontal sync pulse: negative
//   Visible area: 720 pixels, 25.0000 us
//	... additional borders 2*0.211025 us = 2*6 pixels
//   Front porch: 24 pixels, 0.833333 us
//   Sync pulse: 110 pixels, 3.819444 us
//   Back porch: 61 pixels, 2.118056 us
//   Whole line: 915 pixels, 31.77083 us
// Vertical timing (frame):
//   Vertical sync poles: negative
//   Visible area: 480 lines, 15.25 ms
//   Front porch: 10 lines, 0.317708 ms
//   Sync pulse: 2 lines, 0.0635417 ms
//   Back porch: 33 lines, 1.048437 ms
//   Whole frame: 525 lines, 16.6797 ms

#include "../../global.h"

#if USE_DISPHSTXMINI		// 1=use HSTX Display Mini driver

#include "disphstxmini.h"

// PicoLibSDK library
#include "../../_sdk/inc/sdk_clocks.h"
#include "../../_sdk/inc/sdk_dma.h"
#include "../../_sdk/inc/sdk_flash.h"
#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_hstx.h"
#include "../../_sdk/inc/sdk_irq.h"
#include "../../_sdk/inc/sdk_pll.h"
#include "../../_sdk/inc/sdk_reset.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_lib/inc/lib_pwmsnd.h"
#include "../../_sdk/inc/sdk_multicore.h"

// TMDS commands (10 bits)
#define TMDS_CTRL_00	0x0354		// VSYNC=0, HSYNC=0
#define TMDS_CTRL_01	0x00ab		// VSYNC=0, HSYNC=1
#define TMDS_CTRL_10	0x0154		// VSYNC=1, HSYNC=0
#define TMDS_CTRL_11	0x02ab		// VSYNC=1, HSYNC=1

// DVI synchronization commands (3 x 10 = 30 bits, lanes 0=blue+SYNC 1=green 2=red)
#define HDMI_SYNC_V0_H0	(TMDS_CTRL_00 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=0, HSYNC=0
#define HDMI_SYNC_V0_H1	(TMDS_CTRL_01 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=0, HSYNC=1
#define HDMI_SYNC_V1_H0	(TMDS_CTRL_10 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=1, HSYNC=0
#define HDMI_SYNC_V1_H1	(TMDS_CTRL_11 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=1, HSYNC=1

// current DVI synchronization commands of 640x480 mode - both SYNC are negative
#define HDMI_SYNC_V0H0	HDMI_SYNC_V1_H1		// VSYNC off, HSYNC off
#define HDMI_SYNC_V0H1	HDMI_SYNC_V1_H0		// VSYNC off, HSYNC on
#define HDMI_SYNC_V1H0	HDMI_SYNC_V0_H1		// VSYNC on, HSYNC off
#define HDMI_SYNC_V1H1	HDMI_SYNC_V0_H0		// VSYNC on, HSYNC on

// current VGA synchronization commands - both SYNC are positive (will be inverted on output)
#define VGA_SYNC_V0H0	0				// VSYNC off, HSYNC off
#define VGA_SYNC_V0H1	(DISPHSTX_VGA_HSYNCVAL | (DISPHSTX_VGA_HSYNCVAL<<16))	// VSYNC off, HSYNC on
#define VGA_SYNC_V1H0	(DISPHSTX_VGA_VSYNCVAL | (DISPHSTX_VGA_VSYNCVAL<<16))	// VSYNC on, HSYNC off
#define VGA_SYNC_V1H1	(VGA_SYNC_V0H1 | VGA_SYNC_V1H0)	// VSYNC on, HSYNC on

// frame buffer
#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
FRAMETYPE ALIGNED DispBuf[FRAMESIZE];	// display buffer
#endif // USE_DISPHSTX_DISPBUF

#if USE_FRAMEBUF
FRAMETYPE ALIGNED FrameBuf[FRAMESIZE];	// frame buffer
#endif // USE_FRAMEBUF

// display timings (HSYNC and VSYNC polarity is negative)
#if USE_DISPHSTXMINI_VMODE == 6	// 720x240 (360x240) videomode
#define DISPHSTX_HACTIVE	720	// horizontal active pixels
#define DISPHSTX_HFRONT		24	// horizontal front porch in pixels
#define DISPHSTX_HSYNC		110	// HSYNC width in pixels
#define DISPHSTX_HBACK		61	// horizontal back porch in pixels
#define DISPHSTX_HTOTAL		915	// horizontal total width in pixels
#elif USE_DISPHSTXMINI_VMODE >= 3	// use 144 MHz (USB clock)
#define DISPHSTX_HACTIVE	640	// horizontal active pixels
#define DISPHSTX_HFRONT		64	// horizontal front porch in pixels
#define DISPHSTX_HSYNC		110	// HSYNC width in pixels
#define DISPHSTX_HBACK		101	// horizontal back porch in pixels
#define DISPHSTX_HTOTAL		915	// horizontal total width in pixels
#else // USE_DISPHSTXMINI_VMODE >= 3	// use 126 or 252 MHz (SYS clock)
#define DISPHSTX_HACTIVE	640	// horizontal active pixels
#define DISPHSTX_HFRONT		16	// horizontal front porch in pixels
#define DISPHSTX_HSYNC		96	// HSYNC width in pixels
#define DISPHSTX_HBACK		48	// horizontal back porch in pixels
#define DISPHSTX_HTOTAL		800	// horizontal total width in pixels
#endif // USE_DISPHSTXMINI_VMODE >= 3

#define DISPHSTX_VACTIVE	480	// vertical active scanlines
#define DISPHSTX_VFRONT		10	// vertical front porch in scanlines
#define DISPHSTX_VSYNC		2	// VSYNC height in scanlines
#define DISPHSTX_VBACK		33	// vertical back porch in scanlines
#define DISPHSTX_VTOTAL		525	// vertical total height in scanlines

// length of one command (number of u32 words, 16 bytes)
#define DISPHSTX_CMDLEN	4
	// each command contains 4x u32 words as alias 3:
	// - control word
	// - write address
	// - transfer count
	// - read address + trigger

// number of commands per one frame (= 1006, almost 16 KB)
#define DISPHSTX_CMDNUM		(DISPHSTX_VACTIVE*2 + DISPHSTX_VFRONT + DISPHSTX_VSYNC + DISPHSTX_VBACK + 1)
	// - active scanlines (1 command horizontal sync + 1 command active data)
	// - vertical front porch
	// - vertical sync
	// - vertical back porch
	// - NULL terminator

// HSTX command - HSYNC before active data
// - (2) horizontal front porch
// - (2) HSYNC
// - (2) horizontal back porch
// - (1) start active data
u32 DispHstxCmdHSync[7];

// HSTX command - blank scanline
// - (2) horizontal front porch
// - (2) HSYNC
// - (2) horizontal back porch, active pixels
u32 DispHstxCmdBlank[6];

// HSTX command - VSYNC
// - (2) VSYNC + horizontal front porch
// - (2) VSYNC + HSYNC
// - (2) VSYNC + horizontal back porch, active pixels
u32 DispHstxCmdVSync[6];

// command buffer (16 KB)
u32 DispHstxCmdBuf[DISPHSTX_CMDNUM*DISPHSTX_CMDLEN];

// flag - HSTX VGA mode is active
Bool DispHstxVgaRunning = False;

// flag - HSTX DVI mode is active
Bool DispHstxDviRunning = False;

// current selected display mode DISPHSTX_DISPMODE_*
int DispHstxDispMode = DISPHSTX_DISPMODE_NONE;
Bool DispHstxDispSelInitOK = False;

// last system time of auto update
u32 DispAutoUpdateLast;

// ============================================================================
//                               Selection switch
// ============================================================================

// init display selection switch
void DispHstxDispSelInit()
{
#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	// initialize display selection switch
	GPIO_Init(DISPHSTX_DISP_SEL);
	GPIO_InDisable(DISPHSTX_DISP_SEL); // disable input (reason: RP2350-A2 errata RP2350-E9)
#endif
	DispHstxDispSelInitOK = True;
}

// terminate display selection switch
void DispHstxDispSelTerm()
{
#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	GPIO_Reset(DISPHSTX_DISP_SEL);
#endif
	DispHstxDispSelInitOK = False;
}

// get current display selection switch (display selection switch is auto-initialized)
int DispHstxCheckDispSel()
{
	int res;

#if DISPHSTX_USE_DVI && DISPHSTX_USE_VGA

	res = DISPHSTX_DISPMODE_NONE; // do auto-detection

#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch

	u8 in;

	// init display selection switch
	if (!DispHstxDispSelInitOK) DispHstxDispSelInit();

	// select pull-up, check VGA position
	GPIO_PullUp(DISPHSTX_DISP_SEL);		// enable pull-up resistor
	WaitUs(50);				// short delay to stabilize signal
	GPIO_InEnable(DISPHSTX_DISP_SEL);	// input enable
	WaitUs(2);				// short delay to stabilize signal
	cb();
	in = GPIO_In(DISPHSTX_DISP_SEL);	// read input state
	cb();
	GPIO_InDisable(DISPHSTX_DISP_SEL);	// input disable (reason: RP2350-A2 errata RP2350-E9)
	GPIO_NoPull(DISPHSTX_DISP_SEL);		// disable pull-up resistor
	if (in == 0)
		res = DISPHSTX_DISPMODE_VGA; // VGA selected
	else
	{
		// select pull-down, check DVI position
		GPIO_PullDown(DISPHSTX_DISP_SEL);	// enable pull-down resistor
		WaitUs(50);				// short delay to stabilize signal
		GPIO_InEnable(DISPHSTX_DISP_SEL);	// input enable
		WaitUs(2);				// short delay to stabilize signal
		cb();
		in = GPIO_In(DISPHSTX_DISP_SEL);	// read input state
		cb();
		GPIO_InDisable(DISPHSTX_DISP_SEL);	// input disable (reason: RP2350-A2 errata RP2350-E9)
		GPIO_NoPull(DISPHSTX_DISP_SEL);		// disable pull-up resistor
		if (in == 1) res = DISPHSTX_DISPMODE_DVI; // DVI selected
	}

#endif // DISPHSTX_DISP_SEL >= 0

#elif DISPHSTX_USE_DVI // only DVI
	res = DISPHSTX_DISPMODE_DVI;
#else // only VGA
	res = DISPHSTX_DISPMODE_VGA;
#endif
	return res;
}

// auto-detect display selection on program start (returns DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA)
// - Tests by pull-ups whether a VGA monitor is connected (that is, whether it pulls down
//   the input signal to 0). A connected HDMI monitor is not pulling down the signal to 0.
// - Can only be used at program start or before initializing the video mode because it resets the GPIO pins.
// - If the display selection switch is enabled, it is used in preference.
// - The RGB pins of the VGA display are supposed to be connected to GPIO 12 to 17.
int DispHstxAutoDispSel()
{
	int res;

#if DISPHSTX_USE_DVI && DISPHSTX_USE_VGA

	// selected by the switch
	res = DispHstxCheckDispSel();

	// auto-detection
	if (res == DISPHSTX_DISPMODE_NONE)
	{
#define DISPHSTX_VGA_RGBMASK ( BIT(DISPHSTX_VGA_B0) | BIT(DISPHSTX_VGA_B1) | BIT(DISPHSTX_VGA_G0) \
		| BIT(DISPHSTX_VGA_G1) | BIT(DISPHSTX_VGA_R0) | BIT(DISPHSTX_VGA_R1) )

		// setup GPIO pins to input with pull-ups
		u32 mask = DISPHSTX_VGA_RGBMASK;
		GPIO_InitMask(mask);	// initialize pins (enable inputs)

		GPIO_PullUp(DISPHSTX_VGA_B0);
		GPIO_PullUp(DISPHSTX_VGA_B1);
		GPIO_PullUp(DISPHSTX_VGA_G0);
		GPIO_PullUp(DISPHSTX_VGA_G1);
		GPIO_PullUp(DISPHSTX_VGA_R0);
		GPIO_PullUp(DISPHSTX_VGA_R1);

		// Delay to stabilise capacities
		WaitUs(500);

		// check input - VGA returns 0, HDMI returns 0x3f
		mask >>= 12;
		res = (((GPIO_InAll() >> 12) & mask) == mask) ? DISPHSTX_DISPMODE_DVI : DISPHSTX_DISPMODE_VGA;

		// reset GPIP pins
		GPIO_ResetMask(DISPHSTX_VGA_RGBMASK);
	}

#elif DISPHSTX_USE_DVI // only DVI
	res = DISPHSTX_DISPMODE_DVI;
#else // only VGA
	res = DISPHSTX_DISPMODE_VGA;
#endif
	return res;
}

// ============================================================================
//                               VGA driver
// ============================================================================

// DMA IRQ handler
void FASTCODE NOFLASH(DispHstxIrqHandler)()
{
	// Clear the interrupt request for DMA control channel
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);

	// start next control buffer
	DMA_SetReadTrig(DISPHSTX_DMA_CMD, DispHstxCmdBuf);
}

// DMA control word to write data to HSTX
#define DISPHSTX_CTRL_DATA (				\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ(DREQ_HSTX) |		\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_INC_READ |			\
		DMA_CTRL_SIZE(DMA_SIZE_16) |		\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// DMA control word to write sync command to HSTX
#define DISPHSTX_CTRL_SYNC (				\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ(DREQ_HSTX) |		\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_INC_READ |			\
		DMA_CTRL_SIZE(DMA_SIZE_32) |		\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// DMA control word to stop transfer
#define DISPHSTX_CTRL_STOP (				\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ_FORCE |			\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// current VGA synchronization commands - both SYNC are positive (will be inverted on output)
#define VGA_SYNC_V0H0	0				// VSYNC off, HSYNC off
#define VGA_SYNC_V0H1	(DISPHSTX_VGA_HSYNCVAL | (DISPHSTX_VGA_HSYNCVAL<<16))	// VSYNC off, HSYNC on
#define VGA_SYNC_V1H0	(DISPHSTX_VGA_VSYNCVAL | (DISPHSTX_VGA_VSYNCVAL<<16))	// VSYNC on, HSYNC off
#define VGA_SYNC_V1H1	(VGA_SYNC_V0H1 | VGA_SYNC_V1H0)	// VSYNC on, HSYNC on

// initialize VGA
void DispHstxVgaInit()
{
	int i, j;
	u32 *hsync, *blank, *vsync, *cmd;
	u16 *src;

	// prepare "HSYNC" command before active data
	hsync = DispHstxCmdHSync;
	hsync[0] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HFRONT;	// active pixels, horizontal front porch
	hsync[1] = VGA_SYNC_V0H0;				// blank
	hsync[2] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HSYNC;	// HSYNC
	hsync[3] = VGA_SYNC_V0H1;				// HSYNC
	hsync[4] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HBACK;	// horizontal back porch
	hsync[5] = VGA_SYNC_V0H0;				// blank
	hsync[6] = HSTX_CMD_RAW | DISPHSTX_HACTIVE;		// start active data

	// prepare "blank scanline" command
	blank = DispHstxCmdBlank;
	blank[0] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HFRONT;	// horizontal front porch
	blank[1] = VGA_SYNC_V0H0;				// blank
	blank[2] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HSYNC;	// HSYNC
	blank[3] = VGA_SYNC_V0H1;				// HSYNC
	blank[4] = HSTX_CMD_RAW_REPEAT | (DISPHSTX_HBACK + DISPHSTX_HACTIVE); // horizontal back porch, active pixels
	blank[5] = VGA_SYNC_V0H0;				// blank

	// prepare "VSYNC" command
	vsync = DispHstxCmdVSync;
	vsync[0] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HFRONT;	// horizontal front porch
	vsync[1] = VGA_SYNC_V1H0;				// VSYNC+blank
	vsync[2] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HSYNC;	// HSYNC
	vsync[3] = VGA_SYNC_V1H1; 				// VSYNC+HSYNC
	vsync[4] = HSTX_CMD_RAW_REPEAT | (DISPHSTX_HBACK + DISPHSTX_HACTIVE); // horizontal back porch, active pixels
	vsync[5] = VGA_SYNC_V1H0;				// VSYNC+blank

	// prepare write address to HSTX FIFO
	u32 w = (u32)&hstx_fifo_hw->fifo;

	// fill-up scanline commands - active scanlines (480 scanlines)
	cmd = DispHstxCmdBuf;
	for (i = 0; i < DISPHSTX_VACTIVE; i++)
	{
		// buffer with source image data
#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
		src = DispBuf;
#else
		src = FrameBuf;
#endif

		// index of source line
#if USE_DISPHSTXMINI_VMODE == 5	// 144-line videomode
		j = i*144/480;	// j=0 0 0 1 1 1 2 2 2 3 ... 142 143 143 143
#else
		j = i/2;	// j=0 0 1 1 2 2 3 3 ... 238 238 239 239
#endif

		// address of source line
		src += j*WIDTH;

		// HSYNC command
		// - control word
		// - write address
		// - transfer count
		// - read address + trigger
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdHSync);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdHSync;		// DMA read address

		// data command ... every pixel (u16) is duplicated on bus
		cmd[4] = DISPHSTX_CTRL_DATA;		// DMA control word
		cmd[5] = w;				// DMA write address
		cmd[6] = WIDTH;				// DMA transfer count
		cmd[7] = (u32)src;			// DMA read address

		// shift to next scanline
		cmd += 2*DISPHSTX_CMDLEN;
	}

	// vertical front porch scanlines
	for (i = DISPHSTX_VFRONT; i > 0; i--)
	{
		// "blank scanline" command
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdBlank);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdBlank;		// DMA read address

		// shift to next scanline
		cmd += DISPHSTX_CMDLEN;
	}

	// vertical VSYNC scanlines
	for (i = DISPHSTX_VSYNC; i > 0; i--)
	{
		// "blank scanline" command
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdVSync);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdVSync;		// DMA read address

		// shift to next scanline
		cmd += DISPHSTX_CMDLEN;
	}

	// vertical back porch scanlines
	for (i = DISPHSTX_VBACK; i > 0; i--)
	{
		// "blank scanline" command
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdBlank);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdBlank;		// DMA read address

		// shift to next scanline
		cmd += DISPHSTX_CMDLEN;
	}

	// add DMA NULL stop mark
	cmd[0] = DISPHSTX_CTRL_STOP;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	// disable HSTX
	hstx_ctrl_hw->csr = 0;			// disable HSTX

	// setup expand shift register
	hstx_ctrl_hw->expand_shift = 
			(1 << 24) |		// number of times to shift before refilling TMDS
			(0 << 16) |		// number of bits to rotate right TMDS = color depth
			(2 << 8) |		// number of times to shift before refilling raw data (applies only in repeat mode)
			(16 << 0);		// number of bits to rotate right raw data (applies in both repeat and single mode)

	// serial output config (output 16-bit pixel RGB565):
	// - clock period 5 cycles
	// - pop from command expander every 5 cycles
	// - no shift
	hstx_ctrl_hw->csr =
			(5 << 28) |		// clock period of the generated clock, 5 = generate complete output period (10 half-clocks)
			(5 << 16) |		// number of times to shift the shift register before refilling = 5 (one TMDS word is 2*5=10 bits)
			(0 << 8) |		// number of bits to right-rotate shift register by each clock cycle = 0
			(1 << 1) |		// enable command expander
			1;			// enable HSTX

	// setup HSTX outputs
//#define DISPHSTX_VGA_B0		12	// GPIO pin to output B0 (use 12..19)
//#define DISPHSTX_VGA_B1		13	// GPIO pin to output B1 (use 12..19)
//#define DISPHSTX_VGA_G0		14	// GPIO pin to output G0 (use 12..19)
//#define DISPHSTX_VGA_G1		15	// GPIO pin to output G1 (use 12..19)
//#define DISPHSTX_VGA_R0		16	// GPIO pin to output R0 (use 12..19)
//#define DISPHSTX_VGA_R1		17	// GPIO pin to output R1 (use 12..19)
//#define DISPHSTX_VGA_HSYNC		18	// GPIO pin to output HSYNC (use 12..19)
//#define DISPHSTX_VGA_VSYNC		19	// GPIO pin to output VSYNC (use 12..19)
// Pixel format:
//	15 14 13 12 11 10  9  8  7  6  5  4  3  2  1  0
//	R4 R3 R2 R1 R0 G5 G4 G3 G2 G1  V B4 B3 B2 B1  H
#define HSTX_BIT(bit) (bit|(bit<<8))
	hstx_ctrl_hw->bit[DISPHSTX_VGA_B0-12] = HSTX_BIT(3);		// output B0 (B3 from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_B1-12] = HSTX_BIT(4);		// output B1 (B4 from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_G0-12] = HSTX_BIT(9);		// output G0 (G4 from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_G1-12] = HSTX_BIT(10);		// output G1 (G5 from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_R0-12] = HSTX_BIT(14);		// output R0 (R3 from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_R1-12] = HSTX_BIT(15);		// output R1 (R4 from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_HSYNC-12] = HSTX_BIT(DISPHSTX_VGA_HSYNCBIT) | B16; // output HSYNC, inverted (H from pixel)
	hstx_ctrl_hw->bit[DISPHSTX_VGA_VSYNC-12] = HSTX_BIT(DISPHSTX_VGA_VSYNCBIT) | B16; // output VSYNC, inverted (V from pixel)
#undef HSTX_BIT

	// set GPIOs to HSTX mode
	GPIO_FncMask(PinRangeMask(12, 19), GPIO_FNC_HSTX);

	// set output strength
	for (i = 12; i <= 19; i++) GPIO_Drive8mA(i); // 2mA, 4mA, 8mA, 12mA

	// setup DMA command channel (no trigger yet)
	DMA_Config(
		DISPHSTX_DMA_CMD,			// channel
		DispHstxCmdBuf,				// read address
		&dma_hw->ch[DISPHSTX_DMA_DATA].al3_ctrl, // write address = DMA data channel (alias: CTRL, WRITE, CNT, READ)
		4,					// number of transfers = 4 (4x u32)
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ_FORCE |		// permanent request
			DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) | // disable chaining
			DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(4) | 	// ring size = 16 bytes
			// DMA_CTRL_INC_WRITE_REV |	// write address is 1 = decremented, 0 = incremented
			DMA_CTRL_INC_WRITE |		// increment write
			// DMA_CTRL_INC_READ_REV |	// read address is 1 = decremented, 0 = incremented
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			DMA_CTRL_HIGH_PRIORITY |	// high priority
			DMA_CTRL_EN);			// enable DMA

	// setup DMA IRQ
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);		// clear pending IRQ
	DMA_IRQ1Enable(DISPHSTX_DMA_DATA);		// enable IRQ from data DMA on NULL
	SetHandler(IRQ_DMA_1, DispHstxIrqHandler);	// set DMA IRQ handler
	NVIC_IRQEnable(IRQ_DMA_1);			// enable IRQ
	NVIC_IRQPrio(IRQ_DMA_1, IRQ_PRIO_REALTIME);	// set highest IRQ priority

	// flag - HSTX VGA mode is active
	DispHstxVgaRunning = True;

	// start first DMA transfer
	DMA_SetReadTrig(DISPHSTX_DMA_CMD, DispHstxCmdBuf);
}

// terminate driver
void DispHstxTerm()
{
	// driver is not active
	if (!DispHstxDviRunning && !DispHstxVgaRunning) return;

	// abort DMA channels (interrupt data channel first and then control channel)
	DMA_Abort(DISPHSTX_DMA_DATA); // pre-abort, could be chaining right now
	DMA_Abort(DISPHSTX_DMA_CMD);
	DMA_Abort(DISPHSTX_DMA_DATA);

	// disable IRQ1 from DMA
	NVIC_IRQDisable(IRQ_DMA_1);
	DMA_IRQ1Disable(DISPHSTX_DMA_DATA);

	// disable DMA channels
	DMA_Disable(DISPHSTX_DMA_DATA);
	DMA_Disable(DISPHSTX_DMA_CMD);

	// Clear the interrupt request for DMA control channel
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);

	// disable HSTX
	hstx_ctrl_hw->csr = 0;
	WaitUs(100);
	ResetPeriphery(RESET_HSTX);

	// set GPIOs to default mode
	GPIO_ResetMask(PinRangeMask(12, 19));

	// flag - HSTX mode is not active
	DispHstxDviRunning = False;
	DispHstxVgaRunning = False;
}

// ============================================================================
//                             DVI (HDMI) driver
// ============================================================================

// initialize DVI (HDMI)
void DispHstxDviInit()
{
	int i, j;
	u32 *hsync, *blank, *vsync, *cmd;
	u16 *src;

	// prepare "HSYNC" command before active data
	hsync = DispHstxCmdHSync;
	hsync[0] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HFRONT;	// active pixels, horizontal front porch
	hsync[1] = HDMI_SYNC_V0H0;				// blank
	hsync[2] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HSYNC;	// HSYNC
	hsync[3] = HDMI_SYNC_V0H1;				// HSYNC
	hsync[4] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HBACK;	// horizontal back porch
	hsync[5] = HDMI_SYNC_V0H0;				// blank
	hsync[6] = HSTX_CMD_TMDS | DISPHSTX_HACTIVE;		// start active data

	// prepare "blank scanline" command
	blank = DispHstxCmdBlank;
	blank[0] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HFRONT;	// horizontal front porch
	blank[1] = HDMI_SYNC_V0H0;				// blank
	blank[2] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HSYNC;	// HSYNC
	blank[3] = HDMI_SYNC_V0H1;				// HSYNC
	blank[4] = HSTX_CMD_RAW_REPEAT | (DISPHSTX_HBACK + DISPHSTX_HACTIVE); // horizontal back porch, active pixels
	blank[5] = HDMI_SYNC_V0H0;				// blank

	// prepare "VSYNC" command
	vsync = DispHstxCmdVSync;
	vsync[0] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HFRONT;	// horizontal front porch
	vsync[1] = HDMI_SYNC_V1H0;				// VSYNC+blank
	vsync[2] = HSTX_CMD_RAW_REPEAT | DISPHSTX_HSYNC;	// HSYNC
	vsync[3] = HDMI_SYNC_V1H1;				// VSYNC+HSYNC
	vsync[4] = HSTX_CMD_RAW_REPEAT | (DISPHSTX_HBACK + DISPHSTX_HACTIVE); // horizontal back porch, active pixels
	vsync[5] = HDMI_SYNC_V1H0;				// VSYNC+blank

	// prepare write address to HSTX FIFO
	u32 w = (u32)&hstx_fifo_hw->fifo;

	// fill-up scanline commands - active scanlines (480 scanlines)
	cmd = DispHstxCmdBuf;
	for (i = 0; i < DISPHSTX_VACTIVE; i++)
	{
		// buffer with source image data
#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
		src = DispBuf;
#else
		src = FrameBuf;
#endif

		// index of source line
#if USE_DISPHSTXMINI_VMODE == 5	// 144-line videomode
		j = i*144/480;	// j=0 0 0 1 1 1 2 2 2 3 ... 142 143 143 143
#else
		j = i/2;	// j=0 0 1 1 2 2 3 3 ... 238 238 239 239
#endif

		// address of source line
		src += j*WIDTH;

		// HSYNC command
		// - control word
		// - write address
		// - transfer count
		// - read address + trigger
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdHSync);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdHSync;		// DMA read address

		// data command ... every pixel (u16) is duplicated on bus
		cmd[4] = DISPHSTX_CTRL_DATA;		// DMA control word
		cmd[5] = w;				// DMA write address
		cmd[6] = WIDTH;				// DMA transfer count
		cmd[7] = (u32)src;			// DMA read address

		// shift to next scanline
		cmd += 2*DISPHSTX_CMDLEN;
	}

	// vertical front porch scanlines
	for (i = DISPHSTX_VFRONT; i > 0; i--)
	{
		// "blank scanline" command
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdBlank);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdBlank;		// DMA read address

		// shift to next scanline
		cmd += DISPHSTX_CMDLEN;
	}

	// vertical VSYNC scanlines
	for (i = DISPHSTX_VSYNC; i > 0; i--)
	{
		// "blank scanline" command
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdVSync);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdVSync;		// DMA read address

		// shift to next scanline
		cmd += DISPHSTX_CMDLEN;
	}

	// vertical back porch scanlines
	for (i = DISPHSTX_VBACK; i > 0; i--)
	{
		// "blank scanline" command
		cmd[0] = DISPHSTX_CTRL_SYNC;		// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = count_of(DispHstxCmdBlank);	// DMA transfer count
		cmd[3] = (u32)DispHstxCmdBlank;		// DMA read address

		// shift to next scanline
		cmd += DISPHSTX_CMDLEN;
	}

	// add DMA NULL stop mark
	cmd[0] = DISPHSTX_CTRL_STOP;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;

	// disable HSTX
	hstx_ctrl_hw->csr = 0;			// disable HSTX

	// setup expand shift register
	hstx_ctrl_hw->expand_shift = 
			(2 << 24) |		// number of times to shift before refilling TMDS
			(16 << 16) |		// number of bits to rotate right TMDS = color depth
			(1 << 8) |		// number of times to shift before refilling raw data
			(0 << 0);		// number of bits to rotate right raw data

	// setup expand tmds register
	hstx_ctrl_hw->expand_tmds =
			(4 << 21) |		// lane 2: get 5 bits
			(8 << 16) |		// lane 2: number of right-pre-rotate = 8 (shift red 8 bits right from position B11-B15 to position B3-B7)
			(5 << 13) |		// lane 1: get 6 bits
			(3 << 8)  |		// lane 1: number of right-pre-rotate = 3 (shift green 3 bits right from position B5-B10 to position B2-B7)
			(4 << 5)  |		// lane 0: get 5 bits
			(29 << 0);		// lane 0: number of right-pre-rotate = 32-3 = 29 (shift blue 3 bits left from position B0-B4 to position B3-B7)

	// serial output config (output 3 lane TMDS tokens, every token is 10 bits long):
	// - clock period 5 cycles
	// - pop from command expander every 5 cycles
	// - shift output shift reg by 2 every cycle
	hstx_ctrl_hw->csr =
			(5 << 28) |		// clock period of the generated clock, 5 = generate complete output period (10 half-clocks)
			(5 << 16) |		// number of times to shift the shift register before refilling = 5 (one TMDS word is 2*5=10 bits)
			(2 << 8) |		// number of bits to right-rotate shift register by each clock cycle = 2
			(1 << 1) |		// enable command expander
			1;			// enable HSTX

	// setup HSTX outputs
// #define DISPHSTX_DVI_D2P		16	// GPIO pin to output D2+ (use 12..19)
// #define DISPHSTX_DVI_D2M		17	// GPIO pin to output D2- (use 12..19)
// #define DISPHSTX_DVI_D1P		18	// GPIO pin to output D1+ (use 12..19)
// #define DISPHSTX_DVI_D1M		19	// GPIO pin to output D1- (use 12..19)
// #define DISPHSTX_DVI_D0P		12	// GPIO pin to output D0+ (use 12..19)
// #define DISPHSTX_DVI_D0M		13	// GPIO pin to output D0- (use 12..19)
// #define DISPHSTX_DVI_CLKP		14	// GPIO pin to output CLK+ (use 12..19)
// #define DISPHSTX_DVI_CLKM		15	// GPIO pin to output CLK- (use 12..19)
#define HSTX_BIT(lane) (((lane)*10)|(((lane)*10+1)<<8))
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D2P-12] = HSTX_BIT(2);		// output D2+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D2M-12] = HSTX_BIT(2)|B16;	// output D2-
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D1P-12] = HSTX_BIT(1);		// output D1+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D1M-12] = HSTX_BIT(1)|B16;	// output D1-
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D0P-12] = HSTX_BIT(0);		// output D0+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D0M-12] = HSTX_BIT(0)|B16;	// output D0-
	hstx_ctrl_hw->bit[DISPHSTX_DVI_CLKP-12] = B17;			// output CLK+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_CLKM-12] = B17|B16;		// output CLK-
#undef HSTX_BIT

	// set GPIOs to HSTX mode
	GPIO_FncMask(PinRangeMask(12, 19), GPIO_FNC_HSTX);

	// set output strength
	for (i = 12; i <= 19; i++) GPIO_Drive8mA(i); // 2mA, 4mA, 8mA, 12mA

	// setup DMA command channel (no trigger yet)
	DMA_Config(
		DISPHSTX_DMA_CMD,			// channel
		DispHstxCmdBuf,				// read address
		&dma_hw->ch[DISPHSTX_DMA_DATA].al3_ctrl, // write address = DMA data channel (alias: CTRL, WRITE, CNT, READ)
		4,					// number of transfers = 4 (4x u32)
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ_FORCE |		// permanent request
			DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) | // disable chaining
			DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(4) | 	// ring size = 16 bytes
			// DMA_CTRL_INC_WRITE_REV |	// write address is 1 = decremented, 0 = incremented
			DMA_CTRL_INC_WRITE |		// increment write
			// DMA_CTRL_INC_READ_REV |	// read address is 1 = decremented, 0 = incremented
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			DMA_CTRL_HIGH_PRIORITY |	// high priority
			DMA_CTRL_EN);			// enable DMA

	// setup DMA IRQ
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);		// clear pending IRQ
	DMA_IRQ1Enable(DISPHSTX_DMA_DATA);		// enable IRQ from data DMA on NULL
	SetHandler(IRQ_DMA_1, DispHstxIrqHandler);	// set DMA IRQ handler
	NVIC_IRQEnable(IRQ_DMA_1);			// enable IRQ
	NVIC_IRQPrio(IRQ_DMA_1, IRQ_PRIO_REALTIME);	// set highest IRQ priority

	// flag - HSTX DVI mode is active
	DispHstxDviRunning = True;

	// start first DMA transfer
	DMA_SetReadTrig(DISPHSTX_DMA_CMD, DispHstxCmdBuf);
}

// ============================================================================
//                             Common driver functions
// ============================================================================

// Terminate HSTX Mini display driver
void DispHstxAllTerm()
{
	// terminate driver
	DispHstxTerm();

#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	// terminate display selection switch
	DispHstxDispSelTerm();
#endif
}

// Initialize HSTX Mini display driver
//  dispmode ... display mode DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA; set to DISPHSTX_DISPMODE_NONE or 0 to auto-detect display mode selection
void DispHstxInit(int dispmode)
{
	// terminate all current devices
	DispHstxAllTerm();

	// auto-detect display mode
	if (dispmode == DISPHSTX_DISPMODE_NONE)
	{
		dispmode = DispHstxAutoDispSel();
	}

	// set new display mode
	DispHstxDispMode = dispmode;

	// setup clock
#if USE_DISPHSTXMINI_VMODE == 1 // 126 MHz
	ClockPllSysFreqVolt(126000); // set system clock to 126 MHz
	ClockSetup(CLK_HSTX, CLK_PLL_SYS, 126*MHZ, 0); // source HSTX from system clock
#elif USE_DISPHSTXMINI_VMODE == 2 // 252 MHz
	ClockPllSysFreqVolt(252000); // set system clock to 252 MHz
	ClockSetup(CLK_HSTX, CLK_PLL_SYS, 126*MHZ, 0); // source HSTX from system clock
#else // variable system clock
	PllSetFreq(PLL_USB, 3*48000); // set PLL USB clock to 3*48=144 MHz
	ClockSetup(CLK_USB, CLK_PLL_USB, 48*MHZ, 0); // setup CLK_USB to PLL_USB
	ClockSetup(CLK_ADC, CLK_PLL_USB, 48*MHZ, 0); // setup CLK_ADC to PLL_USB
	ClockSetup(CLK_PERI, CLK_PLL_USB, 0, 0); // setup CLK_PERI to PLL_USB
	ClockSetup(CLK_HSTX, CLK_PLL_USB, 0, 0); // setup CLK_HSTX to PLL_USB
#endif

	// initialize new devices (require initialized pDispHstxVMode descriptor)
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	if (dispmode == DISPHSTX_DISPMODE_DVI) DispHstxDviInit();
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if (dispmode == DISPHSTX_DISPMODE_VGA) DispHstxVgaInit();
#endif

#if USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
	// update all sound speeds after changing system clock
	SpeedSoundUpdate();
#endif
}

void (* volatile DispHstxCore1Fnc)() = NULL; // core 1 remote function

#define DISPHSTX_REQNO		0	// request - no
#define DISPHSTX_REQINIT	1	// request - init
#define DISPHSTX_REQTERM	2	// request - terminate
volatile int DispHstxReq = DISPHSTX_REQNO; // current request

// flag - HSTX core1 is running
volatile Bool DispHstxRunning = False;

void NOFLASH(DispHstxCore)()
{
	void (*fnc)();
	int req;

	// infinite loop
	while (True)
	{
		// data memory barrier
		dmb();

		// initialize/terminate
		req = DispHstxReq;
		if (req != DISPHSTX_REQNO)
		{
			if (req == DISPHSTX_REQINIT)
			{
				DispHstxInit(DispHstxDispMode); // initialize
			}
			else
			{
				DispHstxAllTerm(); // terminate
			}

			DispHstxReq = DISPHSTX_REQNO;
		}

		// execute remote function
		fnc = DispHstxCore1Fnc;
		if (fnc != NULL)
		{
			fnc();
			dmb();
			DispHstxCore1Fnc = NULL;
		}
	}
}

// execute core 1 remote function
void DispHstxCore1Exec(void (*fnc)())
{
	dmb();
	DispHstxCore1Fnc = fnc;
	dmb();
}

// check if core 1 is busy (executing remote function)
Bool DispHstxCore1Busy()
{
	dmb();
	return DispHstxCore1Fnc != NULL;
}

// wait if core 1 is busy (executing remote function)
void DispHstxCore1Wait()
{
	while (DispHstxCore1Busy()) {}
}

// start display driver on core 1 from core 0 (must be paired with DispHstxStop())
void DispHstxStart(int dispmode)
{
	dmb();
	if (DispHstxRunning) return;

	// required display mode
	DispHstxDispMode = dispmode;

	// flag - HSTX core1 is running
	DispHstxRunning = True;

	// run core
	dmb();
	Core1Exec(DispHstxCore);

	// initialize
	DispHstxReq = DISPHSTX_REQINIT;
	dmb();
	while (DispHstxReq != DISPHSTX_REQNO) { dmb(); }
}

// terminate display driver on core 1 from core 0 (must be paired with DispHstxStart())
void DispHstxStop()
{
	dmb();
	if (!DispHstxRunning) return;

	// terminate
	DispHstxReq = DISPHSTX_REQTERM;
	dmb();
	while (DispHstxReq != DISPHSTX_REQNO) { dmb(); }

	// core 1 reset
	Core1Reset();

	// flag - HSTX core1 is running
	DispHstxRunning = False;
}

// get current scanline (0..524: 0-479 active, 480-489 FP, 490-491 VSYNC, 492-524 BP)
int DispHstxScanline()
{
	// driver is not active
	if (!DispHstxDviRunning && !DispHstxVgaRunning) return DISPHSTX_VACTIVE;

	// get current DMA read address
	u32* s = (u32*)DMA_GetRead(DISPHSTX_DMA_CMD);

	// check if DMA read address is valid
	if ((s < DispHstxCmdBuf) || (s > &DispHstxCmdBuf[DISPHSTX_CMDNUM*DISPHSTX_CMDLEN])) return DISPHSTX_VACTIVE;

	// active area
	if (s < &DispHstxCmdBuf[DISPHSTX_VACTIVE*2*DISPHSTX_CMDLEN])
		return (s - DispHstxCmdBuf)/8;
	else
		return (s - &DispHstxCmdBuf[DISPHSTX_VACTIVE*2*DISPHSTX_CMDLEN])/4 + DISPHSTX_VACTIVE;
}

// check VSYNC
Bool DispHstxIsVSync()
{
	if (!DispHstxDviRunning && !DispHstxVgaRunning) return True;
	return DispHstxScanline() >= DISPHSTX_VACTIVE;
}

// wait for VSync scanline
void DispHstxWaitVSync()
{
	if (DispHstxDviRunning || DispHstxVgaRunning)
	{
		// wait for end of VSync
		while (DispHstxIsVSync()) { dmb(); }

		// wait for start of VSync
		while (!DispHstxIsVSync()) { dmb(); }
	}
}

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
// load back buffer from frame buffer
void DispLoad()
{
	DMA_MemCopy(FrameBuf, DispBuf, sizeof(DispBuf));
}

// update - send dirty window to display
void DispUpdate()
{
#if DISPHSTX_USE_VGA && USE_DISPHSTX_DISPBUF // 1=use VGA support (VGA requires about 30 KB of RAM)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_VGA)
	{
		u16* d = DispBuf;
		u16* s = FrameBuf;
		u32 k0, k1, k2, k3;
		int i;
		for (i = FRAMESIZE/4; i > 0; i--)
		{
			k0 = s[0]; k1 = s[1]; k2 = s[2]; k3 = s[3];
			s += 4;
			k0 &= DISPHSTX_VGA_MASK;
			k1 &= DISPHSTX_VGA_MASK;
			k2 &= DISPHSTX_VGA_MASK;
			k3 &= DISPHSTX_VGA_MASK;
			d[0] = k0; d[1] = k1; d[2] = k2; d[3] = k3;
			d += 4;
		}
	}
	else
#endif // DISPHSTX_USE_VGA
		DMA_MemCopy(DispBuf, FrameBuf, sizeof(DispBuf));
}

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms)
{
	// interval in [us]
	u32 us = ms*1000;

	// check interval from last update
	if ((u32)(Time() - DispAutoUpdateLast) >= us)
	{
		// update display
		DispUpdate();

		// start measure new time interval of running program
		DispAutoUpdateLast = Time();
	}
}
#endif // USE_DISPHSTX_DISPBUF

// LCD simulation: 320x240 pixels, 16-bit colors
u16* DispHstxDst = FrameBuf;

#if USE_EMUSCREENSHOT && USE_PICOPAD	// use emulator screen shots
#include "../../_devices/picopad/picopad_ss.h"
//volatile Bool DoEmuScreenShot = False;	// request to do emulator screenshot
#endif

// LOW level control: start sending image data to display window (DispSendImg() must follow)
//   Only window on full display is supported (parameters are ignored)
void DispStartImg(u16 x1, u16 x2, u16 y1, u16 y2)
{
#if USE_EMUSCREENSHOT && USE_PICOPAD	// use emulator screen shots
	if (DoEmuScreenShot)	// request to do emulator screenshot
	{
		DoEmuScreenShot = False;

		// open screenshot
		OpenScreenShot();
	}
#endif

#if USE_DISPHSTX_DISPBUF	// 1 = use DispBuf + FrameBuf
	DispHstxDst = DispBuf;
#else
	DispHstxDst = FrameBuf;
#endif
}

// LOW level control: send one word of image data to display (follows after DispStartImg())
void DispSendImg2(u16 data)
{
#if USE_EMUSCREENSHOT && USE_PICOPAD	// use emulator screen shots
	// write data to screenshot file
	WriteScreenShot(&data, 2);
#endif

	if (DispHstxDispMode == DISPHSTX_DISPMODE_VGA) data &= DISPHSTX_VGA_MASK;
	*DispHstxDst++ = data;
}

// LOW level control: stop sending image data (follows after DispStartImg() and DispSendImg())
void DispStopImg()
{
#if USE_EMUSCREENSHOT && USE_PICOPAD	// use emulator screen shots
	// close screenshot
	CloseScreenShot();
#endif
}

#endif // USE_DISPHSTXMINI

