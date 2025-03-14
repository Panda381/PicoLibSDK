
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

// We use a lower SD card read speed to reduce audio output noise.
#define SD_BAUD			4000000	// 4000000 // SD card baud speed (should be max. 7-12 Mbit/s; default standard bus speed
						//   is 12.5 MB/s, suggested max. bitrate 15 Mbit/s, min. writting speed 2 MB/s)

#define PWMSND_GPIO		0		// PWM sound output GPIO pin (left or single channel)
#define PWMSND_GPIO_R		1		// PWM sound output GPIO pin (right channel; -1 = not used, only single channel is used)


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
