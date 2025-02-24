
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                             HSTX VGA driver
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

// Low resistor (B0, G0, R0): 820 Ohms (more precise 800 Ohms)
// High resistor (B1, G1, R1): 390 Ohms (more precise 400 Ohms)
// HSYNC, VSYNC resistor: 100 Ohms
// Output RC filter: 220pF capacitor

// outputs GPIO12-19: B0, B1, G0, G1, R0, R1, HSYNC, VSYNC

#if USE_DISPHSTX		// 1=use HSTX Display driver

#ifndef _DISPHSTX_VGA_H
#define _DISPHSTX_VGA_H

#ifdef __cplusplus
extern "C" {
#endif

// VGA output connection:
//	R1 on channel bit 0: 820 ohms (more precise 800 ohms)
//	R2 on channel bit 1: 390 ohms (mode precise 400 ohms)
//	C on channel output: about 150-220 pF
//
// output voltage at load resistor 75 ohms (2 bits per channel):
//		bit1	bit0	out [V]	relative
//	0:	0	0	0.000	0.000	u0
//	1:	0	1	0.234	0.334	u1
//	2:	1	0	0.468	0.669	u2
//	3:	1	1	0.702	1.003	u3
//
// HSTX output shift register CSR setup: 5 (or 10) shifts right by 16 bits
// HSTX clock = sys_clock
//
// Output pin mapping to HSTX bits BIT*:
//	GPIO12	blue0	SEL_P=0, SEL_N=1
//	GPIO13	blue1	SEL_P=2, SEL_N=3
//	GPIO14	green0	SEL_P=4, SEL_N=5
//	GPIO15	green1	SEL_P=6, SEL_N=7
//	GPIO16	red0	SEL_P=8, SEL_N=9
//	GPIO17	red1	SEL_P=10, SEL_N=11
//	GPIO18	HSYNC	SEL_P=12, SEL_N=12
//	GPIO19	VSYNC	SEL_P=13, SEL_N=13
// - Recommended to keep the order of GPIO pins mentioned, because it allows possible output of VGA signal via PIO.
//
// Ouput 32-bit word:
//	31  29  27  25  23  21  19  17  15  13  11   9   7   5   3   1
//	  30  28  26  24  22  20  18  16  14  12  10   8   6   4   2   0
//
//	 .   V  R1  R0  G1  G0  B1  B0   .   V  R1  R0  G1  G0  B1  B0  
//	   .   H  R1  R0  G1  G0  B1  B0   .   H  R1  R0  G1  G0  B1  B0
//
// 7 levels using PWM on 2 sub-clocks:
//		bit1N	bit1P	bit0N	bit0P	code	relative
//	0:	0	0	0	0	0x00	0.000	= u0		q0
//	1:	0	0	0	1	0x01	0.167	= u1/2		q1
//	  or	0	0	1	0	0x02	0.167	= u1/2
//	2:	0	0	1	1	0x03	0.334	= u1		q2
//	3:	0	1	1	0	0x06	0.502	= (u1+u2)/2	q3
//	  or	1	0	0	1	0x09	0.502	= (u1+u2)/2
//	4:	1	1	0	0	0x0C	0.669	= u2		q4
//	5:	1	1	0	1	0x0D	0.836	= (u2+u3)/2	q5
//	  or	1	1	1	0	0x0E	0.836	= (u2+u3)/2
//	6:	1	1	1	1	0x0F	1.003	= u3		q6

// 16 levels using PWM on 5 repeats of 2 codes:
//   relative = (q_codeH*2 + q_codeL*3)/5
//		codeH (2x)	codeL (3x)	relative	1-bit	2-bits	3-bits
//	0:	0x00 (q0)	0x00 (q0)	0.000		*	*	*
//	1:	0x00 (q0)	0x01 (q1)	0.098
//	2:	0x01 (q1)	0x01 (q1)	0,163				*
//	3:	0x03 (q2)	0x01 (q1)	0,228
//	4:	0x01 (q1)	0x03 (q2)	0,261
//	5:	0x03 (q2)	0x03 (q2)	0,326			*	*
//	6:	0x09 (q3)	0x03 (q2)	0,398
//	7:	0x03 (q2)	0x09 (q3)	0,433				*
//	8:	0x09 (q3)	0x09 (q3)	0,505
//	9:	0x0C (q4)	0x09 (q3)	0,577				*
//	10:	0x09 (q3)	0x0C (q4)	0,613
//	11:	0x0C (q4)	0x0C (q4)	0,685			*	*
//	12:	0x0D (q5)	0x0C (q4)	0,750
//	13:	0x0C (q4)	0x0D (q5)	0,783
//	14:	0x0D (q5)	0x0D (q5)	0,848				*
//	15:	0x0F (q6)	0x0F (q6)	1,010		*	*	*

// (0) 00 00: H=0000000000 L=0000000000
// (1) 00 01: H=0000000000 L=0100010001
// (2) 01 01: H=0000000000 L=0101010101
// (3) 03 01: H=0000000000 L=0111011101
// (4) 01 03: H=0000000000 L=1101110111
// (5) 03 03: H=0000000000 L=1111111111
// (6) 09 03: H=0010001000 L=1101110111
// (7) 03 09: H=1000100010 L=0111011101
// (8) 09 09: H=1010101010 L=0101010101

/*
// simple direct test VGA output
//  GPIO 12..19: blue 0+1, green 0+1, red 0+1, HSYNC, VSYNC
//  [us] time: hactive=25, hfront=1, hsync=4, hback=2
//  lines time: vactive=480, vfront=10, vsync=2, vback=33
void VgaTest() {
  int i; di();
  GPIO_InitMask(PinRangeMask(12, 19));
  GPIO_DirOutMask(PinRangeMask(12, 19));
  while (True) {
    for (i = 480; i > 0; i--) {
      GPIO_SetMask(PinRangeMask(12, 19)); WaitUs(25);
      GPIO_ClrMask(PinRangeMask(12, 17)); WaitUs(1);
      GPIO_Out0(18); WaitUs(4);
      GPIO_Out1(18); WaitUs(2); }
    for (i = 10; i > 0; i--) { WaitUs(26);
      GPIO_Out0(18); WaitUs(4);
      GPIO_Out1(18); WaitUs(2); }
    GPIO_Out0(19);
    for (i = 2; i > 0; i--) { WaitUs(26);
      GPIO_Out0(18); WaitUs(4);
      GPIO_Out1(18); WaitUs(2); }
    GPIO_Out1(19);
    for (i = 33; i > 0; i--) { WaitUs(26);
      GPIO_Out0(18); WaitUs(4);
      GPIO_Out1(18); WaitUs(2); }
  }
}
*/

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

// HSYNC signal, level 0..1
#define HS_0	0
#define HS_1	(B12+B28)

// VSYNC signal, level 0..1
#define VS_0	0
#define VS_1	(B13+B29)

#define VGA_SYNC_V0_H0	(VS_0 | HS_0)	// VSYNC=0, HSYNC=0
#define VGA_SYNC_V0_H1	(VS_0 | HS_1)	// VSYNC=0, HSYNC=1
#define VGA_SYNC_V1_H0	(VS_1 | HS_0)	// VSYNC=1, HSYNC=0
#define VGA_SYNC_V1_H1	(VS_1 | HS_1)	// VSYNC=1, HSYNC=1

// convert 5-bit level 0..31 to VGA 32-bit code (with base on bit 0)
// - Do not use 'const', need to be in faster SRAM
// - Use index+0 with even frame, use index+32 with odd frame
extern u32 DispHstxVgaLev5[32*2];

#if DISPHSTX_USE_FORMAT_12
// convert 4-bit level 0..15 to VGA 32-bit code (with base on bit 0)
// - Do not use 'const', need to be in faster SRAM
// - Use index+0 with even frame, use index+16 with odd frame
extern u32 DispHstxVgaLev4[16*2];
#endif

// VGA palettes (double format with time dithering) ... requires 11 KB of RAM
extern u32 DispHstxPalVga1b[2*2];	// 1-bit, DISPHSTX_FORMAT_1 format, palettes DispHstxPal1b[2]
extern u32 DispHstxPalVga2b[4*2];	// 2-bit, DISPHSTX_FORMAT_2 format, palettes DispHstxPal2b[4]
extern u32 DispHstxPalVga2bcol[4*2];	// 2-bit palettes color, palettes DispHstxPal2bcol[4]
extern u32 DispHstxPalVga3b[8*2];	// 3-bit, DISPHSTX_FORMAT_3 format, palettes DispHstxPal3b[8]
extern u32 DispHstxPalVga4b[16*2];	// 4-bit, DISPHSTX_FORMAT_4 format, palettes DispHstxPal4b[16]
extern u32 DispHstxPalVga4bcol[16*2];	// 4-bit palettes YRGB1111, palettes DispHstxPal4bcol[16]
extern u32 DispHstxPalVga6b[64*2];	// 6-bit, DISPHSTX_FORMAT_6 format, palettes DispHstxPal6b[64]
extern u32 DispHstxPalVga8b[256*2];	// 8-bit, DISPHSTX_FORMAT_8 format, palettes DispHstxPal8b[256]

#if DISPHSTX_USE_FORMAT_15 || DISPHSTX_USE_FORMAT_16 || DISPHSTX_USE_FORMAT_HSTX_15 || DISPHSTX_USE_FORMAT_HSTX_16
extern u32 DispHstxPalVgaRG[1024*2];	// 2*10-bit, even and odd, convert RG components of RGB565/RGB555 to VGA 32-bit (convert B with DispHstxVgaLev5[])
#endif

#if DISPHSTX_USE_FORMAT_12
extern u32 DispHstxPalVgaRG12[256*2];	// 2*8-bit, even and odd, convert RG components of RGB444 to VGA 32-bit (convert B with DispHstxVgaLev4[])
#endif

// convert 16-bit color in RGB565 format to VGA 32-bit palette (frame 0=even, 1=odd)
u32 DispHstxVgaPal1(u16 col, int frame);

// convert table of 16-bit colors in RGB565 format to VGA 32-bit palettes
void DispHstxVgaPal(u32* dst, const u16* src, int num);

// convert table of 16-bit colors in RGB565 format to VGA 32-bit palettes - double format with time dithering
void DispHstxVgaPal2(u32* dst, const u16* src, int num);

// prepare VGA tables
void DispHstxVgaTabSetup();

// flag - HSTX VGA mode is active
extern Bool DispHstxVgaRunning;

// prepare videomode state descriptor before start HSTX VGA mode
void DispHstxVgaPrepare(sDispHstxVModeState* v);

// initialize HSTX VGA mode
void DispHstxVgaInit();

// exchange videomode state descriptor, without terminating HSTX VGA mode
// - HSTX VGA mode must be started using DispHstxVgaInit()
// - videomode state descriptor must be prepared using DispHstxVgaPrepare()
void DispHstxVgaExchange(sDispHstxVModeState* v);

// terminate HSTX VGA mode
void DispHstxVgaTerm();

// execute core 1 remote function
void DispHstxVgaCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool DispHstxVgaCore1Busy();

// wait if core 1 is busy (executing remote function)
void DispHstxVgaCore1Wait();

// start VGA on core 1 from core 0 (must be paired with DispHstxVgaStop())
void DispHstxVgaStart();

// terminate VGA on core 1 from core 0 (must be paired with DispHstxVgaStart())
void DispHstxVgaStop();

#else // DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

#define DispHstxPalVga1b	NULL
#define DispHstxPalVga2b	NULL
#define DispHstxPalVga2bcol	NULL
#define DispHstxPalVga3b	NULL
#define DispHstxPalVga4b	NULL
#define DispHstxPalVga4bcol	NULL
#define DispHstxPalVga6b	NULL
#define DispHstxPalVga8b	NULL

#endif // DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_VGA_H

#endif // USE_DISPHSTX
