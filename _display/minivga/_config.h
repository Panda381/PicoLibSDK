
// ****************************************************************************
//                                 
//                           Mini-VGA configuration
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

// If want to use Mini-VGA display driver, add to _config.h of device setup:
// - set USE_MINIVGA to 1 to 5
// - set COLBITS to 4, 8, 15 or 16
// - set WIDTH to required videomode 256..1024
// - include this file


// Color modes:
//  16 bits RGB565, 15 bits RGB555 (1 pixel = u16; width must be multiple of 2)
//  8 bits RGB332 (1 pixel = u8; width must be multiple of 4)
//  4 bits YRGB1111 (1 pixel = 1/2 u8; width must be multiple of 8)


// Config parameters required to be set on INPUT:
//#define USE_MINIVGA		1		// use mini-VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer
//#define COLBITS		16		// number of color bits: bits 4, 8, 15 or 16
//#define WIDTH			320		// horizontal resolution:
						//	1) 256*192 (*4, real 1024*768)
						//	2) 320*240 (*2, real 640*480)
						//	3) 400*300 (*2, real 800*600) ... not 1/8 buffer
						//	4) 512*384 (*2, real 1024*768)
						//	5) 640*480
						//	6) 800*600

// Available combinations:
//  16 bits RGB565, 15 bits RGB555 (1 pixel = u16; width must be multiple of 2):
//	256*192 1 buffer 98 KB, 1+1 buffer 196 KB, 1+1/2 buffer 147 KB, 1+1/4 buffer 122 KB, 1+1/8 buffer 110 KB
//	320*240 1 buffer 153 KB, 1+1/2 buffer 230 KB, 1+1/4 buffer 192 KB, 1+1/8 buffer 172 KB
//	400*300 1 buffer 240 KB
//  8 bits RGB332 (1 pixel = u8; width must be multiple of 4):
//	256*192 1 buffer 49 KB, 1+1 buffer 98 KB, 1+1/2 buffer 73 KB, 1+1/4 buffer 61 KB, 1+1/8 buffer 55 KB
//	320*240 1 buffer 76 KB, 1+1 buffer 153 KB, 1+1/2 buffer 115 KB, 1+1/4 buffer 96 KB, 1+1/8 buffer 86 KB
//	400*300 1 buffer 120 KB, 1+1 buffer 240 KB, 1+1/2 buffer 180 KB, 1+1/4 buffer 150 KB
//	512*384 1 buffer 196 KB, 1+1/4 buffer 245 KB, 1+1/8 buffer 221 KB
//  4 bits YRGB1111 (1 pixel = 1/2 u8; width must be multiple of 8):
//	256*192 1 buffer 24 KB, 1+1 buffer 49 KB, 1+1/2 buffer 36 KB, 1+1/4 buffer 30 KB, 1+1/8 buffer 27 KB
//	320*240 1 buffer 38 KB, 1+1 buffer 76 KB, 1+1/2 buffer 57 KB, 1+1/4 buffer 48 KB, 1+1/8 buffer 43 KB
//	400*300 1 buffer 60 KB, 1+1 buffer 120 KB, 1+1/2 buffer 90 KB, 1+1/4 buffer 75 KB
//	512*384 1 buffer 98 KB, 1+1 buffer 196 KB, 1+1/2 buffer 147 KB, 1+1/4 buffer 122 KB, 1+1/8 buffer 110 KB
//	640*480 1 buffer 153 KB, 1+1/2 buffer 230 KB, 1+1/4 buffer 192 KB, 1+1/8 buffer 172 KB
//	800*600 1 buffer 240 KB

#if USE_MINIVGA					// use mini-VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer


// === prepare videomode timings
// http://tinyvga.com/vga-timing

// --- 256*192 (*4, real 1024*768)
//  XGA 4:3 1024x768, vert. 60 Hz, hor. 48.36310 kHz, pixel clock 65 MHz, sys_clk 130 MHz
#if WIDTH == 256

