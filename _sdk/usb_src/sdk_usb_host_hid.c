
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

#include "../../global.h"	// globals

#include "../usb_inc/sdk_usb_host_hid.h"

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_HOST	// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#if USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)

#include "../inc/sdk_timer.h"
#include "../inc/sdk_cpu.h"	// dmb()
#include "../usb_inc/sdk_usb_host.h"

// keyboard ring buffer (contains u32 packed keyboard event)
//   keyboard event - u32 number
//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
//    bit 8..15: modifiers USB_KEY_MODI_*
//    bit 16..23: ASCII character CH_* (NOCHAR = no valid character or release key)
//    bit 24: 1=release key, 0=press key
// In case of key release, ASCII character is invalid (= 0).
sEvent UsbHostHidKeyRingBuf[USB_HOST_HID_KEY_BUFSIZE];
sEventRing UsbHostHidKeyRing;

// mouse ring buffer (contains u32 packed mouse event)
//   mouse event - u32 number
//    bit 0..7: buttons mask USB_MOUSE_BTN_*, B7 is set to indicate valid mouse event
//    bit 8..15: delta X movement (signed s8)
//    bit 16..23: delta Y movement (signed s8)
//    bit 24..31: delta wheel movement (signed s8)
sEvent UsbHostHidMouseRingBuf[USB_HOST_HID_MOUSE_BUFSIZE];
sEventRing UsbHostHidMouseRing;

// HID interfaces
sUsbHostHidInter UsbHostHidInter[USE_USB_HOST_HID];

// temporary save interface for UsbHostHidCfg
u8 UsbHostHidCfg_Itf;

// previous keyboard report
sUsbHidKey UsbHidKeyOld;

// time of next repeat key
u8 UsbHidKeyRepeat; // repeated key
u32 UsbHidKeyNextTime;

// previous mouse report
sUsbHidMouse UsbHidMouseOld;

// buffer to load report descriptor
u8 UsbHostRepBuf[USB_HOST_HID_REPSIZE];

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// check if device is mounted
//  hid_inx ... HID interface
Bool UsbHostHidInxIsMounted(u8 hid_inx)
{
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];
	return hid->used && UsbIsMounted();
}

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostHidFindItf(u8 dev_addr, u8 itf_num)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

		// check interface
		if (hid->used && (hid->dev_addr == dev_addr) && (hid->itf_num == itf_num))
		{
			return hid_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// check if next report can be sent to send buffer
//  hid_inx ... interface index
Bool UsbHostHidInxSendReady(u8 hid_inx)
{
	// check if device is mounted
	if (!UsbHostHidInxIsMounted(hid_inx)) return False;

	// check if send buffer is empty	
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];
	return (hid->ep_out != 0) && (hid->tx_len == 0);
}

// check if next report can be received from receive buffer
//  hid_inx ... interface index
Bool UsbHostHidInxRecvReady(u8 hid_inx)
{
	// check if device is mounted
	if (!UsbHostHidInxIsMounted(hid_inx)) return False;

	// check if receive buffer is not empty	
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];
	return (hid->ep_in != 0) && (hid->rx_len != 0);
}

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by device address (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostHidFindAddr(u8 dev_addr, u8 dev_epinx)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

		// check interface
		if (hid->used && (hid->dev_addr == dev_addr) &&
			((hid->ep_in == dev_epinx) || (hid->ep_out == dev_epinx)))
		{
			return hid_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbHostHidNewItf(u8 dev_addr, const sUsbDescItf* itf)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

		// check if interface is not used
		if (!hid->used)
		{
			hid->used = True; // interface is used
			hid->dev_addr = dev_addr; // device address
			hid->itf_num = itf->itf_num; // interface number
			hid->protocol = itf->itf_protocol; // protocol
			hid->mode = USB_HID_PROT_REP; // default is report mode
			hid->ep_out = 0;
			hid->ep_in = 0;
			hid->epout_max = 0;
			hid->epin_max = 0;
			return hid_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// send report from TX buffer
//  hid_inx ... interface index
void UsbHostHidInxSend(u8 hid_inx)
{
	// check if interface is mounted
	if (!UsbHostHidInxIsMounted(hid_inx)) return;

	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// not sending
	if (hid->ep_out == 0) return;

	// no data to send
	u8 len = hid->tx_len;
	if (len == 0) return;

	// check if endpoint is busy
	if (UsbHostIsBusy(hid->dev_addr, hid->ep_out)) return; // transmission is active

	// get endpoint
	//u8 ep = UsbHostDevEp(hid->dev_addr, hid->ep_out);
	//sEndpoint* sep = &UsbEndpoints[ep];

	// send data
	UsbHostXferStart(hid->dev_addr, hid->ep_out, hid->tx_buf, len, False);
}

// send reports of all interfaces
void UsbHostHidAllSend()
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		UsbHostHidInxSend(hid_inx);
	}
}

// receive report to Rx buffer
//  hid_inx ... interface index
void UsbHostHidInxRecv(u8 hid_inx)
{
	// check if interface is mounted
	if (!UsbHostHidInxIsMounted(hid_inx)) return;

	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// not receiving
	if (hid->ep_in == 0) return;

	// check if endpoint is busy
	if (UsbHostIsBusy(hid->dev_addr, hid->ep_in)) return;

	// start receiving report
	UsbHostXferStart(hid->dev_addr, hid->ep_in, hid->rx_buf, hid->epin_max, False);
}

// receive reports to all interfaces
void UsbHostHidAllRecv()
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		UsbHostHidInxRecv(hid_inx);
	}
}

// initialize class driver
void UsbHostHidInit()
{
	// clear HID interfaces
	memset(UsbHostHidInter, 0, sizeof(UsbHostHidInter));

	// initialize ring buffers
	EventRingInit(&UsbHostHidKeyRing, UsbHostHidKeyRingBuf, USB_HOST_HID_KEY_BUFSIZE, USB_SPIN);
	EventRingInit(&UsbHostHidMouseRing, UsbHostHidMouseRingBuf, USB_HOST_HID_MOUSE_BUFSIZE, USB_SPIN);

	memset(&UsbHidKeyOld, 0, sizeof(sUsbHidKey));
	memset(&UsbHidMouseOld, 0, sizeof(sUsbHidMouse));
	UsbHidKeyRepeat = 0;
}

