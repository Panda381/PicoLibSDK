
// ****************************************************************************
//
//                          USB Device Class Driver
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
//  and Copyright (c) 2020 Raspberry Pi (Trading) Ltd.

#include "../../global.h"	// globals

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)

#include "../inc/sdk_irq.h"
#include "../inc/sdk_timer.h"
#include "../../_lib/inc/lib_text.h" // StrLen
#include "../usb_inc/sdk_usb_phy.h" // physical layer
#include "../usb_inc/sdk_usb_dev.h" // devices

//#include "../usb_inc/sdk_usb_dev_audio.h" // USB Audio Device Class (device)
//#include "../usb_inc/sdk_usb_dev_bth.h"	// USB Bluetooth Device Class (device)

#include "../usb_inc/sdk_usb_dev_cdc.h"	// USB Communication Device Class (device)

//#include "../usb_inc/sdk_usb_dev_dfu.h"	// USB Firmware Upgrade Device Class (device)
#include "../usb_inc/sdk_usb_dev_hid.h"	// USB Human Interface Device Class (device)
//#include "../usb_inc/sdk_usb_dev_midi.h" // USB Midi Device Class (device)
//#include "../usb_inc/sdk_usb_dev_msc.h"	// USB Mass Storage Class (device)
//#include "../usb_inc/sdk_usb_dev_net.h"	// USB Network Adapter Device Class (device)
//#include "../usb_inc/sdk_usb_dev_tmc.h"	// USB Test and Measurement Device Class (device)
//#include "../usb_inc/sdk_usb_dev_vendor.h"  // USB Vendor Device Class (device)
//#include "../usb_inc/sdk_usb_setupd.h"	// device control

// Device: setup buffer for device
ALIGNED u8 UsbDevSetupBuff[USB_DEVSETUPBUFF_MAX]; // size 64 bytes

// Device: mapping endpoint to device driver (USB_DRVID_INVALID = invalid driver)
u8 UsbDevEp2Drv[USB_ENDPOINT_NUM2]; // size 32 bytes

// Device: mapping interface number to device driver (USB_DRVID_INVALID = invalid)
u8 UsbDevItf2Drv[USB_ENDPOINT_NUM]; // size 16 bytes

// Device: process control transfer complete callback (returns False to stall; NULL = none)
// Used internally to call device 'ctrl' function.
pUsbDevSetupCompCB UsbDevSetupCompCB;

// pointer to application device setup descriptor
const sUsbDevSetupDesc* UsbDevSetupDesc;

// Device: device assigned address (>0 device is addressed)
volatile u8 UsbDevAddress;

// device data
volatile Bool UsbDevConnected;		// device is connected (= 1st SETUP packet has been received)
volatile Bool UsbDevSuspended;		// device is suspended
volatile Bool UsbDevSelfPowered;		// device is self powered (updated by UsbDevSetCfg)
volatile Bool UsbDevWakeupEn;		// remote wake up host by driver is enabled

