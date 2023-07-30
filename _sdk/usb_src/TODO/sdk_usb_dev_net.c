
// ****************************************************************************
//
//                     USB Network Adapter Device Class (device)
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

#include "../../global.h"	// globals

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_NET	// use USB NET Network adapter (device)

#include "../usb_inc/sdk_usb_dev_net.h"

// initialize class driver
void NetdInit()
{


}

// reset class driver
void NetdReset()
{


}

// open device class interface (returns size of used interface, 0=not supported)
u16 NetdOpen(const sUsbDescItf* itf, u16 max_len)
{

	return 0;
}

// control request callback (returns False to stall)
Bool NetdCtrlReq(u8 stage, const sUsbSetupPkt* setup)
{

	return False;
}

// transfer complete callback
Bool NetdXferComp(u8 epinx, u8 xres, u16 len)
{

	return False;
}

#endif // USE_USB_DEV_NET	// use USB NET Network adapter (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