// terminate class driver
void UsbHostHidTerm()
{
	memset(&UsbHidKeyOld, 0, sizeof(sUsbHidKey));
	memset(&UsbHidMouseOld, 0, sizeof(sUsbHidMouse));
}

// open class interface (returns size of used interface, 0=not supported)
// - At this point, no communications are allowed yet.
u16 UsbHostHidOpen(u8 dev_addr, const sUsbDescItf* itf, u16 max_len)
{
	// check interface class
	if (itf->itf_class != USB_CLASS_HID) return 0;

	// check descriptor size
	if (USB_DESC_LEN(itf) > max_len) return 0;

	// skip interface descriptor
	u16 drv_len = USB_DESC_LEN(itf);
	const u8* p_desc = USB_DESC_NEXT(itf);

	// HID descriptor
	if ((USB_DESC_TYPE(p_desc) != USB_HID_DESC_HID) ||
		(drv_len + USB_DESC_LEN(p_desc) > max_len))
		return 0;
	const sUsbDescHid* desc_hid = (const sUsbDescHid*)p_desc; // pointer to HID descriptor

	// allocate new interface
	u8 hid_inx = UsbHostHidNewItf(dev_addr, itf);
	if (hid_inx == USB_DRVID_INVALID) return 0;

	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// get report size and type
	hid->rep_type = desc_hid->rep_type;
	hid->rep_len = desc_hid->rep_len;

	// skip HID descriptor
	drv_len += USB_DESC_LEN(p_desc);
	p_desc = USB_DESC_NEXT(p_desc);

	// number of endpoints
	u8 n = itf->num_ep;

	// open endpoints
	if (drv_len + n*sizeof(sUsbDescEp) > max_len) return 0;
	if (!UsbHostOpenEpPair(dev_addr, p_desc, n, &hid->ep_out, &hid->ep_in, &hid->epout_max, &hid->epin_max)) return 0;
	drv_len += n*sizeof(sUsbDescEp);

	// protocol
	hid->protocol = (itf->itf_subclass == USB_HID_SUBCLASS_BOOT) ? itf->itf_protocol : USB_HID_ITF_NONE;
	hid->mode = (hid->protocol == USB_HID_ITF_NONE) ? USB_HID_PROT_REP : USB_HID_PROT_BOOT;

	return drv_len;
}

// parse report descriptor
void UsbHostHidParse(u8 hid_inx)
{
	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// clear info array
	memset(hid->rep_info, 0, sizeof(hid->rep_info));

	// remaining length of the descriptor
	int rep_len = hid->rep_len;

	// source data
	u8* s = UsbHostRepBuf;

	// prepare info array
	sUsbHostHidRepInfo* info = hid->rep_info;
	u8 rep_num = 0;		// number of info entries
	int report_in_bits = 0;	// accumulator of bits of current IN report
	int report_out_bits = 0; // accumulator of bits of current OUT report
	int col_depth = 0;	// reset collection depth

	// push entries
#define PUSH_MAX	6
	u8 report_count[PUSH_MAX];	// number of elements
	u8 report_size[PUSH_MAX];	// size of elements (bits)
	u8 push_lev = 0;		// push level
	report_count[0] = 1;
	report_size[0] = 1;

	// parse
	while ((rep_len > 0) && (rep_num < USB_HOST_HID_REP_MAX))
	{
// Report entry:
// 0: (1) prefix byte
//  bit 0..1: (2 bits) size of optional data in bytes (0, 1, 2 or 4 bytes)
//  bit 2..3: (2 bits) type of this report descriptor (0=main, 1=global, 2=local, 3=reserved for long item)
//  bit 4..7: (4 bits) short item tag: function of the item (subtype)
// 1: (0 to 3) optional item data

		// load one entry
		u8 e = *s++;
		rep_len--;

		// decompose
		u8 size = e & 3;	// size of optional data in bytes (0, 1, 2 or 4 bytes)
		if (size == 3) size = 4;
		u8 type = (e >> 2) & 3;	// type of this entry (0=main, 1=global, 2=local, 3=reserved for long item)
		u8 tag = (e >> 4) & 0x0f; // tag

		// check entry type
		switch (type)
		{
		// main entry type
		case HID_TYPE_MAIN:

			// check main tag
			switch (tag)
			{
			// data entries - only calculate report size
			case HID_MAIN_INPUT:		// HID_INPUT(x)
				report_in_bits += report_count[push_lev] * report_size[push_lev]; // accumulate bits
				break;

			case HID_MAIN_OUTPUT:		// HID_OUTPUT(x)
				report_out_bits += report_count[push_lev] * report_size[push_lev]; // accumulate bits
				break;

			// dive into collection
			case HID_MAIN_COLLECTION:
				col_depth++;
				break;

			// emerge from collection
			case HID_MAIN_COLLECTION_END:
				col_depth--;
				if (col_depth == 0)
				{
					// count this collection
					info++;
					rep_num++;
				}
				break;

			default:
				break;
			}
			break;

		// global entry type
		case HID_TYPE_GLOBAL:

			// check global tag
			switch (tag)
			{
			// open usage page - load ID of usage page
			case HID_GLOBAL_USAGE_PAGE:
				if (col_depth == 0)
				{
					info->usage_page = 0;
					UsbMemcpy(&info->usage_page, s, (size > 1) ? 2 : size);
				}
				break;

			// report size
			case HID_GLOBAL_REPORT_SIZE:
				report_size[push_lev] = *s;
				break;

			// report count
			case HID_GLOBAL_REPORT_COUNT:
				report_count[push_lev] = *s;
				break;

			// push
			case HID_GLOBAL_PUSH:
				if (push_lev < PUSH_MAX-1)
				{
					report_count[push_lev+1] = report_count[push_lev];
					report_size[push_lev+1] = report_size[push_lev];
					push_lev++;
				}
				break;

			// pop
			case HID_GLOBAL_POP:
				if (push_lev > 0) push_lev--;
				break;

			// report ID
			case HID_GLOBAL_REPORT_ID:
				info->rep_id = *s;
				break;

			default:
				break;
			}
			break;

		// local entry type
		case HID_TYPE_LOCAL:

			// item usage
			if (tag == HID_LOCAL_USAGE)
			{
				if (col_depth == 0) info->usage = *s;
			}
			break;

		// long entry is not supported
		default:
			break;
		}

		// skip data bytes
		s += size;
		rep_len -= size;
	}

	// store number of reports
	hid->rep_num = rep_num;
}