#ifndef HEIGHT
#define HEIGHT 192			// display height
#endif
#if HEIGHT != 192
#error Incorrect HEIGHT!
#endif

//  htotal = 20.67692	// total scanline in [us]
//  hfront = 0.36923	// H front porch (after image, before HSYNC) in [us]
//  hsync = 2.09231	// H sync pulse in [us]
//  hback = 2.46154	// H back porch (after HSYNC, before image) in [us]
//  hfull = 15.75385	// H full visible in [us]
//  wfull = 256		// H full width
//  sys_freq = 125 MHz	// base system frequency

//  freq = wfull / hfull = 16.25 MHz	// pixel clock
//  clkdiv*cpp = sys_freq / freq = 7.69 -> round up to 8	// system_clock / pixel_clock
//  clkdiv = 4		// SM divide clock ticks
//  cpp = 2		// SM clock ticks per pixel
//  sys_freq = clkdiv*cpp*freq = 130 MHz // real system frequency

//  hsync = hsync * sys_freq / clkdiv = 68	// horizontal sync clock ticks
//  hback = hback * sys_freq / clkdiv = 80	// back porch clock ticks
//  hfront = hfront * sys_freq / clkdiv = 12	// front porch clock ticks
//  htotal = hsync + hback + hfront + wfull*cpp = 672 	// total clock ticks
//  ... check htotal = htotal * sys_freq / clkdiv = 672

#define PLL_KHZ		130000		// PLL system frequency in kHz (default 125000 kHz)
#define MINIVGA_CLKDIV	2		// SM divide clock ticks
#define MINIVGA_CPP	2		// SM clock ticks per pixel
#define MINIVGA_HSYNC	136		// horizontal sync clock ticks
#define MINIVGA_HBACK	160		// horizontal back porch clock ticks
#define MINIVGA_HFRONT	24		// horizontal front porch clock ticks
#define MINIVGA_HTOTAL	1344		// horizontal total clock ticks
#define MINIVGA_VACT	768		// vertical active scanlines (= 4*HEIGHT)
#define MINIVGA_VSYNC	6		// vertical sync scanlines
#define MINIVGA_VBACK	29		// vertical back porch scanlines
#define MINIVGA_VFRONT	3		// vertical front porch scanlines
#define MINIVGA_VTOTAL	806		// vertical total scanlines (=vact + vsync + vback + vfront)
#define MINIVGA_VMUL	4		// vertical multiplier (= vact / HEIGHT)

// --- 320*240 (*2, real 640*480)
//  VGA 4:3 640x480, vert. 60 Hz, hor. 31.4685 kHz, pixel clock 12.5874 MHz, sys_clk 126 MHz
#elif WIDTH == 320

#ifndef HEIGHT
#define HEIGHT 240			// display height
#endif
#if HEIGHT != 240
#error Incorrect HEIGHT!
#endif

//  htotal = 31.77781	// total scanline in [us]
//  hfront = 0.63556	// H front porch (after image, before HSYNC) in [us]
//  hsync = 3.81334	// H sync pulse in [us]
//  hback = 1.90667	// H back porch (after HSYNC, before image) in [us]
//  hfull = 25.42224	// H full visible in [us]
//  wfull = 320		// H full width
//  sys_freq = 125 MHz	// base system frequency

//  freq = wfull / hfull = 12.587404 MHz	// pixel clock
//  clkdiv*cpp = sys_freq / freq = 9.93 -> round up 10	// system_clock / pixel_clock
//  clkdiv = 2		// SM divide clock ticks
//  cpp = 5		// SM clock ticks per pixel
//  sys_freq = clkdiv*cpp*freq = 125.87404 MHz -> round up 126 MHz // real system frequency

