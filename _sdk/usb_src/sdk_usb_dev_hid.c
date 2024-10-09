
// ****************************************************************************
//
//                     USB Human Interface Device Class (device)
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

#include "../usb_inc/sdk_usb_dev_hid.h"

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_HID	// use USB HID Human Interface Device, value = number of interfaces (device)

#include "../usb_inc/sdk_usb_phy.h" // physical layer
#include "../usb_inc/sdk_usb_dev.h" // devices
#include "../inc/sdk_timer.h" // devices

// HID interfaces
sUsbDevHidInter UsbDevHidInter[USE_USB_DEV_HID];

// get HID report callback (returns data length, 0 = error)
pUsbDevHidGetRepCB UsbDevHidGetRepCB = NULL; // called from IRQ

// set HID report callback
pUsbDevHidSetRepCB UsbDevHidSetRepCB = NULL; // called from IRQ

// === default descriptors

// buffer to load unique board string ID from flash memory
char BoardIdStr_hid[9] = "12345";

// Strings
const char* UsbDevHidDescStr[10] = {
				// 0: language code (not included here)
	"Raspberry Pi",		// 1: manufacturer
	"PicoPad",		// 2: product
	BoardIdStr_hid,		// 3: board serial number
	"USB HID Keyboard",	// 4: configuration descriptor keyboard
	"USB HID Mouse",	// 5: configuration descriptor mouse
	"USB HID Joystick",	// 6: configuration descriptor joystick
	"USB HID Gamepad",	// 7: configuration descriptor gamepad
	"USB HID Power",	// 8: configuration descriptor power
	"USB HID",		// 9: configuration descriptor
};

// HID device descriptor
const sUsbDescDev UsbDevHidDescDev = {
	18,			// 0: size of this descriptor in bytes (= 18)
	USB_DESC_DEVICE,	// 1: descriptor type (= USB_DESC_DEVICE)
	0x0200,			// 2: USB specification in BCD code (0x200 = 2.00)
	0,			// 4: device class code (class is defined by interface descriptor)
	0,			// 5: device subclass code
	0,			// 6: device protocol code
	USB_PACKET_MAX,		// 7: max. packet size for endpoint 0 (valid size 8, 16, 32, 64) = size of controll buffer
	0x2E8A,			// 8: vendor ID (0x2E8A = Raspberry Pi)
	USB_PID,		// 10: product ID (0x000A = Raspbery Pi Pico SDK)
	0x0100,			// 12: device release number in BCD code (0x0100 = 1.00)
	1,			// 14: index of manufacturer string descriptor (0=none)
	2,			// 15: index of product string descriptor (0=none)
	3,			// 16: index of serial number string descriptor (0=none)
	1,			// 17: number of possible configurations
};

// keyboard report descriptor (sUsbHidKey structure)
const u8 UsbHidRepDescKeyboard[] = { HID_REPORT_DESC_KEYBOARD };

// mouse report descriptor (sUsbHidMouse structure)
const u8 UsbHidRepDescMouse[] = { HID_REPORT_DESC_MOUSE };

// joystick report descriptor (sUsbHidJoy structure)
const u8 UsbHidRepDescJoystick[] = { HID_REPORT_DESC_JOYSTICK };

// gamepad report descriptor (sUsbHidPad structure)
const u8 UsbHidRepDescGamepad[] = { HID_REPORT_DESC_GAMEPAD };

// system power control report descriptor (sUsbHidPwr structure)
const u8 UsbHidRepDescPower[] = { HID_REPORT_DESC_SYSTEM_CONTROL };

// 2 interfaces: keyboard and mouse
#if USE_USB_DEV_HID == 2

#define USB_HIDD_ITF_KEY	0			// index of keyboard interface
#define USB_HIDD_ITF_MOUSE	1			// index of mouse interface
#define USB_HIDD_EP_KEY		USB_EPADDR(1, USB_DIR_IN) // keyboard IN endpoint
#define USB_HIDD_EP_MOUSE	USB_EPADDR(2, USB_DIR_IN) // mouse IN endpoint

// === keyboard and mouse

// HID configuration descriptor - keyboard and mouse (9+25+25 = 59 bytes)
const u8 UsbDevHidDescCfg2[9+2*USB_TEMP_HIDD_LEN] =
{
	// (9) configuration descriptor (index=1, 2 interfaces, no string, length, 100 mA)
	USB_TEMP_CFG(1, 2, 0, 9+2*USB_TEMP_HIDD_LEN, 100),

	// (25) keyboard: HID device descriptor (index of interface, string = 4, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF_KEY, 4, USB_HID_ITF_KEYB, sizeof(UsbHidRepDescKeyboard), USB_HIDD_EP_KEY, USB_PACKET_MAX, 10),

	// (25) mouse: HID device descriptor (index of interface, string = 5, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF_MOUSE, 5, USB_HID_ITF_MOUSE, sizeof(UsbHidRepDescMouse), USB_HIDD_EP_MOUSE, USB_PACKET_MAX, 10),
};

