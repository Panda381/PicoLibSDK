
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

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define USE_DISPHSTXMINI	1	// 1=use HSTX Display Mini driver
#define USE_DISPHSTX		0	// 1=use HSTX Display driver
//#define DISPHSTX_DISP_SEL	-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#define DISPHSTX_USE_DVI	1	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#define DISPHSTX_USE_VGA	1	// 1=use VGA support (VGA requires about 30 KB of RAM)

#define USE_DISPHSTXMINI_VMODE	4	// DispHstxMini videomode (default 1; data must be DISPHSTX_VGA_MASK masked in VGA case if only FrameBuf is used)
					//	1=320x240/16bit, FrameBuf+DispBuf, sys_clock=126 MHz
					//	2=320x240/16bit, FrameBuf+DispBuf, sys_clock=252 MHz
					//	3=320x240/16bit (borders 2x46 pixels), FrameBuf+DispBuf, sys_clock variable, clocked from USB 144 MHz
					//	4=320x240/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	5=320x144/16bit (borders 2x46 pixels), only FrameBuf, sys_clock variable, clocked from USB 144 MHz
					//	6=360x240/16bit, only FrameBuf, sys_clock variable, clocked from USB 144 MHz

#define USE_DISPHSTX_VMODE	4	// DispHstx videomode (default 1)
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

#define USE_FRAMEBUF		1		// do not use pre-defined Frame buffer
#else // USE_PICOPADHSTX
#define USE_FRAMEBUF		0		// do not use pre-defined Frame buffer
#endif // USE_PICOPADHSTX

//#define FONT			FontBold8x8	// default system font
//#define FONTW			8		// width of system font
//#define FONTH			8		// height of system font

#define USE_DRAW_STDIO		0		// use DRAW stdio (DrawPrint function)
#define USE_USB_STDIO		0		// use USB stdio (UsbPrint function)
#define USE_UART_STDIO		0		// use UART stdio (UartPrint function)

//#define USE_EMU			1		// use emulators
//#define USE_EMU_I4004		1		// use I4004 CPU emulator
//#define USE_EMU_I4040		1		// use I4040 CPU emulator
//#define USE_EMU_I8008		1		// use I8008 CPU emulator
//#define USE_EMU_I8048		1		// use I8048 CPU emulator
//#define USE_EMU_I8052		1		// use I8051/I8052 CPU emulator
//#define USE_EMU_I8080		1		// use I8080 CPU emulator
//#define USE_EMU_I8085		1		// use I8085 CPU emulator
//#define USE_EMU_I8086		1		// use I8086/I8088/I80186/I80188 CPU emulator
//#define USE_EMU_M6502		1		// use M6502/M65C02 CPU emulator
//#define USE_EMU_X80		1		// use Sharp X80 (LR35902) CPU emulator
//#define USE_EMU_Z80		1		// use Z80 CPU emulator

// M6502 modifications
//#define M6502_CPU_65C02	1		// 1=use modifications of 65C02 and later

// x86 modifications
//#define I8086_CPU_INTEL	1		// 1=use Intel vendor alternative
//#define I8086_CPU_AMD		0		// 1=use AMD (and partially NEC) vendor alternative
//#define I8086_CPU_8088	0		// 1=use 8088/80188 timings limitations
//#define I8086_CPU_80186	1		// 1=use 80186 extension

//#define USE_EMU_PC		1		// use PC emulator
//#define USE_EMU_PC_LCD	1		// use output PC to LCD display
//#define USE_FRAMEBUF		0		// do not use pre-defined Frame buffer
//#define PC_RAM_PAGE_NUM	50		// number of 4 KB RAM pages in Frame buffer
//#define PC_RAM_BASE		((u8*)FrameBuf)	// RAM base address (declared as u8*)
//#define PC_ROM_BASE		((const u8*)(((u32)&__etext+(u32)&__data_end__-(u32)&__data_start__+0xfff+256+4096)&~0xfff)) // base address to load program to the flash (must be aligned to 4 KB)

//#define USE_EMU_GB		2		// 1=use Game Boy emulator, 2=use Game Boy Color emulator
//#define USE_FRAMEBUF		0		// do not use pre-defined Frame buffer
//#define GB_RAM_PAGE_NUM		57		// number of 4 KB RAM pages in Frame buffer
//#define GB_RAM_BASE		((u8*)FrameBuf)	// RAM base address (declared as u8*)
//#define GB_RAM_SIZE		(200*1024)	// RAM size in number of bytes

#define USE_PWMSND		0		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
//#define USE_ORIGSDK		1		// include interface of original-SDK
//#define EMU_DEBUG_SYNC	1		// 1 = debug measure time synchronization
//#define USE_SCREENSHOT	1		// use screen shots
#define USE_EMUSCREENSHOT	1		// use emulator screen shots
//#define USE_USBPAD		1		// simulate keypad with USB keyboard
//#define USE_FILESEL		1		// use file selection (lib_filesel.c, lib_filesel.h)
//#define SYSTICK_MS		1		// increment of system time in [ms] on SysTick interrupt (default 5)

//#define USE_USBPORT		1		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
//#define USE_USB_DEV_CDC	4		// use USB CDC Communication Device Class, value = number of interfaces (device)
//#define USE_USB_DEV_HID	1		// use USB HID Human Interface Device, value = number of interfaces (device)
//#define USE_USB_HOST_CDC	4		// use USB CDC Communication Device Class, value = number of interfaces (host)
#define USE_USB_HOST_HID	4		// use USB HID Human Interface Device, value = number of interfaces (host)

#define SD_BAUD		4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
#define SD_BAUDWRITE	1000000 // SD card baud speed of write (should be max. 7-12 Mbit/s; default standard bus speed

//#define USE_REAL16		1		// 1 = use real16 numbers (3 digits, exp +-4)
//#define USE_REAL32		1		// 1 = use real32 numbers (float, 7 digits, exp +-38)
//#define USE_REAL48		1		// 1 = use real48 numbers (11 digits, exp +-153)
//#define USE_REAL64		1		// 1 = use real64 numbers (double, 16 digits, exp +-308)
//#define USE_REAL80		1		// 1 = use real80 numbers (19 digits, exp +-4932)
//#define USE_REAL96		1		// 1 = use real96 numbers (25 digits, exp +-1233)
//#define USE_REAL128		1		// 1 = use real128 numbers (34 digits, exp +-4932)
//#define USE_REAL160		1		// 1 = use real160 numbers (43 digits, exp +-9864)
//#define USE_REAL192		1		// 1 = use real192 numbers (52 digits, exp +-19728)
//#define USE_REAL256		1		// 1 = use real256 numbers (71 digits, exp +-78913)
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

#include CONFIG_DEF_H				// default configuration

#endif // _CONFIG_H