//  hsync = hsync * sys_freq / clkdiv = 240	// horizontal sync clock ticks
//  hback = hback * sys_freq / clkdiv = 121	// back porch clock ticks
//  hfront = hfront * sys_freq / clkdiv = 41	// front porch clock ticks
//  htotal = hsync + hback + hfront + wfull*cpp = 2002	// total clock ticks
//  ... check htotal = htotal * sys_freq / clkdiv = 2002 // correction hback 120->121, hfront 40->41

#if defined(PLL_KHZ) && ((PLL_KHZ == 252000) || (PLL_KHZ == 250000))
#define MINIVGA_CLKDIV	4		// SM divide clock ticks
#else
#define PLL_KHZ		126000		// PLL system frequency in kHz (default 125000 kHz)
#define MINIVGA_CLKDIV	2		// SM divide clock ticks
#endif

#define MINIVGA_CPP	5		// SM clock ticks per pixel
#define MINIVGA_HSYNC	240		// horizontal sync clock ticks
#define MINIVGA_HBACK	121		// horizontal back porch clock ticks
#define MINIVGA_HFRONT	41		// horizontal front porch clock ticks
#define MINIVGA_HTOTAL	2002		// horizontal total clock ticks
#define MINIVGA_VACT	480		// vertical active scanlines (= 2*HEIGHT)
#define MINIVGA_VSYNC	2		// vertical sync scanlines
#define MINIVGA_VBACK	33		// vertical back porch scanlines
#define MINIVGA_VFRONT	10		// vertical front porch scanlines
#define MINIVGA_VTOTAL	525		// vertical total scanlines (=vact + vsync + vback + vfront)
#define MINIVGA_VMUL	2		// vertical multiplier (= vact / HEIGHT)

// --- 400*300 (*2, real 800*600)
//  SVGA 4:3 800x600, vert. 60 Hz, hor. 37.897 kHz, pixel clock 20 MHz, sys_clk 140 MHz
#elif WIDTH == 400

#ifndef HEIGHT
#define HEIGHT 300			// display height
#endif
#if HEIGHT != 300
#error Incorrect HEIGHT!
#endif

//  htotal = 26.40000	// total scanline in [us]
//  hfront = 1.00000	// H front porch (after image, before HSYNC) in [us]
//  hsync = 3.20000	// H sync pulse in [us]
//  hback = 2.20000	// H back porch (after HSYNC, before image) in [us]
//  hfull = 20.00000	// H full visible in [us]
//  wfull = 400		// H full width
//  sys_freq = 125 MHz	// base system frequency

//  freq = wfull / hfull = 20 MHz	// pixel clock
//  clkdiv*cpp = sys_freq / freq = 6.25 -> round up 7	// system_clock / pixel_clock
//  clkdiv = 1		// SM divide clock ticks
//  cpp = 7		// SM clock ticks per pixel
//  sys_freq = clkdiv*cpp*freq = 140 MHz // real system frequency

//  hsync = hsync * sys_freq / clkdiv = 448	// horizontal sync clock ticks
//  hback = hback * sys_freq / clkdiv = 308	// back porch clock ticks
//  hfront = hfront * sys_freq / clkdiv = 140	// front porch clock ticks
//  htotal = hsync + hback + hfront + wfull*cpp = 3696 	// total clock ticks
//  ... check htotal = htotal * sys_freq / clkdiv = 3696

#define PLL_KHZ		140000		// PLL system frequency in kHz (default 125000 kHz)
#define MINIVGA_CLKDIV	1		// SM divide clock ticks
#define MINIVGA_CPP	7		// SM clock ticks per pixel
#define MINIVGA_HSYNC	448		// horizontal sync clock ticks
#define MINIVGA_HBACK	308		// horizontal back porch clock ticks
#define MINIVGA_HFRONT	140		// horizontal front porch clock ticks
#define MINIVGA_HTOTAL	3696		// horizontal total clock ticks
#define MINIVGA_VACT	600		// vertical active scanlines (= 2*HEIGHT)
#define MINIVGA_VSYNC	4		// vertical sync scanlines
#define MINIVGA_VBACK	23		// vertical back porch scanlines
#define MINIVGA_VFRONT	1		// vertical front porch scanlines
#define MINIVGA_VTOTAL	628		// vertical total scanlines (=vact + vsync + vback + vfront)
#define MINIVGA_VMUL	2		// vertical multiplier (= vact / HEIGHT)