// list of configurations
const sUsbDescCfg* UsbDescHidCfgList2[1] = { (const sUsbDescCfg*)&UsbDevHidDescCfg2, };

// list of report descriptors
const u8* UsbDescHidRepList2[2] = { UsbHidRepDescKeyboard, UsbHidRepDescMouse };

// application device setup descriptor - keyboard and mouse
const sUsbDevSetupDesc UsbDevHidSetupDesc2 =
{
	// descriptors
	.desc_dev = &UsbDevHidDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescHidCfgList2,	// pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = (const void**)UsbDescHidRepList2, // list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevHidDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 10,				// number of ASCIIZ strings in the list
};

// 1 interface
#else

#define USB_HIDD_ITF		0			// index of interface
#define USB_HIDD_EP		USB_EPADDR(1, USB_DIR_IN) // IN endpoint

// === keyboard (sUsbHidKey structure)

// HID configuration descriptor - keyboard (9+25 = 34 bytes)
const u8 UsbDevHidDescCfgKeyb[9+USB_TEMP_HIDD_LEN] =
{
	// (9) configuration descriptor (index=1, 1 interface, no string, length, 100 mA)
	USB_TEMP_CFG(1, 1, 0, 9+USB_TEMP_HIDD_LEN, 100),

	// (25) keyboard: HID device descriptor (index of interface, string = 4, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF, 4, USB_HID_ITF_KEYB, sizeof(UsbHidRepDescKeyboard), USB_HIDD_EP, USB_PACKET_MAX, 10),
};

// list of configurations
const sUsbDescCfg* UsbDescHidCfgListKeyb[1] = { (const sUsbDescCfg*)&UsbDevHidDescCfgKeyb, };

// list of report descriptors
const u8* UsbDescHidRepListKeyb[1] = { UsbHidRepDescKeyboard };

// application device setup descriptor - keyboard (sUsbHidKey structure)
const sUsbDevSetupDesc UsbDevHidSetupDescKeyb =
{
	// descriptors
	.desc_dev = &UsbDevHidDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescHidCfgListKeyb,	// pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = (const void**)UsbDescHidRepListKeyb, // list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevHidDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 10,				// number of ASCIIZ strings in the list
};

// === mouse (sUsbHidMouse structure)

// HID configuration descriptor - mouse (9+25 = bytes)
const u8 UsbDevHidDescCfgMouse[9+USB_TEMP_HIDD_LEN] =
{
	// (9) configuration descriptor (index=1, 1 interface, no string, length, 100 mA)
	USB_TEMP_CFG(1, 1, 0, 9+USB_TEMP_HIDD_LEN, 100),

	// (25) mouse: HID device descriptor (index of interface, string = 5, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF, 5, USB_HID_ITF_MOUSE, sizeof(UsbHidRepDescMouse), USB_HIDD_EP, USB_PACKET_MAX, 10),
};

// list of configurations
const sUsbDescCfg* UsbDescHidCfgListMouse[1] = { (const sUsbDescCfg*)&UsbDevHidDescCfgMouse, };

// list of report descriptors
const u8* UsbDescHidRepListMouse[1] = { UsbHidRepDescMouse };

// application device setup descriptor - mouse (sUsbHidMouse structure)
const sUsbDevSetupDesc UsbDevHidSetupDescMouse =
{
	// descriptors
	.desc_dev = &UsbDevHidDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescHidCfgListMouse, // pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = (const void**)UsbDescHidRepListMouse, // list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevHidDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 10,				// number of ASCIIZ strings in the list
};

// === joystick (sUsbHidJoy structure)

// HID configuration descriptor - joystick (9+25 = bytes)
const u8 UsbDevHidDescCfgJoystick[9+USB_TEMP_HIDD_LEN] =
{
	// (9) configuration descriptor (index=1, 1 interface, no string, length, 100 mA)
	USB_TEMP_CFG(1, 1, 0, 9+USB_TEMP_HIDD_LEN, 100),

	// (25) joystick: HID device descriptor (index of interface, string = 6, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF, 6, USB_HID_ITF_NONE, sizeof(UsbHidRepDescJoystick), USB_HIDD_EP, USB_PACKET_MAX, 10),
};

// list of configurations
const sUsbDescCfg* UsbDescHidCfgListJoystick[1] = { (const sUsbDescCfg*)&UsbDevHidDescCfgJoystick, };

// list of report descriptors
const u8* UsbDescHidRepListJoystick[1] = { UsbHidRepDescJoystick };

// application device setup descriptor - joystick (sUsbHidJoy structure)
const sUsbDevSetupDesc UsbDevHidSetupDescJoystick =
{
	// descriptors
	.desc_dev = &UsbDevHidDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescHidCfgListJoystick, // pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = (const void**)UsbDescHidRepListJoystick, // list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevHidDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 10,				// number of ASCIIZ strings in the list
};