// device class drivers
const sUsbDevDrv UsbDevDrv[] = {

#if USE_USB_DEV_CDC	// use USB CDC Communication Device Class (device)
	{
		UsbDevCdcInit,		// initialize class driver
		UsbDevCdcTerm,		// terminate class driver
		UsbDevCdcReset,		// reset class driver
		UsbDevCdcOpen,		// open device class interface
		UsbDevCdcCtrl,		// process control transfer complete
		UsbDevCdcComp,		// process data transfer complete
		UsbDevCdcSof, 		// receiving SOF (start of frame; NULL=not used)
		NULL,			// schedule driver (NULL=not used, raised with UsbRescheduleDrv())
	},
#endif

/*
#if USE_USB_DEV_MSC	// use USB MSC Mass Storage Class (device)
	{
		UsbDevMscInit,		// initialize class driver
		UsbDevMscTerm,		// terminate class driver
		UsbDevMscReset,		// reset class driver
		UsbDevMscOpen,		// open device class interface
		UsbDevMscCtrl,		// process control transfer complete
		UsbDevMscComp,		// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif
*/

#if USE_USB_DEV_HID	// use USB HID Human Interface Device (device)
	{
		UsbDevHidInit,		// initialize class driver
		UsbDevHidTerm,		// terminate class driver
		UsbDevHidReset,		// reset class driver
		UsbDevHidOpen,		// open device class interface
		UsbDevHidCtrl,		// process control transfer complete
		UsbDevHidComp,		// process data transfer complete
		NULL, 			// receiving SOF (start of frame; NULL=not used)
		UsbDevHidSched,		// schedule driver (NULL=not used, raised with UsbRescheduleDrv())
	},
#endif

/*
#if USE_USB_DEV_AUDIO	// use USB AUDIO Audio device (device)
	{
		UsbDevAudioInit,	// initialize class driver
		UsbDevAudioTerm,	// terminate class driver
		UsbDevAudioReset,	// reset class driver
		UsbDevAudioOpen,	// open device class interface
		UsbDevAudioCtrl,	// process control transfer complete
		UsbDevAudioComp,	// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif

#if USE_USB_DEV_VIDEO	// use USB VIDEO Video device (device)
	{
		UsbDevVideoInit,	// initialize class driver
		UsbDevVideoTerm,	// terminate class driver
		UsbDevVideoReset,	// reset class driver
		UsbDevVideoOpen,	// open device class interface
		UsbDevVideoCtrl,	// process control transfer complete
		UsbDevVideoComp,	// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif

#if USE_USB_DEV_MIDI	// use USB MIDI Midi device (device)
	{
		UsbDevMidiInit,		// initialize class driver
		UsbDevMidiTerm,		// terminate class driver
		UsbDevMidiReset,	// reset class driver
		UsbDevMidiOpen,		// open device class interface
		UsbDevMidiCtrl,		// process control transfer complete
		UsbDevMidiComp,		// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif
*/

/*
#if USE_USB_DEV_VENDOR	// use USB VENDOR Vendor specific device (device)
	{
		UsbDevVendorInit,	// initialize class driver
		UsbDevVendorTerm,	// terminate class driver
		UsbDevVendorReset,	// reset class driver
		UsbDevVendorOpen,	// open device class interface
		UsbDevVendorCtrl,	// process control transfer complete
		UsbDevVendorComp,	// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif
*/

/*
#if USE_USB_DEV_TMC	// use USB TMC Test and Measurement Class (device)
	{
		UsbDevTmcInit,		// initialize class driver
		UsbDevTmcTerm,		// terminate class driver
		UsbDevTmcReset,		// reset class driver
		UsbDevTmcOpen,		// open device class interface
		UsbDevTmcCtrl,		// process control transfer complete
		UsbDevTmcComp,		// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif

#if USE_USB_DEV_DFU	// use USB DFU-RT Device Firmware Upgrade (device)
	{
		UsbDevDfuInit,		// initialize class driver
		UsbDevDfuTerm,		// terminate class driver
		UsbDevDfuReset,		// reset class driver
		UsbDevDfuOpen,		// open device class interface
		UsbDevDfuCtrl,		// process control transfer complete
		UsbDevDfuComp,		// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif

#if USE_USB_DEV_NET	// use USB NET Network adapter (device)
	{
		UsbDevNetInit,		// initialize class driver
		UsbDevNetTerm,		// terminate class driver
		UsbDevNetReset,		// reset class driver
		UsbDevNetOpen,		// open device class interface
		UsbDevNetCtrl,		// process control transfer complete
		UsbDevNetComp,		// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif

#if USE_USB_DEV_BTH	// use USB BTH Bluetooth (device)
	{
		UsbDevBthInit,		// initialize class driver
		UsbDevBthTerm,		// terminate class driver
		UsbDevBthReset,		// reset class driver
		UsbDevBthOpen,		// open device class interface
		UsbDevBthCtrl,		// process control transfer complete
		UsbDevBthComp,		// process data transfer complete
		NULL,			// receiving SOF (start of frame; NULL=not used)
	},
#endif
*/
};

// number of device drivers
const u8 UsbDevDrvNum = count_of(UsbDevDrv);

// ----------------------------------------------------------------------------
//                              Utilities
// ----------------------------------------------------------------------------

// enable SOF interrupt
void UsbDevSofEnable()
{
	RegSet(USB_INTE, B17);
}

// disable SOF interrupt (if not used by drivers)
void UsbDevSofDisable()
{
	if (!UsbProcessSOFEvent) RegClr(USB_INTE, B17);
}

// request host to wake up (only if suspended and if wakeup is enabled)
// In case of remote wake up, no RESUME signal will be received, we must resume by SOF packet.
void UsbDevWakeup()
{
	if (UsbDevSuspended && UsbDevWakeupEn)
	{
		// enable SOF interrupt
		UsbDevSofEnable();

		// request to remote wake up
		RegSet(USB_SIE_CTRL, B12);
	}
}

// send STALL signal to the host
//  epinx ... endpoint index 0..31
void UsbDevSetStall(u8 epinx)
{
	// set stall bit on EP0 (IN or OUT), so it can be cleared on next setup packet
	if (epinx < 2) RegSet(USB_STALL_ARM, (USB_DIR(epinx) == USB_DIR_IN) ? B0 : B1);

	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// set buffer control STALL request (do not use RegSet, it does not work)
	*sep->buf_ctrl |= B11;

	// set stalled flag
	sep->stalled = True;
}

// clear STALL request
//  epinx ... endpoint index 0..31
void UsbDevClrStall(u8 epinx)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// stall bits on EP0 are cleared by receiving SETUP packet, so need to clear only data endpoints
	if (epinx >= 2)
	{
		// clear stall also resets toggle to DATA0 (EP0 was set to DATA1 by receiving SETUP pacet)
		sep->next_pid = 0; // reset DATA0/DATA1 toggle
	}

	// clear buffer control STALL request (do not use RegClr, it does not work)
	*sep->buf_ctrl &= ~B11;

	// clear stalled flag
	sep->stalled = False;
}

// ----------------------------------------------------------------------------
//                         Process SETUP control request
// ----------------------------------------------------------------------------

// get configuration descriptor by configuration number (returns NULL if not found)
const sUsbDescCfg* UsbDevGetDescCfg(u8 cfg)
{
	int i;
	const sUsbDescCfg** desc = UsbDevSetupDesc->desc_cfg_list;
	for (i = 0; i < UsbDevSetupDesc->desc_cfg_num; i++)
	{
		if ((*desc)->config == cfg) return *desc;
		desc++;
	}
	return NULL;
}

// get configuration descriptor by configuration index (returns NULL if not found)
const sUsbDescCfg* UsbDevGetDescCfgInx(u8 cfginx)
{
	if ((int)cfginx >= UsbDevSetupDesc->desc_cfg_num) return NULL;
	return UsbDevSetupDesc->desc_cfg_list[cfginx];
}

