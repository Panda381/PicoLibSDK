
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

#include "../usb_inc/sdk_usb_hid.h"

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_HID	// use USB HID Human Interface Device, value = number of interfaces (device)

#ifndef _SDK_USB_DEV_HID_H
#define _SDK_USB_DEV_HID_H

#include "../../_lib/inc/lib_ring.h"
#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USB_DEV_HID_RX_BUFSIZE
#define USB_DEV_HID_RX_BUFSIZE	16	// USB HID device RX buffer size
#endif

#ifndef USB_DEV_HID_TX_BUFSIZE
#define USB_DEV_HID_TX_BUFSIZE	32	// USB HID device TX buffer size
#endif

// HID device interface structure
typedef struct {
	Bool	used;		// interface is used
	u8	itf_num;	// interface number (zero based)
	u8	ep_in;		// endpoint index for input (send data from device to host)
	u8	ep_out;		// endpoint index for output (receive data from host to device)
	u8	protocol;	// interface protocol
	u8	mode;		// protocol mode USB_HID_PROT_* (0=Boot, 1=Report)
	u8	idle;		// idle rate

	volatile u8	rx_len;	// length of data in receive buffer (0 = no received data)
	volatile u8	tx_len;	// length of data in send buffer (0 = no data to send)
	u8	rx_buf[USB_DEV_HID_RX_BUFSIZE];	// receive buffer
	u8	tx_buf[USB_DEV_HID_TX_BUFSIZE];	// send buffer

	const sUsbDescHid*  desc_hid; // pointer to HID descriptor
} sUsbDevHidInter;

// HID interfaces
extern sUsbDevHidInter UsbDevHidInter[USE_USB_DEV_HID];

// get HID report callback (returns data length, 0 = error, report data will be in tx_buf)
//  itf_num ... interface number 
//  len ... requested data length (including rep_id, if used)
//  rep_id ... report ID (0=default)
//  rep_type ... report type
typedef u8 (*pUsbDevHidGetRepCB) (u8 itf_num, u8 len, u8 rep_id, u8 rep_type);
extern pUsbDevHidGetRepCB UsbDevHidGetRepCB; // called from IRQ

// set HID report callback
//  itf_num ... interface number 
//  len ... requested data length (including rep_id, if used)
//  rep_id ... report ID (0=default)
//  rep_type ... report type
typedef void (*pUsbDevHidSetRepCB) (u8 itf_num, u8 len, u8 rep_id, u8 rep_type);
extern pUsbDevHidSetRepCB UsbDevHidSetRepCB; // called from IRQ

// 2 interfaces: keyboard and mouse
#if USE_USB_DEV_HID == 2

// application device setup descriptor - keyboard and mouse
extern const sUsbDevSetupDesc UsbDevHidSetupDesc2;

// 1 interface
#else

// application device setup descriptor - keyboard (sUsbHidKey structure)
extern const sUsbDevSetupDesc UsbDevHidSetupDescKeyb;

// application device setup descriptor - mouse (sUsbHidMouse structure)
extern const sUsbDevSetupDesc UsbDevHidSetupDescMouse;

// application device setup descriptor - joystick (sUsbHidJoy structure)
extern const sUsbDevSetupDesc UsbDevHidSetupDescJoystick;

// application device setup descriptor - gamepad (sUsbHidPad structure)
extern const sUsbDevSetupDesc UsbDevHidSetupDescGamepad;

// application device setup descriptor - power control (sUsbHidPwr structure)
extern const sUsbDevSetupDesc UsbDevHidSetupDescPower;

#endif

// print report descriptor
void PrintRepDesc();

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
//  itf_num ... HID interface number
u8 UsbDevHidFindItf(u8 itf_num);

// check if device is mounted
//  hid_inx ... interface index
Bool UsbDevHidInxIsMounted(u8 hid_inx);
INLINE Bool UsbDevHidIsMounted() { return UsbDevHidInxIsMounted(0); }

// check if next report can be sent
//  hid_inx ... interface index
Bool UsbDevHidInxSendReady(u8 hid_inx);
INLINE Bool UsbDevHidSendReady() { return UsbDevHidInxSendReady(0); }

