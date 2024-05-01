
// ****************************************************************************
//
//               USB Human Interface Device Class - common definitions
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
#if USE_USB_DEV_HID || USE_USB_HOST_HID

#ifndef _SDK_USB_HID_H
#define _SDK_USB_HID_H

#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

// HID specification: https://www.usb.org/sites/default/files/hid1_11.pdf

// HID request report type
#define USB_HID_REP_TYPE_IN	1	// input
#define USB_HID_REP_TYPE_OUT	2	// output
#define USB_HID_REP_TYPE_FEAT	3	// feature

// HID request protocol type (get or set protocol request)
#define USB_HID_PROT_BOOT	0	// boot
#define USB_HID_PROT_REP	1	// report

// HID class specific control request
#define USB_HID_REQ_GET_REP	1	// get report (value = HIGH report type and LOW report ID, length = report length, data = report)
#define USB_HID_REQ_GET_IDLE	2	// get idle (value = LOW report ID, length = 1, data = idle rate byte)
#define USB_HID_REQ_GET_PROT	3	// get protocol (length = 1, data = 0 boot or 1 report, )
#define USB_HID_REQ_SET_REP	9	// set report (set LEDs; value = report type HIGH and report ID LOW, length = report length, data = report)
#define USB_HID_REQ_SET_IDLE	10	// set idle (request silence; value = HIGH duration and LOW report ID, length = 0)
#define USB_HID_REQ_SET_PROT	11	// set protocol (value = 0 boot or 1 report; length = 0)

// HID country code
#define USB_HID_COUNTRY_NONE	0	// not localized
#define USB_HID_COUNTRY_INT	13	// international (ISO)
#define USB_HID_COUNTRY_US	33	// USA

// report format
//  0: (1) report ID (if no report ID tags are used, there is only one report and report ID is omitted)
//  1: (x) report data

// keyboard modifiers
#define USB_KEY_MODI_LCTRL	B0	// left Control
#define USB_KEY_MODI_LSHIFT	B1	// left Shift
#define USB_KEY_MODI_LALT	B2	// left Alt
#define USB_KEY_MODI_LWIN	B3	// left Window (GUI)
#define USB_KEY_MODI_RCTRL	B4	// right Control
#define USB_KEY_MODI_RSHIFT	B5	// right Shift
#define USB_KEY_MODI_RALT	B6	// right Alt
#define USB_KEY_MODI_RWIN	B7	// right Window (GUI)

// keyboard LEDs
#define USB_KEY_LED_NUM		B0	// Num Lock LED
#define USB_KEY_LED_CAPS	B1	// CapsLock LED
#define USB_KEY_LED_SCROLL	B2	// Scroll Lock LED
#define USB_KEY_LED_COMP	B3	// Composition mode
#define USB_KEY_LED_KANA	B4	// Kana mode

// keyboard report (3 to 8 bytes; 6KRO = 6-key rollover; used by BIOS at boot time; 8 bytes)
typedef struct {
	u8	modi;	// 0: (1) modifiers mask USB_KEY_MODI_* (1 = modifier is pressed)
	u8	res;	// 1: (1) ... reserved, set to 0
	u8	key[6];	// 2: (6) key codes of currently pressed keys (0 = key is not pressed)
			// If user will press more than 6 keys, key will be
			// reported with phantom code HID_KEY_ERR_ROLLOVER=0x01
} sUsbHidKey;

// mouse buttons bitmap
#define USB_MOUSE_BTN_LEFT	B0	// left button
#define USB_MOUSE_BTN_RIGHT	B1	// right button
#define USB_MOUSE_BTN_MID	B2	// middle button
#define USB_MOUSE_BTN_BACK	B3	// backward button
#define USB_MOUSE_BTN_FOR	B4	// forward button

// mouse report (USB mice is polled at 125 Hz = 8 ms; 5 bytes))
typedef struct {
	u8	btn;	// 0: (1) buttons mask for currently pressed buttons
	s8	x;	// 1: (1) delta X movement
	s8	y;	// 2: (1) delta Y movement
	s8	wheel;	// 3: (1) delta wheel movement
	s8	pan;	// 4: (1) AC pan
}  sUsbHidMouse;

// joystick report (4 bytes)
typedef struct PACKED {
	s8	x;	// 0: (1) X postion
	s8	y;	// 1: (1) Y postion
	u8	btn;	// 2: (1) buttons
			//	bit 0..3: hat switch
			//	bit 4..7: button 1..4
	s8	throt;	// 3: (1) throttle
} sUsbHidJoy;

// gamepad report (11 bytes)
typedef struct PACKED {
	s8	x;	// 0: (1) delta x movement
	s8	y;	// 1: (1) delta y movement
	s8	z;	// 2: (1) delta z movement
	s8	rz;	// 3: (1) delta Rz rotation
	s8	rx;	// 4: (1) delta Rx rotation
	s8	ry;	// 5: (1) delta Ry rotation
	u8	hat;	// 6: (1) 8-bit hat direction switch
	u32	btn;	// 7: (4) buttons mask for currently pressed buttons
} sUsbHidPad;

// power control report (1 byte)
typedef struct PACKED {
	u8	val;	// value 0..3 (0=do nothing, 1=power off, 2 standby, 3 wakeup host)
} sUsbHidPwr;

// consumer control report (2 bytes)
typedef struct PACKED {
	u16	val;	// value 0..1000
} sUsbHidCtrl;

// === report descriptor
// HID report descriptor comes as collection of short and long items

// Short item:
// 0: (1) prefix byte
//  bit 0..1: (2 bits) size of optional data in bytes (0, 1, 2 or 4 bytes)
//  bit 2..3: (2 bits) type of this report descriptor (0=main, 1=global, 2=local, 3=reserved for long item)
//  bit 4..7: (4 bits) short item tag: function of the item (subtype)
// 1: (0, 1, 2 or 4) optional item data

// Long item:
// 0: (1) prefix byte = 0xfe
//  bit 0..1: (2 bits) size = 2 bytes
//  bit 2..3: (2 bits) type = 3
//  bit 4..7: (4 bits) tag = 0x0f
// 1: (1) data size in bytes
// 2: (1) long item tag
// 3: (x) item data

// data array on end of report field
#define HID_REPORT_DATA_0(data)				// +0x00
#define HID_REPORT_DATA_1(data)	, (data)		// +0x01
#define HID_REPORT_DATA_2(data)	, U16_U8_LE(data)	// +0x02
#define HID_REPORT_DATA_3(data)	, U32_U8_LE(data)	// +0x03

// prefix byte of report item
//  size ... size of data 0, 1, 2, 3 (3 means 4 bytes)
//  type ... type of this report descriptor HID_TYPE_MAIN=0, HID_TYPE_GLOBAL=1 or HID_TYPE_LOCAL=2
//  tag ... item tag (function, subtype)
//  data ... data
#define HID_REPORT_ITEM(size, type, tag, data) \
	((size) | ((type) << 2) | ((tag) << 4)) HID_REPORT_DATA_##size(data)

// type of report descriptor
#define HID_TYPE_MAIN	0		// +0x00
#define HID_TYPE_GLOBAL	1		// +0x04
#define HID_TYPE_LOCAL	2		// +0x08

// 'main' report descriptor item tags
#define HID_MAIN_INPUT		8		// input		+0x80
#define HID_MAIN_OUTPUT		9		// output		+0x90
#define HID_MAIN_COLLECTION	10		// collection		+0xA0
#define HID_MAIN_FEATURE	11		// feature		+0xB0
#define HID_MAIN_COLLECTION_END	12		// end of collection	+0xC0

#define HID_INPUT(x)		HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_INPUT, (x))		// 0x01+0x00+0x80 = 0x81 xx
#define HID_OUTPUT(x)		HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_OUTPUT, (x))		// 0x01+0x00+0x90 = 0x91 xx
#define HID_COLLECTION(x)	HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_COLLECTION, (x))	// 0x01+0x00+0xA0 = 0xA1 xx
#define HID_FEATURE(x)		HID_REPORT_ITEM(1, HID_TYPE_MAIN, HID_MAIN_FEATURE, (x))	// 0x01+0x00+0xB0 = 0xB1 xx
#define HID_COLLECTION_END	HID_REPORT_ITEM(0, HID_TYPE_MAIN, HID_MAIN_COLLECTION_END, 0)	// 0x00+0x00+0xC0 = 0xC0

