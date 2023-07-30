
// ****************************************************************************
//
//                       USB Vendor Device Class (host)
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
#if USE_USB_HOST	// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#if USE_USB_HOST_VENDOR	// use USB VENDOR Vendor specific device (host)

#ifndef _SDK_USB_VENDORH_H
#define _SDK_USB_VENDORH_H

#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

// initialize class driver
void VendorhInit();

// open class interface (returns size of used interface, 0=not supported)
u16 VendorhOpen(const sUsbDescItf* itf, u16 max_len);

// set config interface
Bool VendorhSetConfig(u8 itf_num);

// transfer complete callback
Bool VendorhXferComp(u8 dev_epinx, u8 xres, u16 len);

// close device interface
void VendorhClose();

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_VENDORH_H

#endif // USE_USB_HOST_VENDOR	// use USB VENDOR Vendor specific device (host)
#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
