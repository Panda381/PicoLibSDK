
// ****************************************************************************
//                                 
//                             DVI configuration
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


// Color modes: 16 bits RGB565

#if USE_DVI					// use DVI (HDMI) display with simple frame buffer:

// --- 320*240 (*2, real 640*480)
//  VGA 4:3 640x480, vert. 60 Hz, hor. 31.4685 kHz, pixel clock 12.5874 MHz (real l , sys_clk 252 MHz
#if WIDTH == 320

#ifndef HEIGHT
#define HEIGHT 240			// display height
#endif
#if HEIGHT != 240
#error Incorrect HEIGHT!
#endif

#ifndef PLL_KHZ
#define PLL_KHZ		252000		// PLL system frequency in kHz (default 125000 kHz)
#endif

// horizontal ticks must be even number
//  HSYNC=negative, VSYNC=negative
#define DVI_HACT	640		// horizontal active pixels (visible area)
#define DVI_HFRONT	16		// horizontal front porch in pixels
#define DVI_HSYNC	96		// horizontal sync clock in pixels
#define DVI_HBACK	48		// horizontal back porch in pixels
#define DVI_HTOTAL	800		// horizontal total length in pixels (=hact + hfront + hsync + hback)

#define DVI_VACT	480		// vertical active scanlines (= 2*HEIGHT)
#define DVI_VFRONT	10		// vertical front porch scanlines
#define DVI_VSYNC	2		// vertical sync scanlines
#define DVI_VBACK	33		// vertical back porch scanlines
#define DVI_VTOTAL	525		// vertical total scanlines (=vact + vfront + vsync + vback)

#define DVI_HMUL	2		// horizontal multiplier (= hact / WIDTH)
#define DVI_VMUL	2		// vertical multiplier (= vact / HEIGHT)

// --- unsupported
#else
#error Unsupported WIDTH!
#endif

#define COLBITS 	16		// number of output color bits (4, 8, 15 or 16)
#define COLTYPE		u16		// type of color: u8 or u16
#define FRAMETYPE	u16		// type of frame entry: u8 or u16
#define WIDTHLEN	WIDTH		// length of one line, in number of frame elements

#define FRAMESIZE	(WIDTHLEN*HEIGHT)	// frame size of front buffer, in number of frame elements

//#define COLOR_B_BITS	5			// blue color bits
//#define COLOR_G_BITS	6			// greem color bits
//#define COLOR_R_BITS	5			// red color bits
//#define COLOR_B_MASK	0x001F			// blue color mask
//#define COLOR_G_MASK	0x07E0			// green color mask
//#define COLOR_R_MASK	0xF800			// red color mask
#define COLOR(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
//#define FRAME_ALIGN_DN(x)	(x)		// align X down, to whole FRAMETYPE
//#define FRAME_ALIGN_UP(x)	(x)		// align X up, to whole FRAMETYPE
//#define FRAME_SHIFTX	0			// number of shifts to get frame index from the X

// === prepare back buffer size

// 1=use only frame buffer
#if USE_DVI == 1
#define BACKBUFSIZE		0		// size of back buffer
#define DISP_STRIP_NUM		1		// number of back strips
#define DISP_STRIP_HEIGHT	HEIGHT		// height of back buffer strip in number of lines

// 2=add full back buffer
#elif USE_DVI == 2
#define BACKBUFSIZE		FRAMESIZE	// size of back buffer
#define DISP_STRIP_NUM		1		// number of back strips
#define DISP_STRIP_HEIGHT	HEIGHT		// height of back buffer strip in number of lines

// 3=add 1/2 back buffer
#elif USE_DVI == 3
#define BACKBUFSIZE		(FRAMESIZE/2)	// size of back buffer
#define DISP_STRIP_NUM		2		// number of back strips
#define DISP_STRIP_HEIGHT	(HEIGHT/2)	// height of back buffer strip in number of lines

#if (DISP_STRIP_HEIGHT*2) != HEIGHT
#error Unsupported back buffer size!
#endif

// 4=add 1/4 back buffer
#elif USE_DVI == 4
#define BACKBUFSIZE		(FRAMESIZE/4)	// size of back buffer
#define DISP_STRIP_NUM		4		// number of back strips
#define DISP_STRIP_HEIGHT	(HEIGHT/4)	// height of back buffer strip in number of lines

#if (DISP_STRIP_HEIGHT*4) != HEIGHT
#error Unsupported back buffer size!
#endif

// 5=add 1/8 back buffer
#elif USE_DVI == 5
#define BACKBUFSIZE		(FRAMESIZE/8)	// size of back buffer
#define DISP_STRIP_NUM		8		// number of back strips
#define DISP_STRIP_HEIGHT	(HEIGHT/8)	// height of back buffer strip in number of lines

#if (DISP_STRIP_HEIGHT*8) != HEIGHT
#error Unsupported back buffer size!
#endif

// unsupported
#else
#error Unsupported value of USE_DVI!
#endif

#endif // USE_DVI
