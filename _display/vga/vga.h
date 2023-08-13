
// ****************************************************************************
//
//                                  VGA
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

#if USE_VGA		// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)

#ifndef _VGA_H
#define _VGA_H

#ifdef __cplusplus
extern "C" {
#endif

// QVGA horizonal timings
#define VGA_CLKDIV	2	// SM divide clock ticks
#define VGA_CPP	5	// state machine clock ticks per pixel
//#define VGA_SYSCPP	(VGA_CLKDIV*VGA_CPP) // system clock ticks per pixel (=10)

// 126 MHz timings (126000000 Hz)
/*
#define VGA_VCO	(1008*1000*1000) // PLL VCO frequency in Hz
#define VGA_FBDIV	84	// PLL FBDIV
#define VGA_PD1	4	// PLL PD1
#define VGA_PD2	2	// PLL PD2
*/
#define VGA_TOTAL	2002	// total clock ticks (= VGA_HSYNC + VGA_BP + WIDTH*VGA_CPP[1600] + VGA_FP)
#define VGA_HSYNC	240	// horizontal sync clock ticks
#define VGA_BP		121	// back porch clock ticks
#define VGA_FP		41	// front porch clock ticks

// VGA vertical timings
#define VGA_VTOT	525	// total scanlines (= VGA_VSYNC + VGA_VBACK + VGA_VACT + VGA_VFRONT)
#define VGA_VSYNC	2	// length of V sync (number of scanlines)
#define VGA_VBACK	33	// V back porch
#define VGA_VACT	480	// V active scanlines (= 2*HEIGHT)
#define VGA_VFRONT	10	// V front porch

// default timings (125 MHz)
//#define VGA_DEF_VCO	(1500*1000*1000) // PLL VCO frequency in Hz
//#define VGA_DEF_FBDIV	125	// PLL FBDIV
//#define VGA_DEF_PD1	4	// PLL PD1
//#define VGA_DEF_PD2	3	// PLL PD2

// frame buffer in BGR565 pixel format
extern ALIGNED u16 FrameBuf[]; // display buffer (153 KB)

#if USE_VGA == 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)

// no back buffer
#define BACKBUFSIZE		0	// size of back buffer (in number of pixels)
#define VGA_STRIP_NUM		1	// number of back strips

#elif USE_VGA == 2

// 2 strips of back buffer
#define BACKBUFSIZE		(FRAMESIZE/2)	// size of back buffer (in number of pixels)
#define VGA_STRIP_NUM		2	// number of back buffer strips
extern ALIGNED u16 BackBuf[BACKBUFSIZE]; // back buffer strip (77 KB)

#else

// 4 strips of back buffer
#define BACKBUFSIZE		(FRAMESIZE/4)	// size of back buffer (in number of pixels)
#define VGA_STRIP_NUM		4	// number of back buffer strips
extern ALIGNED u16 BackBuf[BACKBUFSIZE]; // back buffer strip (38 KB)

#endif

// dirty window to update
/*
extern u8 DispRot;		// current display rotation
extern u16 DispWidth, DispHeight; // current display size
*/
#define DispRot	1		// current display rotation
#define DispWidth	WIDTH	// current display width
#define DispHeight	HEIGHT	// current display height

extern int DispDirtyX1, DispDirtyX2, DispDirtyY1, DispDirtyY2;

// strip of back buffer
extern u16* pDrawBuf; 		// current draw buffer
extern int DispStripInx;	// current index of back buffer strip (-1 = use full FrameBuf)
extern int DispMinY;		// minimal Y; base of back buffer strip
extern int DispMaxY;		// maximal Y + 1; end of back buffer strip

// initialize VGA (required system clock CLK_SYS to be set to 126 MHz)
void VgaInit();

// terminate VGA
void VgaTerm();

// execute core 1 remote function
void VgaCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool VgaCore1Busy();

// wait if core 1 is busy (executing remote function)
void VgaCore1Wait();

// check VSYNC
Bool VgaIsVSync();

// wait for VSync scanline
void VgaWaitVSync();

// start VGA (must be paired with VgaStop())
void VgaStart();

// terminate VGA (must be paired with VgaStart())
void VgaStop();

// set strip of back buffer (-1 = use full FrameBuffer)
void DispSetStrip(int inx);
INLINE void DispSetStripNext() { DispSetStrip(DispStripInx + 1); }

// switch off the back buffer, use only frame buffer to output
INLINE void DispSetStripOff() { DispSetStrip(-1); }

// load back buffer from frame buffer
#if USE_VGA > 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
void DispLoad();
#else
INLINE void DispLoad() {};
#endif

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

// update - send dirty window to display (or write back buffer to frame buffer)
#if USE_VGA > 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
void DispUpdate();
#else
INLINE void DispUpdate() {}
#endif

// auto update after delta time in [ms] of running program
INLINE void DispAutoUpdate(u32 ms) {}

// refresh update all display
INLINE void DispUpdateAll() {}

#ifdef __cplusplus
}
#endif

#endif // _VGA_H

#endif // USE_VGA		// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