// set config interface ... 4th step = "config complete"
void UsbHostHidCfgComp(u8 dev_addr, u8 xres)
{
	// get temporary interface
	u8 itf_num = UsbHostHidCfg_Itf;

	// pointer to HID interface
	u8 hid_inx = UsbHostHidFindItf(dev_addr, itf_num);
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// error
	if (xres != USB_XRES_OK)
	{
		hid->rep_len = 0;
		return;
	}
	hid->rep_len = UsbSetupDataXfer;

	// start receiving (do not use UsbHostHidInxRecv, device is not mounted yet)
	if ((hid->ep_in != 0) && !UsbHostIsBusy(hid->dev_addr, hid->ep_in)) 
	{
		// start receiving report
		UsbHostXferStart(hid->dev_addr, hid->ep_in, hid->rx_buf, hid->epin_max, False);
	}

	// parse report descriptor
	UsbHostHidParse(hid_inx);

	// continue set config interface
	UsbHostCfgComp(dev_addr, itf_num);
}

// set config interface ... 3rd step = "get report descriptor"
void UsbHostHidCfgGetRepDesc(u8 dev_addr, u8 xres)
{
	// get temporary interface
	u8 itf_num = UsbHostHidCfg_Itf;

	// pointer to HID interface
	u8 hid_inx = UsbHostHidFindItf(dev_addr, itf_num);
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// pointer to setup packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare setup packet - "get report descriptor"
	setup->type = 1 | B7; // recipient = interface, type = standard, direction = in
	setup->request = USB_REQ_GET_DESCRIPTOR; // request = get descriptor
	setup->value =  (u16)hid->rep_type << 8; // descriptor type, index = 0
	setup->index = itf_num; // interface number
	setup->length = hid->rep_len; // report length

	// send data (continue to "complete")
	UsbHostCtrlXfer(dev_addr, UsbHostRepBuf, UsbHostHidCfgComp);
}

// set config interface ... 2nd step = "set protocol" to BOOT in case of keyboard or mouse
void UsbHostHidCfgSetProt(u8 dev_addr, u8 xres)
{
	// get temporary interface
	u8 itf_num = UsbHostHidCfg_Itf;

	// pointer to HID interface
	u8 hid_inx = UsbHostHidFindItf(dev_addr, itf_num);
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// pointer to setup packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare setup packet - "set protocol" to BOOT mode if keyboard or mouse, or REPORT mode otherwise
	setup->type = 1 | (1 << 5); // recipient = interface, type = class, direction = out
	setup->request = USB_HID_REQ_SET_PROT; // request = set protocol
	setup->value = (hid->protocol != USB_HID_ITF_NONE) ? USB_HID_PROT_BOOT : USB_HID_PROT_REP; 
	setup->index = itf_num; // interface number
	setup->length = 0; // no data

	// limit report descriptor size
	if (hid->rep_len > USB_HOST_HID_REPSIZE) hid->rep_len = USB_HOST_HID_REPSIZE;

	// send data (continue to "get report descriptor")
	UsbHostCtrlXfer(dev_addr, NULL, UsbHostHidCfgGetRepDesc);
}

// set config interface ... 1st step = "set idle"
Bool UsbHostHidCfg(u8 dev_addr, u8 itf_num)
{
	// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
	u8 hid_inx = UsbHostHidFindItf(dev_addr, itf_num);
	if (hid_inx == USB_DRVID_INVALID) return False;

	// pointer to HID interface
	//sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// pointer to setup packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare setup packet - "set idle" to 0, which means report only changes
	setup->type = 1 | (1 << 5); // recipient = interface, type = class, direction = out
	setup->request = USB_HID_REQ_SET_IDLE; // request = set idle
	setup->value = 0;	// idle = 0, report only changes
	setup->index = itf_num; // interface number
	setup->length = 0; // no data

	// send data (continue to "set protocol")
	UsbHostHidCfg_Itf = itf_num; // temporary save interface
	UsbHostCtrlXfer(dev_addr, NULL, UsbHostHidCfgSetProt);
	return True; // all OK
}

// find key in report
Bool UsbHostHidFindKey(const sUsbHidKey* report, u8 key)
{
	u8 i;
	for (i = 0; i < 6; i++)
	{
		if (report->key[i] == key) return True;
	}
	return False;
}

// repeat keys
void UsbHostHidRepKey()
{
	u8 key = UsbHidKeyRepeat;
	if ((key != 0) && ((s32)(Time() - UsbHidKeyNextTime) >= 0))
	{
		sUsbHidKey* rep = &UsbHidKeyOld;

		// check SHIFT
		u8 shift = ((rep->modi & (USB_KEY_MODI_LSHIFT | USB_KEY_MODI_RSHIFT)) != 0) ? 1 : 0;
		u8 ctrl = ((rep->modi & (USB_KEY_MODI_LCTRL | USB_KEY_MODI_RCTRL)) != 0) ? 1 : 0;

		// get ASCII character
		u8 ch = NOCHAR;
		if (ctrl && (key >= HID_KEY_A) && (key <= HID_KEY_Z))
			ch = key - HID_KEY_A + 1;
		else
		{
			if (key < HIDKEY_TO_ASCII_NUM) ch = (u8)HidKeyToAscii[2*key + shift];
		}

		// send keyboard event
		sEvent e;
		e.data[0] = key | ((u32)rep->modi << 8) | ((u32)ch << 16);
		EventRingWrite(&UsbHostHidKeyRing, &e);

		// time of next repeat key
		UsbHidKeyNextTime = Time() + KEY_REP_TIME2*1000;
	}
}