// === gamepad (sUsbHidPad structure)

// HID configuration descriptor - gamepad (9+25 = bytes)
const u8 UsbDevHidDescCfgGamepad[9+USB_TEMP_HIDD_LEN] =
{
	// (9) configuration descriptor (index=1, 1 interface, no string, length, 100 mA)
	USB_TEMP_CFG(1, 1, 0, 9+USB_TEMP_HIDD_LEN, 100),

	// (25) gamepad: HID device descriptor (index of interface, string = 7, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF, 7, USB_HID_ITF_NONE, sizeof(UsbHidRepDescGamepad), USB_HIDD_EP, USB_PACKET_MAX, 10),
};

// list of configurations
const sUsbDescCfg* UsbDescHidCfgListGamepad[1] = { (const sUsbDescCfg*)&UsbDevHidDescCfgGamepad, };

// list of report descriptors
const u8* UsbDescHidRepListGamepad[1] = { UsbHidRepDescGamepad };

// application device setup descriptor - gamepad (sUsbHidPad structure)
const sUsbDevSetupDesc UsbDevHidSetupDescGamepad =
{
	// descriptors
	.desc_dev = &UsbDevHidDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescHidCfgListGamepad, // pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = (const void**)UsbDescHidRepListGamepad, // list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevHidDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 10,				// number of ASCIIZ strings in the list
};

// === power control (sUsbHidPwr structure)

// HID configuration descriptor - power control (9+25 = bytes)
const u8 UsbDevHidDescCfgPower[9+USB_TEMP_HIDD_LEN] =
{
	// (9) configuration descriptor (index=1, 1 interface, no string, length, 100 mA)
	USB_TEMP_CFG(1, 1, 0, 9+USB_TEMP_HIDD_LEN, 100),

	// (25) power control: HID device descriptor (index of interface, string = 8, protocol, size of report, in ep, packet size, polling = 10 ms)
	USB_TEMP_HIDD(USB_HIDD_ITF, 8, USB_HID_ITF_NONE, sizeof(UsbHidRepDescPower), USB_HIDD_EP, USB_PACKET_MAX, 10),
};

// list of configurations
const sUsbDescCfg* UsbDescHidCfgListPower[1] = { (const sUsbDescCfg*)&UsbDevHidDescCfgPower, };

// list of report descriptors
const u8* UsbDescHidRepListPower[1] = { UsbHidRepDescPower };

// application device setup descriptor - power control (sUsbHidPwr structure)
const sUsbDevSetupDesc UsbDevHidSetupDescPower =
{
	// descriptors
	.desc_dev = &UsbDevHidDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescHidCfgListPower, // pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = (const void**)UsbDescHidRepListPower, // list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevHidDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 10,				// number of ASCIIZ strings in the list
};

#endif

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
//  itf_num ... HID interface number
u8 UsbDevHidFindItf(u8 itf_num)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

		// check interface
		if (hid->used && (hid->itf_num == itf_num))
		{
			return hid_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// check if device is mounted
//  hid_inx ... interface index
Bool UsbDevHidInxIsMounted(u8 hid_inx)
{
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];
	return hid->used && UsbIsMounted() && (hid->ep_in != 0);
}

// check if next report can be sent
//  hid_inx ... interface index
Bool UsbDevHidInxSendReady(u8 hid_inx)
{
	// check if device is mounted
	if (!UsbDevHidInxIsMounted(hid_inx)) return False;

	// check if send buffer is empty	
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];
	return (hid->tx_len == 0);
}

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by endpoint (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbDevHidFindAddr(u8 epinx)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

		// check interface
		if (hid->used && ((hid->ep_in == epinx) || (hid->ep_out == epinx)))
		{
			return hid_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbDevHidNewItf(const sUsbDescItf* itf)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

		// check if interface is not used
		if (!hid->used)
		{
			hid->used = True; // interface is used
			hid->itf_num = itf->itf_num; // interface number
			hid->protocol = itf->itf_protocol; // interface protocol
			hid->mode = USB_HID_PROT_REP; // default is report mode
			return hid_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// send report from TX buffer
//  hid_inx ... interface index
void UsbDevHidInxSend(u8 hid_inx)
{
	// check if device is mounted
	if (!UsbDevHidInxIsMounted(hid_inx)) return;

	// pointer to HID interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

	// no data to send
	u8 len = hid->tx_len;
	if (len == 0) return;

	// get endpoint
	sEndpoint* sep = &UsbEndpoints[hid->ep_in];

	// check if endpoint is busy
	if (UsbEpIsBusy(hid->ep_in)) return; // transmission is active

	// send data
	UsbXferStart(hid->ep_in, hid->tx_buf, len, False);
}

// send reports of all interfaces
void UsbDevHidAllSend()
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		UsbDevHidInxSend(hid_inx);
	}
}

// receive report to RX buffer
//  hid_inx ... interface index
void UsbDevHidInxRecv(u8 hid_inx)
{
	// check if device is mounted
	if (!UsbDevHidInxIsMounted(hid_inx)) return;

	// pointer to HID interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

	// not receiving
	if (hid->ep_out == 0) return;

	// check if endpoint is busy
	if (UsbEpIsBusy(hid->ep_out)) return; // transmission is active

	// receive
	UsbXferStart(hid->ep_out, hid->rx_buf, USB_DEV_HID_RX_BUFSIZE, False);
}

// receive reports to all interfaces
void UsbDevHidAllRecv()
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		UsbDevHidInxRecv(hid_inx);
	}
}