// data following INPUT, OUTPUT and FEATURE (0, 1, 2 or 4 bytes - missing data are assumed to be 0):
//	bit 0: 0=data, 1=constant
//	bit 1: 0=array, 1=variable
//	bit 2: 0=absolute, 1=relative
//	bit 3: 0=no wrap, 1=wrap
//	bit 4: 0=linear, 1=non linear
//	bit 5: 0=preferred state, 1=no preffered
//	bit 6: 0=no null position, 1=null state
//	bit 7: 0=non volatile, 1=volatile
//	bit 8: 0=bit field, 1=buffered bytes
//	bit 9..31: 0=reserved

#define HID_DATA		0	// 0x00
#define HID_CONSTANT		B0	// 0x01

#define HID_ARRAY		0	// 0x00
#define HID_VARIABLE		B1	// 0x02

#define HID_ABSOLUTE		0	// 0x00
#define HID_RELATIVE		B2	// 0x04

#define HID_NO_WRAP		0	// 0x00
#define HID_WRAP		B3	// 0x08

#define HID_LINEAR		0 	// 0x00
#define HID_NON_LINEAR		B4	// 0x10

#define HID_PREFERRED_STATE	0	// 0x00
#define HID_NO_PREFERRED	B5	// 0x20

#define HID_NO_NULL_POSITION	0	// 0x00
#define HID_NULL_STATE		B6	// 0x40

#define HID_NON_VOLATILE	0	// 0x00
#define HID_VOLATILE		B7	// 0x80

#define HID_BITFIELD		0	// 0x00
#define HID_BUFFERED_BYTES	B8	// 0x1000 if used, data size must be 2 !

// data following COLLECTION:
//	0x00: physical (group of axes)
//	0x01: application (mouse, keyboard)
//	0x02: logical (interrelated data)
//	0x03: report
//	0x04: named array
//	0x05: usage switch
//	0x06: usage modifier

#define HID_COLLECTION_PHYSICAL		0	// 0x00
#define HID_COLLECTION_APPLICATION	1	// 0x01
#define HID_COLLECTION_LOGICAL		2	// 0x02
#define HID_COLLECTION_REPORT		3	// 0x03
#define HID_COLLECTION_NAMED_ARRAY	4	// 0x04
#define HID_COLLECTION_USAGE_SWITCH	5	// 0x05
#define HID_COLLECTION_USAGE_MODIFIER	6	// 0x06

// 'global' report item tags
#define HID_GLOBAL_USAGE_PAGE		0	// usage page			+0x00
#define HID_GLOBAL_LOGICAL_MIN		1	// logical minimum value	+0x10
#define HID_GLOBAL_LOGICAL_MAX		2	// logical maximum value	+0x20
#define HID_GLOBAL_PHYSICAL_MIN		3	// physical minimum value	+0x30
#define HID_GLOBAL_PHYSICAL_MAX		4	// physical maximum value	+0x40
#define HID_GLOBAL_UNIT_EXPONENT	5	// unit exponent in base 2	+0x50
#define HID_GLOBAL_UNIT			6	// unit values			+0x60
#define HID_GLOBAL_REPORT_SIZE		7	// report size in bits		+0x70
#define HID_GLOBAL_REPORT_ID		8	// report ID			+0x80
#define HID_GLOBAL_REPORT_COUNT		9	// report count			+0x90
#define HID_GLOBAL_PUSH			10	// push global state on stack	+0xA0
#define HID_GLOBAL_POP			11	// pop global state from stack	+0xB0

#define HID_USAGE_PAGE(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_USAGE_PAGE, (x))		// 0x01+0x04+0x00 = 0x05 xx
#define HID_USAGE_PAGE2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_USAGE_PAGE, (x))		// 0x02+0x04+0x00 = 0x06 xx xx

#define HID_LOGICAL_MIN(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MIN, (x))	// 0x01+0x04+0x10 = 0x15 xx
#define HID_LOGICAL_MIN2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MIN, (x))	// 0x02+0x04+0x10 = 0x16 xx xx

#define HID_LOGICAL_MAX(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MAX, (x))	// 0x01+0x04+0x20 = 0x25 xx
#define HID_LOGICAL_MAX2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_LOGICAL_MAX, (x))	// 0x02+0x04+0x20 = 0x26 xx xx

#define HID_PHYSICAL_MIN(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MIN, (x))	// 0x01+0x04+0x30 = 0x35 xx
#define HID_PHYSICAL_MIN2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MIN, (x))	// 0x02+0x04+0x30 = 0x36 xx xx

#define HID_PHYSICAL_MAX(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MAX, (x))	// 0x01+0x04+0x40 = 0x45 xx
#define HID_PHYSICAL_MAX2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_PHYSICAL_MAX, (x))	// 0x02+0x04+0x40 = 0x46 xx xx

#define HID_UNIT_EXPONENT(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT_EXPONENT, (x))	// 0x01+0x04+0x50 = 0x55 xx
#define HID_UNIT_EXPONENT2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT_EXPONENT, (x))	// 0x02+0x04+0x50 = 0x56 xx xx

#define HID_UNIT(x)			HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT, (x))		// 0x01+0x04+0x60 = 0x65 xx
#define HID_UNIT2(x)			HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_UNIT, (x))		// 0x02+0x04+0x60 = 0x66 xx xx

#define HID_REPORT_SIZE(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_SIZE, (x))	// 0x01+0x04+0x70 = 0x75 xx
#define HID_REPORT_SIZE2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_SIZE, (x))	// 0x02+0x04+0x70 = 0x76 xx xx

#define HID_REPORT_ID(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_ID, (x))		// 0x01+0x04+0x80 = 0x85 xx
#define HID_REPORT_ID2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_ID, (x))		// 0x02+0x04+0x80 = 0x86 xx xx

#define HID_REPORT_COUNT(x)		HID_REPORT_ITEM(1, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_COUNT, (x))	// 0x01+0x04+0x90 = 0x95 xx
#define HID_REPORT_COUNT2(x)		HID_REPORT_ITEM(2, HID_TYPE_GLOBAL, HID_GLOBAL_REPORT_COUNT, (x))	// 0x02+0x04+0x90 = 0x96 xx xx

#define HID_PUSH			HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_PUSH, 0)			// 0x00+0x04+0xA0 = 0xA4
#define HID_POP				HID_REPORT_ITEM(0, HID_TYPE_GLOBAL, HID_GLOBAL_POP, 0)			// 0x00+0x04+0xB0 = 0xB4

// 'local' report descriptor item tags
#define HID_LOCAL_USAGE			0	// item usage		+0x00
#define HID_LOCAL_USAGE_MIN		1	// usage minimum	+0x10
#define HID_LOCAL_USAGE_MAX		2	// usage maximum	+0x20
#define HID_LOCAL_DESIGNATOR_INX	3	// designator index	+0x30
#define HID_LOCAL_DESIGNATOR_MIN	4	// designator minimum	+0x40
#define HID_LOCAL_DESIGNATOR_MAX	5	// designator maximum	+0x50
#define HID_LOCAL_STRING_INX		7	// string index		+0x70
#define HID_LOCAL_STRING_MIN		8	// string minimum	+0x80
#define HID_LOCAL_STRING_MAX		9	// string maximum	+0x90
#define HID_LOCAL_DELIMITER		10	// delimiter		+0xA0

#define HID_USAGE(x)			HID_REPORT_ITEM(1, HID_TYPE_LOCAL, HID_LOCAL_USAGE, (x))		// 0x01+0x08+0x00 = 0x09 xx
#define HID_USAGE2(x)			HID_REPORT_ITEM(2, HID_TYPE_LOCAL, HID_LOCAL_USAGE, (x))		// 0x02+0x08+0x00 = 0x0A xx xx

#define HID_USAGE_MIN(x)		HID_REPORT_ITEM(1, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MIN, (x))		// 0x01+0x08+0x10 = 0x19 xx
#define HID_USAGE_MIN2(x)		HID_REPORT_ITEM(2, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MIN, (x))		// 0x02+0x08+0x10 = 0x1A xx xx

#define HID_USAGE_MAX(x)		HID_REPORT_ITEM(1, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MAX, (x))		// 0x01+0x08+0x20 = 0x29 xx
#define HID_USAGE_MAX2(x)		HID_REPORT_ITEM(2, HID_TYPE_LOCAL, HID_LOCAL_USAGE_MAX, (x))		// 0x02+0x08+0x20 = 0x2A xx xx