// process "get descriptor" request (returns False to stall)
Bool UsbDevGetDesc()
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// check IN direction (B7 = direction IN)
	if (setup->type < B7) return False;

	// pointer to device setup descriptor
	const sUsbDevSetupDesc* s = UsbDevSetupDesc;

	// prepare setup type and index
	u8 desc_type = (u8)(setup->value >> 8); // descriptor type
	u8 desc_index = (u8)setup->value; // descriptor index

	switch (desc_type)
	{
	// get device descriptor
	case USB_DESC_DEVICE:
		UsbDevSetupStart((void*)s->desc_dev, USB_DESC_LEN(s->desc_dev));
		break;

	// get configuration descriptor
	case USB_DESC_CONFIGURATION:
		{
			// get configuration descriptor by descriptor index
			const sUsbDescCfg* desc = UsbDevGetDescCfgInx(desc_index);
			if (desc == NULL) return False; // invalid configuration

			// send descriptor
			UsbDevSetupStart((void*)desc, desc->totallen);
		}
		break;

	// get binary device object store (BOS) descriptor
	case USB_DESC_BOS:
		if (s->desc_bos == NULL) return False;
		UsbDevSetupStart((void*)s->desc_bos, s->desc_bos->totallen);
		break;

	// get string descriptor
	case USB_DESC_STRING:
		{
			// check if string descriptor is supported
			if (s->str_cb != NULL)
			{
				const sUsbDescStr* desc = s->str_cb(desc_index, setup->index); // get string
				if (desc != NULL)
				{
					UsbDevSetupStart((void*)desc, desc->len); // send descriptor
					break;
				}
			}

			// alternative - index 0 to get language ID
			if (desc_index == 0)
			{
				UsbDevSetupBuff[0] = 4; // length
				UsbDevSetupBuff[1] = USB_DESC_STRING; // descriptor type
				UsbDevSetupBuff[2] = 0x09; // language ID = 0x0409 (English)
				UsbDevSetupBuff[3] = 0x04;
				UsbDevSetupStart(UsbDevSetupBuff, 4); // send descriptor
				break;
			}

			// alternative - use ASCIIZ string
			desc_index--;
			if ((s->str_list == NULL) || ((int)desc_index >= s->str_num)) return False;
			const char* txt = s->str_list[desc_index];

			// length of the text
			int len = StrLen(txt); // get length of the string
			if (len > (USB_PACKET_MAX-2)/2) len = (USB_PACKET_MAX-2)/2; // limit to packet size

			// prepare header
			UsbDevSetupBuff[0] = len*2 + 2; // length including header
			UsbDevSetupBuff[1] = USB_DESC_STRING; // descriptor type

			// decode UTF16 text (national characters are not supported)
			int i;
			u16* d = (u16*)(UsbDevSetupBuff+2);
			for (i = 0; i < len; i++) *d++ = *txt++;

			// send descriptor
			UsbDevSetupStart(UsbDevSetupBuff, len*2+2);
		}
		break;

	// invalid descriptor
	default:
		return False;
	}

	return True;
}

// process "set configure" request (cfg = 1..., returns False = stall control endpoint)
// - Function finds interface and map it to driver.
Bool UsbDevSetCfg(u8 cfg)
{
	// get pointer to required configuration by configuration number
	const sUsbDescCfg* desc_cfg = UsbDevGetDescCfg(cfg);
	if (desc_cfg == NULL) return False; // invalid configuration

	// get "self powered" flag
	UsbDevSelfPowered = ((desc_cfg->attrib & B6) != 0); // self powered

	// prepare pointers to additional descriptors
	const u8* p_desc = USB_DESC_NEXT(desc_cfg); // skip configuration descriptor
	const u8* desc_end = ((const u8*)desc_cfg) + desc_cfg->totallen; // end of all descriptors

	// parse interface descriptors
	while ((p_desc < desc_end) && (USB_DESC_NEXT(p_desc) <= desc_end))
	{
		// class will always start with interface association (if any) and then interface descriptor
		u8 assoc = 1; // number of interface associations
		if (USB_DESC_TYPE(p_desc) == USB_DESC_INTERFACE_ASSOCIATION)
		{
			assoc = ((const sUsbDescItfAssoc*)p_desc)->itf_count; // get number of interface associations
			p_desc = USB_DESC_NEXT(p_desc); // skip descriptor
		}

		// interface descriptor should follow
		if ((USB_DESC_NEXT(p_desc) > desc_end) || (USB_DESC_TYPE(p_desc) != USB_DESC_INTERFACE)) return False; // no interface descriptor
		const sUsbDescItf* desc_itf = (const sUsbDescItf*)p_desc;

		// search support of this interface in class device drivers
		u16 rem_len = desc_end - p_desc; // remaining length
		u8 drv_id;
		for (drv_id = 0; drv_id < UsbDevDrvNum; drv_id++)
		{
			// pointer to class driver
			const sUsbDevDrv* drv = &UsbDevDrv[drv_id];

			// open device class interface (returns size of used interface, 0=not supported)
			u16 desc_len = drv->open(desc_itf, rem_len);

			// check if interface is supported
			if ((desc_len >= sizeof(sUsbDescItf)) && (desc_len <= rem_len))
			{
				// bind interfaces to the driver
				u8 i;
				for (i = 0; i < assoc; i++)
				{
					// interface number
					u8 n = desc_itf->itf_num + i;
					if (n >= USB_ENDPOINT_NUM) return False;

					// set mapping of interface number to device driver
					UsbDevItf2Drv[n] = drv_id;
				}

			        // bind all endpoints to found driver
				UsbEpBindDrv(UsbDevEp2Drv, p_desc, desc_len, drv_id);

				// shift interface pointer
				p_desc += desc_len;
				break;
			}
		}

		// failed if cannot find supported driver
		if (drv_id >= UsbDevDrvNum) return False;
	}

	return True;
}