// initialize class driver
void UsbDevHidInit()
{
	// clear HID interfaces
	memset(UsbDevHidInter, 0, sizeof(UsbDevHidInter));
}

// terminate class driver
void UsbDevHidTerm()
{

}

// reset class driver
void UsbDevHidReset()
{
	// clear HID interfaces
	memset(UsbDevHidInter, 0, sizeof(UsbDevHidInter));
}

// open device class interface (returns size of used interface, 0=not supported)
u16 UsbDevHidOpen(const sUsbDescItf* itf, u16 max_len)
{
	// check interface class
	if (itf->itf_class != USB_CLASS_HID) return 0;

	// check descriptor size
	if (USB_DESC_LEN(itf) > max_len) return 0;

	// allocate new interface
	u8 hid_inx = UsbDevHidNewItf(itf);
	if (hid_inx == USB_DRVID_INVALID) return 0;

	// pointer to HID interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

	// skip interface descriptor
	u16 drv_len = USB_DESC_LEN(itf);
	const u8* p_desc = USB_DESC_NEXT(itf);

	// HID descriptor
	if ((USB_DESC_TYPE(p_desc) != USB_HID_DESC_HID) ||
		(drv_len + USB_DESC_LEN(p_desc) > max_len))
		return 0;
	hid->desc_hid = (const sUsbDescHid*)p_desc; // save pointer to HID descriptor

	// skip HID descriptor
	drv_len += USB_DESC_LEN(p_desc);
	p_desc = USB_DESC_NEXT(p_desc);

	// open endpoints
	u8 n = itf->num_ep; // number of endpoints
	if (drv_len + n*sizeof(sUsbDescEp) > max_len) return 0;
	if (!UsbDevOpenEpPair(p_desc, n, &hid->ep_out, &hid->ep_in)) return 0;
	drv_len += n*sizeof(sUsbDescEp);

	// start receiving data
	UsbDevHidInxRecv(hid_inx);

	return drv_len;
}