// check if Boot mode is selected (mode: boot=device active sends report, report=get report via callback)
//  hid_inx ... interface index
INLINE Bool UsbDevHidInxIsBoot(u8 hid_inx) { return UsbDevHidInter[hid_inx].mode == USB_HID_PROT_BOOT; }
INLINE Bool UsbDevHidIsBoot() { return UsbDevHidInxIsBoot(0); }

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by endpoint (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbDevHidFindAddr(u8 epinx);

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbDevHidNewItf(const sUsbDescItf* itf);

// send report from RX buffer
//  hid_inx ... interface index
void UsbDevHidInxSend(u8 hid_inx);

// send reports of all interfaces
void UsbDevHidAllSend();

// receive report to RX buffer
//  hid_inx ... interface index
void UsbDevHidInxRecv(u8 hid_inx);

// receive reports to all interfaces
void UsbDevHidAllRecv();

// initialize class driver
void UsbDevHidInit();

// terminate class driver
void UsbDevHidTerm();

// reset class driver
void UsbDevHidReset();

// open device class interface (returns size of used interface, 0=not supported)
u16 UsbDevHidOpen(const sUsbDescItf* itf, u16 max_len);

// process control transfer complete (returns False to stall)
Bool UsbDevHidCtrl(u8 stage);

// process data transfer complete
void UsbDevHidComp(u8 epinx, u8 xres, u16 len);

// schedule driver, synchronized packets in frames
//void UsbDevHidSof(u16 sof);

// schedule driver (raised with UsbRescheduleDrv())
void UsbDevHidSched();

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// save report to send buffer (returns data size, does not set tx_len)
//  itf_num ... HID interface number
//  buf ... buffer with report structure
//  len ... report length in bytes
//  rep_id ... report ID (0 = do not use)
u8 UsbDevHidTxSave(u8 itf_num, const void* buf, u8 len, u8 rep_id);

// send report (check UsbDevHidInxSendReady if next report can be sent; returns False if cannot send)
//  itf_num ... HID interface number
//  buf ... buffer with report structure
//  len ... report length in bytes
//  rep_id ... report ID (0 = do not use)
Bool UsbDevHidSendReport(u8 itf_num, const void* buf, u8 len, u8 rep_id);

// send keyboard report (returns False on error)
Bool UsbDevHidSendKeyRep(const sUsbHidKey* rep);

// send keyboard key (returna False on error), output speed 30 chars/sec
Bool UsbDevHidSendKey(u8 key, u8 modi);

// send keyboard character (returna False on error), output speed 30 chars/sec
// - The target computer must have an English keyboard layout in order to maintain the correct character mapping.
Bool UsbDevHidSendChar(char ch);

// send keyboard text (returna False on error), output speed 30 chars/sec
// - The target computer must have an English keyboard layout in order to maintain the correct character mapping.
Bool UsbDevHidSendText(const char* txt);

// send mouse report (returns False on error)
Bool UsbDevHidSendMouseRep(const sUsbHidMouse* rep);

// send mouse (returns False on error), output speed max 1000 rep/sec
Bool UsbDevHidSendMouse(s8 dx, s8 dy, Bool left, Bool right, Bool mid);

// send joystick report (returns False on error)
Bool UsbDevHidSendJoyRep(const sUsbHidJoy* rep);

// send gamepad report (returns False on error)
Bool UsbDevHidSendPadRep(const sUsbHidPad* rep);

// send power report (returns False on error)
Bool UsbDevHidSendPwrRep(const sUsbHidPwr* rep);

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_DEV_HID_H

#endif // USE_USB_DEV_HID	// use USB HID Human Interface Device, value = number of interfaces (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)


// convert ASCII character (0x00..0x7f) to HID key code
//	modifier = (AsciiToHidKey[char*2] != 0) ? USB_KEY_MODI_LSHIFT : 0;
//	key_code = AsciiToHidKey[char*2 + 1];
#define ASCII_TO_HIDKEY_NUM	0x80	// number of codes
extern const u8 AsciiToHidKey[2*ASCII_TO_HIDKEY_NUM];