// acknowledging at Status Stage
void UsbDevSetupAck()
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// use opposite direction to endpoint in Data Stage (B7 = direction IN)
	u8 epinx = USB_EPINX(0, ((setup->type & B7) != 0) ? USB_DIR_OUT : USB_DIR_IN);

	// queue zero-length-packet (ZLP) in STATUS stage to acknowledge
	// Note: Status must always be DATA1. This is ensured when the SETUP packet
	//       is received because descriptor for opposite direction is used.
	UsbXferStart(epinx, NULL, 0, False);
}

// invoke class driver control request at SETUP stage, set continue to DATA stage (returns False to stall)
Bool UsbDevClassCtrl(const sUsbDevDrv* drv)
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// set control complete callback at DATA stage
	UsbDevSetupCompCB = drv->ctrl;

	// call control request callback at SETUP stage
	return drv->ctrl(USB_STAGE_SETUP);
}

// start transfer data to/from control endpoint (in required SETUP direction)
// - In some special cases data to be sent can be in UsbDevSetupBuff buffer (ASCII text descriptor).
void UsbDevSetupStart(void* buffer, u16 len)
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare data
	UsbSetupDataBuff = (u8*)buffer;	// buffer with data
	UsbSetupDataLen = len;		// total length of data
	if (len > setup->length) UsbSetupDataLen = setup->length; // limit length of data
	UsbSetupDataXfer = 0;		// already transferred data
                                              
	// required some data from host
	if (setup->length > 0)
	{
		// invalid buffer
		if (buffer == NULL) UsbSetupDataLen = 0;

		// transfer first part of data stage
		UsbDevSetupNext();
	}
	else
	{
		// acknowledging at Status Stage
		UsbDevSetupAck();
	}
}