// usage page
#define HID_USAGE_PAGE_DESKTOP		0x01
#define HID_USAGE_PAGE_SIMULATE		0x02
#define HID_USAGE_PAGE_VIRTUAL_REALITY	0x03
#define HID_USAGE_PAGE_SPORT		0x04
#define HID_USAGE_PAGE_GAME		0x05
#define HID_USAGE_PAGE_GENERIC_DEVICE	0x06
#define HID_USAGE_PAGE_KEYBOARD		0x07
#define HID_USAGE_PAGE_LED		0x08
#define HID_USAGE_PAGE_BUTTON		0x09
#define HID_USAGE_PAGE_ORDINAL		0x0a
#define HID_USAGE_PAGE_TELEPHONY	0x0b
#define HID_USAGE_PAGE_CONSUMER		0x0c
#define HID_USAGE_PAGE_DIGITIZER	0x0d
#define HID_USAGE_PAGE_PID		0x0f
#define HID_USAGE_PAGE_UNICODE		0x10
#define HID_USAGE_PAGE_ALPHA_DISPLAY	0x14
#define HID_USAGE_PAGE_MEDICAL		0x40
#define HID_USAGE_PAGE_MONITOR		0x80	// 0x80 - 0x83
#define HID_USAGE_PAGE_POWER		0x84	// 0x84 - 0x87
#define HID_USAGE_PAGE_BARCODE_SCANNER	0x8c
#define HID_USAGE_PAGE_SCALE		0x8d
#define HID_USAGE_PAGE_MSR		0x8e
#define HID_USAGE_PAGE_CAMERA		0x90
#define HID_USAGE_PAGE_ARCADE		0x91
#define HID_USAGE_PAGE_FIDO		0xF1D0	// FIDO alliance HID usage page
#define HID_USAGE_PAGE_VENDOR		0xFF00	// 0xFF00 - 0xFFFF

// generic desktop page
#define HID_USAGE_DESKTOP_POINTER				0x01
#define HID_USAGE_DESKTOP_MOUSE					0x02
#define HID_USAGE_DESKTOP_JOYSTICK				0x04
#define HID_USAGE_DESKTOP_GAMEPAD				0x05
#define HID_USAGE_DESKTOP_KEYBOARD				0x06
#define HID_USAGE_DESKTOP_KEYPAD				0x07
#define HID_USAGE_DESKTOP_MULTI_AXIS_CONTROLLER			0x08
#define HID_USAGE_DESKTOP_TABLET_PC_SYSTEM			0x09
#define HID_USAGE_DESKTOP_X					0x30
#define HID_USAGE_DESKTOP_Y					0x31
#define HID_USAGE_DESKTOP_Z					0x32
#define HID_USAGE_DESKTOP_RX					0x33
#define HID_USAGE_DESKTOP_RY					0x34
#define HID_USAGE_DESKTOP_RZ					0x35
#define HID_USAGE_DESKTOP_SLIDER				0x36
#define HID_USAGE_DESKTOP_DIAL					0x37
#define HID_USAGE_DESKTOP_WHEEL					0x38
#define HID_USAGE_DESKTOP_HAT_SWITCH				0x39
#define HID_USAGE_DESKTOP_COUNTED_BUFFER			0x3A
#define HID_USAGE_DESKTOP_BYTE_COUNT				0x3B
#define HID_USAGE_DESKTOP_MOTION_WAKEUP				0x3C
#define HID_USAGE_DESKTOP_START					0x3D
#define HID_USAGE_DESKTOP_SELECT				0x3E
#define HID_USAGE_DESKTOP_VX					0x40
#define HID_USAGE_DESKTOP_VY					0x41
#define HID_USAGE_DESKTOP_VZ					0x42
#define HID_USAGE_DESKTOP_VBRX					0x43
#define HID_USAGE_DESKTOP_VBRY					0x44
#define HID_USAGE_DESKTOP_VBRZ					0x45
#define HID_USAGE_DESKTOP_VNO					0x46
#define HID_USAGE_DESKTOP_FEATURE_NOTIFICATION			0x47
#define HID_USAGE_DESKTOP_RESOLUTION_MULTIPLIER			0x48
#define HID_USAGE_DESKTOP_SYSTEM_CONTROL			0x80
#define HID_USAGE_DESKTOP_SYSTEM_POWER_DOWN			0x81
#define HID_USAGE_DESKTOP_SYSTEM_SLEEP				0x82
#define HID_USAGE_DESKTOP_SYSTEM_WAKE_UP			0x83
#define HID_USAGE_DESKTOP_SYSTEM_CONTEXT_MENU			0x84
#define HID_USAGE_DESKTOP_SYSTEM_MAIN_MENU			0x85
#define HID_USAGE_DESKTOP_SYSTEM_APP_MENU			0x86
#define HID_USAGE_DESKTOP_SYSTEM_MENU_HELP			0x87
#define HID_USAGE_DESKTOP_SYSTEM_MENU_EXIT			0x88
#define HID_USAGE_DESKTOP_SYSTEM_MENU_SELECT			0x89
#define HID_USAGE_DESKTOP_SYSTEM_MENU_RIGHT			0x8A
#define HID_USAGE_DESKTOP_SYSTEM_MENU_LEFT			0x8B
#define HID_USAGE_DESKTOP_SYSTEM_MENU_UP			0x8C
#define HID_USAGE_DESKTOP_SYSTEM_MENU_DOWN			0x8D
#define HID_USAGE_DESKTOP_SYSTEM_COLD_RESTART			0x8E
#define HID_USAGE_DESKTOP_SYSTEM_WARM_RESTART			0x8F
#define HID_USAGE_DESKTOP_DPAD_UP				0x90
#define HID_USAGE_DESKTOP_DPAD_DOWN				0x91
#define HID_USAGE_DESKTOP_DPAD_RIGHT				0x92
#define HID_USAGE_DESKTOP_DPAD_LEFT				0x93
#define HID_USAGE_DESKTOP_SYSTEM_DOCK				0xA0
#define HID_USAGE_DESKTOP_SYSTEM_UNDOCK				0xA1
#define HID_USAGE_DESKTOP_SYSTEM_SETUP				0xA2
#define HID_USAGE_DESKTOP_SYSTEM_BREAK				0xA3
#define HID_USAGE_DESKTOP_SYSTEM_DEBUGGER_BREAK			0xA4
#define HID_USAGE_DESKTOP_APPLICATION_BREAK			0xA5
#define HID_USAGE_DESKTOP_APPLICATION_DEBUGGER_BREAK		0xA6
#define HID_USAGE_DESKTOP_SYSTEM_SPEAKER_MUTE			0xA7
#define HID_USAGE_DESKTOP_SYSTEM_HIBERNATE			0xA8
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_INVERT			0xB0
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_INTERNAL		0xB1
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_EXTERNAL		0xB2
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_BOTH			0xB3
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_DUAL			0xB4
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_TOGGLE_INT_EXT		0xB5
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_SWAP_PRIMARY_SECONDARY	0xB6
#define HID_USAGE_DESKTOP_SYSTEM_DISPLAY_LCD_AUTOSCALE		0xB7
#define HID_USAGE_DESKTOP_THROTTLE				0xBB

// consumer page (part)
// - Generic Control
#define HID_USAGE_CONSUMER_CONTROL				0x0001
// - Power Control
#define HID_USAGE_CONSUMER_POWER				0x0030
#define HID_USAGE_CONSUMER_RESET				0x0031
#define HID_USAGE_CONSUMER_SLEEP				0x0032
// - Screen Brightness
#define HID_USAGE_CONSUMER_BRIGHTNESS_INCREMENT			0x006F
#define HID_USAGE_CONSUMER_BRIGHTNESS_DECREMENT			0x0070
// - Mobile systems with Windows 8
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_CONTROLS		0x000C
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_BUTTONS		0x00C6
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_LED			0x00C7
#define HID_USAGE_CONSUMER_WIRELESS_RADIO_SLIDER_SWITCH		0x00C8
// - Media Control
#define HID_USAGE_CONSUMER_PLAY_PAUSE				0x00CD
#define HID_USAGE_CONSUMER_SCAN_NEXT				0x00B5
#define HID_USAGE_CONSUMER_SCAN_PREVIOUS			0x00B6
#define HID_USAGE_CONSUMER_STOP					0x00B7
#define HID_USAGE_CONSUMER_VOLUME				0x00E0
#define HID_USAGE_CONSUMER_MUTE					0x00E2
#define HID_USAGE_CONSUMER_BASS					0x00E3
#define HID_USAGE_CONSUMER_TREBLE				0x00E4
#define HID_USAGE_CONSUMER_BASS_BOOST				0x00E5
#define HID_USAGE_CONSUMER_VOLUME_INCREMENT			0x00E9
#define HID_USAGE_CONSUMER_VOLUME_DECREMENT			0x00EA
#define HID_USAGE_CONSUMER_BASS_INCREMENT			0x0152
#define HID_USAGE_CONSUMER_BASS_DECREMENT			0x0153
#define HID_USAGE_CONSUMER_TREBLE_INCREMENT			0x0154
#define HID_USAGE_CONSUMER_TREBLE_DECREMENT			0x0155
// - Application Launcher
#define HID_USAGE_CONSUMER_AL_CONSUMER_CONTROL_CONFIGURATION	0x0183
#define HID_USAGE_CONSUMER_AL_EMAIL_READER			0x018A
#define HID_USAGE_CONSUMER_AL_CALCULATOR			0x0192
#define HID_USAGE_CONSUMER_AL_LOCAL_BROWSER			0x0194
// - Browser/Explorer Specific
#define HID_USAGE_CONSUMER_AC_SEARCH				0x0221
#define HID_USAGE_CONSUMER_AC_HOME				0x0223
#define HID_USAGE_CONSUMER_AC_BACK				0x0224
#define HID_USAGE_CONSUMER_AC_FORWARD				0x0225
#define HID_USAGE_CONSUMER_AC_STOP				0x0226
#define HID_USAGE_CONSUMER_AC_REFRESH				0x0227
#define HID_USAGE_CONSUMER_AC_BOOKMARKS				0x022A
// - Mouse Horizontal scroll
#define HID_USAGE_CONSUMER_AC_PAN				0x0238