// process control transfer complete (returns False to stall)
Bool UsbDevHidCtrl(u8 stage)
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// recipient must be interface
	if ((setup->type & 0x1f) != USB_REQ_RCPT_INTERFACE) return False;

	// find interface by interface number
	u8 hid_inx = UsbDevHidFindItf((u8)setup->index);
	if (hid_inx == USB_DRVID_INVALID) return False; // interface not found

	// pointer to HID interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

	// request type
	u8 reqtype = ((setup->type >> 5) & 3);
	switch (reqtype)
	{
	// standard request type
	case USB_REQ_TYPE_STANDARD:
		if (stage == USB_STAGE_SETUP)
		{
			// get descriptor type
			u8 rep_type = (u8)(setup->value >> 8);

			// get descriptor
			if (setup->request == USB_REQ_GET_DESCRIPTOR)
			{
				// get HID descriptor
				if (rep_type == USB_HID_DESC_HID)
				{
					UsbDevSetupStart((void*)hid->desc_hid, USB_DESC_LEN(hid->desc_hid));
				}

				// get report descriptor
				else if (rep_type == USB_HID_DESC_REP)
				{


					const void** desc_list = UsbDevSetupDesc->desc_list;
					if (desc_list == NULL) return False;
					UsbDevSetupStart((void*)desc_list[hid_inx], hid->desc_hid->rep_len);
				
				}
				else
					return False;
			}
			else
				return False;
		}
		break;

	// class request type
	case USB_REQ_TYPE_CLASS:
		switch (setup->request)
		{
		// get report (value = HIGH report type and LOW report ID, length = report length, data = report)
		case USB_HID_REQ_GET_REP:
			if (stage == USB_STAGE_SETUP)
			{
				// get report type
				u8 rep_type = (u8)(setup->value >> 8);

				// get report ID
				u8 rep_id = (u8)setup->value;

				// destination buffer
				u8* buf = hid->tx_buf;

				// required length
				u16 len = setup->length;
				if (len > USB_DEV_HID_TX_BUFSIZE) len = USB_DEV_HID_TX_BUFSIZE;

				// request report
				if (UsbDevHidGetRepCB != NULL)
				{
					len = UsbDevHidGetRepCB(hid->itf_num, len, rep_id, rep_type);
					if (len == 0) return False;

					// send report
					UsbDevSetupStart(hid->tx_buf, len);
				}
				else
					return False;
			}
			break;
				
		// set report (set LEDs; value = report type HIGH and report ID LOW, length = report length, data = report)
		case USB_HID_REQ_SET_REP:
			if (stage == USB_STAGE_SETUP)
			{
				// check report length
				if (setup->length > USB_DEV_HID_RX_BUFSIZE) return False;

				// receive report
				UsbDevSetupStart(hid->rx_buf, setup->length);
			}
			else if (stage == USB_STAGE_ACK)
			{
				// get report type
				u8 rep_type = (u8)(setup->value >> 8);

				// get report ID
				u8 rep_id = (u8)setup->value;

				// source buffer
				u8* buf = hid->rx_buf;

				// length of report
				u16 len = setup->length;
				if (len > USB_DEV_HID_RX_BUFSIZE) len = USB_DEV_HID_RX_BUFSIZE;

				// send report to application
				if (UsbDevHidSetRepCB != NULL) UsbDevHidSetRepCB(hid->itf_num, len, rep_id, rep_type);
			}
			break;

		// set idle (request silence; value = HIGH duration and LOW report ID, length = 0)
		case USB_HID_REQ_SET_IDLE:
			if (stage == USB_STAGE_SETUP)
			{
				// get idle value
				hid->idle = (u8)(setup->value >> 8);

				// acknowledging at Status Stage
				UsbDevSetupAck();
			}
			break;

		// get idle (value = LOW report ID, length = 1, data = idle rate byte)
		case USB_HID_REQ_GET_IDLE:
			if (stage == USB_STAGE_SETUP)
			{
				UsbDevSetupStart(&hid->idle, 1);
			}
			break;

		// set protocol (value = 0 boot or 1 report; length = 0)
		case USB_HID_REQ_SET_PROT:
			if (stage == USB_STAGE_SETUP)
			{
				// get protocol mode
				hid->mode = (u8)setup->value;

				// acknowledging at Status Stage
				UsbDevSetupAck();
			}
			break;

		// get protocol (length = 1, data = 0 boot or 1 report, )
		case USB_HID_REQ_GET_PROT:
			if (stage == USB_STAGE_SETUP)
			{
				UsbDevSetupStart(&hid->mode, 1);
			}
			break;

		// invalid request
		default:
			return False;
		}
		break;

	// invalid request type
	default:
		return False;
	}

	return True;
}

// process data transfer complete
void UsbDevHidComp(u8 epinx, u8 xres, u16 len)
{
	// find interface by endpoint
	u8 hid_inx = UsbDevHidFindAddr(epinx);
	if (hid_inx == USB_DRVID_INVALID)
	{
		return; // invalid endpoint
	}

	// pointer to HDI interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

	// process next report
	if (hid->ep_out == epinx)
	{
		// receive next data
		UsbDevHidInxRecv(hid_inx);
	}
	else
	{
		// data sent to host
		UsbDevHidInxSend(hid_inx);
	}
}

// schedule driver, synchronized packets in frames
/*void UsbDevHidSof(u16 sof)
{

}*/

// schedule driver in boot mode (raised with UsbRescheduleDrv())
void UsbDevHidSched()
{
	// send reports of all interfaces
	UsbDevHidAllSend();
}

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// save report to send buffer (returns data size, does not set tx_len)
// - can be called from UsbDevHidGetRepCB to save report to send buffer
//  itf_num ... HID interface number
//  buf ... buffer with report structure
//  len ... report length in bytes, including rep_id byte if it is used
//  rep_id ... report ID (0 = do not use)
u8 UsbDevHidTxSave(u8 itf_num, const void* buf, u8 len, u8 rep_id)
{
	// find interface
	u8 hid_inx = UsbDevHidFindItf(itf_num);
	if (hid_inx == USB_DRVID_INVALID) return 0; // interface not found

	// pointer to HID interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];
	hid->tx_len = 0; // destroy data in buffer

	// limit length of report
	if (len > USB_DEV_HID_TX_BUFSIZE) len = USB_DEV_HID_TX_BUFSIZE;
	u8 len2 = len;

	// store report ID to buffer
	u8* d = hid->tx_buf;
	if (rep_id != 0)
	{
		*d++ = rep_id;
		if (len2 > 0) len2--;
	}

	// copy report to buffer
	UsbMemcpy(d, buf, len2);

	return len;
}