// process SETUP control request (start of SETUP transaction; returns False = stall control endpoint)
Bool UsbDevSetupReq()
{
	// clear complete callback
	UsbDevSetupCompCB = NULL;

	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare request type
	u8 type = (setup->type >> 5) & 3;
	if (type == USB_REQ_TYPE_INVALID) return False; // invalid request type

/*
	// vendor request
	if (type == USB_REQ_TYPE_VENDOR) // request of vendor type
	{
#if USE_USB_VENDOR	// use USB VENDOR Vendor specific device (device, host)
		UsbDevSetupCompCB = UsbDevVendorCtrl; // vendor request callback
		return UsbDevVendorCtrl(USB_STAGE_SETUP); // vendor control request callback
#else
		return False; // vendor is not supported
#endif
	}
*/

	// check recipient
	switch (setup->type & 0x1f) // get recipient
	{
	// recipient is device - process enumeration process
	case USB_REQ_RCPT_DEVICE:

		// request type is class
		if (type == USB_REQ_TYPE_CLASS)
		{
			// interface index
			u8 itf = (u8)setup->index;
			if (itf >= USB_ENDPOINT_NUM) return False;

			// driver address (with remapping interface to driver)
			itf = UsbDevItf2Drv[itf];
			if (itf >= UsbDevDrvNum) return False;
			const sUsbDevDrv* drv = &UsbDevDrv[itf];

			// invoke class control request (returns False to stall)
			return UsbDevClassCtrl(drv);
		}

		// invalid request type
		if (type != USB_REQ_TYPE_STANDARD) return False;

		// STANDARD request type, check request
		switch (setup->request)
		{
		// set device address
		case USB_REQ_SET_ADDRESS:

			// Send empty data packet to acknowledging transfaction. 
			UsbXferStart(USB_EPINX(0, USB_DIR_IN), NULL, 0, False);
			break;

		// get current device configuration
		case USB_REQ_GET_CONFIGURATION:
			{
				// configuration number
				u8 cfg = UsbDevCfgNum;

				// transmit data to control endpoint
				UsbDevSetupStart(&cfg, 1);
			}
			break;

		// set current device configuration
		case USB_REQ_SET_CONFIGURATION:
			{
				// new configuration number
				u8 cfg = (u8)setup->value;

				// check if configuration is changing
				if (UsbDevCfgNum != cfg)
				{
					// reset old configuration
					if (UsbDevCfgNum != 0)
					{
						// reset all non-control endpoints
						UsbDevEpReset();

						// reset configuration of USB device drivers
						// (resets drivers and clears base variables, leaves SETUP data untouched)
						UsbDevCfgReset();
					}

					// set new configuration
					if (cfg != 0)
					{
						if (!UsbDevSetCfg(cfg)) return False;
					}

					// store new current configuration
					UsbDevCfgNum = cfg;

					// invoke mount callback
					if ((cfg != 0) && (UsbDevSetupDesc->mounted_cb != NULL)) UsbDevSetupDesc->mounted_cb();
				}

				// acknowledging at Status Stage
				UsbDevSetupAck();
			}
			break;

		// get descriptor
		case USB_REQ_GET_DESCRIPTOR:

			// process "get descriptor" request
			if (!UsbDevGetDesc()) return False;
			break;

		// set feature - enable remote wake up
		case USB_REQ_SET_FEATURE:

			// check supported feature
			if (setup->value != USB_REQ_FEATURE_REMOTE_WAKEUP) return False;

			// enable remote wake up
			UsbDevWakeupEn = True;

			// acknowledging at Status Stage
			UsbDevSetupAck();
			break;

		// clear feature - disable remote wake up
		case USB_REQ_CLEAR_FEATURE:

			// check supported feature
			if (setup->value != USB_REQ_FEATURE_REMOTE_WAKEUP) return False;

			// disable remote wake up
			UsbDevWakeupEn = False;

			// acknowledging at Status Stage
			UsbDevSetupAck();
			break;

		// get status
		case USB_REQ_GET_STATUS:
			{
				// B0: self powered, B1: remote wakeup enabled
				u16 status = (UsbDevSelfPowered ? B0 : 0) | (UsbDevWakeupEn ? B1 : 0);

				// send status
				UsbDevSetupStart(&status, 2);
			}
			break;

		// unsupported request
		default:
			return False;
		}
		break;

	// recipient is interface
	case USB_REQ_RCPT_INTERFACE:
		{
			// prepare interface index
			u8 itf = (u8)setup->index;
			if (itf >= USB_ENDPOINT_NUM) return False;

			// driver address (with remapping interface to driver)
			itf = UsbDevItf2Drv[itf];
			if (itf >= UsbDevDrvNum) return False;
			const sUsbDevDrv* drv = &UsbDevDrv[itf];

			// process class control (GET HID REPORT DESCRIPTOR, SET_INTERFACE, GET_INTERFACE)
			if (!UsbDevClassCtrl(drv))
			{
				// driver does not support this, continue only for standard type
				if (type != USB_REQ_TYPE_STANDARD) return False;

				// GET interface
				if (setup->request == USB_REQ_GET_INTERFACE)
				{
					// clear complete callback
					UsbDevSetupCompCB = NULL;

					// response even if class driver doesn't use alternate settings
					u8 alt = 0;
					UsbDevSetupStart(&alt, 1);
				}

				// SET interface
				else if (setup->request == USB_REQ_SET_INTERFACE)
				{
					// clear complete callback
					UsbDevSetupCompCB = NULL;

					// acknowledging at Status Stage
					UsbDevSetupAck();
				}
				else
					return False;
			}
		}
		break;

	// recipient is endpoint
	case USB_REQ_RCPT_ENDPOINT:
		{
			// prepare endpoint address
			u8 ep_addr = (u8)setup->index;
			u8 epinx = USB_EPADDR_EPINX(ep_addr);

			// get driver for this endpoint
			u8 drv_inx = UsbDevEp2Drv[epinx];
			const sUsbDevDrv* drv = NULL;
			if (drv_inx < UsbDevDrvNum) drv = &UsbDevDrv[drv_inx];

			// non-standard request, forward class request
			if (type != USB_REQ_TYPE_STANDARD)
			{
				if (drv == NULL) return False; // invalid driver
				return UsbDevClassCtrl(drv); // invoke class control
			}

			// handle standard request
			switch (setup->request)
			{
			// get STALL status
			case USB_REQ_GET_STATUS:
				{
					// get stalled status
					u16 status = UsbEndpoints[epinx].stalled ? 1 : 0;

					// send status
					UsbDevSetupStart(&status, 2);
				}
				break;

			// clear/set STALL status
			case USB_REQ_CLEAR_FEATURE:
			case USB_REQ_SET_FEATURE:

				// halt endpoint feature -> clear/set stall status
				if (setup->value == USB_REQ_FEATURE_EDPT_HALT)
				{
					if (setup->request == USB_REQ_CLEAR_FEATURE)
						UsbDevClrStall(epinx); // clear stall
					else
						UsbDevSetStall(epinx); // set stall
				}

				// invoke class control request to clear buffers
				if (drv != NULL)
				{
					UsbDevClassCtrl(drv);
					UsbDevSetupCompCB = NULL;
				}

				// acknowledging at Status Stage (skip if driver already did that)
				if (!UsbEndpoints[epinx].active) UsbDevSetupAck();
				break;

			// unsupported request
			default:
				return False;
			}
		}
		break;

	// invalid recipient
	default:
		return False; // stall transfer
	}

	return True;
}

// ----------------------------------------------------------------------------
//                            Transfer complete
// ----------------------------------------------------------------------------

// queue next transaction in Data Stage
// - In some special cases data to be sent can be in UsbDevSetupBuff buffer (ASCII text descriptor).
void UsbDevSetupNext()
{
	// prepare length of data
	u16 len = UsbSetupDataLen - UsbSetupDataXfer; // remaining data
	if (len > USB_PACKET_MAX) len = USB_PACKET_MAX; // limit length of data to max. packet size

	// prepare endpoint index (IN or OUT direction)
	u8 epinx = USB_EPINX(0, USB_DIR_OUT); // endpoint 0, direction OUT (receive data from host)
	if ((UsbSetupRequest.type & B7) != 0) // direction IN ? (send data to host)
	{
		// IN: prepare data to send to host
		epinx = USB_EPINX(0, USB_DIR_IN); // endpoint0, direction IN (send data to host)
		if ((len != 0) && (UsbDevSetupBuff != UsbSetupDataBuff))
			UsbMemcpy(UsbDevSetupBuff, UsbSetupDataBuff, len); // prepare data to send to host
	}

	// start transfer
	UsbXferStart(epinx, (len != 0) ? UsbDevSetupBuff : NULL, len, False);
}