// FIDO alliance page (0xF1D0)
#define HID_USAGE_FIDO_U2FHID		0x01	// U2FHID usage for top-level collection
#define HID_USAGE_FIDO_DATA_IN		0x20	// Raw IN data report
#define HID_USAGE_FIDO_DATA_OUT		0x21	// Raw OUT data report

// keyboard report descriptor template (sUsbHidKey structure, 8 bytes, descriptor size 67 bytes)
//	u8	modi;	// 0: (1) modifiers mask USB_KEY_MODI_* (1 = modifier is pressed)
//	u8	res;	// 1: (1) ... reserved, set to 0
//	u8	key[6];	// 2: (6) key codes of currently pressed keys (0 = key is not pressed)
#define HID_REPORT_DESC_KEYBOARD \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), /* 0x05 0x01 */ \
	HID_USAGE      ( HID_USAGE_DESKTOP_KEYBOARD ), /* 0x09 0x06 */ \
	HID_COLLECTION ( HID_COLLECTION_APPLICATION ), /* 0xA1 0x01 */ \
		/* Input: 8 bits modifier keys (Shift, Control, Alt,... codes 0xE0=224 to 0xE7=231) */ \
		HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ), /* 0x05 0x07 */ \
			HID_USAGE_MIN2   ( 224 ),	/* 0x1A 0xE0 0x00, minimal key code 0xE0=224 */ \
			HID_USAGE_MAX2   ( 231 ),	/* 0x2A 0xE7 0x00, maximal key code 0xE7=231 */ \
			HID_LOGICAL_MIN  ( 0 ),		/* 0x15 0x00, minimal value of modifier flag */ \
			HID_LOGICAL_MAX  ( 1 ),		/* 0x25 0x01, maximal value of modifier flag */ \
			HID_REPORT_COUNT ( 8 ),		/* 0x95 0x08, number of elements = 8 bits */ \
			HID_REPORT_SIZE  ( 1 ),		/* 0x75 0x01, size of element = 1 bit */ \
			HID_INPUT ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* 0x81 0x02, this elements contain absolute data */ \
			/* 8 bits reserved */ \
			HID_REPORT_COUNT ( 1 ),		/* 0x95 0x01, number of elements = 1 byte */ \
			HID_REPORT_SIZE  ( 8 ),		/* 0x75 0x08, size of element = 8 bits */ \
			HID_INPUT ( HID_CONSTANT ),	/* 0x81 0x01, this reserved element contains constant */ \
		/* Output: 5-bit LED Indicator Kana | Compose | ScrollLock | CapsLock | NumLock */ \
		HID_USAGE_PAGE ( HID_USAGE_PAGE_LED ),	/* 0x05 0x08 */ \
			HID_USAGE_MIN    ( 1 ),		/* 0x19 0x01, minimal LED code */ \
			HID_USAGE_MAX    ( 5 ),		/* 0x29 0x05, maximal LED code */ \
			HID_REPORT_COUNT ( 5 ),		/* 0x95 0x05, number of elements = 5 bits */ \
			HID_REPORT_SIZE  ( 1 ),		/* 0x75 0x01, size of element = 1 bit */ \
			HID_OUTPUT ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* 0x91 0x02, this elements contain absolute data */ \
			/* led padding */ \
			HID_REPORT_COUNT ( 1 ),		/* 0x95 0x01, number of elements = 1 */ \
			HID_REPORT_SIZE  ( 3 ),		/* 0x75 0x03, size of element = 3 bits */ \
			HID_OUTPUT ( HID_CONSTANT ),	/* 0x91 0x01, this reserved element contains constant */ \
		/* Input: 6-byte keycodes */ \
		HID_USAGE_PAGE ( HID_USAGE_PAGE_KEYBOARD ), /* 0x05 0x07 */ \
			HID_USAGE_MIN    ( 0 ),		/* 0x19 0x00, minimal key code 0 */ \
			HID_USAGE_MAX2   ( 255 ),	/* 0x2A 0xFF 0x00, maximal key code 0xFF=255 */ \
			HID_LOGICAL_MIN  ( 0 ),		/* 0x15 0x00, minimal value of key */ \
			HID_LOGICAL_MAX2 ( 255 ),	/* 0x26 0xFF 0x00, maximal value of key */ \
			HID_REPORT_COUNT ( 6 ),		/* 0x95 0x06, number of elements = 6 bytes */ \
			HID_REPORT_SIZE  ( 8 ),		/* 0x75 0x08, size of element = 8 bits */ \
			HID_INPUT ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ), /* 0x81 0x00, this elements contain absolute data array */ \
	HID_COLLECTION_END /* 0xC0 */

// mouse report descriptor template (sUsbHidMouse structure, 5 bytes)
//	u8	btn;	// 0: (1) buttons mask for currently pressed buttons
//	s8	x;	// 1: (1) delta X movement
//	s8	y;	// 2: (1) delta Y movement
//	s8	wheel;	// 3: (1) delta wheel movement
//	s8	pan;	// 4: (1) AC pan
#define HID_REPORT_DESC_MOUSE \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), \
	HID_USAGE      ( HID_USAGE_DESKTOP_MOUSE ), \
	HID_COLLECTION ( HID_COLLECTION_APPLICATION ), \
		HID_USAGE      ( HID_USAGE_DESKTOP_POINTER ), \
		HID_COLLECTION ( HID_COLLECTION_PHYSICAL ), \
			/* mouse buttons */ \
			HID_USAGE_PAGE ( HID_USAGE_PAGE_BUTTON ), \
				HID_USAGE_MIN   ( 1 ),		/* minimal button code */ \
				HID_USAGE_MAX   ( 5 ),		/* maximal button code */ \
				HID_LOGICAL_MIN ( 0 ),		/* minimal button value */ \
				HID_LOGICAL_MAX ( 1 ),		/* maximal button value */ \
				/* Left, Right, Middle, Backward, Forward buttons */ \
				HID_REPORT_COUNT( 5 ),		/* number of elements = 5 bits */ \
				HID_REPORT_SIZE ( 1 ),		/* size of one element = 1 bit */ \
				HID_INPUT ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
				/* 3 bit padding */ \
				HID_REPORT_COUNT( 1 ),		/* number of elements = 1 */ \
				HID_REPORT_SIZE ( 3 ),		/* size of element = 3 bits */ \
				HID_INPUT ( HID_CONSTANT ),	/* this reserved element contains constant */ \
			/* mouse delta movement */ \
			HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), \
				/* X, Y position [-127, 127] */ \
				HID_USAGE       ( HID_USAGE_DESKTOP_X ), /* X coordinate */ \
				HID_USAGE       ( HID_USAGE_DESKTOP_Y ), /* Y coordinate */ \
				HID_LOGICAL_MIN ( 0x81 ),	/* minimal value = -127 */ \
				HID_LOGICAL_MAX ( 0x7f ),	/* maximal value = +127 */ \
				HID_REPORT_COUNT( 2 ),		/* number of elements = 2 bytes */ \
				HID_REPORT_SIZE ( 8 ),		/* size of element = 8 bits */ \
				HID_INPUT ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* this elements contain relative data */ \
				/* Verital wheel scroll [-127, 127] */ \
				HID_USAGE       ( HID_USAGE_DESKTOP_WHEEL ), /* mouse wheel */ \
				HID_LOGICAL_MIN ( 0x81 ),	/* minimal value = -127 */ \
				HID_LOGICAL_MAX ( 0x7f ),	/* maximal value = +127 */ \
				HID_REPORT_COUNT( 1 ),		/* number of elements = 1 byte */ \
				HID_REPORT_SIZE ( 8 ),		/* size of element = 8 bits */ \
				HID_INPUT ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* this elements contain relative data */ \
			/* mouse AC pan - horizontal scroll */ \
			HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ), \
				/* Horizontal wheel scroll [-127, 127] */ \
				HID_USAGE2      ( HID_USAGE_CONSUMER_AC_PAN ), /* horizontal scroll */ \
				HID_LOGICAL_MIN ( 0x81 ),	/* minimal value = -127 */ \
				HID_LOGICAL_MAX ( 0x7f ),	/* maximal value = +127 */ \
				HID_REPORT_COUNT( 1 ),		/* number of elements = 1 byte */ \
				HID_REPORT_SIZE ( 8 ),		/* size of element = 8 bits */ \
				HID_INPUT ( HID_DATA | HID_VARIABLE | HID_RELATIVE ), /* this elements contain relative data */ \
		HID_COLLECTION_END, \
	HID_COLLECTION_END