// --- 512*384 (*2, real 1024*768)
//  XGA 4:3 1024x768, vert. 60 Hz, hor. 48.36310 kHz, pixel clock 32.5 MHz, sys_clk 130 MHz
#elif WIDTH == 512

#ifndef HEIGHT
#define HEIGHT 384			// display height
#endif
#if HEIGHT != 384
#error Incorrect HEIGHT!
#endif

//  htotal = 20.67692	// total scanline in [us]
//  hfront = 0.36923	// H front porch (after image, before HSYNC) in [us]
//  hsync = 2.09231	// H sync pulse in [us]
//  hback = 2.46154	// H back porch (after HSYNC, before image) in [us]
//  hfull = 15.75385	// H full visible in [us]
//  wfull = 512		// H full width
//  sys_freq = 125 MHz	// base system frequency

//  freq = wfull / hfull = 32.5 MHz	// pixel clock
//  clkdiv*cpp = sys_freq / freq = 3.85 -> round up to 4	// system_clock / pixel_clock
//  clkdiv = 2		// SM divide clock ticks
//  cpp = 2		// SM clock ticks per pixel
//  sys_freq = clkdiv*cpp*freq = 130 MHz // real system frequency

//  hsync = hsync * sys_freq / clkdiv = 136	// horizontal sync clock ticks
//  hback = hback * sys_freq / clkdiv = 160	// back porch clock ticks
//  hfront = hfront * sys_freq / clkdiv = 24	// front porch clock ticks
//  htotal = hsync + hback + hfront + wfull*cpp = 1344 	// total clock ticks
//  ... check htotal = htotal * sys_freq / clkdiv = 1344

#define PLL_KHZ		130000		// PLL system frequency in kHz (default 125000 kHz)
#define MINIVGA_CLKDIV	2		// SM divide clock ticks
#define MINIVGA_CPP	2		// SM clock ticks per pixel
#define MINIVGA_HSYNC	136		// horizontal sync clock ticks
#define MINIVGA_HBACK	160		// horizontal back porch clock ticks
#define MINIVGA_HFRONT	24		// horizontal front porch clock ticks
#define MINIVGA_HTOTAL	1344		// horizontal total clock ticks
#define MINIVGA_VACT	768		// vertical active scanlines (= 2*HEIGHT)
#define MINIVGA_VSYNC	6		// vertical sync scanlines
#define MINIVGA_VBACK	29		// vertical back porch scanlines
#define MINIVGA_VFRONT	3		// vertical front porch scanlines
#define MINIVGA_VTOTAL	806		// vertical total scanlines (=vact + vsync + vback + vfront)
#define MINIVGA_VMUL	2		// vertical multiplier (= vact / HEIGHT)

// --- 640*480
//  VGA 4:3 640x480, vert. 60 Hz, hor. 31.4685 kHz, pixel clock 25.175 MHz, sys_clk 126 MHz
#elif WIDTH == 640

#ifndef HEIGHT
#define HEIGHT 480			// display height
#endif
#if HEIGHT != 480
#error Incorrect HEIGHT!
#endif

//  htotal = 31,77781	// total scanline in [us]
//  hfront = 0.63556	// H front porch (after image, before HSYNC) in [us]
//  hsync = 3.81334	// H sync pulse in [us]
//  hback = 1.90667	// H back porch (after HSYNC, before image) in [us]
//  hfull = 25.42224	// H full visible in [us]
//  wfull = 640		// H full width
//  sys_freq = 125 MHz	// base system frequency

