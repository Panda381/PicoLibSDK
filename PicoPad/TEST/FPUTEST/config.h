
// ****************************************************************************
//                                 
//                        Project library configuration
//
// ****************************************************************************

#ifndef _CONFIG_H
#define _CONFIG_H

// *********
// At this place you can specify the switches and settings you want
// to change from the default configuration in config_def.h.
// *********

//#define FONT			FontBold8x8	// default system font
//#define FONTW			8		// width of system font
//#define FONTH			8		// height of system font

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define USE_DISPHSTXMINI	0	// 1=use HSTX Display Mini driver
#define USE_DISPHSTX		1	// 1=use HSTX Display driver
//#define DISPHSTX_DISP_SEL	-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#define DISPHSTX_USE_DVI	1	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#define DISPHSTX_USE_VGA	1	// 1=use VGA support (VGA requires about 30 KB of RAM)

#define USE_DISPHSTXMINI_VMODE	3	// DispHstxMini videomode (default 1; data must be DISPHSTX_VGA_MASK masked in VGA case if only FrameBuf is used)
					//	1=320x240/16bit, FrameBuf+DispBuf, sys_clock=126 MHz
					//	2=320x240/16bit, FrameBuf+DispBuf, sys_clock=252 MHz
					//	3=320x240/16bit (borders 2x46 pixels), FrameBuf+DispBuf, sys_clock variable, clocked from USB 144 MHz
					//	4=320x240/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	5=320x144/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	6=360x240/16bit, only FrameBuf, sys_clock variable, clocked from USB 144 MHz

#define USE_DISPHSTX_VMODE	1	// DispHstx videomode (default 1)
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
#endif

//#define USE_ORIGSDK		1		// include interface of original-SDK

#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)
//#define USE_USB_STDIO		1		// use USB stdio (UsbPrint function)
//#define USE_UART_STDIO	1		// use UART stdio (UartPrint function)

//#define USE_ORIGSDK		1		// include interface of original-SDK
//#define USE_SCREENSHOT	1		// use screen shots

#define USE_FLOAT	1		// use Single-floating point 1=in RAM, 2=in Flash (sdk_float.c, sdk_float_asm.S, sdk_float.h)
#define USE_DOUBLE	1		// use Double-floating point 1=in RAM, 2=in Flash (sdk_double.c, sdk_double_asm.S, sdk_double.h)

#if RP2040 && USE_FLOATLIBC && (USE_FLOAT == 1)	// insufficient RAM with libc
#undef USE_FLOAT
#define USE_FLOAT	2
#endif

#if RP2040 && USE_DOUBLELIBC && (USE_DOUBLE == 1) // insufficient RAM with libc
#undef USE_DOUBLE
#define USE_DOUBLE	2
#endif

//#define USE_M33_VFP		1		// 1=enable VFP floating point instructions on RP2350-ARM, 0=emulate with DCP coprocessor
// float:
//#define USE_M33_SQRTF		1		// RP2350-ARM sqrtf() function: 1=fast (recommended), 2=precise
//#define USE_M33_FDIV		1		// RP2350-ARM fdiv() function: 1=fast (recommended), 2=precise
//#define USE_RISCV_SQRTF	1		// RISC-V sqrtf() function: 0=libc, 1=fast (recommended)
//#define USE_RISCV_EXPF	1		// RISC-V expf(), exp2f() and exp10f() function: 0=libc, 1=Chebyshev (recommended), 2=Taylor, 3=Cordic
//#define USE_RISCV_LOGF	1		// RISC-V logf(), log2f() and log10f() function: 0=libc, 1=Chebyshev (recommended), 2=Mercator, 3=Cordic
//#define USE_RISCV_SINF	1		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 0=libc, 1=Chebyshev, 2=Taylor (recommended), 3=Cordic
//#define USE_RISCV_ASINF	2		// RISC-V asinf(), acosf() functions: 0=libc, 2=Taylor (recommended), 3=Cordic
//#define USE_RISCV_ATANF	1		// RISC-V atanf(), acotanf(), atan2f() functions: 0=libc, 1=Chebyshev (recommended), 3=Cordic
// double:
//#define USE_M33_SQRT		1		// RP2350-ARM sqrt() function: 1=fast (recommended), 2=precise
//#define USE_M33_DDIV		1		// RP2350-ARM ddiv() function: 1=fast (recommended), 2=precise

//#define USE_USBPAD		1		// simulate keypad with USB keyboard
//#define USE_USBPORT		1		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
//#define USE_USB_DEV_CDC	4		// use USB CDC Communication Device Class, value = number of interfaces (device)
//#define USE_USB_DEV_HID	1		// use USB HID Human Interface Device, value = number of interfaces (device)
//#define USE_USB_HOST_CDC	4		// use USB CDC Communication Device Class, value = number of interfaces (host)
//#define USE_USB_HOST_HID	4		// use USB HID Human Interface Device, value = number of interfaces (host)

//#define USE_REAL16		1		// 1 = use real16 numbers (3 digits, exp +-4)
#define USE_REAL32		1		// 1 = use real32 numbers (float, 7 digits, exp +-38)
//#define USE_REAL48		1		// 1 = use real48 numbers (11 digits, exp +-153)
#define USE_REAL64		1		// 1 = use real64 numbers (double, 16 digits, exp +-308)
//#define USE_REAL80		1		// 1 = use real80 numbers (19 digits, exp +-4932)
//#define USE_REAL96		1		// 1 = use real96 numbers (25 digits, exp +-1233)
#define USE_REAL128		1		// 1 = use real128 numbers (34 digits, exp +-4932)
//#define USE_REAL160		1		// 1 = use real160 numbers (43 digits, exp +-9864)
//#define USE_REAL192		1		// 1 = use real192 numbers (52 digits, exp +-19728)
#define USE_REAL256		1		// 1 = use real256 numbers (71 digits, exp +-78913)
//#define USE_REAL384		1		// 1 = use real384 numbers (109 digits, exp +-315652)
//#define USE_REAL512		1		// 1 = use real512 numbers (147 digits, exp +-1262611)
//#define USE_REAL768		1		// 1 = use real768 numbers (224 digits, exp +-5050445)
//#define USE_REAL1024		1		// 1 = use real1024 numbers (300 digits, exp +-20201781)
//#define USE_REAL1536		1		// 1 = use real1536 numbers (453 digits, exp +-161614248)
//#define USE_REAL2048		1		// 1 = use real2048 numbers (607 digits, exp +-161614248)
//#define USE_REAL3072		1		// 1 = use real3072 numbers (915 digits, exp +-161614248)
//#define USE_REAL4096		1		// 1 = use real4096 numbers (1224 digits, exp +-161614248)
//#define USE_REAL6144		1		// 1 = use real6144 numbers
//#define USE_REAL8192		1		// 1 = use real8192 numbers
//#define USE_REAL12288		1		// 1 = use real12288 numbers

#define MAXCHEB_LN	1536		// max. REAL number supporting Chebyshev approximations of Ln()
#define MAXCHEB_EXP	1536		// max. REAL number supporting Chebyshev approximations of Exp()
#define MAXCHEB_SIN	1536		// max. REAL number supporting Chebyshev approximations of Sin()
#define MAXCHEB_ASIN	1536		// max. REAL number supporting Chebyshev approximations of ASin()
#define MAXCHEB_ATAN	1536		// max. REAL number supporting Chebyshev approximations of ATan()
#define MAXCHEB_SQRT	1536		// max. REAL number supporting Chebyshev approximations of Sqrt()

#include CONFIG_DEF_H				// default configuration

#endif // _CONFIG_H