// joystick report descriptor template (sUsbHidJoy structure, 4 bytes)
//	s8	x;	// 0: (1) X postion
//	s8	y;	// 1: (1) Y postion
//	u8	btn;	// 2: (1) buttons (bit 0..3: hat switch, bit 4..7: button 1..4)
//	s8	throt;	// 3: (1) throttle
#define HID_REPORT_DESC_JOYSTICK \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), \
	HID_USAGE      ( HID_USAGE_DESKTOP_JOYSTICK ), \
	HID_COLLECTION ( HID_COLLECTION_APPLICATION ), \
		HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), \
		HID_USAGE      ( HID_USAGE_DESKTOP_POINTER ), \
		HID_COLLECTION ( HID_COLLECTION_PHYSICAL ), \
			/* 8-bit X, Y position (min -127, max 127 ) */ \
			HID_USAGE       ( HID_USAGE_DESKTOP_X ), /* X coordinate */ \
			HID_USAGE       ( HID_USAGE_DESKTOP_Y ), /* Y coordinate */ \
			HID_LOGICAL_MIN ( 0x81 ),	/* minimal value = -127 */ \
			HID_LOGICAL_MAX ( 0x7f ),	/* maximal value = +127 */ \
			HID_REPORT_COUNT( 2 ),		/* number of elements = 2 bytes */ \
			HID_REPORT_SIZE ( 8 ),		/* size of element = 8 bits */ \
			HID_INPUT ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
			/* 4-bit Hat direction switch, 4 directions: 0, 90, 180, 270 degrees  */ \
			HID_USAGE          ( HID_USAGE_DESKTOP_HAT_SWITCH ), \
			HID_LOGICAL_MIN    ( 0 ),	/* minimal value = 0 */ \
			HID_LOGICAL_MAX    ( 3 ),	/* maximal value = 3 */ \
			HID_PHYSICAL_MIN   ( 0 ),	/* minimal physical value = 0 degrees */ \
			HID_PHYSICAL_MAX2  ( 270 ),	/* maximal physical value = 270 degrees */ \
			HID_REPORT_COUNT   ( 1 ),	/* number of elements = 1 */ \
			HID_REPORT_SIZE    ( 4 ),	/* size of element = 4 bits */ \
			HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE | HID_NULL_STATE ), /* this elements contain absolute data */ \
			/* 4-bit button map */ \
			HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON ), \
			HID_USAGE_MIN      ( 1 ),	/* minimal code = 1 */ \
			HID_USAGE_MAX      ( 4 ),	/* maximal code = 4 */ \
			HID_LOGICAL_MIN    ( 0 ),	/* minimal value = 0 */ \
			HID_LOGICAL_MAX    ( 1 ),	/* maximal value = 1 */ \
			HID_REPORT_COUNT   ( 4 ),	/* number of elements = 4 bits */ \
			HID_REPORT_SIZE    ( 1 ),	/* size of element = 1 bit */ \
			HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
		HID_COLLECTION_END, \
		/* 8-bit throttle (min -127, max 127 ) */ \
		HID_USAGE_PAGE ( HID_USAGE_PAGE_SIMULATE ), \
		HID_USAGE      ( HID_USAGE_DESKTOP_THROTTLE ), \
		HID_LOGICAL_MIN ( 0x81 ),	/* minimal value = -127 */ \
		HID_LOGICAL_MAX ( 0x7f ),	/* maximal value = +127 */ \
		HID_REPORT_COUNT( 1 ),		/* number of elements = 2 bytes */ \
		HID_REPORT_SIZE ( 8 ),		/* size of element = 8 bits */ \
		HID_INPUT ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
	HID_COLLECTION_END

// gamepad report descriptor template (sUsbHidPad structure, 11 bytes)
//  with 32 buttons, 2 joysticks and 1 hat/dpad
//	s8	x;	// 0: (1) delta x movement
//	s8	y;	// 1: (1) delta y movement
//	s8	z;	// 2: (1) delta z movement
//	s8	rz;	// 3: (1) delta Rz rotation
//	s8	rx;	// 4: (1) delta Rx rotation
//	s8	ry;	// 5: (1) delta Ry rotation
//	u8	hat;	// 6: (1) 8-bit hat direction switch
//	u32	btn;	// 7: (4) 32 buttons mask for currently pressed buttons
#define HID_REPORT_DESC_GAMEPAD \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), \
	HID_USAGE      ( HID_USAGE_DESKTOP_GAMEPAD ), \
	HID_COLLECTION ( HID_COLLECTION_APPLICATION ), \
		/* 8-bit X, Y, Z, Rz, Rx, Ry delta (min -127, max 127 ) */ \
		HID_USAGE_PAGE   ( HID_USAGE_PAGE_DESKTOP ), \
		HID_USAGE        ( HID_USAGE_DESKTOP_X ),	/* delta x movement */ \
		HID_USAGE        ( HID_USAGE_DESKTOP_Y ),	/* delta y movement */ \
		HID_USAGE        ( HID_USAGE_DESKTOP_Z ),	/* delta z movement */ \
		HID_USAGE        ( HID_USAGE_DESKTOP_RZ ),	/* delta z rotation */ \
		HID_USAGE        ( HID_USAGE_DESKTOP_RX ),	/* delta x rotation */ \
		HID_USAGE        ( HID_USAGE_DESKTOP_RY ),	/* delta y rotation */ \
		HID_LOGICAL_MIN  ( 0x81 ),			/* minimal value = -127 */ \
		HID_LOGICAL_MAX  ( 0x7f ),			/* maximal value = +127 */ \
		HID_REPORT_COUNT ( 6 ),				/* number of elements = 6 bytes */ \
		HID_REPORT_SIZE  ( 8 ),				/* size of element = 8 bits */ \
		HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
		/* 8-bit Hat direction switch, 8 directions: 0, 45, ... 315 degrees */ \
		HID_USAGE_PAGE     ( HID_USAGE_PAGE_DESKTOP ), \
		HID_USAGE          ( HID_USAGE_DESKTOP_HAT_SWITCH ), \
		HID_LOGICAL_MIN    ( 1 ),			/* minimal value = 1 */ \
		HID_LOGICAL_MAX    ( 8 ),			/* maximal value = 8 */ \
		HID_PHYSICAL_MIN   ( 0 ),			/* minimal physical value = 0 degrees */ \
		HID_PHYSICAL_MAX2  ( 315 ),			/* maximal physical value = 315 degrees */ \
		HID_REPORT_COUNT   ( 1 ),			/* number of elements = 1 byte */ \
		HID_REPORT_SIZE    ( 8 ),			/* size of element = 8 bits */ \
		HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
		/* 32-bit Button Map */ \
		HID_USAGE_PAGE     ( HID_USAGE_PAGE_BUTTON ), \
		HID_USAGE_MIN      ( 1 ),			/* minimal code = 1 */ \
		HID_USAGE_MAX      ( 32 ),			/* maximal code = 32 */ \
		HID_LOGICAL_MIN    ( 0 ),			/* minimal value = 0 */ \
		HID_LOGICAL_MAX    ( 1 ),			/* maximal value = 1 */ \
		HID_REPORT_COUNT   ( 32 ),			/* number of elements = 32 bits */ \
		HID_REPORT_SIZE    ( 1 ),			/* size of element = 1 bit */ \
		HID_INPUT          ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), /* this elements contain absolute data */ \
	HID_COLLECTION_END