// transfer complete callback
Bool UsbHostHidComp(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len)
{
	// check result OK
	if (xres != USB_XRES_OK) return False;

	// find interface by device address
	u8 hid_inx = UsbHostHidFindAddr(dev_addr, dev_epinx);
	if (hid_inx == USB_DRVID_INVALID) return False; // invalid address

	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx]; // pointer to HID interface

	// receive data
	if (dev_epinx == hid->ep_in)
	{
		// size of received data
		hid->rx_len = len;

		// process received report - keyboard
		if (hid->protocol == USB_HID_ITF_KEYB)
		{
			// length correction
			if (len > sizeof(sUsbHidKey)) len = sizeof(sUsbHidKey);

			// clear rest of report
			if (len < sizeof(sUsbHidKey)) memset(hid->rx_buf+len, 0, sizeof(sUsbHidKey) - len);

			// check change
			sUsbHidKey* rep = (sUsbHidKey*)hid->rx_buf; // new report
			if (memcmp(&UsbHidKeyOld, rep, len) != 0)
			{
				sEvent e;

				// process keys
				u8 i;
				for (i = 0; i < 6; i++)
				{
					// get this key
					u8 key = rep->key[i];

					// this key is valid
					if (key != 0)
					{
						// check old key - is it first press?
						if (UsbHostHidFindKey(&UsbHidKeyOld, key))
						{
							// key is pressed and was pressed ... nothing to do
						}

						// key is pressed and was not pressed - send key event
						else
						{
							// check SHIFT
							u8 shift = ((rep->modi & (USB_KEY_MODI_LSHIFT | USB_KEY_MODI_RSHIFT)) != 0) ? 1 : 0;
							u8 ctrl = ((rep->modi & (USB_KEY_MODI_LCTRL | USB_KEY_MODI_RCTRL)) != 0) ? 1 : 0;

							// get ASCII character
							u8 ch = NOCHAR;
							if (ctrl && (key >= HID_KEY_A) && (key <= HID_KEY_Z))
								ch = key - HID_KEY_A + 1;
							else
							{
								if (key < HIDKEY_TO_ASCII_NUM) ch = (u8)HidKeyToAscii[2*key + shift];
							}

							// send keyboard event
							e.data[0] = key | ((u32)rep->modi << 8) | ((u32)ch << 16);
							EventRingWrite(&UsbHostHidKeyRing, &e);

							// firt press, time of next repeat key
							UsbHidKeyNextTime = Time() + KEY_REP_TIME1*1000;
							UsbHidKeyRepeat = key;
						}
						break;
					}
				}

				// no key
				if (i == 6) UsbHidKeyRepeat = 0;

				// check release keys
				for (i = 0; i < 6; i++)
				{
					// get old key
					u8 key = UsbHidKeyOld.key[i];

					// this key is valid
					if (key != 0)
					{
						// check new key - is it release?
						if (!UsbHostHidFindKey(rep, key))
						{
							// send keyboard event
							e.data[0] = key | ((u32)rep->modi << 8) | B24;
							EventRingWrite(&UsbHostHidKeyRing, &e);
						}
					}
				}

				// check modifiers, simulate key press
				u8 mask = B0;
				for (i = 0; i < 8; i++)
				{
					// press modifier
					if (((rep->modi & mask) != 0) && ((UsbHidKeyOld.modi & mask) == 0))
					{
						e.data[0] = (i + 0xe0) | ((u32)rep->modi << 8);
						EventRingWrite(&UsbHostHidKeyRing, &e);
					}

					// release modifier
					if (((rep->modi & mask) == 0) && ((UsbHidKeyOld.modi & mask) != 0))
					{
						e.data[0] = (i + 0xe0) | ((u32)rep->modi << 8) | B24;
						EventRingWrite(&UsbHostHidKeyRing, &e);
					}

					mask <<= 1;
				}

				// backup current report
				UsbMemcpy(&UsbHidKeyOld, rep, sizeof(sUsbHidKey));
			}
			hid->rx_len = 0;
		}
	
		// process received report - mouse
		else if (hid->protocol == USB_HID_ITF_MOUSE)
		{
			// length correction
			if (len > sizeof(sUsbHidMouse)) len = sizeof(sUsbHidMouse);

			// clear rest of report
			if (len < sizeof(sUsbHidMouse)) memset(hid->rx_buf+len, 0, sizeof(sUsbHidMouse) - len);

			// check change
			if (memcmp(&UsbHidMouseOld, hid->rx_buf, sizeof(sUsbHidMouse)) != 0)
			{
				sEvent e;
				e.data[0] = *(u32*)hid->rx_buf | B7;
				EventRingWrite(&UsbHostHidMouseRing, &e);
				UsbMemcpy(&UsbHidMouseOld, hid->rx_buf, sizeof(sUsbHidMouse));
			}
			hid->rx_len = 0;
		}

		// receive next data
		UsbHostHidInxRecv(hid_inx);
	}

	// send data
	else
	{
		// send next data
		UsbHostHidInxSend(hid_inx);
	}

	return True;
}

// close device
void UsbHostHidClose(u8 dev_addr)
{
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx]; // pointer to HID interface

		// deinit interface
		if (hid->used && (hid->dev_addr == dev_addr))
		{
			// keyboard interface
			if (hid->protocol == USB_HID_ITF_KEYB)
			{
				EventRingClear(&UsbHostHidKeyRing);
				memset(&UsbHidKeyOld, 0, sizeof(sUsbHidKey));
			}

			// mouse interface
			if (hid->protocol == USB_HID_ITF_MOUSE)
				EventRingClear(&UsbHostHidMouseRing);

			// clear HID interfaces
			memset(hid, 0, sizeof(sUsbHostHidInter));
		}
	}
}

// schedule driver, synchronized packets in frames
void UsbHostHidSof(u16 sof)
{
	UsbHostHidAllRecv();

	// send reports of all interfaces
	UsbHostHidAllSend();

	// repeat keys
	UsbHostHidRepKey();
}

// schedule driver (raised with UsbRescheduleDrv())
void UsbHostHidSched()
{
	// send reports of all interfaces
	UsbHostHidAllSend();
}

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
u8 UsbHostHidTxSave(u8 hid_inx, const void* buf, u8 len, u8 rep_id)
{
	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];
	if (hid->ep_out == 0) return 0; // output not supported
	hid->tx_len = 0; // destroy data in buffer

	// limit length of report
	if (len > USB_HOST_HID_TX_BUFSIZE) len = USB_HOST_HID_TX_BUFSIZE;
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

