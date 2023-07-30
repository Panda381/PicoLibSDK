
// ****************************************************************************
//
//                       USB Vendor Device Class (device)
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

// This code is based on TinyUSB library, Copyright (c) 2019 Ha Thach (tinyusb.org)

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_VENDOR	// use USB VENDOR Vendor specific device, value = number of interfaces (device)

#ifndef _SDK_USB_DEV_VENDOR_H
#define _SDK_USB_DEV_VENDOR_H

#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

// initialize class driver
void UsbDevVendorInit();

// terminate class driver
void UsbDevVendorTerm();

// reset class driver
void UsbDevVendorReset();

// open device class interface (returns size of used interface, 0=not supported)
u16 UsbDevVendorOpen(const sUsbDescItf* itf, u16 max_len);

// process control transfer complete (returns False to stall)
Bool UsbDevVendorCtrl(u8 stage);

// process data transfer complete
void UsbDevVendorComp(u8 epinx, u8 xres, u16 len);

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_DEV_VENDOR_H

#endif // USE_USB_DEV_VENDOR	// use USB VENDOR Vendor specific device, value = number of interfaces (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