// system power control report descriptor template (sUsbHidPwr structure, 1 byte)
//	u8	val;	// value 0..3 (0=do nothing, 1=power off, 2 standby, 3 wakeup host)
#define HID_REPORT_DESC_SYSTEM_CONTROL \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_DESKTOP ), \
	HID_USAGE      ( HID_USAGE_DESKTOP_SYSTEM_CONTROL ), \
	HID_COLLECTION ( HID_COLLECTION_APPLICATION ), \
		/* 2-bit system power control */ \
		HID_LOGICAL_MIN  ( 1 ),		/* minimal value = 1 */ \
		HID_LOGICAL_MAX  ( 3 ),		/* maximal value = 3 */ \
		HID_REPORT_COUNT ( 1 ),		/* number of elements = 1 */ \
		HID_REPORT_SIZE  ( 2 ),		/* size of element = 2 bits */ \
		HID_USAGE        ( HID_USAGE_DESKTOP_SYSTEM_POWER_DOWN ), \
		HID_USAGE        ( HID_USAGE_DESKTOP_SYSTEM_SLEEP ), \
		HID_USAGE        ( HID_USAGE_DESKTOP_SYSTEM_WAKE_UP ), \
		HID_INPUT ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ), /* this elements contain absolute data array */ \
		/* 6-bit padding */ \
		HID_REPORT_COUNT ( 1 ),		/* number of elements = 1 */ \
		HID_REPORT_SIZE  ( 6 ),		/* size of element = 6 bits */ \
		HID_INPUT ( HID_CONSTANT ),	/* this reserved element contains constant */ \
	HID_COLLECTION_END

// consumer control report descriptor template (sUsbHidCtrl structure, 2 bytes)
//	u16	val;	// value 0..1000
#define HID_REPORT_DESC_CONSUMER \
	HID_USAGE_PAGE ( HID_USAGE_PAGE_CONSUMER ), \
	HID_USAGE      ( HID_USAGE_CONSUMER_CONTROL ), \
	HID_COLLECTION ( HID_COLLECTION_APPLICATION ), \
		HID_LOGICAL_MIN  ( 0x00 ),	/* minimal value = 0 */ \
		HID_LOGICAL_MAX2 ( 0x03FF ),	/* maximal value = 1000 = 0x3FF */ \
		HID_USAGE_MIN    ( 0x00 ),	/* minimal code = 0 */ \
		HID_USAGE_MAX2   ( 0x03FF ),	/* maximal code = 1000 = 0x3FF */ \
		HID_REPORT_COUNT ( 1 ),		/* number of elements = 1 word */ \
		HID_REPORT_SIZE  ( 16 ),	/* size of element = 16 bits */ \
		HID_INPUT ( HID_DATA | HID_ARRAY | HID_ABSOLUTE ), /* this elements contain absolute data */ \
	HID_COLLECTION_END

// FIDO U2F authenticator descriptor template
// - 1st parameter is report size, which is 64 bytes maximum in U2F
// - 2nd parameter is HID_REPORT_ID(n) (optional)
#define HID_REPORT_DESC_FIDO_U2F(report_size, ...) \
	HID_USAGE_PAGE2 ( HID_USAGE_PAGE_FIDO ), \
	HID_USAGE       ( HID_USAGE_FIDO_U2FHID ), \
	HID_COLLECTION  ( HID_COLLECTION_APPLICATION ), \
		/* Report ID if any */ \
		__VA_ARGS__ \
		/* Usage Data In */ \
		HID_USAGE         ( HID_USAGE_FIDO_DATA_IN ), \
		HID_LOGICAL_MIN   ( 0 ), \
		HID_LOGICAL_MAX2  ( 0xff ), \
		HID_REPORT_SIZE   ( 8 ), \
		HID_REPORT_COUNT  ( report_size ), \
		HID_INPUT         ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
		/* Usage Data Out */ \
		HID_USAGE         ( HID_USAGE_FIDO_DATA_OUT ), \
		HID_LOGICAL_MIN   ( 0 ), \
		HID_LOGICAL_MAX2  ( 0xff ), \
		HID_REPORT_SIZE   ( 8 ), \
		HID_REPORT_COUNT  ( report_size ), \
		HID_OUTPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
	HID_COLLECTION_END

// HID generic input & output
// - 1st parameter is report size (mandatory)
// - 2nd parameter is report id HID_REPORT_ID(n) (optional)
#define HID_REPORT_DESC_GENERIC_INOUT(report_size, ...) \
	HID_USAGE_PAGE2  ( HID_USAGE_PAGE_VENDOR ), \
	HID_USAGE        ( 0x01 ), \
	HID_COLLECTION   ( HID_COLLECTION_APPLICATION ), \
		/* Report ID if any */ \
		__VA_ARGS__ \
		/* Input */ \
		HID_USAGE        ( 0x02 ), \
		HID_LOGICAL_MIN  ( 0x00 ), \
		HID_LOGICAL_MAX2 ( 0xff ), \
		HID_REPORT_SIZE  ( 8 ), \
		HID_REPORT_COUNT ( report_size ), \
		HID_INPUT        ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
		/* Output */ \
		HID_USAGE        ( 0x03 ), \
		HID_LOGICAL_MIN  ( 0x00 ), \
		HID_LOGICAL_MAX2 ( 0xff ), \
		HID_REPORT_SIZE ( 8 ), \
		HID_REPORT_COUNT( report_size ), \
		HID_OUTPUT      ( HID_DATA | HID_VARIABLE | HID_ABSOLUTE ), \
	HID_COLLECTION_END

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_HID_H

#endif // USE_USB_DEV_HID || USE_USB_HOST_HID
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)