// send report (check UsbHostHidInxSendReady if next report can be sent; returns False if cannot send)
//  hid_inx ... interface index
//  buf ... buffer with report structure
//  len ... report length in bytes, including rep_id byte if it is used
//  rep_id ... report ID (0 = do not use)
Bool UsbHostHidSendReport(u8 hid_inx, const void* buf, u8 len, u8 rep_id)
{
	// check if interface is mounted
	if (!UsbHostHidInxIsMounted(hid_inx)) return False; // not mounted

	// pointer to HID interface
	sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

	// not sending
	if (hid->ep_out == 0) return False;

	// save data to send buffer
	len = UsbHostHidTxSave(hid->itf_num, buf, len, rep_id);
	if (len == 0) return False;

	// update length
	dmb(); // data memory barrier
	hid->tx_len = len;
	dmb(); // data memory barrier

	// request to send report
	UsbRescheduleDrv();

	return True;
}

// check if USB keyboard is mounted
Bool UsbKeyIsMounted()
{
	// host is not initialized
	if (!UsbHostIsInit) return False;

	// USB is not mounted
	if (!UsbIsMounted()) return False;

	// search keyboard interface
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

		// check interface
		if (hid->used && (hid->protocol == USB_HID_ITF_KEYB)) return True;
	}
	return False;
}

// check if USB mouse is mounted
Bool UsbMouseIsMounted()
{
	// host is not initialized
	if (!UsbHostIsInit) return False;

	// USB is not mounted
	if (!UsbIsMounted()) return False;

	// search mouse interface
	u8 hid_inx;
	for (hid_inx = 0; hid_inx < USE_USB_HOST_HID; hid_inx++)
	{
		// pointer to HID interface
		sUsbHostHidInter* hid = &UsbHostHidInter[hid_inx];

		// check interface
		if (hid->used && (hid->protocol == USB_HID_ITF_MOUSE)) return True;
	}
	return False;
}

// get USB key (returns u32 packed keyboard event, or 0 if no key)
//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
//    bit 8..15: modifiers USB_KEY_MODI_*
//    bit 16..23: ASCII character CH_* (NOCHAR = no valid character)
u32 UsbGetKey()
{
	sEvent e;
	u32 k;
	while (EventRingRead(&UsbHostHidKeyRing, &e))
	{
		k = e.data[0];
		if ((k & B24) == 0) return k;
	}
	return 0;
}

// get USB key, including release key (returns u32 packed keyboard event, or 0 if no key)
//    bit 0..7: key code HID_KEY_* (NOKEY = no valid key code)
//    bit 8..15: modifiers USB_KEY_MODI_*
//    bit 16..23: ASCII character CH_* (NOCHAR = no valid character or release key)
//    bit 24: 0=press key, 1=release key
// In case of key release, ASCII character is invalid (= 0).
u32 UsbGetKeyRel()
{
	sEvent e;
	if (!EventRingRead(&UsbHostHidKeyRing, &e)) return 0;
	return e.data[0];
}

// convert USB key to PC scan code
//    key ... USB key, as returned from UsbGetKeyRel() function (including release flag)
// Output:
//    bit 0..6: PC key scan code (0=invalid)
//    bit 7: 1=release key
//    bit 8: 1=extended key
u16 UsbKeyToScan(u32 key)
{
	u8 hid = key & 0xff;
	u16 scan = 0;

	if ((hid >= HIDKEY_TO_SCAN1_FIRST) && (hid <= HIDKEY_TO_SCAN1_LAST))
		scan = HidKeyToScan1[hid - HIDKEY_TO_SCAN1_FIRST];
	else if ((hid >= HIDKEY_TO_SCAN2_FIRST) && (hid <= HIDKEY_TO_SCAN2_LAST))
		scan = HidKeyToScan2[hid - HIDKEY_TO_SCAN2_FIRST];

	if (scan != 0)
	{
		scan = (scan & 0x7f) | ((scan & B7) << 1); // extended flag
		if ((key & B24) != 0) scan |= B7; // release flag
	}

	return scan;
}

// check if key is pressed
Bool UsbKeyIsPressed(u8 key)
{
	switch (key)
	{
	case HID_KEY_CONTROL_LEFT: return (UsbHidKeyOld.modi & USB_KEY_MODI_LCTRL) != 0;
	case HID_KEY_SHIFT_LEFT: return (UsbHidKeyOld.modi & USB_KEY_MODI_LSHIFT) != 0;
	case HID_KEY_ALT_LEFT: return (UsbHidKeyOld.modi & USB_KEY_MODI_LALT) != 0;
	case HID_KEY_GUI_LEFT: return (UsbHidKeyOld.modi & USB_KEY_MODI_LWIN) != 0;
	case HID_KEY_CONTROL_RIGHT: return (UsbHidKeyOld.modi & USB_KEY_MODI_RCTRL) != 0;
	case HID_KEY_SHIFT_RIGHT: return (UsbHidKeyOld.modi & USB_KEY_MODI_RSHIFT) != 0;
	case HID_KEY_ALT_RIGHT: return (UsbHidKeyOld.modi & USB_KEY_MODI_RALT) != 0;
	case HID_KEY_GUI_RIGHT: return (UsbHidKeyOld.modi & USB_KEY_MODI_RWIN) != 0;
	case NOKEY: return False;
	}
	return UsbHostHidFindKey(&UsbHidKeyOld, key);
}

// get USB character (returns NOCHAR if no character)
char UsbGetChar()
{
	u32 k;
	char ch;
	do {
		k = UsbGetKey();
		if (k == 0) return NOCHAR;
		ch = (char)(k >> 16);
	} while (ch == NOCHAR);
	return ch;
}

// flush keys
void UsbFlushKey()
{
	while (UsbGetKey() != NOKEY) {}
}

// get USB mouse (returns u32 packed mouse event, or 0 if no mouse event)
//    bit 0..7: buttons mask USB_MOUSE_BTN_*, B7 is set to indicate valid mouse event
//    bit 8..15: delta X movement (signed s8)
//    bit 16..23: delta Y movement (signed s8)
//    bit 24..31: delta wheel movement (signed s8) ... most mouse does not report wheel movement on uset boot mode
u32 UsbGetMouse()
{
	sEvent e;
	if (!EventRingRead(&UsbHostHidMouseRing, &e)) return 0;
	return e.data[0];
}

// check no pressed key
Bool UsbKeyNoPressed()
{
	const sUsbHidKey* report = &UsbHidKeyOld;

	u8 i;
	for (i = 0; i < 6; i++)
	{
		if (report->key[i] != 0) return False;
	}

	return report->modi == 0;
}