// Process SETUP transfer complete (DATA or STATUS stage)
//  epinx ... endpoint index 0 or 1
//  xres ... transfer result USB_XRES_*
//  len ... transferred bytes
void UsbDevSetupComp(u8 epinx, u8 xres, u16 len)
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// get transfer type
	u8 type = setup->type;

// ---- STATUS stage

	// if endpoint address is opposite to direction bit, this is STATUS stage complete event
	if (USB_DIR(epinx) != USB_EPADDR_DIR(type))
	{
		// endpoint 0 status complete, set device address if needed
		if ((xres == USB_XRES_OK) && // if result is OK
		    ((type & 0x7f) == 0) &&  // recipient 0=device, request type 0=standard
		    (setup->request == USB_REQ_SET_ADDRESS)) // request to set device address
		{
			u8 addr = (u8)setup->value; // get new required address
			UsbDevAddress = addr; // save new device address
			*USB_ADDR_ENDP = addr; // set new device address
		}

		// this is total end of control transfer
		return;
	}

// ---- DATA stage

	// OUT data from host to device, load data to buffer
	if ((type & B7) == 0)
	{
		if (UsbSetupDataBuff != NULL) UsbMemcpy(UsbSetupDataBuff, UsbDevSetupBuff, len);
	}

	// shift data
	UsbSetupDataXfer += len;
	UsbSetupDataBuff += len;

	// Data Stage is complete when all request's length are transferred or
	//  a short packet is sent including zero-length packet
	if ((setup->length == UsbSetupDataXfer) || (len < USB_PACKET_MAX))
	{
		// data stage is complete
		Bool ok = True;

		// invoke control complete callback in DATA stage
		// callback can still stall control in status phase (e.g. out data does not make sense)
		if (UsbDevSetupCompCB != NULL) ok = UsbDevSetupCompCB(USB_STAGE_DATA);

		// go to Status Stage if all is OK
		if (ok)
		{
			// acknowledging at Status Stage
			UsbDevSetupAck();
		}

		// error: stall both IN and OUT control endpoints
		else
		{
			UsbDevSetStall(1);
			UsbDevSetStall(0);
		}
	}

	// more data to transfer
	else
	{
		// transfer next part of data stage
		UsbDevSetupNext();
	}
}

// Process "transfer complete" event after transfer all buffers
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  xres ... transfer result USB_XRES_*
void UsbDevComp(u8 epinx, u8 xres)
{
	// get length of transferred data
	u16 len = UsbEndpoints[epinx].xfer_len;

	// clear active flag
	UsbEndpoints[epinx].active = False;

	// complete SETUP transfer (endpoint 0 IN/OUT)
	if (epinx < 2)
	{
		// clear claimed flag
		UsbEndpoints[epinx].claimed = False;

		// process SETUP transfer complete
		UsbDevSetupComp(epinx, xres, len);
	}

	// complete data transfer (endpoint 1..15)
	else
	{
		// get driver for this endpoint
		u8 drv_inx = UsbDevEp2Drv[epinx];
		if (drv_inx < UsbDevDrvNum)
		{
			// process transfer complete
			const sUsbDevDrv* drv = &UsbDevDrv[drv_inx];
			drv->comp(epinx, xres, len);
		}
	}
}

// ----------------------------------------------------------------------------
//                            Interrupt service
// ----------------------------------------------------------------------------

// resume device (used internally from UsbDevIrq)
void UsbDevResume()
{
	// if suspended
	if (UsbDevSuspended)
	{
		// device is not suspended
		UsbDevSuspended = False;

		// resume callback
		if (UsbDevSetupDesc->resume_cb != NULL) UsbDevSetupDesc->resume_cb();
	}
}