//  freq = wfull / hfull = 25.174808 MHz	// pixel clock
//  clkdiv*cpp = sys_freq / freq = 4.97 -> round up 5	// system_clock / pixel_clock
//  clkdiv = 1		// SM divide clock ticks
//  cpp = 5		// SM clock ticks per pixel
//  sys_freq = clkdiv*cpp*freq = 125.87404 MHz -> round up 126 MHz // real system frequency

//  hsync = hsync * sys_freq / clkdiv = 480	// horizontal sync clock ticks
//  hback = hback * sys_freq / clkdiv = 242	// back porch clock ticks
//  hfront = hfront * sys_freq / clkdiv = 82	// front porch clock ticks
//  htotal = hsync + hback + hfront + wfull*cpp = 4004	// total clock ticks
//  ... check htotal = htotal * sys_freq / clkdiv = 4004 // correction hback 240->242, hfront 80->82

#define PLL_KHZ		126000		// PLL system frequency in kHz (default 125000 kHz)
#define MINIVGA_CLKDIV	1		// SM divide clock ticks
#define MINIVGA_CPP	5		// SM clock ticks per pixel
#define MINIVGA_HSYNC	480		// horizontal sync clock ticks
#define MINIVGA_HBACK	242		// horizontal back porch clock ticks
#define MINIVGA_HFRONT	82		// horizontal front porch clock ticks
#define MINIVGA_HTOTAL	4004		// horizontal total clock ticks
#define MINIVGA_VACT	480		// vertical active scanlines (= HEIGHT)
#define MINIVGA_VSYNC	2		// vertical sync scanlines
#define MINIVGA_VBACK	33		// vertical back porch scanlines
#define MINIVGA_VFRONT	10		// vertical front porch scanlines
#define MINIVGA_VTOTAL	525		// vertical total scanlines (=vact + vsync + vback + vfront)
#define MINIVGA_VMUL	1		// vertical multiplier (= vact / HEIGHT)

// --- 800*600
//  SVGA 4:3 800x600, vert. 60 Hz, hor. 37.8788 kHz, pixel clock 40 MHz, sys_clk 160 MHz
#elif WIDTH == 800

#ifndef HEIGHT
#define HEIGHT 600			// display height
#endif
#if HEIGHT != 600
#error Incorrect HEIGHT!
#endif

//  htotal = 26.40000	// total scanline in [us]
//  hfront = 1.00000	// H front porch (after image, before HSYNC) in [us]
//  hsync = 3.20000	// H sync pulse in [us]
//  hback = 2.20000	// H back porch (after HSYNC, before image) in [us]
//  hfull = 20.00000	// H full visible in [us]
//  wfull = 800		// H full width
//  sys_freq = 125 MHz	// base system frequency

//  freq = wfull / hfull = 40 MHz	// pixel clock
//  clkdiv*cpp = sys_freq / freq = 3.125 -> round up 4	// system_clock / pixel_clock
//  clkdiv = 2		// SM divide clock ticks
//  cpp = 2		// SM clock ticks per pixel
//  sys_freq = clkdiv*cpp*freq = 160 MHz // real system frequency

//  hsync = hsync * sys_freq / clkdiv = 256	// horizontal sync clock ticks
//  hback = hback * sys_freq / clkdiv = 176	// back porch clock ticks
//  hfront = hfront * sys_freq / clkdiv = 80	// front porch clock ticks
//  htotal = hsync + hback + hfront + wfull*cpp = 2112 	// total clock ticks
//  ... check htotal = htotal * sys_freq / clkdiv = 2112