// send report in boot mode (check UsbDevHidInxSendReady if next report can be sent; returns False if cannot send)
//  itf_num ... HID interface number
//  buf ... buffer with report structure
//  len ... report length in bytes, including rep_id byte if it is used
//  rep_id ... report ID (0 = do not use)
Bool UsbDevHidSendReport(u8 itf_num, const void* buf, u8 len, u8 rep_id)
{
	// find interface
	u8 hid_inx = UsbDevHidFindItf(itf_num);
	if (hid_inx == USB_DRVID_INVALID) return False; // interface not found

	// check if device is mounted
	if (!UsbDevHidInxIsMounted(hid_inx)) return False; // not mounted

	// save data to send buffer
	len = UsbDevHidTxSave(itf_num, buf, len, rep_id);
	if (len == 0) return False;

	// pointer to HID interface
	sUsbDevHidInter* hid = &UsbDevHidInter[hid_inx];

	// update length
	dmb(); // data memory barrier
	hid->tx_len = len;
	dmb(); // data memory barrier

	// request to send report
	UsbRescheduleDrv();

	return True;
}

// send keyboard report in boot mode (returns False on error)
Bool UsbDevHidSendKeyRep(const sUsbHidKey* rep)
{
	// search keyboard interface
	u8 hid_inx;
	sUsbDevHidInter* hid = UsbDevHidInter;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		if (hid->used && (hid->protocol == USB_HID_ITF_KEYB)) break;
		hid++;
	}
	if (hid_inx == USE_USB_DEV_HID) return False;

	// send report
	return UsbDevHidSendReport(hid->itf_num, rep, sizeof(sUsbHidKey), 0);
}

// send keyboard key (returna False on error), output speed 30 chars/sec
Bool UsbDevHidSendKey(u8 key, u8 modi)
{
	sUsbHidKey rep;

	// clear structure
	memset(&rep, 0, sizeof(sUsbHidKey));

	// set modifiers
	rep.modi = modi;

	// send report - only modifiers
	if (!UsbDevHidSendKeyRep(&rep)) return False;

	// short delay
	WaitMs(1);

	// set key
	rep.key[0] = key;

	// send report
	if (!UsbDevHidSendKeyRep(&rep)) return False;

	// short delay (Windows driver: minimum 7)
	WaitMs(15);

	// clear structure
	memset(&rep, 0, sizeof(sUsbHidKey));

	// send report
	if (!UsbDevHidSendKeyRep(&rep)) return False;

	// short delay
	WaitMs(17);

	return True;
}

// send keyboard character (returna False on error), output speed 30 chars/sec
// - The target computer must have an English keyboard layout in order to maintain the correct character mapping.
Bool UsbDevHidSendChar(char ch)
{
	// check character range
	if ((u8)ch >= ASCII_TO_HIDKEY_NUM) return True;

	// get character index
	int i = (int)ch * 2;

	// get modifier
	u8 modi = (AsciiToHidKey[i] != 0) ? USB_KEY_MODI_LSHIFT : 0;

	// get key code
	u8 key = AsciiToHidKey[i+1];

	// send key
	return UsbDevHidSendKey(key, modi);
}

// send keyboard text (returna False on error), output speed 30 chars/sec
// - The target computer must have an English keyboard layout in order to maintain the correct character mapping.
Bool UsbDevHidSendText(const char* txt)
{
	char ch;
	for (;;)
	{
		ch = *txt++;
		if (ch == 0) break;
		if (!UsbDevHidSendChar(ch)) return False;
	}
	return True;
}

// send mouse report in boot mode (returns False on error)
Bool UsbDevHidSendMouseRep(const sUsbHidMouse* rep)
{
	// search mouse interface
	u8 hid_inx;
	sUsbDevHidInter* hid = UsbDevHidInter;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		if (hid->used && (hid->protocol == USB_HID_ITF_MOUSE)) break;
		hid++;
	}
	if (hid_inx == USE_USB_DEV_HID) return False;

	// send report
	return UsbDevHidSendReport(hid->itf_num, rep, sizeof(sUsbHidMouse), 0);
}

// send mouse (returns False on error), output speed max 1000 rep/sec
Bool UsbDevHidSendMouse(s8 dx, s8 dy, Bool left, Bool right, Bool mid)
{
	sUsbHidMouse rep;

	// setup
	rep.btn = (left ? USB_MOUSE_BTN_LEFT : 0) | (right ? USB_MOUSE_BTN_RIGHT : 0) | (mid ? USB_MOUSE_BTN_MID : 0);
	rep.x = dx;
	rep.y = dy;
	rep.wheel = 0;
	rep.pan = 0;

	// send report
	if (!UsbDevHidSendMouseRep(&rep)) return False;

	// short delay
	WaitMs(1);

	return True;
}

// send joystick report in boot mode (returns False on error)
Bool UsbDevHidSendJoyRep(const sUsbHidJoy* rep)
{
	// search none interface
	u8 hid_inx;
	sUsbDevHidInter* hid = UsbDevHidInter;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		if (hid->used && (hid->protocol == USB_HID_ITF_NONE)) break;
		hid++;
	}
	if (hid_inx == USE_USB_DEV_HID) return False;

	// send report
	return UsbDevHidSendReport(hid->itf_num, rep, sizeof(sUsbHidJoy), 0);
}