// wait for no key pressed
void UsbKeyWaitNoPressed()
{
	while (!UsbKeyNoPressed()) {}
}

#endif // USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)
#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)


// Convert HID key code to ASCII character
//	ascii = table[key_code*2 + shift]
const char HidKeyToAscii[2*HIDKEY_TO_ASCII_NUM] = {
	0,	0,		// 0x00 HID_KEY_NONE
	0,	0,		// 0x01 HID_KEY_ERR_ROLLOVER
	0,	0,		// 0x02 HID_KEY_POSTFAIL
	0,	0,		// 0x03 HID_KEY_ERR_UNDEFINED
	'a',	'A',		// 0x04 HID_KEY_A
	'b',	'B',		// 0x05 HID_KEY_B
	'c',	'C',		// 0x06 HID_KEY_C
	'd',	'D',		// 0x07 HID_KEY_D
	'e',	'E',		// 0x08 HID_KEY_E
	'f',	'F',		// 0x09 HID_KEY_F
	'g',	'G',		// 0x0a HID_KEY_G
	'h',	'H',		// 0x0b HID_KEY_H
	'i',	'I',		// 0x0c HID_KEY_I
	'j',	'J',		// 0x0d HID_KEY_J
	'k',	'K',		// 0x0e HID_KEY_K
	'l',	'L',		// 0x0f HID_KEY_L
	'm',	'M',		// 0x10 HID_KEY_M
	'n',	'N',		// 0x11 HID_KEY_N
	'o',	'O',		// 0x12 HID_KEY_O
	'p',	'P',		// 0x13 HID_KEY_P
	'q',	'Q',		// 0x14 HID_KEY_Q
	'r',	'R',		// 0x15 HID_KEY_R
	's',	'S',		// 0x16 HID_KEY_S
	't',	'T',		// 0x17 HID_KEY_T
	'u',	'U',		// 0x18 HID_KEY_U
	'v',	'V',		// 0x19 HID_KEY_V
	'w',	'W',		// 0x1a HID_KEY_W
	'x',	'X',		// 0x1b HID_KEY_X
	'y',	'Y',		// 0x1c HID_KEY_Y
	'z',	'Z',		// 0x1d HID_KEY_Z
	'1',	'!',		// 0x1e HID_KEY_1
	'2',	'@',		// 0x1f HID_KEY_2
	'3',	'#',		// 0x20 HID_KEY_3
	'4',	'$',		// 0x21 HID_KEY_4
	'5',	'%',		// 0x22 HID_KEY_5
	'6',	'^',		// 0x23 HID_KEY_6
	'7',	'&',		// 0x24 HID_KEY_7
	'8',	'*',		// 0x25 HID_KEY_8
	'9',	'(',		// 0x26 HID_KEY_9
	'0',	')',		// 0x27 HID_KEY_0
	'\r',	'\r',		// 0x28 HID_KEY_ENTER
	'\33',	'\33',		// 0x29 HID_KEY_ESCAPE ('\e')
	'\b',	'\b',		// 0x2a HID_KEY_BACKSPACE
	'\t',	'\t',		// 0x2b HID_KEY_TAB
	' ',	' ',		// 0x2c HID_KEY_SPACE
	'-',	'_',		// 0x2d HID_KEY_MINUS
	'=',	'+',		// 0x2e HID_KEY_EQUAL
	'[',	'{',		// 0x2f HID_KEY_BRACKET_LEFT
	']',	'}',		// 0x30 HID_KEY_BRACKET_RIGHT
	'\\',	'|',		// 0x31 HID_KEY_BACKSLASH
	'#',	'~',		// 0x32 HID_KEY_HASH
	';',	':',		// 0x33 HID_KEY_SEMICOLON
	'\'',	'\"',		// 0x34 HID_KEY_APOSTROPHE
	'`',	'~',		// 0x35 HID_KEY_GRAVE
	',',	'<',		// 0x36 HID_KEY_COMMA
	'.',	'>',		// 0x37 HID_KEY_PERIOD
	'/',	'?',		// 0x38 HID_KEY_SLASH
	0,	0,		// 0x39 HID_KEY_CAPS_LOCK
	0,	0,		// 0x3a HID_KEY_F1
	0,	0,		// 0x3b HID_KEY_F2
	0,	0,		// 0x3c HID_KEY_F3
	0,	0,		// 0x3d HID_KEY_F4
	0,	0,		// 0x3e HID_KEY_F5
	0,	0,		// 0x3f HID_KEY_F6
	0,	0,		// 0x40 HID_KEY_F7
	0,	0,		// 0x41 HID_KEY_F8
	0,	0,		// 0x42 HID_KEY_F9
	0,	0,		// 0x43 HID_KEY_F10
	0,	0,		// 0x44 HID_KEY_F11
	0,	0,		// 0x45 HID_KEY_F12
	0,	0,		// 0x46 HID_KEY_PRINT_SCREEN
	0,	0,		// 0x47 HID_KEY_SCROLL_LOCK
	0,	0,		// 0x48 HID_KEY_PAUSE
	CH_INS,	CH_INS,		// 0x49 HID_KEY_INSERT (^T)
	CH_HOME,CH_HOME,	// 0x4a HID_KEY_HOME (^U)
	CH_PGUP,CH_PGUP,	// 0x4b HID_KEY_PAGE_UP (^K)
	CH_DEL,	CH_DEL,		// 0x4c HID_KEY_DELETE
	CH_END,	CH_END,		// 0x4d HID_KEY_END (^D)
	CH_PGDN,CH_PGDN,	// 0x4e HID_KEY_PAGE_DOWN (^L)
	CH_RIGHT,CH_RIGHT,	// 0x4f HID_KEY_ARROW_RIGHT ("^\")
	CH_LEFT,CH_LEFT,	// 0x50 HID_KEY_ARROW_LEFT (^^)
	CH_DOWN,CH_DOWN,	// 0x51 HID_KEY_ARROW_DOWN (^_)
	CH_UP,	CH_UP,		// 0x52 HID_KEY_ARROW_UP (^])
	0,	0,		// 0x53 HID_KEY_NUM_LOCK
	'/',	'/',		// 0x54 HID_KEY_KEYPAD_DIVIDE
	'*',	'*',		// 0x55 HID_KEY_KEYPAD_MULTIPLY
	'-',	'-',		// 0x56 HID_KEY_KEYPAD_SUBTRACT
	'+',	'+',		// 0x57 HID_KEY_KEYPAD_ADD
	'\r',	'\r',		// 0x58 HID_KEY_KEYPAD_ENTER
	'1',	CH_END,		// 0x59 HID_KEY_KEYPAD_1
	'2',	CH_DOWN,	// 0x5a HID_KEY_KEYPAD_2
	'3',	CH_PGDN,	// 0x5b HID_KEY_KEYPAD_3
	'4',	CH_LEFT,	// 0x5c HID_KEY_KEYPAD_4
	'5',	'5',		// 0x5d HID_KEY_KEYPAD_5
	'6',	CH_RIGHT,	// 0x5e HID_KEY_KEYPAD_6
	'7',	CH_HOME,	// 0x5f HID_KEY_KEYPAD_7
	'8',	CH_UP,		// 0x60 HID_KEY_KEYPAD_8
	'9',	CH_PGUP,	// 0x61 HID_KEY_KEYPAD_9
	'0',	CH_INS,		// 0x62 HID_KEY_KEYPAD_0
	'.',	CH_DEL,		// 0x63 HID_KEY_KEYPAD_DECIMAL
	'\\',	'|',		// 0x64 HID_KEY_INTL_BACKSLASH
	0,	0,		// 0x65 HID_KEY_APPLICATION
	0,	0,		// 0x66 HID_KEY_POWER
	'=',	'='		// 0x67 HID_KEY_KEYPAD_EQUAL
};