// key HID codes (US layout)
#define HID_KEY_NONE			0x00
#define HID_KEY_ERR_ROLLOVER		0x01	// protocol internal: ErrorRollOver (user pressed more than 6 keys)
#define HID_KEY_POSTFAIL		0x02	// protocol internal: PostFail (keyboard self test failed)
#define HID_KEY_ERR_UNDEFINED		0x03	// protocol internal: ErrorUndefined (undefined error)
#define HID_KEY_A			0x04
#define HID_KEY_B			0x05
#define HID_KEY_C			0x06
#define HID_KEY_D			0x07
#define HID_KEY_E			0x08
#define HID_KEY_F			0x09
#define HID_KEY_G			0x0A
#define HID_KEY_H			0x0B
#define HID_KEY_I			0x0C
#define HID_KEY_J			0x0D
#define HID_KEY_K			0x0E
#define HID_KEY_L			0x0F
#define HID_KEY_M			0x10
#define HID_KEY_N			0x11
#define HID_KEY_O			0x12
#define HID_KEY_P			0x13
#define HID_KEY_Q			0x14
#define HID_KEY_R			0x15
#define HID_KEY_S			0x16
#define HID_KEY_T			0x17
#define HID_KEY_U			0x18
#define HID_KEY_V			0x19
#define HID_KEY_W			0x1A
#define HID_KEY_X			0x1B
#define HID_KEY_Y			0x1C
#define HID_KEY_Z			0x1D
#define HID_KEY_1			0x1E	// 1 and !
#define HID_KEY_2			0x1F	// 2 and @
#define HID_KEY_3			0x20	// 3 and #
#define HID_KEY_4			0x21	// 4 and $
#define HID_KEY_5			0x22	// 5 and %
#define HID_KEY_6			0x23	// 6 and ^
#define HID_KEY_7			0x24	// 7 and &
#define HID_KEY_8			0x25	// 8 and *
#define HID_KEY_9			0x26	// 9 and (
#define HID_KEY_0			0x27	// 0 and )
#define HID_KEY_ENTER			0x28
#define HID_KEY_ESCAPE			0x29
#define HID_KEY_BACKSPACE		0x2A
#define HID_KEY_TAB			0x2B
#define HID_KEY_SPACE			0x2C
#define HID_KEY_MINUS			0x2D	// - and _
#define HID_KEY_EQUAL			0x2E	// = and +
#define HID_KEY_BRACKET_LEFT		0x2F	// [ and {
#define HID_KEY_BRACKET_RIGHT		0x30	// ] and }
#define HID_KEY_BACKSLASH		0x31	// '\' and |
#define HID_KEY_HASH			0x32	// (Europe) # and ~
#define HID_KEY_SEMICOLON		0x33	// ; and :
#define HID_KEY_APOSTROPHE		0x34	// ' and "
#define HID_KEY_GRAVE			0x35	// ` and ~
#define HID_KEY_COMMA			0x36	// , and <
#define HID_KEY_PERIOD			0x37	// . and >
#define HID_KEY_SLASH			0x38	// / and ?
#define HID_KEY_CAPS_LOCK		0x39
#define HID_KEY_F1			0x3A
#define HID_KEY_F2			0x3B
#define HID_KEY_F3			0x3C
#define HID_KEY_F4			0x3D
#define HID_KEY_F5			0x3E
#define HID_KEY_F6			0x3F
#define HID_KEY_F7			0x40
#define HID_KEY_F8			0x41
#define HID_KEY_F9			0x42
#define HID_KEY_F10			0x43
#define HID_KEY_F11			0x44
#define HID_KEY_F12			0x45
#define HID_KEY_PRINT_SCREEN		0x46
#define HID_KEY_SCROLL_LOCK		0x47
#define HID_KEY_PAUSE			0x48
#define HID_KEY_INSERT			0x49
#define HID_KEY_HOME			0x4A
#define HID_KEY_PAGE_UP			0x4B
#define HID_KEY_DELETE			0x4C
#define HID_KEY_END			0x4D
#define HID_KEY_PAGE_DOWN		0x4E
#define HID_KEY_ARROW_RIGHT		0x4F
#define HID_KEY_ARROW_LEFT		0x50
#define HID_KEY_ARROW_DOWN		0x51
#define HID_KEY_ARROW_UP		0x52
#define HID_KEY_NUM_LOCK		0x53
#define HID_KEY_KEYPAD_DIVIDE		0x54	// keypad /
#define HID_KEY_KEYPAD_MULTIPLY		0x55	// keypad *
#define HID_KEY_KEYPAD_SUBTRACT		0x56	// keypad -
#define HID_KEY_KEYPAD_ADD		0x57	// keypad +
#define HID_KEY_KEYPAD_ENTER		0x58	// keypad Enter
#define HID_KEY_KEYPAD_1		0x59	// keypad 1 and End
#define HID_KEY_KEYPAD_2		0x5A	// keypad 2 and Arrow Down
#define HID_KEY_KEYPAD_3		0x5B	// keypad 3 and Page Down
#define HID_KEY_KEYPAD_4		0x5C	// keypad 4 and Arrow Left
#define HID_KEY_KEYPAD_5		0x5D	// keypad 5
#define HID_KEY_KEYPAD_6		0x5E	// keypad 6 and Arrow Right
#define HID_KEY_KEYPAD_7		0x5F	// keypad 7 and Home
#define HID_KEY_KEYPAD_8		0x60	// keypad 8 and Arrow Up
#define HID_KEY_KEYPAD_9		0x61	// keypad 9 and Page Up
#define HID_KEY_KEYPAD_0		0x62	// keypad 0 and Insert
#define HID_KEY_KEYPAD_DECIMAL		0x63	// keypad . and Delete
#define HID_KEY_INTL_BACKSLASH		0x64	// (Europe) '\' and |
#define HID_KEY_APPLICATION		0x65
#define HID_KEY_POWER			0x66
#define HID_KEY_KEYPAD_EQUAL		0x67	// keypad =
#define HID_KEY_F13			0x68
#define HID_KEY_F14			0x69
#define HID_KEY_F15			0x6A
#define HID_KEY_F16			0x6B
#define HID_KEY_F17			0x6C
#define HID_KEY_F18			0x6D
#define HID_KEY_F19			0x6E
#define HID_KEY_F20			0x6F
#define HID_KEY_F21			0x70
#define HID_KEY_F22			0x71
#define HID_KEY_F23			0x72
#define HID_KEY_F24			0x73
#define HID_KEY_EXECUTE			0x74
#define HID_KEY_HELP			0x75
#define HID_KEY_MENU			0x76
#define HID_KEY_SELECT			0x77
#define HID_KEY_STOP			0x78
#define HID_KEY_AGAIN			0x79
#define HID_KEY_UNDO			0x7A
#define HID_KEY_CUT			0x7B
#define HID_KEY_COPY			0x7C
#define HID_KEY_PASTE			0x7D
#define HID_KEY_FIND			0x7E
#define HID_KEY_MUTE			0x7F
#define HID_KEY_VOLUME_UP		0x80
#define HID_KEY_VOLUME_DOWN		0x81
#define HID_KEY_LOCKING_CAPS_LOCK	0x82
#define HID_KEY_LOCKING_NUM_LOCK	0x83
#define HID_KEY_LOCKING_SCROLL_LOCK	0x84
#define HID_KEY_KEYPAD_COMMA		0x85	// keypad ,
#define HID_KEY_KEYPAD_EQUAL_SIGN	0x86	// keypad =
#define HID_KEY_KANJI1			0x87
#define HID_KEY_KANJI2			0x88
#define HID_KEY_KANJI3			0x89
#define HID_KEY_KANJI4			0x8A
#define HID_KEY_KANJI5			0x8B
#define HID_KEY_KANJI6			0x8C
#define HID_KEY_KANJI7			0x8D
#define HID_KEY_KANJI8			0x8E
#define HID_KEY_KANJI9			0x8F
#define HID_KEY_LANG1			0x90
#define HID_KEY_LANG2			0x91
#define HID_KEY_LANG3			0x92
#define HID_KEY_LANG4			0x93
#define HID_KEY_LANG5			0x94
#define HID_KEY_LANG6			0x95
#define HID_KEY_LANG7			0x96
#define HID_KEY_LANG8			0x97
#define HID_KEY_LANG9			0x98
#define HID_KEY_ALTERNATE_ERASE		0x99
#define HID_KEY_SYSREQ_ATTENTION	0x9A
#define HID_KEY_CANCEL			0x9B
#define HID_KEY_CLEAR			0x9C
#define HID_KEY_PRIOR			0x9D
#define HID_KEY_RETURN			0x9E
#define HID_KEY_SEPARATOR		0x9F
#define HID_KEY_OUT			0xA0
#define HID_KEY_OPER			0xA1
#define HID_KEY_CLEAR_AGAIN		0xA2
#define HID_KEY_CRSEL_PROPS		0xA3
#define HID_KEY_EXSEL			0xA4
// reserved				0xA5..0xAF
#define HID_KEY_KPAD_00			0xB0	// Keypad 00
#define HID_KEY_KPAD_000		0xB1	// Keypad 000
#define HID_KEY_THOUSANDS_SEP		0xB2	// Thousands Separator
#define HID_KEY_DECIMAL_SEP		0xB3	// Decimal Separator
#define HID_KEY_CURRENCY		0xB4	// Currency Unit
#define HID_KEY_SUB_CURRENCY		0xB5	// Currency Sub-unit
#define HID_KEY_KPAD_LEFT_PAREN		0xB6	// Keypad (
#define HID_KEY_KPAD_RIGHT_PAREN	0xB7	// Keypad )
#define HID_KEY_KPAD_LEFT_BRACE		0xB8	// Keypad {
#define HID_KEY_KPAD_RIGHT_BRACE	0xB9	// Keypad }
#define HID_KEY_KPAD_TAB		0xBA	// Keypad Tab
#define HID_KEY_KPAD_BACKSPACE		0xBB	// Keypad Backspace
#define HID_KEY_KPAD_A			0xBC	// Keypad A
#define HID_KEY_KPAD_B			0xBD	// Keypad B
#define HID_KEY_KPAD_C			0xBE	// Keypad C
#define HID_KEY_KPAD_D			0xBF	// Keypad D
#define HID_KEY_KPAD_E			0xC0	// Keypad E
#define HID_KEY_KPAD_F			0xC1	// Keypad F
#define HID_KEY_KPAD_XOR		0xC2	// Keypad XOR
#define HID_KEY_KPAD_CARET		0xC3	// Keypad ^
#define HID_KEY_KPAD_PERCENT		0xC4	// Keypad %
#define HID_KEY_KPAD_LESS_THAN		0xC5	// Keypad <
#define HID_KEY_KPAD_GREAT_THAN		0xC6	// Keypad >
#define HID_KEY_KPAD_AND		0xC7	// Keypad &
#define HID_KEY_KPAD_DBL_AND		0xC8	// Keypad &&
#define HID_KEY_KPAD_OR			0xC9	// Keypad |
#define HID_KEY_KPAD_DBL_OR		0xCA	// Keypad ||
#define HID_KEY_KPAD_COLON		0xCB	// Keypad :
#define HID_KEY_KPAD_HASH		0xCC	// Keypad #
#define HID_KEY_KPAD_SPACE		0xCD	// Keypad Space
#define HID_KEY_KPAD_AT			0xCE	// Keypad @
#define HID_KEY_KPAD_EXCLAMATION	0xCF	// Keypad !
#define HID_KEY_KPAD_MEM_STORE		0xD0	// Keypad Memory Store
#define HID_KEY_KPAD_MEM_RECALL		0xD1	// Keypad Memory Recall
#define HID_KEY_KPAD_MEM_CLEAR		0xD2	// Keypad Memory Clear
#define HID_KEY_KPAD_MEM_ADD		0xD3	// Keypad Memory Add
#define HID_KEY_KPAD_MEM_SUB		0xD4	// Keypad Memory Subtract
#define HID_KEY_KPAD_MEM_MULTIPLY	0xD5	// Keypad Memory Multiply
#define HID_KEY_KPAD_MEM_DIVIDE		0xD6	// Keypad Memory Divide
#define HID_KEY_PLUS_MINUS		0xD7	// Keypad +/-
#define HID_KEY_KCLEAR			0xD8	// Keypad Clear
#define HID_KEY_CLEAR_ENTRY		0xD9	// Keypad Clear Entry
#define HID_KEY_BINARY			0xDA	// Keypad Binary
#define HID_KEY_OCTAL			0xDB	// Keypad Octal
#define HID_KEY_DECIMAL			0xDC	// Keypad Decimal
#define HID_KEY_HEXADECIMAL		0xDD	// Keypad Hexadecimal
// reserved				0xDE..0xDF
#define HID_KEY_CONTROL_LEFT		0xE0	// Left Control
#define HID_KEY_SHIFT_LEFT		0xE1	// Left Shift
#define HID_KEY_ALT_LEFT		0xE2	// Left Alt
#define HID_KEY_GUI_LEFT		0xE3	// Left Window (GUI)
#define HID_KEY_CONTROL_RIGHT		0xE4	// Right Control
#define HID_KEY_SHIFT_RIGHT		0xE5	// Right Shift
#define HID_KEY_ALT_RIGHT		0xE6	// Right Alt
#define HID_KEY_GUI_RIGHT		0xE7	// Right Window (GUI)