// send gamepad report in boot mode (returns False on error)
Bool UsbDevHidSendPadRep(const sUsbHidPad* rep)
{
	// search none interface
	u8 hid_inx;
	sUsbDevHidInter* hid = UsbDevHidInter;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		if (hid->used && (hid->protocol == USB_HID_ITF_NONE)) break;
		hid++;
	}
	if (hid_inx == USE_USB_DEV_HID) return False;

	// send report
	return UsbDevHidSendReport(hid->itf_num, rep, sizeof(sUsbHidPad), 0);
}

// send power report in boot mode (returns False on error)
Bool UsbDevHidSendPwrRep(const sUsbHidPwr* rep)
{
	// search none interface
	u8 hid_inx;
	sUsbDevHidInter* hid = UsbDevHidInter;
	for (hid_inx = 0; hid_inx < USE_USB_DEV_HID; hid_inx++)
	{
		if (hid->used && (hid->protocol == USB_HID_ITF_NONE)) break;
		hid++;
	}
	if (hid_inx == USE_USB_DEV_HID) return False;

	// send report
	return UsbDevHidSendReport(hid->itf_num, rep, sizeof(sUsbHidPwr), 0);
}

#endif // USE_USB_DEV_HID	// use USB HID Human Interface Device, value = number of interfaces (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)