// USB device IRQ handler
void UsbDevIrq()
{
	// schedule drivers
	if ((*USB_INTF & USB_RESCHEDULE_DRV) != 0)
	{
		// clear request to schedule drivers
		RegClr(USB_INTF, USB_RESCHEDULE_DRV);

		// schedule drivers
		u8 drv_id;
		const sUsbDevDrv* drv = UsbDevDrv;
		for (drv_id = 0; drv_id < UsbDevDrvNum; drv_id++)
		{
			if (drv->sched != NULL) drv->sched();
			drv++;
		}

		// data memory barrier to isolate status register
		dmb();
	}

	// get interrupt status
	u32 status = *USB_INTS;

	// buffer status (transfer has been completed)
	if ((status & B4) != 0) // BUFF_STATUS
	{
		u32 buf = *USB_BUFF_STATUS;	// get buffer status
		u32 mask = 1;
		u8 epinx;
		for (epinx = 0; epinx < USB_ENDPOINT_NUM2; epinx++) // index 
		{
			// check one buffer
			if ((buf & mask) != 0)
			{
				// reset buffer bit
				RegClr(USB_BUFF_STATUS, mask);

				// resume device
				UsbDevResume();

				// continue transfer (returns True if transfer is complete)
				if (UsbXferCont(epinx))
				{
					// process "transfer complete" event
					UsbDevComp(epinx, USB_XRES_OK);
				}
			}

			// shift mask
			mask <<= 1;
		}
	}

	// bus reset
	if ((status & B12) != 0) // BUS_RESET
	{
		// reset BUS_RESET status flag
		RegClr(USB_SIE_STATUS, B19);

		// reset device assigned address
		*USB_ADDR_ENDP = 0;

		// reset all non-control endpoints
		UsbDevEpReset();

		// reset USB device drivers (resets drivers and clears base variables, resets SETUP data)
		UsbDevReset();
	}

	// suspend (within 7 ms, device must draw current less than 2.5 mA from bus)
	// Suspend signal can be invoked after the device is connected before host starts sending SOF.
	if ((status & B14) != 0) // DEV_SUSPEND
	{
		// reset status flag
		RegClr(USB_SIE_STATUS, B4);

		// if not suspended
		// Must be connected - suspend signal may be raised after connecting cable, before host starts sending SOF.
		if (!UsbDevSuspended && UsbDevConnected)
		{
			// device is suspended
			UsbDevSuspended = True;

			// suspend callback
			if (UsbDevSetupDesc->suspend_cb != NULL) UsbDevSetupDesc->suspend_cb();
		}
	}

	// resume
	if ((status & B15) != 0) // DEV_RESUME_FROM_HOST
	{
		// reset status flag
		RegClr(USB_SIE_STATUS, B11);

		// resume device
		if (UsbDevConnected) UsbDevResume();
	}

	// setup packet request
	if ((status & B16) != 0) // SETUP_REQ
	{
		// save setup packet
		UsbMemcpy(&UsbSetupRequest, USB_SETUP_PKT, USB_SETUP_PKT_SIZE);
		dmb(); // data memory barrier

		// reset status flag (needs to be reset later after content of SETUP packet is copied)
		RegClr(USB_SIE_STATUS, B17);

		// resume device
		UsbDevResume();

		// clear setup buffer
		UsbSetupDataBuff = NULL;
		UsbSetupDataLen = 0;
		UsbSetupDataXfer = 0;

		// reset endpoint 0 (IN and OUT)
		UsbEndpoints[0].next_pid = 1;
		UsbEndpoints[1].next_pid = 1;
		UsbEndpoints[0].active = False;
		UsbEndpoints[1].active = False;
		UsbEndpoints[0].claimed = False;
		UsbEndpoints[1].claimed = False;

		// device is connected
		UsbDevConnected = True;

		// process SETUP control request (start of SETUP process; returns False = stall control endpoint)
		if (!UsbDevSetupReq())
		{
			// failed - stall both control endpoints IN and OUT
			UsbDevSetStall(0);
			UsbDevSetStall(1);
		}
	}

	// SOF sent
	if ((status & B17) != 0) // SOF
	{
		// read SOF (to clear SOF request)
		u16 sof = UsbGetSof();

		// resume device
		UsbDevResume();

		// send SOF event
		if (UsbProcessSOFEvent)
		{
			int i;
			for (i = 0; i < UsbDevDrvNum; i++) if (UsbDevDrv[i].sof != NULL) UsbDevDrv[i].sof(sof);
		}

		// disable SOF interrupt, if used to wakeup from host and not used by drivers
		UsbDevSofDisable();
	}
}

// ----------------------------------------------------------------------------
//                                Initialize
// ----------------------------------------------------------------------------

// reset all non-control endpoints
void UsbDevEpReset()
{
	// disable all non-control endpoint control registers
	int i;
	for (i = 2; i < USB_ENDPOINT_NUM2; i++) *USB_EP_CTRL(i) = 0;

	// clear all non-control endpoint descriptors
	memset((void*)&UsbEndpoints[2], 0, sizeof(UsbEndpoints) - 2*sizeof(sEndpoint));

	// reset memory map
	UsbRamMap = 0x3full;
}

// reset configuration of USB device drivers (resets drivers and clears base variables, leaves SETUP data untouched)
void UsbDevCfgReset()
{
	// reset drivers
	int i;
	for (i = 0; i < UsbDevDrvNum; i++) UsbDevDrv[i].reset();

	// reset mapping endpoint to device driver
	for (i = 0; i < USB_ENDPOINT_NUM2; i++) UsbDevEp2Drv[i] = USB_DRVID_INVALID;

	// reset mapping interface number to device driver
	for (i = 0; i < USB_ENDPOINT_NUM; i++) UsbDevItf2Drv[i] = USB_DRVID_INVALID;

	// reset base variables
	UsbDevCfgNum = 0;		// current active configuration (0 = not configured)
	UsbDevAddress = 0;		// no address assigned
	UsbDevConnected = False;	// device is connected
	UsbDevSuspended = False;	// device is suspended
	UsbDevSelfPowered = 0;		// device is self powered
}

// reset SETUP data
void UsbDevSetupReset()
{
	memset(&UsbSetupRequest, 0, sizeof(UsbSetupRequest));
	UsbSetupDataBuff = NULL;
	UsbSetupDataLen = 0;
	UsbSetupDataXfer = 0;
	UsbDevSetupCompCB = NULL;
}

// reset USB device drivers (resets drivers and clears base variables, resets SETUP data)
void UsbDevReset()
{
	// reset configuration of USB device drivers (resets drivers and clears base variables)
	UsbDevCfgReset();

	// reset SETUP data
	UsbDevSetupReset();
}