#define PLL_KHZ		160000		// PLL system frequency in kHz (default 125000 kHz)
#define MINIVGA_CLKDIV	2		// SM divide clock ticks
#define MINIVGA_CPP	2		// SM clock ticks per pixel
#define MINIVGA_HSYNC	256		// horizontal sync clock ticks
#define MINIVGA_HBACK	176		// horizontal back porch clock ticks
#define MINIVGA_HFRONT	80		// horizontal front porch clock ticks
#define MINIVGA_HTOTAL	2112		// horizontal total clock ticks
#define MINIVGA_VACT	600		// vertical active scanlines (= HEIGHT)
#define MINIVGA_VSYNC	4		// vertical sync scanlines
#define MINIVGA_VBACK	23		// vertical back porch scanlines
#define MINIVGA_VFRONT	1		// vertical front porch scanlines
#define MINIVGA_VTOTAL	628		// vertical total scanlines (=vact + vsync + vback + vfront)
#define MINIVGA_VMUL	1		// vertical multiplier (= vact / HEIGHT)

// --- unsupported
#else
#error Unsupported WIDTH!
#endif



// === prepare color type

#if COLBITS > 8
#define COLTYPE			u16		// type of color: u8 or u16
#define FRAMETYPE		u16		// type of frame entry: u8 or u16
#else
#define COLTYPE			u8		// type of color: u8 or u16
#define FRAMETYPE		u8		// type of frame entry: u8 or u16
#endif


// === prepare front buffer size and colors

// 16 bits RGB565 (1 pixel = u16)
#if COLBITS == 16
#define WIDTHLEN	WIDTH			// length of one line, in number of frame elements

#if (WIDTH & 0x01) != 0
#error WIDTH must be multiple of 2 with COLBITS=16!
#endif

#define COLOR_B_BITS	5			// blue color bits
#define COLOR_G_BITS	6			// greem color bits
#define COLOR_R_BITS	5			// red color bits
#define COLOR_B_MASK	0x001F			// blue color mask
#define COLOR_G_MASK	0x07E0			// green color mask
#define COLOR_R_MASK	0xF800			// red color mask
#define COLOR(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
#define FRAME_ALIGN_DN(x)	(x)		// align X down, to whole FRAMETYPE
#define FRAME_ALIGN_UP(x)	(x)		// align X up, to whole FRAMETYPE
#define FRAME_SHIFTX	0			// number of shifts to get frame index from the X


// 15 bits RGB555 (1 pixel = u16)
#elif COLBITS == 15
#define WIDTHLEN	WIDTH			// length of one line, in number of frame elements

#if (WIDTH & 0x01) != 0
#error WIDTH must be multiple of 2 with COLBITS=15!
#endif

#define COLOR_B_BITS	5			// blue color bits
#define COLOR_G_BITS	5			// greem color bits
#define COLOR_R_BITS	5			// red color bits
#define COLOR_B_MASK	0x001F			// blue color mask
#define COLOR_G_MASK	0x03E0			// green color mask
#define COLOR_R_MASK	0x7C00			// red color mask
#define COLOR(r,g,b)	((u16)( (((r)&0xf8)<<7) | (((g)&0xf8)<<2) | (((b)&0xf8)>>3) ))
#define FRAME_ALIGN_DN(x)	(x)		// align X down, to whole FRAMETYPE
#define FRAME_ALIGN_UP(x)	(x)		// align X up, to whole FRAMETYPE
#define FRAME_SHIFTX	0			// number of shifts to get frame index from the X


//  8 bits RGB332 (1 pixel = u8):
#elif COLBITS == 8
#define WIDTHLEN	WIDTH			// length of one line, in number of frame elements

#if (WIDTH & 0x03) != 0
#error WIDTH must be multiple of 4 with COLBITS=8!
#endif

#define COLOR_B_BITS	3			// blue color bits
#define COLOR_G_BITS	3			// greem color bits
#define COLOR_R_BITS	2			// red color bits
#define COLOR_B_MASK	0x03			// blue color mask
#define COLOR_G_MASK	0x1C			// green color mask
#define COLOR_R_MASK	0xE0			// red color mask
#define COLOR(r,g,b)	((u8)( ((r)&0xe0) | (((g)&0xe0)>>3) | (((b)&0xc0)>>6) ))
#define FRAME_ALIGN_DN(x)	(x)		// align X down, to whole FRAMETYPE
#define FRAME_ALIGN_UP(x)	(x)		// align X up, to whole FRAMETYPE
#define FRAME_SHIFTX	0			// number of shifts to get frame index from the X


