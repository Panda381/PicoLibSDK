
// ****************************************************************************
//
//                                Mini-VGA
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


#if USE_MINIVGA					// use mini-VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

/*
Required configuration from device setup:
  USE_MINIVGA .. config of back buffers
  COLBITS ... number of output color bits 4, 8, 15 or 16
  WIDTH ... width of videomode
  ....(optional, default 0) VGA_PIO_OFF ... offset of VGA program in PIO memory
  VGA_PIO ... used PIO
  VGA_SM ... state machine
  VGA_DMA ... first DMA channel (use 2 channels)
  VGA_GPIO_FIRST ... GPIO of first color pin (colors are in order: blue, green, red)
  VGA_GPIO_NUM ... number of VGA color GPIOs, without HSYNC and VSYNC, including 1 pin if neede to be skipped
  VGA_GPIO_HSYNC ... VGA HSYNC GPIO (VGA requires VSYNC = HSYNC+1)
  ... VGA_GPIO_SKIP ... skip this pin; comment out this line if you don't want the VGA pin to skip
  ... (optional, default 0) VGA_USECSYNC ... 1=use only CSYNC signal instead of HSYNC (VSYNC not used)

Auto generated configuration:
  HEIGHT ... height of videomode
  PLL_KHZ ... PLL system frequency in kHz (default 125000 kHz)
  MINIVGA_CLKDIV ... SM divide clock ticks
  MINIVGA_CPP ... SM clock ticks per pixel
  MINIVGA_HSYNC ... horizontal sync clock ticks
  MINIVGA_HBACK ... horizontal back porch clock ticks
  MINIVGA_HFRONT ... horizontal front porch clock ticks
  MINIVGA_HTOTAL ... horizontal total clock ticks
  MINIVGA_VACT ... vertical active scanlines (= 2*HEIGHT)
  MINIVGA_VSYNC ... vertical sync scanlines
  MINIVGA_VBACK ... vertical back porch scanlines
  MINIVGA_VFRONT ... vertical front porch scanlines
  MINIVGA_VTOTAL ... vertical total scanlines (=vact + vsync + vback + vfront)
  MINIVGA_VMUL ... vertical multiplier (= vact / HEIGHT)
  COLTYPE ... type of color: u8 or u16
  FRAMETYPE ... type of frame entry: u8 or u16
  WIDTHLEN ... length of one line, in number of frame elements
  FRAMESIZE ... frame size of front buffer, in number of frame elements
  BACKBUFSIZE ... size of back buffer in number of frame types (0=not used)
  DISP_STRIP_NUM ... number of back strips
  DISP_STRIP_HEIGHT ... height of back buffer strip in number of lines
*/

#ifndef _MINIVGA_H
#define _MINIVGA_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MINIVGA_IRQTIME
#define MINIVGA_IRQTIME		0	// debug flag - measure delta time of VGA service
#endif
// Measured: IN=1 us, OUT=31 us

// frame buffer
//  Planar mode: Lines are interlaced in order blue, green and red.
extern ALIGNED FRAMETYPE FrameBuf[];

// back buffer
#if USE_FRAMEBUF && (BACKBUFSIZE > 0)
//  Planar mode: Lines are interlaced in order blue, green and red.
extern ALIGNED FRAMETYPE BackBuf[BACKBUFSIZE]; // back buffer strip
#endif

// display setup
extern FRAMETYPE* pDrawBuf;	// current draw buffer
extern int DispStripInx;	// current index of back buffer strip (-1 = use full FrameBuf)
extern int DispMinY;		// minimal Y; base of back buffer strip
extern int DispMaxY;		// maximal Y + 1; end of back buffer strip

// dirty window to update (used only with full back buffer, USE_MINIVGA = 2)
extern int DispDirtyX1, DispDirtyX2, DispDirtyY1, DispDirtyY2;

// handler variables
extern volatile int VgaScanLine; // current processed scan line 0... (next displayed scan line)
extern volatile u32 VgaFrame;	// frame counter

#if MINIVGA_IRQTIME			// debug flag - measure delta time of VGA service
extern volatile u32 VgaTimeIn;		// time in interrupt service, in [us]
extern volatile u32 VgaTimeOut;		// time out interrupt service, in [us]
#endif // MINIVGA_IRQTIME

#ifdef VGA_KEYSCAN_GPIO		// scan keyboard
extern volatile u32 VgaKeyScan;	// key scan
#endif

// last system time of auto update
extern u32 DispAutoUpdateLast;

// initialize VGA (required system clock CLK_SYS to be set to PLL_KHZ) ... use VgaStart() to start on core 1
void VgaInit();

// terminate VGA ... use VgaStop() to stop on core 1
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

// start VGA on core 1 from core 0 (must be paired with VgaStop())
void VgaStart();

// terminate VGA on core 1 from core 0 (must be paired with VgaStart())
void VgaStop();

#if USE_DOUBLE		// use double support
// retune VGA to different system frequency (freq = new system frequency in Hz)
void VgaRetune(int freq);
#endif

// use back buffer
#if BACKBUFSIZE > 0

// set strip of back buffer (-1 = use full FrameBuffer)
void DispSetStrip(int inx);
INLINE void DispSetStripNext() { DispSetStrip(DispStripInx + 1); }

// switch off the back buffer, use only frame buffer to output
INLINE void DispSetStripOff() { DispSetStrip(-1); }

// load back buffer from frame buffer
void DispLoad();

// use full back buffer
#if USE_MINIVGA == 2

// set dirty all frame buffer
void DispDirtyAll();

// set dirty none (clear after update)
void DispDirtyNone();

// update dirty area by rectangle (check valid range)
void DispDirtyRect(int x, int y, int w, int h);

// update dirty area by pixel (check valid range)
void DispDirtyPoint(int x, int y);

#else // USE_MINIVGA == 2

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

#endif // USE_MINIVGA == 2

// update - send dirty window to display (or write back buffer to frame buffer)
void DispUpdate();

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms);

// refresh update all display
void DispUpdateAll();

// no back bufer
#else // BACKBUFSIZE > 0

// set strip of back buffer (-1 = use full FrameBuffer)
INLINE void DispSetStrip(int inx) {}
INLINE void DispSetStripNext() {}

// switch off the back buffer, use only frame buffer to output
INLINE void DispSetStripOff() {}

// load back buffer from frame buffer
INLINE void DispLoad() {}

// set dirty all frame buffer
INLINE void DispDirtyAll() {}

// set dirty none (clear after update)
INLINE void DispDirtyNone() {}

// update dirty area by rectangle (check valid range)
INLINE void DispDirtyRect(int x, int y, int w, int h) {}

// update dirty area by pixel (check valid range)
INLINE void DispDirtyPoint(int x, int y) {}

// update - send dirty window to display (or write back buffer to frame buffer)
INLINE void DispUpdate() {}

// auto update after delta time in [ms] of running program
INLINE void DispAutoUpdate(u32 ms) {}

// refresh update all display
INLINE void DispUpdateAll() {}

#endif // BACKBUFSIZE > 0

#ifdef __cplusplus
}
#endif

#endif // _MINIVGA_H

#endif // USE_MINIVGA
