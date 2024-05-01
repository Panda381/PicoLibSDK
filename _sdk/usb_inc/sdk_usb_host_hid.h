
// ****************************************************************************
//
//                     USB Human Interface Device Class (host)
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
#if USE_USB_HOST	// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#if USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)

#ifndef _SDK_USB_HOST_HID_H
#define _SDK_USB_HOST_HID_H

#include "../../_lib/inc/lib_event.h"
//#include "../../_lib/inc/lib_ring.h"
#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USB_HOST_HID_RX_BUFSIZE
#define USB_HOST_HID_RX_BUFSIZE	32	// USB HID host RX buffer size
#endif

#ifndef USB_HOST_HID_TX_BUFSIZE
#define USB_HOST_HID_TX_BUFSIZE	16	// USB HID host TX buffer size
#endif

#ifndef USB_HOST_HID_REPSIZE
#define USB_HOST_HID_REPSIZE	256	// USB HID host buffer to load report descriptor
#endif

#ifndef USB_HOST_HID_KEY_BUFSIZE
#define USB_HOST_HID_KEY_BUFSIZE 32	// USB HID host keyboard ring buffer size
#endif

#ifndef USB_HOST_HID_MOUSE_BUFSIZE
#define USB_HOST_HID_MOUSE_BUFSIZE 16	// USB HID host mouse ring buffer size
#endif

// keyboard ring buffer (contains u32 packed keyboard event)
//   keyboard event - u32 number (only pressed key, not released)
//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
//    bit 8..15: modifiers USB_KEY_MODI_*
//    bit 16..23: ASCII character (NOCHAR = no valid character)
extern sEvent UsbHostHidKeyRingBuf[USB_HOST_HID_KEY_BUFSIZE];
extern sEventRing UsbHostHidKeyRing;

// mouse ring buffer (contains u32 packed mouse event)
//   mouse event - u32 number
//    bit 0..7: buttons mask USB_MOUSE_BTN_*, B7 is set to indicate valid mouse event
//    bit 8..15: delta X movement (signed s8)
//    bit 16..23: delta Y movement (signed s8)
//    bit 24..31: delta wheel movement (signed s8)
extern sEvent UsbHostHidMouseRingBuf[USB_HOST_HID_MOUSE_BUFSIZE];
extern sEventRing UsbHostHidMouseRing;

// debug: print report descriptor
//  0 = nothing
//  1 = print all
//  2 = print only interface info
//  3 = print only report info
//  4 = print only report descriptor HEX list
//extern u8 UsbHostHidPrintDesc;

// report info
typedef struct {
	u8	rep_id;		// report ID
	u8	usage;		// usage
	u16	usage_page;	// usage page
	u8	in_len;		// IN report length (in bytes)
	u8	out_len;	// OUT report length (in bytes)
	u8	in_num;		// IN number of entries
	u8	out_num;	// OUT number of entries
} sUsbHostHidRepInfo;

#define USB_HOST_HID_REP_MAX	8	// max. number of reports in info

extern u8 UsbHostRepBuf[USB_HOST_HID_REPSIZE]; // buffer to load report descriptor

// HID host interface structure
typedef struct {
	Bool	used;		// interface is used
	u8	dev_addr;	// device address
	u8	itf_num;	// interface number (zero based)
	u8	protocol;	// interface protocol
	u8	mode;		// protocol mode USB_HID_PROT_* (0=Boot, 1=Report)
	u8	ep_in;		// device endpoint index for input (send data from device to host)
	u8	ep_out;		// device endpoint index for output (receive data from host to device)
	u16	epin_max;	// max. size of packet for input
	u16	epout_max;	// max. size of packet for output

	u8	rep_type;	// report type
	u16	rep_len;	// report length

//	u16	repin_len;	// IN report length
//	u16	repout_len;	// OUT report length
	u8	rep_num;	// count of report info
	sUsbHostHidRepInfo rep_info[USB_HOST_HID_REP_MAX]; // report info list

	volatile u8	rx_len;	// length of data in receive buffer (0 = no received data)
	volatile u8	tx_len;	// length of data in send buffer (0 = no data to send)
	u8	ALIGNED rx_buf[USB_HOST_HID_RX_BUFSIZE]; // receive buffer
	u8	ALIGNED tx_buf[USB_HOST_HID_TX_BUFSIZE]; // send buffer
} sUsbHostHidInter;

// HID interfaces
extern sUsbHostHidInter UsbHostHidInter[USE_USB_HOST_HID];

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// check if device is mounted
//  hid_inx ... HID interface
Bool UsbHostHidInxIsMounted(u8 hid_inx);
INLINE Bool UsbHostHidIsMounted() { return UsbHostHidInxIsMounted(0); }

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostHidFindItf(u8 dev_addr, u8 itf_num);

// check if Boot mode is selected (mode: boot=device active sends report, report=get report via callback)
//  hid_inx ... interface index
INLINE Bool UsbHostHidInxIsBoot(u8 hid_inx) { return UsbHostHidInter[hid_inx].mode == USB_HID_PROT_BOOT; }
INLINE Bool UsbHostHidIsBoot() { return UsbHostHidInxIsBoot(0); }

// check if next report can be sent to send buffer
//  hid_inx ... interface index
Bool UsbHostHidInxSendReady(u8 hid_inx);
INLINE Bool UsbHostHidSendReady() { return UsbHostHidInxSendReady(0); }