//  4 bits YRGB1111 (1 pixel = 1/2 u8):
#elif COLBITS == 4
#define WIDTHLEN	(WIDTH/2)		// length of one line, in number of frame elements

#if (WIDTH & 0x07) != 0
#error WIDTH must be multiple of 8 with COLBITS=4!
#endif

#define COLOR_B_BITS	1			// blue color bits
#define COLOR_G_BITS	1			// greem color bits
#define COLOR_R_BITS	1			// red color bits
#define COLOR_Y_BITS	1			// intensity bits
#define COLOR_B_MASK	0x01			// blue color mask
#define COLOR_G_MASK	0x02			// green color mask
#define COLOR_R_MASK	0x04			// red color mask
#define COLOR_Y_MASK	0x08			// intensity mask
#define COLOR4(y,r,g,b) ((u8)( ((y)<<3) | ((r)<<2) | ((g)<<1) | (b) )) // components are 0 or 1
#define FRAME_ALIGN_DN(x)	((x) & ~1)	// align X down, to whole FRAMETYPE
#define FRAME_ALIGN_UP(x)	(((x)+1) & ~1)	// align X up, to whole FRAMETYPE
#define FRAME_SHIFTX	1			// number of shifts to get frame index from the X

// unsupported
#else
#error Unsupported COLBITS!
#endif

#define FRAMESIZE	(WIDTHLEN*HEIGHT)	// frame size of front buffer, in number of frame elements


// === prepare back buffer size

// 1=use only frame buffer
#if USE_MINIVGA == 1
#define BACKBUFSIZE		0		// size of back buffer
#define DISP_STRIP_NUM		1		// number of back strips
#define DISP_STRIP_HEIGHT	HEIGHT		// height of back buffer strip in number of lines

// 2=add full back buffer
#elif USE_MINIVGA == 2
#define BACKBUFSIZE		FRAMESIZE	// size of back buffer
#define DISP_STRIP_NUM		1		// number of back strips
#define DISP_STRIP_HEIGHT	HEIGHT		// height of back buffer strip in number of lines

// 3=add 1/2 back buffer
#elif USE_MINIVGA == 3
#define BACKBUFSIZE		(FRAMESIZE/2)	// size of back buffer
#define DISP_STRIP_NUM		2		// number of back strips
#define DISP_STRIP_HEIGHT	(HEIGHT/2)	// height of back buffer strip in number of lines

#if (DISP_STRIP_HEIGHT*2) != HEIGHT
#error Unsupported back buffer size!
#endif

// 4=add 1/4 back buffer
#elif USE_MINIVGA == 4
#define BACKBUFSIZE		(FRAMESIZE/4)	// size of back buffer
#define DISP_STRIP_NUM		4		// number of back strips
#define DISP_STRIP_HEIGHT	(HEIGHT/4)	// height of back buffer strip in number of lines

#if (DISP_STRIP_HEIGHT*4) != HEIGHT
#error Unsupported back buffer size!
#endif

// 5=add 1/8 back buffer
#elif USE_MINIVGA == 5
#define BACKBUFSIZE		(FRAMESIZE/8)	// size of back buffer
#define DISP_STRIP_NUM		8		// number of back strips
#define DISP_STRIP_HEIGHT	(HEIGHT/8)	// height of back buffer strip in number of lines

#if (DISP_STRIP_HEIGHT*8) != HEIGHT
#error Unsupported back buffer size!
#endif

// unsupported
#else
#error Unsupported value of USE_MINIVGA!
#endif

#endif // USE_MINIVGA			// use mini-VGA display with simple frame buffer: 1=use only frame buffer,
