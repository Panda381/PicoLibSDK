
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

#define USE_MP3		1		// use MP3 decoder (lib_mp3*.c, lib_mp3*.h)

#define MP3_CHECK_LOAD	1		// check MP3 load

#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
#define USE_DISPHSTXMINI	0	// 1=use HSTX Display Mini driver
#define USE_DISPHSTX		1	// 1=use HSTX Display driver
//#define DISPHSTX_DISP_SEL	-1	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
#define DISPHSTX_USE_DVI	1	// 1=use DVI (HDMI) support (DVI requires about 15 KB of RAM)
#define DISPHSTX_USE_VGA	1	// 1=use VGA support (VGA requires about 30 KB of RAM)

#define USE_DISPHSTXMINI_VMODE	1	// DispHstxMini videomode (default 1; data must be DISPHSTX_VGA_MASK masked in VGA case if only FrameBuf is used)
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
#endif

// We use a lower SD card read speed to reduce audio output noise.
#define SD_BAUD			4000000	// 4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
						//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)

//#define PWMSND_GPIO		20		// PWM sound output GPIO pin (left or single channel)
//#define PWMSND_GPIO_R		21		// PWM sound output GPIO pin (right channel; -1 = not used, only single channel is used)


//#define FONT			FontBold8x8	// default system font
//#define FONTW			8		// width of system font
//#define FONTH			8		// height of system font

//#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)
//#define USE_USB_STDIO		1		// use USB stdio (UsbPrint function)
//#define USE_UART_STDIO	1		// use UART stdio (UartPrint function)

//#define USE_ORIGSDK		1		// include interface of original-SDK
//#define USE_SCREENSHOT	1		// use screen shots

#define USE_USBPAD		1		// simulate keypad with USB keyboard
//#define USE_USBPORT		1		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
//#define USE_USB_DEV_CDC	4		// use USB CDC Communication Device Class, value = number of interfaces (device)
//#define USE_USB_DEV_HID	1		// use USB HID Human Interface Device, value = number of interfaces (device)
//#define USE_USB_HOST_CDC	4		// use USB CDC Communication Device Class, value = number of interfaces (host)
//#define USE_USB_HOST_HID	4		// use USB HID Human Interface Device, value = number of interfaces (host)

//#define USE_VIDEO	1		// use video player - it will disable default frame buffer (lib_video.c, lib_video.h)

#include CONFIG_DEF_H				// default configuration

#endif // _CONFIG_H