// Convert HID key code to PC scan code (B7: extended scan code with 0xE0 prefix)
//   1st part: HID code 0x04..0x65
const u8 HidKeyToScan1[HIDKEY_TO_SCAN1_LAST+1-HIDKEY_TO_SCAN1_FIRST] = {
	PC_KEYSCAN_A,		// 0x04 HID_KEY_A
	PC_KEYSCAN_B,		// 0x05 HID_KEY_B
	PC_KEYSCAN_C,		// 0x06 HID_KEY_C
	PC_KEYSCAN_D,		// 0x07 HID_KEY_D
	PC_KEYSCAN_E,		// 0x08 HID_KEY_E
	PC_KEYSCAN_F,		// 0x09 HID_KEY_F
	PC_KEYSCAN_G,		// 0x0A HID_KEY_G
	PC_KEYSCAN_H,		// 0x0B HID_KEY_H
	PC_KEYSCAN_I,		// 0x0C HID_KEY_I
	PC_KEYSCAN_J,		// 0x0D HID_KEY_J
	PC_KEYSCAN_K,		// 0x0E HID_KEY_K
	PC_KEYSCAN_L,		// 0x0F HID_KEY_L
	PC_KEYSCAN_M,		// 0x10 HID_KEY_M
	PC_KEYSCAN_N,		// 0x11 HID_KEY_N
	PC_KEYSCAN_O,		// 0x12 HID_KEY_O
	PC_KEYSCAN_P,		// 0x13 HID_KEY_P
	PC_KEYSCAN_Q,		// 0x14 HID_KEY_Q
	PC_KEYSCAN_R,		// 0x15 HID_KEY_R
	PC_KEYSCAN_S,		// 0x16 HID_KEY_S
	PC_KEYSCAN_T,		// 0x17 HID_KEY_T
	PC_KEYSCAN_U,		// 0x18 HID_KEY_U
	PC_KEYSCAN_V,		// 0x19 HID_KEY_V
	PC_KEYSCAN_W,		// 0x1A HID_KEY_W
	PC_KEYSCAN_X,		// 0x1B HID_KEY_X
	PC_KEYSCAN_Y,		// 0x1C HID_KEY_Y
	PC_KEYSCAN_Z,		// 0x1D HID_KEY_Z
	PC_KEYSCAN_1,		// 0x1E HID_KEY_1 (1 and !)
	PC_KEYSCAN_2,		// 0x1F HID_KEY_2 (2 and @)
	PC_KEYSCAN_3,		// 0x20 HID_KEY_3 (3 and #)
	PC_KEYSCAN_4,		// 0x21 HID_KEY_4 (4 and $)
	PC_KEYSCAN_5,		// 0x22 HID_KEY_5 (5 and %)
	PC_KEYSCAN_6,		// 0x23 HID_KEY_6 (6 and ^)
	PC_KEYSCAN_7,		// 0x24 HID_KEY_7 (7 and &)
	PC_KEYSCAN_8,		// 0x25 HID_KEY_8 (8 and *)
	PC_KEYSCAN_9,		// 0x26 HID_KEY_9 (9 and ()
	PC_KEYSCAN_0,		// 0x27 HID_KEY_0 (0 and ))
	PC_KEYSCAN_ENTER,	// 0x28 HID_KEY_ENTER
	PC_KEYSCAN_ESC,		// 0x29 HID_KEY_ESCAPE
	PC_KEYSCAN_BS,		// 0x2A HID_KEY_BACKSPACE
	PC_KEYSCAN_TAB,		// 0x2B HID_KEY_TAB
	PC_KEYSCAN_SPACE,	// 0x2C HID_KEY_SPACE
	PC_KEYSCAN_HYPHEN,	// 0x2D HID_KEY_MINUS (- and _)
	PC_KEYSCAN_EQU,		// 0x2E HID_KEY_EQUAL (= and +)
	PC_KEYSCAN_LBRACKET,	// 0x2F HID_KEY_BRACKET_LEFT ([ and {)
	PC_KEYSCAN_RBRACKET,	// 0x30 HID_KEY_BRACKET_RIGHT (] and })
	PC_KEYSCAN_BACKSLASH,	// 0x31 HID_KEY_BACKSLASH ('\' and |)
	PC_KEYSCAN_BACKQUOTE,	// 0x32 HID_KEY_HASH (Europe # and ~)
	PC_KEYSCAN_SEMICOLON,	// 0x33 HID_KEY_SEMICOLON (; and :)
	PC_KEYSCAN_SQUOTE,	// 0x34 HID_KEY_APOSTROPHE (' and ")
	PC_KEYSCAN_BACKQUOTE,	// 0x35 HID_KEY_GRAVE (` and ~)
	PC_KEYSCAN_COMMA,	// 0x36 HID_KEY_COMMA (, and <)
	PC_KEYSCAN_PERIOD,	// 0x37 HID_KEY_PERIOD (. and >)
	PC_KEYSCAN_SLASH,	// 0x38 HID_KEY_SLASH (/ and ?)
	PC_KEYSCAN_CAPSLOCK,	// 0x39 HID_KEY_CAPS_LOCK
	PC_KEYSCAN_F1,		// 0x3A HID_KEY_F1
	PC_KEYSCAN_F2,		// 0x3B HID_KEY_F2
	PC_KEYSCAN_F3,		// 0x3C HID_KEY_F3
	PC_KEYSCAN_F4,		// 0x3D HID_KEY_F4
	PC_KEYSCAN_F5,		// 0x3E HID_KEY_F5
	PC_KEYSCAN_F6,		// 0x3F HID_KEY_F6
	PC_KEYSCAN_F7,		// 0x40 HID_KEY_F7
	PC_KEYSCAN_F8,		// 0x41 HID_KEY_F8
	PC_KEYSCAN_F9,		// 0x42 HID_KEY_F9
	PC_KEYSCAN_F10,		// 0x43 HID_KEY_F10
	PC_KEYSCAN_F11,		// 0x44 HID_KEY_F11
	PC_KEYSCAN_F12,		// 0x45 HID_KEY_F12
	B7 + PC_KEYSCAN_PRINTSCR, // 0x46 HID_KEY_PRINT_SCREEN
	PC_KEYSCAN_SCROLL,	// 0x47 HID_KEY_SCROLL_LOCK
	B7 + PC_KEYSCAN_PAUSE,	// 0x48 HID_KEY_PAUSE
	B7 + PC_KEYSCAN_INS,	// 0x49 HID_KEY_INSERT
	B7 + PC_KEYSCAN_HOME,	// 0x4A HID_KEY_HOME
	B7 + PC_KEYSCAN_PGUP,	// 0x4B HID_KEY_PAGE_UP
	B7 + PC_KEYSCAN_DEL,	// 0x4C HID_KEY_DELETE
	B7 + PC_KEYSCAN_END,	// 0x4D HID_KEY_END
	B7 + PC_KEYSCAN_PGDN,	// 0x4E HID_KEY_PAGE_DOWN
	B7 + PC_KEYSCAN_RIGHT,	// 0x4F HID_KEY_ARROW_RIGHT
	B7 + PC_KEYSCAN_LEFT,	// 0x50 HID_KEY_ARROW_LEFT
	B7 + PC_KEYSCAN_DOWN,	// 0x51 HID_KEY_ARROW_DOWN
	B7 + PC_KEYSCAN_UP,	// 0x52 HID_KEY_ARROW_UP
	PC_KEYSCAN_NUMLOCK,	// 0x53 HID_KEY_NUM_LOCK
	B7 + PC_KEYSCAN_GREYSLASH, // 0x54 HID_KEY_KEYPAD_DIVIDE
	PC_KEYSCAN_GREYSTAR,	// 0x55 HID_KEY_KEYPAD_MULTIPLY
	PC_KEYSCAN_GREYMINUS,	// 0x56 HID_KEY_KEYPAD_SUBTRACT
	PC_KEYSCAN_GREYPLUS,	// 0x57 HID_KEY_KEYPAD_ADD
	B7 + PC_KEYSCAN_GREYENTER, // 0x58 HID_KEY_KEYPAD_ENTER
	PC_KEYSCAN_NUM1,	// 0x59 HID_KEY_KEYPAD_1 (keypad 1 and End)
	PC_KEYSCAN_NUM2,	// 0x5A HID_KEY_KEYPAD_2 (keypad 2 and Arrow Down)
	PC_KEYSCAN_NUM3,	// 0x5B HID_KEY_KEYPAD_3 (keypad 3 and Page Down)
	PC_KEYSCAN_NUM4,	// 0x5C HID_KEY_KEYPAD_4 (keypad 4 and Arrow Left)
	PC_KEYSCAN_NUM5,	// 0x5D HID_KEY_KEYPAD_5
	PC_KEYSCAN_NUM6,	// 0x5E HID_KEY_KEYPAD_6 (keypad 6 and Arrow Right)
	PC_KEYSCAN_NUM7,	// 0x5F HID_KEY_KEYPAD_7 (keypad 7 and Home)
	PC_KEYSCAN_NUM8,	// 0x60 HID_KEY_KEYPAD_8 (keypad 8 and Arrow Up)
	PC_KEYSCAN_NUM9,	// 0x61 HID_KEY_KEYPAD_9 (keypad 9 and Page Up)
	PC_KEYSCAN_NUM0,	// 0x62 HID_KEY_KEYPAD_0 (keypad 0 and Insert)
	PC_KEYSCAN_DECIMAL,	// 0x63 HID_KEY_KEYPAD_DECIMAL
	PC_KEYSCAN_BACKSLASH2,	// 0x64 HID_KEY_INTL_BACKSLASH (Europe '\' and |)
	B7 + PC_KEYSCAN_MENU,	// 0x65 HID_KEY_APPLICATION (Menu)
};

//   2nd part: HID code 0xE0..0xE7
const u8 HidKeyToScan2[HIDKEY_TO_SCAN2_LAST+1-HIDKEY_TO_SCAN2_FIRST] = {
	PC_KEYSCAN_LCTRL,	// 0xE0 HID_KEY_CONTROL_LEFT
	PC_KEYSCAN_LSHIFT,	// 0xE1 HID_KEY_SHIFT_LEFT
	PC_KEYSCAN_LALT,	// 0xE2 HID_KEY_ALT_LEFT
	B7 + PC_KEYSCAN_LWIN,	// 0xE3 HID_KEY_GUI_LEFT (Left Window)
	B7 + PC_KEYSCAN_RCTRL,	// 0xE4 HID_KEY_CONTROL_RIGHT
	PC_KEYSCAN_RSHIFT,	// 0xE5 HID_KEY_SHIFT_RIGHT
	B7 + PC_KEYSCAN_RALT,	// 0xE6 HID_KEY_ALT_RIGHT
	B7 + PC_KEYSCAN_RWIN,	// 0xE7 HID_KEY_GUI_RIGHT (Right Window)
};
