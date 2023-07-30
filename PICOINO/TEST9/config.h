
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

#define USE_USB_STDIO	1		// use USB stdio (UsbPrint function)
#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)


// Flags to use USB devices on this Pico (uncomment the row to use the device)
//#define USE_USB_DEV_AUDIO	1		// use USB AUDIO Audio device, value = number of interfaces (device)
//#define USE_USB_DEV_BTH	1		// use USB BTH Bluetooth (device)
//#define USE_USB_DEV_CDC	1		// use USB CDC Communication Device Class, value = number of interfaces (device)
//#define USE_USB_DEV_DFU	1		// use USB DFU-RT Device Firmware Upgrade (device)
//#define USE_USB_DEV_HID	1		// use USB HID Human Interface Device, value = number of interfaces (device)
//#define USE_USB_DEV_MIDI	1		// use USB MIDI Midi device, value = number of interfaces (device)
//#define USE_USB_DEV_MSC	1		// use USB MSC Mass Storage Class (device)
//#define USE_USB_DEV_NET	1		// use USB NET Network adapter (device)
//#define USE_USB_DEV_TMC	1		// use USB TMC Test and Measurement Class (device)
//#define USE_USB_DEV_VENDOR	1		// use USB VENDOR Vendor specific device, value = number of interfaces (device)

// Flags to use USB host on this Pico (uncomment the row to use the device)
//#define USE_USB_HOST_CDC	2		// use USB CDC Communication Device Class, value = number of interfaces (host)
//#define USE_USB_HOST_HID	1		// use USB HID Human Interface Device, value = number of interfaces (host)
//#define USE_USB_HOST_HUB	1		// use USB HUB (host; value = number of HUBs)
//#define USE_USB_HOST_MSC	1		// use USB MSC Mass Storage Class (host)
//#define USE_USB_HOST_VENDOR	1		// use USB VENDOR Vendor specific device (host)


#define USE_REAL16	1		// 1 = use real16 numbers (3 digits, exp +-4)
#define USE_REAL32	1		// 1 = use real32 numbers (float, 7 digits, exp +-38)
#define USE_REAL48	1		// 1 = use real48 numbers (11 digits, exp +-153)
#define USE_REAL64	1		// 1 = use real64 numbers (double, 16 digits, exp +-308)
#define USE_REAL80	1		// 1 = use real80 numbers (19 digits, exp +-4932)
#define USE_REAL96	1		// 1 = use real96 numbers (25 digits, exp +-1233)
#define USE_REAL128	1		// 1 = use real128 numbers (34 digits, exp +-4932)
#define USE_REAL160	1		// 1 = use real160 numbers (43 digits, exp +-9864)
#define USE_REAL192	1		// 1 = use real192 numbers (52 digits, exp +-19728)
#define USE_REAL256	1		// 1 = use real256 numbers (71 digits, exp +-78913)
#define USE_REAL384	1		// 1 = use real384 numbers (109 digits, exp +-315652)
#define USE_REAL512	1		// 1 = use real512 numbers (147 digits, exp +-1262611)
#define USE_REAL768	1		// 1 = use real768 numbers (224 digits, exp +-5050445)
#define USE_REAL1024	1		// 1 = use real1024 numbers (300 digits, exp +-20201781)
#define USE_REAL1536	1		// 1 = use real1536 numbers (453 digits, exp +-161614248)
#define USE_REAL2048	1		// 1 = use real2048 numbers (607 digits, exp +-161614248)
#define USE_REAL3072	1		// 1 = use real3072 numbers (915 digits, exp +-161614248)
#define USE_REAL4096	1		// 1 = use real4096 numbers (1224 digits, exp +-161614248)
#define USE_REAL6144	1		// 1 = use real6144 numbers
#define USE_REAL8192	1		// 1 = use real8192 numbers
#define USE_REAL12288	1		// 1 = use real12288 numbers

#include "../../config_def.h"	// default configuration

#endif // _CONFIG_H