// PC key scan codes
#define PC_KEYSCAN_ESC		0x01	// Esc
#define PC_KEYSCAN_1		0x02	// 1 !
#define PC_KEYSCAN_2		0x03	// 2 @
#define PC_KEYSCAN_3		0x04	// 3 #
#define PC_KEYSCAN_4		0x05	// 4 $
#define PC_KEYSCAN_5		0x06	// 5 %
#define PC_KEYSCAN_6		0x07	// 6 ^
#define PC_KEYSCAN_7		0x08	// 7 &
#define PC_KEYSCAN_8		0x09	// 8 *
#define PC_KEYSCAN_9		0x0A	// 9 ( 
#define PC_KEYSCAN_0		0x0B	// 0 )
#define PC_KEYSCAN_HYPHEN	0x0C	// - _
#define PC_KEYSCAN_EQU		0x0D	// = +
#define PC_KEYSCAN_BS		0x0E	// BackSpace
#define PC_KEYSCAN_TAB		0x0F	// Tab
#define PC_KEYSCAN_Q		0x10	// Q
#define PC_KEYSCAN_W		0x11	// W
#define PC_KEYSCAN_E		0x12	// E
#define PC_KEYSCAN_R		0x13	// R
#define PC_KEYSCAN_T		0x14	// T
#define PC_KEYSCAN_Y		0x15	// Y
#define PC_KEYSCAN_U		0x16	// U
#define PC_KEYSCAN_I		0x17	// I
#define PC_KEYSCAN_O		0x18	// O
#define PC_KEYSCAN_P		0x19	// P
#define PC_KEYSCAN_LBRACKET	0x1A	// [ {
#define PC_KEYSCAN_RBRACKET	0x1B	// ] }
#define PC_KEYSCAN_ENTER	0x1C	// Enter
#define PC_KEYSCAN_LCTRL	0x1D	// Left Ctrl
#define PC_KEYSCAN_A		0x1E	// A
#define PC_KEYSCAN_S		0x1F	// S
#define PC_KEYSCAN_D		0x20	// D
#define PC_KEYSCAN_F		0x21	// F
#define PC_KEYSCAN_G		0x22	// G
#define PC_KEYSCAN_H		0x23	// H
#define PC_KEYSCAN_J		0x24	// J
#define PC_KEYSCAN_K		0x25	// K
#define PC_KEYSCAN_L		0x26	// L
#define PC_KEYSCAN_SEMICOLON	0x27	// ; :
#define PC_KEYSCAN_SQUOTE	0x28	// ' "
#define PC_KEYSCAN_BACKQUOTE	0x29	// ` ~
#define PC_KEYSCAN_LSHIFT	0x2A	// Left Shift
#define PC_KEYSCAN_BACKSLASH	0x2B	// \ |
#define PC_KEYSCAN_Z		0x2C	// Z
#define PC_KEYSCAN_X		0x2D	// X
#define PC_KEYSCAN_C		0x2E	// C
#define PC_KEYSCAN_V		0x2F	// V
#define PC_KEYSCAN_B		0x30	// B
#define PC_KEYSCAN_N		0x31	// N
#define PC_KEYSCAN_M		0x32	// M
#define PC_KEYSCAN_COMMA	0x33	// , <
#define PC_KEYSCAN_PERIOD	0x34	// . >
#define PC_KEYSCAN_SLASH	0x35	// / ?
#define PC_KEYSCAN_RSHIFT	0x36	// Right Shift
#define PC_KEYSCAN_GREYSTAR	0x37	// Grey [*]
#define PC_KEYSCAN_LALT		0x38	// Left Alt
#define PC_KEYSCAN_SPACE	0x39	// SpaceBar
#define PC_KEYSCAN_CAPSLOCK	0x3A	// Caps Lock
#define PC_KEYSCAN_F1		0x3B	// F1
#define PC_KEYSCAN_F2		0x3C	// F2
#define PC_KEYSCAN_F3		0x3D	// F3
#define PC_KEYSCAN_F4		0x3E	// F4
#define PC_KEYSCAN_F5		0x3F	// F5
#define PC_KEYSCAN_F6		0x40	// F6
#define PC_KEYSCAN_F7		0x41	// F7
#define PC_KEYSCAN_F8		0x42	// F8
#define PC_KEYSCAN_F9		0x43	// F9
#define PC_KEYSCAN_F10		0x44	// F10
#define PC_KEYSCAN_NUMLOCK	0x45	// Num Lock
#define PC_KEYSCAN_SCROLL	0x46	// Scroll Lock
#define PC_KEYSCAN_NUM7		0x47	// [7 Home]
#define PC_KEYSCAN_NUM8		0x48	// [8 Up]
#define PC_KEYSCAN_NUM9		0x49	// [9 PgUp]
#define PC_KEYSCAN_GREYMINUS	0x4A	// Grey [-]
#define PC_KEYSCAN_NUM4		0x4B	// [4 Left]
#define PC_KEYSCAN_NUM5		0x4C	// [5]
#define PC_KEYSCAN_NUM6		0x4D	// [6 Right]
#define PC_KEYSCAN_GREYPLUS	0x4E	// Grey [+]
#define PC_KEYSCAN_NUM1		0x4F	// [1 End]
#define PC_KEYSCAN_NUM2		0x50	// [2 Down]
#define PC_KEYSCAN_NUM3		0x51	// [3 PgDn]
#define PC_KEYSCAN_NUM0		0x52	// [0 Ins]
#define PC_KEYSCAN_DECIMAL	0x53	// [. Del]
#define PC_KEYSCAN_SYSRQ	0x54	// SysRq (PrintScr with Alt)
				// 0x55
#define PC_KEYSCAN_BACKSLASH2	0x56	// \ | alternative
#define PC_KEYSCAN_F11		0x57	// F11
#define PC_KEYSCAN_F12		0x58	// F12

// PC extended key scan codes - with prefix 0xE0
#define PC_KEYSCAN_EXT		0xE0	// 0xE0 prefix

#define PC_KEYSCAN_GREYENTER	0x1C	// grey [Enter]
#define PC_KEYSCAN_RCTRL	0x1D	// Right Ctrl
#define PC_KEYSCAN_GREYSLASH	0x35	// grey [/]
#define PC_KEYSCAN_PRINTSCR	0x37	// Print Scrn
#define PC_KEYSCAN_RALT		0x38	// Right Alt
#define PC_KEYSCAN_PAUSE	0x45	// Pause (E1 1D 45 E1 9D C5)
#define PC_KEYSCAN_BREAK	0x46	// Break (Pause with Ctrl)
#define PC_KEYSCAN_HOME		0x47	// Home
#define PC_KEYSCAN_UP		0x48	// Up
#define PC_KEYSCAN_PGUP		0x49	// PgUp
#define PC_KEYSCAN_LEFT		0x4B	// Left
#define PC_KEYSCAN_RIGHT	0x4D	// Right
#define PC_KEYSCAN_END		0x4F	// End
#define PC_KEYSCAN_DOWN		0x50	// Down
#define PC_KEYSCAN_PGDN		0x51	// PgDn
#define PC_KEYSCAN_INS		0x52	// Ins
#define PC_KEYSCAN_DEL		0x53	// Del
#define PC_KEYSCAN_LWIN		0x5B	// Left Window
#define PC_KEYSCAN_RWIN		0x5C	// Right Window
#define PC_KEYSCAN_MENU		0x5D	// Menu