// initialize device endpoint
//  epinx ... endpoint index 0..31
//  pktmax ... max. packet size
//  xfer ... transfer type USB_XFER_*
void UsbDevEpInit(u8 epinx, u16 pktmax, u8 xfer)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// base properties
	sep->used = True;	// endpoint is used
	sep->active = False;	// transfer is not active
	sep->claimed = False;	// unclaim endpoint
	sep->stalled = False;	// not stalled
	sep->rx = (USB_DIR(epinx) == USB_DIR_OUT); // direction from host to device
	sep->xfer = xfer;	// transfer type
	sep->pktmax = pktmax;	// max. packet size
	sep->next_pid = 0;	// next packet identification PID

	// prepare buffer control register
	sep->buf_ctrl = USB_EP_BUF(epinx);

	// clear buffer control
	*sep->buf_ctrl = 0;

	// pointer to data buffer ... endpoint 0 has fixed address
	if (epinx < 2)
	{
		// fixed address of buffer of endpoint 0 (common to IN and OUT)
		sep->data_buf = USB_EP0_BUF0;

		// endpoint 0 has no control register, but we can use USB_SIE_CTRL for double-buffer control
		sep->ep_ctrl = USB_SIE_CTRL;

		// limit buffer size
		if (pktmax > USB_PACKET_MAX) sep->pktmax = USB_PACKET_MAX;
	}
	else
	{
		// allocate data buffer (for bulk transfer we use double-buffer)
		u8* addr = UsbRamAlloc((xfer == USB_XFER_BULK) ? (pktmax*2) : pktmax);
		sep->data_buf = addr; // set buffer address

		// prepare endpoint control register
		sep->ep_ctrl = USB_EP_CTRL(epinx);

		// setup control register (single buffer mode)
		*sep->ep_ctrl = 
			(addr - USB_RAM) |		// offset of data buffer in DPRAM
			(xfer << 26) |			// B26..B27: transfer type (endpoint type)
			B29 |				// enable interrupt for every transferred single-buffer
			B31;				// B31: endpoint enable
	}
}

// initialize device endpoint by endpoint descriptor
//   desc ... endpoint request descriptor
void UsbDevEpOpen(const sUsbDescEp* desc)
{
	UsbDevEpInit(USB_EPADDR_EPINX(desc->ep_addr), desc->pktmax & 0x7ff, desc->attr & 3);
}

// parse endpoint descriptors as IN/OUT pair (returns False to stall)
//  p_desc ... pointer to endpoint desciptors
//  ep_count ... number of endpoints (1 or 2)
//  epinx_out ... pointer to endpoint index OUT
//  epinx_in ... pointer to endpoint index IN
Bool UsbDevOpenEpPair(const u8* p_desc, u8 ep_count, u8* epinx_out, u8* epinx_in)
{
	int i;
	for (i = 0; i < ep_count; i++)
	{
		// endpoint descriptor
		const sUsbDescEp* desc_ep = (const sUsbDescEp*)p_desc;

		// check endpoint descriptor type and transfer type
		if ((desc_ep->type != USB_DESC_ENDPOINT) || ((desc_ep->attr & 3) == USB_XFER_CTRL)) return False;

		// initialize endpoint by the descriptor
		UsbDevEpOpen(desc_ep);

		// save endpoint index
		u8 epinx = USB_EPADDR_EPINX(desc_ep->ep_addr);
		if ((desc_ep->ep_addr & B7) != 0) // direction IN
			*epinx_in = epinx;
		else
			*epinx_out = epinx;

		// shift to next descriptor
		p_desc = USB_DESC_NEXT(p_desc);
	}

	return True;
}

// USB init in device mode
//  desc ... application device setup descriptor
void UsbDevInit(const sUsbDevSetupDesc* desc)
{
	// select device mode
	UsbHostMode = False;

	// save pointer to descriptor
	UsbDevSetupDesc = desc;

	// base USB init
	UsbPhyInit();

	// set IRQ handler
	SetHandler(IRQ_USBCTRL, UsbDevIrq);

	// initialize endpoint 0 (used for SETUP)
	UsbDevEpInit(0, USB_PACKET_MAX, USB_XFER_CTRL);
	UsbDevEpInit(1, USB_PACKET_MAX, USB_XFER_CTRL);

	// initialize USB peripheral for device mode
	*USB_MAIN_CTRL = B0;	// enable controller, use device mode

	// set BUFF_STATUS bit for every buffer EP0 completed, EP0 single buffered
	*USB_SIE_CTRL = B29;

	// check if some driver uses SOF function
	UsbProcessSOFEvent = False;
	int i;
	for (i = 0; i < UsbDevDrvNum; i++) if (UsbDevDrv[i].sof != NULL) UsbProcessSOFEvent = True;

	// reset device data
	UsbDevReset();

	// enable interrupt on: B4 buffer status, B12 bus reset,
	//    B14 suspend, B15 resume, B16 setup request, B17 SOF received
	u32 n = B4 | B12 | B14 | B15 | B16;
	if (UsbProcessSOFEvent) n |= B17;
	*USB_INTE = n;

	// connect to USB bus
	UsbDevConnect();

	// initialize class drivers
	for (i = 0; i < UsbDevDrvNum; i++) UsbDevDrv[i].init();

	// interrupt enable
	UsbIntEnable();

	// device mode is initialized
	UsbDevIsInit = True;
}

#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