// convert ASCII character to HID key code
//	modifier = (AsciiToHidKey[ascii*2] != 0) ? USB_KEY_MODI_LSHIFT : 0;
//	key_code = AsciiToHidKey[ascii*2 + 1];
const u8 AsciiToHidKey[2*ASCII_TO_HIDKEY_NUM] = {
	0,	0,			// 0x00 ^@ Null (end of string, invalid character)
	0,	0,			// 0x01 ^A (select All)
	0,	0,			// 0x02 ^B (mark Block )
	0,	0,			// 0x03 ^C (Copy block, Copy file)
	0,	HID_KEY_END,		// 0x04 ^D End (enD of row, enD of files)
	0,	0,			// 0x05 ^E (rEname files, movE block)
	0,	0,			// 0x06 ^F (Find)
	0,	0,			// 0x07 ^G (Go next, repeat find)
	0,	HID_KEY_BACKSPACE,	// 0x08 ^H BackSpace (delete character before cursor)
	0,	HID_KEY_TAB,		// 0x09 ^I Tab (tabulator)
	0,	0,			// 0x0A ^J (line feed, next row)
	0,	HID_KEY_PAGE_UP,	// 0x0B ^K Page Up
	0,	HID_KEY_PAGE_DOWN,	// 0x0C ^L Page Down
	0,	HID_KEY_ENTER,		// 0x0D ^M Enter (next row, confirm)
	0,	0,			// 0x0E ^N (New file)
	0,	0,			// 0x0F ^O (Open file, edit file)
	0,	0,			// 0x10 ^P (Print file, send file)
	0,	0,			// 0x11 ^Q (Query, display help)
	0,	0,			// 0x12 ^R (find and Replace)
	0,	0,			// 0x13 ^S (Save file)
	0,	HID_KEY_INSERT,		// 0x14 ^T Insert (Toggle Insert, mark file)
	0,	HID_KEY_HOME,		// 0x15 ^U Home (begin of row, begin of files)
	0,	0,			// 0x16 ^V (paste from clipboard)
	0,	0,			// 0x17 ^W (close file)
	0,	0,			// 0x18 ^X (cut selected text)
	0,	0,			// 0x19 ^Y (redo previously undo action)
	0,	0,			// 0x1A ^Z (undo action)
	0,	HID_KEY_ESCAPE,		// 0x1B ^[ Esc (break, menu)
	0,	HID_KEY_ARROW_RIGHT,	// 0x1C "^\" Right
	0,	HID_KEY_ARROW_UP,	// 0x1D ^] Up
	0,	HID_KEY_ARROW_LEFT,	// 0x1E ^^ Left
	0,	HID_KEY_ARROW_DOWN,	// 0x1F ^_ Down
	0,	HID_KEY_SPACE,		// 0x20 space
	1,	HID_KEY_1,		// 0x21 !
	1,	HID_KEY_APOSTROPHE,	// 0x22 "
	1,	HID_KEY_3,		// 0x23 #
	1,	HID_KEY_4,		// 0x24 $
	1,	HID_KEY_5,		// 0x25 %
	1,	HID_KEY_7,		// 0x26 &
	0,	HID_KEY_APOSTROPHE,	// 0x27 '
	1,	HID_KEY_9,		// 0x28 (
	1,	HID_KEY_0,		// 0x29 )
	1,	HID_KEY_8,		// 0x2A *
	1,	HID_KEY_EQUAL,		// 0x2B +
	0,	HID_KEY_COMMA,		// 0x2C ,
	0,	HID_KEY_MINUS,		// 0x2D -
	0,	HID_KEY_PERIOD,		// 0x2E .
	0,	HID_KEY_SLASH,		// 0x2F /
	0,	HID_KEY_0,		// 0x30 0
	0,	HID_KEY_1,		// 0x31 1
	0,	HID_KEY_2,		// 0x32 2
	0,	HID_KEY_3,		// 0x33 3
	0,	HID_KEY_4,		// 0x34 4
	0,	HID_KEY_5,		// 0x35 5
	0,	HID_KEY_6,		// 0x36 6
	0,	HID_KEY_7,		// 0x37 7
	0,	HID_KEY_8,		// 0x38 8
	0,	HID_KEY_9,		// 0x39 9
	1,	HID_KEY_SEMICOLON,	// 0x3A :
	0,	HID_KEY_SEMICOLON,	// 0x3B ;
	1,	HID_KEY_COMMA,		// 0x3C <
	0,	HID_KEY_EQUAL,		// 0x3D =
	1,	HID_KEY_PERIOD,		// 0x3E >
	1,	HID_KEY_SLASH,		// 0x3F ?
	1,	HID_KEY_2,		// 0x40 @
	1,	HID_KEY_A,		// 0x41 A
	1,	HID_KEY_B,		// 0x42 B
	1,	HID_KEY_C,		// 0x43 C
	1,	HID_KEY_D,		// 0x44 D
	1,	HID_KEY_E,		// 0x45 E
	1,	HID_KEY_F,		// 0x46 F
	1,	HID_KEY_G,		// 0x47 G
	1,	HID_KEY_H,		// 0x48 H
	1,	HID_KEY_I,		// 0x49 I
	1,	HID_KEY_J,		// 0x4A J
	1,	HID_KEY_K,		// 0x4B K
	1,	HID_KEY_L,		// 0x4C L
	1,	HID_KEY_M,		// 0x4D M
	1,	HID_KEY_N,		// 0x4E N
	1,	HID_KEY_O,		// 0x4F O
	1,	HID_KEY_P,		// 0x50 P
	1,	HID_KEY_Q,		// 0x51 Q
	1,	HID_KEY_R,		// 0x52 R
	1,	HID_KEY_S,		// 0x53 S
	1,	HID_KEY_T,		// 0x55 T
	1,	HID_KEY_U,		// 0x55 U
	1,	HID_KEY_V,		// 0x56 V
	1,	HID_KEY_W,		// 0x57 W
	1,	HID_KEY_X,		// 0x58 X
	1,	HID_KEY_Y,		// 0x59 Y
	1,	HID_KEY_Z,		// 0x5A Z
	0,	HID_KEY_BRACKET_LEFT,	// 0x5B [
	0,	HID_KEY_BACKSLASH,	// 0x5C '\'
	0,	HID_KEY_BRACKET_RIGHT,	// 0x5D ]
	1,	HID_KEY_6,		// 0x5E ^
	1,	HID_KEY_MINUS,		// 0x5F _
	0,	HID_KEY_GRAVE,		// 0x60 `
	0,	HID_KEY_A,		// 0x61 a
	0,	HID_KEY_B,		// 0x62 b
	0,	HID_KEY_C,		// 0x63 c
	0,	HID_KEY_D,		// 0x66 d
	0,	HID_KEY_E,		// 0x65 e
	0,	HID_KEY_F,		// 0x66 f
	0,	HID_KEY_G,		// 0x67 g
	0,	HID_KEY_H,		// 0x68 h
	0,	HID_KEY_I,		// 0x69 i
	0,	HID_KEY_J,		// 0x6A j
	0,	HID_KEY_K,		// 0x6B k
	0,	HID_KEY_L,		// 0x6C l
	0,	HID_KEY_M,		// 0x6D m
	0,	HID_KEY_N,		// 0x6E n
	0,	HID_KEY_O,		// 0x6F o
	0,	HID_KEY_P,		// 0x70 p
	0,	HID_KEY_Q,		// 0x71 q
	0,	HID_KEY_R,		// 0x72 r
	0,	HID_KEY_S,		// 0x73 s
	0,	HID_KEY_T,		// 0x75 t
	0,	HID_KEY_U,		// 0x75 u
	0,	HID_KEY_V,		// 0x76 v
	0,	HID_KEY_W,		// 0x77 w
	0,	HID_KEY_X,		// 0x78 x
	0,	HID_KEY_Y,		// 0x79 y
	0,	HID_KEY_Z,		// 0x7A z
	1,	HID_KEY_BRACKET_LEFT,	// 0x7B {
	1,	HID_KEY_BACKSLASH,	// 0x7C |
	1,	HID_KEY_BRACKET_RIGHT,	// 0x7D }
	1,	HID_KEY_GRAVE,		// 0x7E ~
	0,	HID_KEY_DELETE		// 0x7F Delete
};