// check if next report can be received from receive buffer
//  hid_inx ... interface index
Bool UsbHostHidInxRecvReady(u8 hid_inx);
INLINE Bool UsbHostHidRecvReady() { return UsbHostHidInxRecvReady(0); }

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by device address (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostHidFindAddr(u8 dev_addr, u8 dev_epinx);

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbHostHidNewItf(u8 dev_addr, const sUsbDescItf* itf);

// send report from TX buffer
//  hid_inx ... interface index
void UsbHostHidInxSend(u8 hid_inx);

// send reports of all interfaces
void UsbHostHidAllSend();

// receive report to Rx buffer
//  hid_inx ... interface index
void UsbHostHidInxRecv(u8 hid_inx);

// receive reports to all interfaces
void UsbHostHidAllRecv();

// initialize class driver
void UsbHostHidInit();

// terminate class driver
void UsbHostHidTerm();

// open class interface (returns size of used interface, 0=not supported)
// - At this point, no communications are allowed yet.
u16 UsbHostHidOpen(u8 dev_addr, const sUsbDescItf* itf, u16 max_len);

// set config interface
Bool UsbHostHidCfg(u8 dev_addr, u8 itf_num);

// transfer complete callback
Bool UsbHostHidComp(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len);

// close device
void UsbHostHidClose(u8 dev_addr);

// schedule driver, synchronized packets in frames
void UsbHostHidSof(u16 sof);

// schedule driver (raised with UsbRescheduleDrv())
void UsbHostHidSched();

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// save report to send buffer (returns data size, does not set tx_len)
// - can be called from UsbDevHidGetRepCB to save report to send buffer
//  hid_inx ... interface index
//  buf ... buffer with report structure
//  len ... report length in bytes, including rep_id byte if it is used
//  rep_id ... report ID (0 = do not use)
u8 UsbHostHidTxSave(u8 hid_inx, const void* buf, u8 len, u8 rep_id);

// send report (check UsbHostHidInxSendReady if next report can be sent; returns False if cannot send)
//  hid_inx ... interface index
//  buf ... buffer with report structure
//  len ... report length in bytes, including rep_id byte if it is used
//  rep_id ... report ID (0 = do not use)
Bool UsbHostHidSendReport(u8 hid_inx, const void* buf, u8 len, u8 rep_id);

// check if USB keyboard is mounted
Bool UsbKeyIsMounted();

// check if USB mouse is mounted
Bool UsbMouseIsMounted();

// get USB key (returns u32 packed keyboard event, or 0 if no key)
//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
//    bit 8..15: modifiers USB_KEY_MODI_*
//    bit 16..23: ASCII character CH_* (NOCHAR = no valid character)
u32 UsbGetKey();

// get USB key, including release key (returns u32 packed keyboard event, or 0 if no key)
//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
//    bit 8..15: modifiers USB_KEY_MODI_*
//    bit 16..23: ASCII character CH_* (NOCHAR = no valid character or release key)
//    bit 24: 0=press key, 1=release key
// In case of key release, ASCII character is invalid (= 0).
u32 UsbGetKeyRel();

// convert USB key to PC scan code
//    key ... USB key, as returned from UsbGetKeyRel() function (including release flag)
// Output:
//    bit 0..6: PC key scan code (0=invalid)
//    bit 7: 1=release key
//    bit 8: 1=extended key
u16 UsbKeyToScan(u32 key);

// check if key is pressed
Bool UsbKeyIsPressed(u8 key);

// get USB character (returns NOCHAR if no character)
char UsbGetChar();

// flush keys
void UsbFlushKey();

// get USB mouse (returns u32 packed mouse event, or 0 if no mouse event)
//    bit 0..7: buttons mask USB_MOUSE_BTN_*, B7 is set to indicate valid mouse event
//    bit 8..15: delta X movement (signed s8)
//    bit 16..23: delta Y movement (signed s8)
//    bit 24..31: delta wheel movement (signed s8) ... most mouse does not report wheel movement on uset boot mode
u32 UsbGetMouse();

// check no pressed key
Bool UsbKeyNoPressed();

// wait for no key pressed
void UsbKeyWaitNoPressed();

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_HOST_HID_H

#endif // USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)
#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)


// Convert HID key code to ASCII character
//	ascii = table[key_code*2 + shift]
#define HIDKEY_TO_ASCII_NUM	0x68	// number of codes
extern const char HidKeyToAscii[2*HIDKEY_TO_ASCII_NUM];

// Convert HID key code to PC scan code (B7: extended scan code with 0xE0 prefix)
#define HIDKEY_TO_SCAN1_FIRST	0x04	// first scan code of 1st part
#define HIDKEY_TO_SCAN1_LAST	0x65	// last scan code of 1st part
//   1st part: HID code 0x04..0x65
extern const u8 HidKeyToScan1[HIDKEY_TO_SCAN1_LAST+1-HIDKEY_TO_SCAN1_FIRST];

#define HIDKEY_TO_SCAN2_FIRST	0xE0	// first scan code of 2nd part
#define HIDKEY_TO_SCAN2_LAST	0xE7	// last scan code of 2nd part
//   2nd part: HID code 0xE0..0xE7
extern const u8 HidKeyToScan2[HIDKEY_TO_SCAN2_LAST+1-HIDKEY_TO_SCAN2_FIRST];
