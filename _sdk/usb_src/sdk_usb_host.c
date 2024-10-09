
// ****************************************************************************
//
//                          USB Host Class Driver
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
#if USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)

#include "../inc/sdk_irq.h"
#include "../inc/sdk_timer.h"
#include "../usb_inc/sdk_usb_host.h"
#include "../usb_inc/sdk_usb_host_cdc.h"	// USB Communication Device Class (host)
#include "../usb_inc/sdk_usb_host_hid.h"	// USB Human Interface Device Class (host)
//#include "../usb_inc/sdk_usb_host_msc.h"	// USB Mass Storage Class (host)
//#include "../usb_inc/sdk_usb_host_vendor.h"	// USB Vendor Device Class (host)
//#include "../usb_inc/sdk_usb_setuph.h"

// setup buffer for host
ALIGNED u8 UsbHostSetupBuff[USB_HOSTSETUPBUFF_MAX];

// host device descriptors (index corresponds to the device address, including address 0)
sUsbHostDev UsbHostDev[USE_USB_HOST_DEVNUM];

// host SETUP complete callback (returns False if stall; NULL = none)
//  dev_addr ... device address
//  xres ... transfer result USB_XRES_*
// Used internally (if OK, UsbSetupDataXfer contains length of data, UsbSetupRequest contains request packet)
pUsbHostSetupCompCB UsbHostSetupCompCB;

// setup stage
u8 UsbHostSetupStage; // setup stage USB_STAGE_*

// enumeration
volatile u8 UsbHostEnumState = USB_HOST_ENUM_IDLE; // current state of device enumeration (0 = not enumerating)
volatile u8 UsbHostEnumTry; // counter of attempts to enumerate
u8 UsbHostEnumAddr; // address of enumerated device
volatile u32 UsbHostEnumNext; // time of next enumeration step, in [us]

// host class drivers
const sUsbHostDrv UsbHostDrv[] = {

#if USE_USB_HOST_CDC	// use USB CDC Communication Device Class, value = number of interfaces (host)
	{
		UsbHostCdcInit,		// initialize class driver
		UsbHostCdcTerm,		// terminate class driver
		UsbHostCdcOpen,		// open device class interface
		UsbHostCdcCfg,		// configure interface
		UsbHostCdcComp,		// transfer complete callback
		UsbHostCdcClose,	// close device
		UsbHostCdcSof,		// sending SOF (start of frame; NULL=not used)
		NULL, 			// schedule driver (NULL=not used, raised with UsbRescheduleDrv())
	},
#endif

/*
#if USE_USB_HOST_MSC	// use USB MSC Mass Storage Class (host)
	{
		MschInit,	// initialize class driver
		MschOpen,	// open device class interface
		MschSetConfig,	// set config interface
		MschXferComp,	// transfer complete callback
		MschClose,	// close device interface
	},
#endif
*/

#if USE_USB_HOST_HID	// use USB HID Human Interface Device, value = number of interfaces (host)
	{
		UsbHostHidInit,		// initialize class driver
		UsbHostHidTerm,		// terminate class driver
		UsbHostHidOpen,		// open device class interface
		UsbHostHidCfg,		// configure interface
		UsbHostHidComp,		// transfer complete callback
		UsbHostHidClose,	// close device
		UsbHostHidSof,		// sending SOF (start of frame; NULL=not used)
		UsbHostHidSched,	// schedule driver (NULL=not used, raised with UsbRescheduleDrv())
	},
#endif

/*
#if USE_USB_HOST_HUB	// use USB HUB (host)
	{
		HubhInit,	// initialize class driver
		HubhOpen,	// open device class interface
		HubhSetConfig,	// set config interface
		HubhXferComp,	// transfer complete callback
		HubhClose,	// close device interface
	},
#endif

#if USE_USB_HOST_VENDOR	// use USB VENDOR Vendor specific device (host)
	{
		VendorhInit,	// initialize class driver
		VendorhOpen,	// open device class interface
		VendorhSetConfig, // set config interface
		VendorhXferComp, // transfer complete callback
		VendorhClose,	// close device interface
	},
#endif
*/
};

// number of host drivers
const u8 UsbHostDrvNum = count_of(UsbHostDrv);

// ----------------------------------------------------------------------------
//                            Utilities
// ----------------------------------------------------------------------------

// enable SOF interrupt
void UsbHostSofEnable()
{
	RegSet(USB_INTE, B2);
}

// disable SOF interrupt (if not used by drivers)
void UsbHostSofDisable()
{
	if (!UsbProcessSOFEvent) RegClr(USB_INTE, B2);
}

// get device structure
//  dev_addr ... device address
// Returns dev0 if the address is invalid, as a fallback treatment,
// since in most cases the address is already checked. To check
// the validity of the address, use UsbHostCheckAddr function.
sUsbHostDev* UsbHostGetDev(u8 dev_addr)
{
	if (!UsbHostCheckAddr(dev_addr)) dev_addr = 0;
	return &UsbHostDev[dev_addr];
}

// get endpoint from device address and device endpoint index
// (returns USB_DRVID_INVALID = not found; tries opposite direction as well)
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
u8 UsbHostDevEp(u8 dev_addr, u8 dev_epinx)
{
	// endpoint 0
	if (dev_epinx <= 1) return 0;

	// search device
	sEndpoint* sep = &UsbEndpoints[1];
	u8 ep;
	for (ep = 1; ep < USB_ENDPOINT_NUM; ep++)
	{
		if (sep->used && (dev_addr == sep->dev_addr) && (dev_epinx == sep->dev_epinx)) return ep;
		sep++;
	}

	// not found, try opposite direction
	dev_epinx ^= 1;
	sep = &UsbEndpoints[1];
	for (ep = 1; ep < USB_ENDPOINT_NUM; ep++)
	{
		if (sep->used && (dev_addr == sep->dev_addr) && (dev_epinx == sep->dev_epinx)) return ep;
		sep++;
	}

	// not found
	return USB_DRVID_INVALID;
}

// check if transfer is busy
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
Bool UsbHostIsBusy(u8 dev_addr, u8 dev_epinx)
{
	// get endpoint
	u8 ep = UsbHostDevEp(dev_addr, dev_epinx);

	// this device endpoint is not transferred
	if (ep == USB_DRVID_INVALID) return False;

	// check if endpoint is active (busy)
	return UsbEpIsBusy(ep);
}

// claim host endpoint by class driver (returns False if cannot be claimed - already claimed or still active)
//  dev_addr ... device address
//  dev_epainx ... device endpoint index 0..31
Bool UsbHostEpClaim(u8 dev_addr, u8 dev_epinx)
{
	// check if device address is valid
	if (!UsbHostCheckAddr(dev_addr)) return False;

	// get device structure
	sUsbHostDev* dev = &UsbHostDev[dev_addr];

	// device must be connected
	if ((dev->flag & USB_HOST_FLAG_CONN) == 0) return False;

	// get endpoint
	u8 ep = UsbHostDevEp(dev_addr, dev_epinx);

	// invalid endpoint
	if (ep == USB_DRVID_INVALID) return False;

	// claim endpoint
	return UsbEpClaim(ep);
}

// unclaim host endpoint by class driver
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
void UsbHostEpUnclaim(u8 dev_addr, u8 dev_epinx)
{
	// check if device address is valid
	if (UsbHostCheckAddr(dev_addr))
	{
		// get endpoint
		u8 ep = UsbHostDevEp(dev_addr, dev_epinx);

		// invalid endpoint
		if (ep == USB_DRVID_INVALID) return;

		// unclaim endpoint
		UsbEpUnclaim(ep);
	}
}

// ----------------------------------------------------------------------------
//                            Transfer complete
// ----------------------------------------------------------------------------

// invoke SETUP transfer complete callback and idle
// If OK, UsbSetupDataXfer contains length of data, UsbSetupRequest contains request packet.
//  dev_addr ... device address
//  xres ... transfer result USB_XRES_*
void UsbHostSetupIdle(u8 dev_addr, u8 xres)
{
	UsbHostSetupStage = USB_STAGE_IDLE;
	if (UsbHostSetupCompCB != NULL) UsbHostSetupCompCB(dev_addr, xres);
}

// process SETUP transfer complete on endpoint 0
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
//  xres ... transfer result USB_XRES_*
//  len ... length of transferred data
void UsbHostSetupComp(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len)
{
	// get setup packet with request
	const sUsbSetupPkt* setup = &UsbSetupRequest;
	
	// not success
	if (xres != USB_XRES_OK)
	{
		// terminate transfer if any stage failed
		UsbHostSetupIdle(dev_addr, xres);
	}

	// transfer OK
	else
	{
		// check transfer stage
		switch(UsbHostSetupStage)
		{
		// setup stage
		case USB_STAGE_SETUP:

			// check if any data in data stage is required
			if (setup->length > 0)
			{
				// go to data stage
				UsbHostSetupStage = USB_STAGE_DATA;

				// prepare required direction on endpoint 0
				u8 dev_epinx = USB_EPINX(0, USB_EPADDR_DIR(setup->type));

				// transfer data
				UsbHostXferStart(dev_addr, dev_epinx, UsbSetupDataBuff, setup->length, False);
				break;
			}

			// if setup->length == 0, continue to DATA stage case

		// data stage
		case USB_STAGE_DATA:
			{
				// store data length
				UsbSetupDataXfer = len;

				// go to acknowledge stage
				UsbHostSetupStage = USB_STAGE_ACK;

				// prepare opposite direction on endpoint 0
				u8 dev_epinx = USB_EPINX(0, USB_EPADDR_DIR(setup->type) ^ 1);

				// transfer zero length packet ZLP to mark end of transmission (endpoint number is 0, oposite direction)
				// Note: status packet is always DATA1 - this is ensured by UsbHostEpInit after changing direction.
				UsbHostXferStart(dev_addr, dev_epinx, NULL, 0, False);
			}
			break;

		// acknowledge stage
		case USB_STAGE_ACK:

			// invoke SETUP transfer complete callback and idle
			UsbHostSetupIdle(dev_addr, xres);
			break;
		}
	}
}

// proces "transfer completed" event after transfer all buffers
//  ep ... endpoint 0..15
//  xres ... transfer result USB_XRES_*
void UsbHostComp(u8 ep, u8 xres)
{
	// get descriptor
	sEndpoint* sep = &UsbEndpoints[ep];

	// device address
	u8 dev_addr = sep->dev_addr;

	// get device structure
	sUsbHostDev* dev = UsbHostGetDev(dev_addr);

	// device endpoint index
	u8 dev_epinx = sep->dev_epinx;

	// length of transferred data
	u16 len = sep->xfer_len;

	// clear active flag
	sep->active = False;

	// complete SETUP transfer (endpoint 0 IN/OUT)
	if (dev_epinx < 2)
	{
		// clear claimed flag
		sep->claimed = False;

		// process SETUP transfer complete
		UsbHostSetupComp(dev_addr, dev_epinx, xres, len);
	}

	// complete data transfer (endpoint 1..15)
	else
	{
		// get driver for this endpoint
		u8 drv_inx = dev->ep2drv[dev_epinx];
		if (drv_inx < UsbHostDrvNum) // check if driver is valid
		{
			// call transfer complete callback
			const sUsbHostDrv* drv = &UsbHostDrv[drv_inx];
			drv->comp(dev_addr, dev_epinx, xres, len);
		}
	}
}

// ----------------------------------------------------------------------------
//                            Enumeration
// ----------------------------------------------------------------------------

// parse configuration descriptor (returns False on error)
//  dev_addr ... device address
Bool UsbHostParseCfg(u8 dev_addr, const u8* p_desc)
{
	// get device structure
	sUsbHostDev* dev = &UsbHostDev[dev_addr];

	// end of descriptors
	const u8* desc_end = p_desc + ((const sUsbDescCfg*)p_desc)->totallen;

	// skip configuration descriptor
	p_desc = USB_DESC_NEXT(p_desc);

	// parse interface descriptors
	while (p_desc < desc_end)
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

		// some CDC devices does not use interface association, need to set 2 interfaces manually
		if ((assoc == 1) && (desc_itf->itf_class == USB_CLASS_CDC) && (desc_itf->itf_subclass == 2)) assoc = 2;

		// search support of this interface in class device drivers
		u16 rem_len = desc_end - p_desc; // remaining length
		u8 drv_id;
		for (drv_id = 0; drv_id < UsbHostDrvNum; drv_id++)
		{
			// pointer to class driver
			const sUsbHostDrv* drv = &UsbHostDrv[drv_id];

			// open device class interface (returns size of used interface, 0=not supported)
			u16 desc_len = drv->open(dev_addr, desc_itf, rem_len);

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
					dev->itf2drv[n] = drv_id;
				}

			        // bind all endpoints to found driver
				UsbEpBindDrv(dev->ep2drv, p_desc, desc_len, drv_id);

				// shift interface pointer
				p_desc += desc_len;
				break;
			}
		}

		// class is not supported, skip this interface descriptor and its endpoint descriptors
		if (drv_id >= UsbHostDrvNum)
		{
			do { p_desc = USB_DESC_NEXT(p_desc); } while (USB_DESC_TYPE(p_desc) == USB_DESC_ENDPOINT);
		}
	}

	return True;
}

// stop unfinished enumeration
void UsbHostEnumStop()
{
	// stop current enumeration
	if (UsbHostEnumState != USB_HOST_ENUM_IDLE)
	{
		// device is addressed, close it
		if (UsbHostEnumState >= USB_HOST_ENUM_SET_ADDRW)
		{
			u8 dev_addr = UsbHostEnumAddr; // address of enumerated device
			UsbHostDevClose(dev_addr); // close all open endpoints belonging to the device
			sUsbHostDev* dev = &UsbHostDev[dev_addr]; // device structure
			dev->flag = 0;		// not connected
		}

		// stop enumeration
		UsbHostSetupStage = USB_STAGE_IDLE;	// no setup stage
		UsbHostEnumState = USB_HOST_ENUM_IDLE;	// not enumerating
		UsbHostSetupCompCB = NULL;		// no callback
	}
}

// get descriptor from device
//  dev_addr ... device address
//  desc_type ... descriptor type
//  inx ... descriptor index
//  lang ... language ID
//  buf ... data buffer to receive descriptor
//  len ... length of data
//  cb ... callback
void UsbHostGetDesc(u8 dev_addr, u8 desc_type, u8 inx, u16 lang, void* buf, u16 len, pUsbHostSetupCompCB cb)
{
	// pointer to setup packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare setup request
	setup->type =	B7 |				// direction: IN
			(USB_REQ_TYPE_STANDARD << 5) |	// request type: standard
			USB_REQ_RCPT_DEVICE;		// recipient: device
	setup->request = USB_REQ_GET_DESCRIPTOR;	// request: get descriptor
	setup->value = ((u16)desc_type << 8) | inx;	// value: descriptor type and index
	setup->index = lang;				// index: language ID
	setup->length = len;				// length of data

	// start control transfer with callback
	UsbHostCtrlXfer(dev_addr, buf, cb);
}

// set/continue config complete
//  dev_addr ... device address
//  itf_num ... current interface number (Completion continues on the following interface, with 0xff on start)
// After enumeration process, the process of setting device configuration continues,
// starting with this function with interface = -1. The function finds a valid driver
// and calls its configuration setup function. The driver can configure the device
// using SETUP packets. When the configuration is complete, the driver calls this
// function again, passing its highest interface number. The function will continue
// by configuring next driver. After all drivers have been configured, the device
// is marked as mounted.
void UsbHostCfgComp(u8 dev_addr, u8 itf_num)
{
	// continue to next interface
	itf_num++;

	// get device structure
	sUsbHostDev* dev = &UsbHostDev[dev_addr];

	// find next valid interface
	for (; itf_num < USB_ENDPOINT_NUM; itf_num++)
	{
		// get driver for this interface
		u8 drv_id = dev->itf2drv[itf_num];

		// check if this driver is valid
		if (drv_id < UsbHostDrvNum)
		{
			// get driver
			const sUsbHostDrv* drv = &UsbHostDrv[drv_id];

			// set config of this driver
			// - this function UsbHostCfgComp will be called from within
			//   drv->cfg to continue configuration of next driver.
			if (drv->cfg != NULL)
			{
				drv->cfg(dev_addr, itf_num);
				break;
			}
		}
	}

	// all interfaces are configured - stop enumeration, device is mounted
	if (itf_num >= USB_ENDPOINT_NUM)
	{
		// stop enumeration
		UsbHostEnumState = USB_HOST_ENUM_IDLE;

		// device is mounted ("some device", it does not say which device)
		UsbDevCfgNum = 1;

		// report change state to the hub
#if USE_USB_HOST_HUB	// use USB HUB (host)
		// if (UsbHostDev[0].hub_addr > 0) UsbHostHubSetStatus(UsbHostDev[0].hub_addr); // @TODO ... hub_edpt_status_xfer
#endif
	}
}

// process enumeration
//  dev_addr ... device address
//  xres ... transfer result USB_XRES_*
void UsbHostEnum(u8 dev_addr, u8 xres)
{
	// idle
	if (UsbHostEnumState == USB_HOST_ENUM_IDLE) return;

	// check if device is still connected (do not check during reset signal)
	if ((UsbGetDevSpeed() == USB_SPEED_OFF) && (UsbHostEnumState >= USB_HOST_ENUM_START))
	{
		// not connected, stop enumeration
		UsbHostEnumStop();

		// report change state to the hub
#if USE_USB_HOST_HUB	// use USB HUB (host)
		// if (UsbHostDev[0].hub_addr > 0) UsbHostHubSetStatus(UsbHostDev[0].hub_addr); // @TODO ... hub_edpt_status_xfer
#endif
		return;
	}

	// check result of last transfer
	if (xres != USB_XRES_OK)
	{
		// next try
		if (UsbHostEnumTry > 0) UsbHostEnumTry--;
		if (UsbHostEnumTry > 0)
		{
			// time of next enumeration step, 50 ms
			UsbHostEnumNext = Time() + 50000;

			// request to start new device enumeration
			dmb(); // data memory barrier
			UsbHostEnumState = USB_HOST_ENUM_RESET_START;
			return;
		}
		else
		{
			// error, stop enumeration
			UsbHostEnumStop();

			// report change state to the hub
#if USE_USB_HOST_HUB	// use USB HUB (host)
			// if (UsbHostDev[0].hub_addr > 0) UsbHostHubSetStatus(UsbHostDev[0].hub_addr); // @TODO ... hub_edpt_status_xfer
#endif
			return;
		}
	}

	// process enumeration state
	switch (UsbHostEnumState)
	{
	// start reset signal
	case USB_HOST_ENUM_RESET_START:

		// time of next enumeration step, 10 ms (reset signal: both D-/D+ goes to low SE0 for 10 ms)
		UsbHostEnumNext = Time() + 10000;

		// start reset signal
		RegSet(USB_SIE_CTRL, B13);
		break;

	// stop reset signal
	case USB_HOST_ENUM_RESET_STOP:

		// time of next enumeration step, 400 ms
		UsbHostEnumNext = Time() + 400000;

		// stop reset signal
		//RegClr(USB_SIE_CTRL, B13);
		break;

	// start enumeration, get part of device descriptor
	case USB_HOST_ENUM_START:

		// invalidate device address
		UsbHostEnumAddr = 0;

		// initialize host control endpoint 0 to get 8 bytes from device descriptor (to get max packet size)
		UsbHostEp0Open(0, 8);

		// request to get part of device descriptor (8 bytes)
		UsbHostGetDesc(0, USB_DESC_DEVICE, 0, 0, UsbHostSetupBuff, 8, UsbHostEnum);
		break;

	// set address
	case USB_HOST_ENUM_SET_ADDR:
		{
			// pointer to received device descriptor
			sUsbDescDev* desc_dev = (sUsbDescDev*)UsbHostSetupBuff;

			// check if device is hub
			Bool ishub = (desc_dev->dev_class == USB_CLASS_HUB);

			// search new device address (not-connected device)
			u8 dev_addr, end;
			if (ishub)
			{
				dev_addr = USB_HOST_HUB_INX; // base index of hub (= end of devices)
				end = USE_USB_HOST_DEVNUM; // end of devices
			}
			else
			{
				dev_addr = 1; // start index of devices
				end = USB_HOST_HUB_INX; // end of devices (= start of hubs)
			}

			for (; dev_addr < end; dev_addr++)
			{
				// check if device is not connected
				if ((UsbHostDev[dev_addr].flag & USB_HOST_FLAG_CONN) == 0) break;
			}

			// no free device slot, stop enumeration
			if (dev_addr >= end)
			{
				UsbHostEnumStop();
				return;
			}
			UsbHostEnumAddr = dev_addr; // address of enumerated device

			// get device structure
			sUsbHostDev* dev = &UsbHostDev[dev_addr];

			// transfer setup of device to new address
			sUsbHostDev* dev0 = &UsbHostDev[0];	// device 0
			dev->hub_addr = dev0->hub_addr;		// hub address (0 = root)
			dev->hub_port = dev0->hub_port;		// hub port
			dev->hub_speed = dev0->hub_speed;	// device speed
			dev->flag = USB_HOST_FLAG_CONN;		// device is connected
			dev->pktmax = desc_dev->pktmax;		// max. packet size for endpoint 0

			// pointer to setup packet
			sUsbSetupPkt* setup = &UsbSetupRequest;

			// prepare setup request
			setup->type =	0 |				// direction: OUT
					(USB_REQ_TYPE_STANDARD << 5) |	// request type: standard
					USB_REQ_RCPT_DEVICE;		// recipient: device
			setup->request = USB_REQ_SET_ADDRESS;		// request: set address
			setup->value = dev_addr;			// value: new address
			setup->index = 0;				// index: 0
			setup->length = 0;				// length of data: 0

			// start control transfer with callback (device address is = 0)
			UsbHostCtrlXfer(0, NULL, UsbHostEnum);
		}
		break;

	// short delay after set address
	case USB_HOST_ENUM_SET_ADDRW:

		// delay for the device to change its address, 10 ms
		UsbHostEnumNext = Time() + 10000;
		break;

	// get full device descriptor (with new address)
	case USB_HOST_ENUM_DEV_DESC:
		{
			// pointer to setup packet
			//sUsbSetupPkt* setup = &UsbSetupRequest;

			// address of the device
			u8 dev_addr = UsbHostEnumAddr;

			// get device structure
			sUsbHostDev* dev = &UsbHostDev[dev_addr];

			// close all open endpoints belonging to device 0
			UsbHostDevClose(0);

			// open control pipe for new address
			UsbHostEp0Open(dev_addr, dev->pktmax);

			// request to get full device descriptor
			UsbHostGetDesc(dev_addr, USB_DESC_DEVICE, 0, 0, UsbHostSetupBuff, sizeof(sUsbDescDev), UsbHostEnum);
		}
		break;

	// get base configuration descriptor
	case USB_HOST_ENUM_CFG_DESC:
		{
			// pointer to received device descriptor
			sUsbDescDev* desc_dev = (sUsbDescDev*)UsbHostSetupBuff;

			// address of the device
			u8 dev_addr = UsbHostEnumAddr;

			// get device structure
			sUsbHostDev* dev = &UsbHostDev[dev_addr];

			// save device properties
			dev->vendor = desc_dev->vendor;		// vendor ID
			dev->product = desc_dev->product;	// product ID
			dev->i_manufact = desc_dev->i_manufact;	// manufacturer text index
			dev->i_product = desc_dev->i_product;	// product text index
			dev->i_serial = desc_dev->i_serial;	// serial text index

			// get base configuration descriptor, index 0
			UsbHostGetDesc(dev_addr, USB_DESC_CONFIGURATION, 0, 0, UsbHostSetupBuff, sizeof(sUsbDescCfg), UsbHostEnum);
		}
		break;

	// get full configuration
	case USB_HOST_ENUM_CFG_FULL:
		{
			// pointer to received configuration descriptor
			sUsbDescCfg* desc_cfg = (sUsbDescCfg*)UsbHostSetupBuff;

			// get size of full configuration descriptor
			u16 len = desc_cfg->totallen;

			// configuration is too large
			if (len > USB_HOSTSETUPBUFF_MAX)
			{
				// error, stop enumeration
				UsbHostEnumStop();
				return;
			}

			// address of the device
			u8 dev_addr = UsbHostEnumAddr;

			// get full configuration descriptor, index 0
			UsbHostGetDesc(dev_addr, USB_DESC_CONFIGURATION, 0, 0, UsbHostSetupBuff, len, UsbHostEnum);
		}
		break;

	// set config
	case USB_HOST_ENUM_SET_CFG:
		{
			// address of the device
			u8 dev_addr = UsbHostEnumAddr;

			// parse configuration
			if (!UsbHostParseCfg(dev_addr, UsbHostSetupBuff))
			{
				// error, stop enumeration
				UsbHostEnumStop();
				return;
			}

			// get configuration number
			u8 cfg = ((sUsbDescCfg*)UsbHostSetupBuff)->config;

			// pointer to setup packet
			sUsbSetupPkt* setup = &UsbSetupRequest;

			// prepare setup request
			setup->type =	0 |				// direction: OUT
					(USB_REQ_TYPE_STANDARD << 5) |	// request type: standard
					USB_REQ_RCPT_DEVICE;		// recipient: device
			setup->request = USB_REQ_SET_CONFIGURATION;	// request: select configuration
			setup->value = cfg;				// value: configuration number (1-based)
			setup->index = 0;				// index: 0
			setup->length = 0;				// length of data: 0

			// start control transfer with callback
			UsbHostCtrlXfer(dev_addr, NULL, UsbHostEnum);
		}
		break;

	// stop enumeration
	case USB_HOST_ENUM_STOP:
		{
			// address of the device
			u8 dev_addr = UsbHostEnumAddr;

			// get device structure
			sUsbHostDev* dev = &UsbHostDev[dev_addr];

			// mark device as configured
			dev->flag |= USB_HOST_FLAG_CONF;

			// set config complete
			UsbHostCfgComp(dev_addr, (u8)-1);
		}
		return;
	}

	// shift enumeration state to next step
	UsbHostEnumState++;
}

// ----------------------------------------------------------------------------
//                            Interrupt service
// ----------------------------------------------------------------------------

// USB host timer
//  - called from SysTick_Handler every 5 ms
void UsbHostOnTime()
{
	// process enumeration
	if (UsbHostEnumState != USB_HOST_ENUM_IDLE) UsbRescheduleTimer();
}

// remove connected device
//  hub_addr ... hub address, 0 = roothub
//  hub_port ... hub port, 0 = all devices on downstream hub
void UsbHostDevRemove(u8 hub_addr, u8 hub_port)
{
	// search device on this hub
	u8 dev_addr; // device address
	for (dev_addr = 0; dev_addr < USE_USB_HOST_DEVNUM; dev_addr++)
	{
		// get device structure
		sUsbHostDev* dev = &UsbHostDev[dev_addr];

		// check hub
		if (((dev->flag & USB_HOST_FLAG_CONN) != 0) && // device is connected
			(dev->hub_addr == hub_addr) && // check hub address, 0 = roothub
			((hub_port == 0) || (dev->hub_port == hub_port))) // check hub port, 0 = all devices on downstream hub
		{
			// check if this device is hub
			if (dev_addr >= USB_HOST_HUB_INX)
			{
				// Remove all devices of this hub
				UsbHostDevRemove(dev_addr, 0);

				// report change state to the hub
#if USE_USB_HOST_HUB	// use USB HUB (host)
				// UsbHostHubSetStatus(dev_addr); // @TODO ... hub_edpt_status_xfer
#endif
			}

			// close class drivers
			u8 drv_id;
			for (drv_id = 0; drv_id < UsbHostDrvNum; drv_id++)
			{
				// pointer to class driver
				const sUsbHostDrv* drv = &UsbHostDrv[drv_id];

				// close device
				drv->close(dev_addr);
			}

			// close all open endpoints belonging to the device
			UsbHostDevClose(dev_addr);

			// clear device
			UsbHostClrDev(dev_addr);

			// stop enumerating
			if (dev_addr == UsbHostEnumAddr) UsbHostSetupStage = USB_STAGE_IDLE;
		}
	}
}

// USB host IRQ handler
void UsbHostIrq()
{
	// schedule drivers
	if ((*USB_INTF & USB_RESCHEDULE_DRV) != 0)
	{
		// clear request to schedule drivers
		RegClr(USB_INTF, USB_RESCHEDULE_DRV);

		// schedule drivers
		u8 drv_id;
		const sUsbHostDrv* drv = UsbHostDrv;
		for (drv_id = 0; drv_id < UsbHostDrvNum; drv_id++)
		{
			if (drv->sched != NULL) drv->sched();
			drv++;
		}

		// data memory barrier to isolate reading status register
		dmb();
	}

	// schedule timer
	if ((*USB_INTF & USB_RESCHEDULE_TIMER) != 0)
	{
		// clear request to schedule drivers
		RegClr(USB_INTF, USB_RESCHEDULE_TIMER);

		// start/continue enumeration from timer
		u8 state = UsbHostEnumState;
		if ((	((state >= USB_HOST_ENUM_RESET_START) && (state <= USB_HOST_ENUM_START)) ||
			(state == USB_HOST_ENUM_DEV_DESC)) &&
			((s32)(Time() - UsbHostEnumNext) >= 0))
		{
			UsbHostEnum(UsbHostEnumAddr, USB_XRES_OK);
		}

		// data memory barrier to isolate reading status register
		dmb();
	}	

	// get interrupt status
	u32 status = *USB_INTS;

	// device connected/disconnected
	if ((status & B0) != 0)	// HOST_CONN_DIS
	{
		// ignore during reset signal
		if ((UsbHostEnumState > USB_HOST_ENUM_IDLE) && (UsbHostEnumState <= USB_HOST_ENUM_START))
		{
			// clear speed change interrupt SIE_STATUS.SPEED
			RegClr(USB_SIE_STATUS, B8+B9);
		}
		else
		{
			// device is not mounted
			UsbDevCfgNum = 0;

			// get device speed (returns USB_SPEED_*)
			u8 speed = UsbGetDevSpeed();
			dmb(); // data memory barrier

			// clear speed change interrupt SIE_STATUS.SPEED
			RegClr(USB_SIE_STATUS, B8+B9);

			// stop current enumeration
			UsbHostEnumStop();

			// attach device
			if (speed != USB_SPEED_OFF)
			{
				// use root hub (local host)
				UsbHostDev[0].hub_addr = 0;
				UsbHostDev[0].hub_port = 0;
				UsbHostDev[0].hub_speed = speed; // device speed

				// counter of attempts
				UsbHostEnumTry = 3;

				// time of next enumeration step, 50 ms
				UsbHostEnumNext = Time() + 50000;

				// request to start new device enumeration
				dmb(); // data memory barrier
				UsbHostEnumState = USB_HOST_ENUM_RESET_START;
			}

			// remove device
			else
			{
				// remove connected device of root hub
				UsbHostDevRemove(0, 0);

				// report change state to the hub
#if USE_USB_HOST_HUB	// use USB HUB (host)
				// if (UsbHostDev[0].hub_addr > 0) UsbHostHubSetStatus(UsbHostDev[0].hub_addr); // @TODO ... hub_edpt_status_xfer
#endif
			}
		}
	}

	// device wakes up the host
	if ((status & B1) != 0) // HOST_RESUME
	{
		// clear interrupt flag SE_STATUS.RESUME
		RegClr(USB_SIE_STATUS, B11);
	}

	// transfer complete
	// - setup packet is sent when no data IN or data OUT transaction follows
	// - IN packet is received and the LAST_BUFF bit is set in buffer control register
	// - IN packet is received with zero length
	// - OUT packet is sent and the LAST_BUFF bit is set
	if ((status & B3) != 0) // TRANS_COMPLETE
	{
		// clear interrupt flag SIE_STATUS.TRANS_COMPLETE
		RegClr(USB_SIE_STATUS, B18);

		// SETUP was sent (B1 = send setup packet)
		if (UsbEndpoints[0].active && ((*USB_SIE_CTRL & B1) != 0)) // _SEND_SETUP
		{
			// short delay (FS min 5 us, LS min. 200 us) to allow the device to prepare to receive next SETUP packet.
			WaitUs(400);

			// proces "transfer complete" of setup packet
			UsbEndpoints[0].xfer_len = USB_SETUP_PKT_SIZE;
			UsbHostComp(0, USB_XRES_OK);
		}
		// other transfers complete are handled in BUFF_STATUS
	}

	// buffer status (transfer has been completed)
	if ((status & B4) != 0) // BUFF_STATUS
	{
		u32 buf = *USB_BUFF_STATUS;	// get buffer status
		u32 mask = 3; // bit mask (IN and OUT transfer together)
		u8 ep;
		for (ep = 0; ep < USB_ENDPOINT_NUM; ep++) // endpoint
		{
			// check one buffer (directions IN and OUT)
			if ((buf & mask) != 0)
			{
				// reset buffer bit
				RegClr(USB_BUFF_STATUS, mask);

				// continue transfer (returns True if transfer is complete)
				if (UsbEndpoints[ep].active && UsbXferCont(ep))
				{
					// process "transfer complete" event
					UsbHostComp(ep, USB_XRES_OK);
				}
			}

			// shift mask
			mask <<= 2;
		}
	}

	// data seq error
	if ((status & B5) != 0) // ERROR_DATA_SEQ
	{
		RegClr(USB_SIE_STATUS, B31);
	}

	// rx timeout
	if ((status & B6) != 0) // ERROR_RX_TIME
	{
		RegClr(USB_SIE_STATUS, B27);
	}

	// stall (this should be before buffer status and trans complete)
	if ((status & B10) != 0) // STALL
	{
		// clear interrupt flag SIE_STATUS.STALL_REC
		RegClr(USB_SIE_STATUS, B29);

		// process "transfer complete" with error
		UsbHostComp(0, USB_XRES_STALL);
	}

	// SOF sent
	if ((status & B2) != 0) // SOF
	{
		// read SOF (to clear SOF request)
		u16 sof = UsbGetSof();

		// send SOF event
		if (UsbProcessSOFEvent)
		{
			int i;
			for (i = 0; i < UsbHostDrvNum; i++) if (UsbHostDrv[i].sof != NULL) UsbHostDrv[i].sof(sof);
		}
	}
}

// ----------------------------------------------------------------------------
//                                Initialize
// ----------------------------------------------------------------------------

// (re)initialize host endpoint
//  ep ... endpoint number 0..15, corresponding to interrupt endpoint number
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
//  pktmax ... max. packet size (limited to 64 bytes)
//  xfer ... transfer type USB_XFER_*
//  inter ... interval the host controller should poll this endpoint, in [ms] (0 is the same as 1, 1 ms);
void UsbHostEpInit(u8 ep, u8 dev_addr, u8 dev_epinx, u16 pktmax, u8 xfer, u8 inter)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[ep];

	// device endpoint and direction
	u8 dev_dir = USB_DIR(dev_epinx);	// device endpoint direction
	u8 dev_ep = USB_EP(dev_epinx);		// device endpoint number

	// limit packet size
	if (pktmax > USB_ISO_PACKET_MAX) pktmax = USB_ISO_PACKET_MAX;

	// (re)allocate data buffer
	if (!sep->used || (pktmax != sep->pktmax))
	{
		// deallocate old buffer
		if (sep->used) UsbRamFree(sep->data_buf, sep->pktmax);

		// allocate new buffer
		sep->data_buf = UsbRamAlloc(pktmax);
	}

	// base properties
	sep->used = True;	// this endpoint is used
	sep->pktmax = pktmax;	// max. packet size
	sep->xfer = xfer;	// transfer type
	sep->dev_addr = dev_addr; // device address
	sep->dev_epinx = dev_epinx; // device endpoint index
	sep->rx = (dev_dir == USB_DIR_IN); // receive, direction from device to host

	// next packet identification (setup packet starts with 1)
	sep->next_pid = (dev_ep == 0) ? 1 : 0;

	// setup control register (single buffer)
	u32 val =	(sep->data_buf - USB_RAM) |	// offset of data buffer in DPRAM
			(xfer << 26) |			// transfer type 
			B29 |				// enable interrupt for every transferred buffer
			B31;				// endpoint enable
	if (inter != 0) val |= (inter - 1) << 16;	// interval to poll this endpoint in [ms]
	*sep->ep_ctrl = val;				// set control register
	
	// delay packets in ISO and interrupt (SOF_SYNC)
	if ((xfer == USB_XFER_ISO) || (xfer == USB_XFER_INT))
		RegSet(USB_SIE_CTRL, B8);
	else
		RegClr(USB_SIE_CTRL, B8);

	// setup interrupt endpoint (can be used for bulk transfer, too)
	if (ep > 0)
	{
		// prepare device address, endpoint and direction
		val = dev_addr | (dev_ep << 16) | (dev_dir << 25);

		// preamble flag (= low speed device is connected to full speed hub)
		if (UsbNeedPreamble(dev_addr)) val |= B26;

		// setup interrupt endpoint
		*USB_ADDR_ENDPN(ep) = val;

		// enable interrupt control register
		RegSet(USB_INT_EP_CTRL, BIT(ep));
	}
}

// allocate endpoint (returns ep0 for transfers USB_XFER_CTRL)
u8 UsbHostEpAlloc(u8 xfer)
{
	// control transfers use endpoint 0
	if (xfer == USB_XFER_CTRL) return 0;

	// search free interrupt endpoint
	// Note: Although datasheet refers to them as "interrupt endpoints",
	//       they are actually "asynchronous endpoints" and can be used
	//       for other transfers, such as bulk.
	sEndpoint* sep = &UsbEndpoints[1];
	u8 ep;
	for (ep = 1; ep < USB_ENDPOINT_NUM-1; ep++)
	{
		if (!sep->used) break;
		sep++;
	}
	return ep;
}

// initialize host endpoint by endpoint descriptor (returns endpoint number 0..15)
//  dev_addr ... device address
//  desc ... endpoint descriptor
u8 UsbHostEpOpen(u8 dev_addr, const sUsbDescEp* desc)
{
	// prepare required transfer type USB_XFER_*
	u8 xfer = desc->attr & 3;

	// allocate endpoint
	u8 ep = UsbHostEpAlloc(xfer);

	// convert endpoint address to endpoint index
	u8 dev_epinx = USB_EPADDR_EPINX(desc->ep_addr);

	// initialite endpoint
	UsbHostEpInit(ep, dev_addr, dev_epinx, desc->pktmax & 0x7ff, xfer, desc->interval);

	return ep;
}

// initialize host control endpoint 0 to transfer data
//  dev_addr ... device address
//  pktmax ... size of data
void UsbHostEp0Open(u8 dev_addr, u16 pktmax)
{
	UsbHostEpInit(0, dev_addr, 0, pktmax, USB_XFER_CTRL, 0);
}

// close all open endpoints belonging to this device
//  dev_addr ... device address
void UsbHostDevClose(u8 dev_addr)
{
	// disable all non-control endpoint control registers of this device
	int ep;
	for (ep = 1; ep < USB_ENDPOINT_NUM; ep++)
	{
		// endpoint descriptor
		sEndpoint* sep = &UsbEndpoints[ep];
		if (sep->used && (sep->dev_addr == dev_addr))
		{
			// disable interrupt
			RegClr(USB_INT_EP_CTRL, BIT(ep));

			// reset interrupt endpoint
			*USB_ADDR_ENDPN(ep) = 0;

			// not used
			sep->used = False;
			sep->active = False;
			sep->claimed = False;

			// reset control register
			if ((sep->ep_ctrl != NULL) && (sep->ep_ctrl != USB_SIE_CTRL)) *sep->ep_ctrl = 0;

			// reset buffer control register
			*sep->buf_ctrl = 0;

			// deallocate memory
			UsbRamFree(sep->data_buf, sep->pktmax);
		}
	}
}

// parse 2 endpoint descriptors as IN/OUT pairs (returns False on error)
//  dev_addr ... device address
//  p_desc ... pointer to endpoint desciptors
//  ep_count ... number of endpoints
//  epinx_out ... pointer to endpoint index OUT
//  epinx_in ... pointer to endpoint index IN
//  epout_max ... max. size of packet OUT
//  epin_max ... max. size of packet IN
Bool UsbHostOpenEpPair(u8 dev_addr, const u8* p_desc, u8 ep_count, u8* epinx_out, u8* epinx_in, u16* epout_max, u16* epin_max)
{
	int i;
	for (i = 0; i < ep_count; i++)
	{
		// endpoint descriptor
		const sUsbDescEp* desc_ep = (const sUsbDescEp*)p_desc;

		// check endpoint descriptor type and transfer type
		if ((desc_ep->type != USB_DESC_ENDPOINT) || ((desc_ep->attr & 3) == USB_XFER_CTRL)) return False;

		// initialize endpoint by the descriptor
		UsbHostEpOpen(dev_addr, desc_ep);

		// save endpoint index
		u8 epinx = USB_EPADDR_EPINX(desc_ep->ep_addr); // get endpoint index
		u16 size = desc_ep->pktmax & 0x7ff;
		if (USB_DIR(epinx) == USB_DIR_IN) // direction IN
		{
			*epinx_in = epinx;
			if (epin_max != NULL) *epin_max = size;
		}
		else
		{
			*epinx_out = epinx;
			if (epout_max != NULL) *epout_max = size;
		}

		// shift to next descriptor
		p_desc = USB_DESC_NEXT(p_desc);
	}

	return True;
}

// clear device
void UsbHostClrDev(u8 dev_addr)
{
	// get device structure
	sUsbHostDev* dev = &UsbHostDev[dev_addr];

	// clear device structure
	memset(dev, 0, sizeof(sUsbHostDev));

	// clear mapping
	memset(dev->itf2drv, USB_DRVID_INVALID, USB_ENDPOINT_NUM);
	memset(dev->ep2drv, USB_DRVID_INVALID, USB_ENDPOINT_NUM2);
}

// USB init in host mode
void UsbHostInit()
{
	// select host mode
	UsbHostMode = True;

	// base USB init
	UsbPhyInit();

	// set IRQ handle
	SetHandler(IRQ_USBCTRL, UsbHostIrq);

	// reset variables
	UsbDevCfgNum = 0;			// device is not mounted
	UsbHostSetupStage = USB_STAGE_IDLE;	// no setup stage
	UsbHostEnumState = USB_HOST_ENUM_IDLE;	// not enumerating
	UsbHostSetupCompCB = NULL;		// no callback

	// initialize endpoint descriptors
	int i;
	sEndpoint* sep = UsbEndpoints;
	for (i = 0; i < USB_ENDPOINT_NUM; i++)
	{
		sep->data_buf = NULL;		// no data buffer
		sep->ep_ctrl = USB_EP_INTCTRL(i); // interrupt endpoint control register
		sep->buf_ctrl = USB_EP_INTBUF(i); // interrupt endpoint buffer control register
		sep->pktmax = USB_PACKET_MAX; // max. packet size
		sep++;
	}
	UsbEndpoints[0].data_buf = UsbRamAlloc(USB_ISO_PACKET_MAX);
	UsbEndpoints[0].pktmax = USB_ISO_PACKET_MAX;
	UsbEndpoints[0].used = True; // mark endpoint 0 as used
	UsbEndpoints[0].ep_ctrl = USB_EPX_CTRL; // endpoint X control register

	// check if some driver uses SOF function
	UsbProcessSOFEvent = False;
	for (i = 0; i < UsbHostDrvNum; i++) if (UsbHostDrv[i].sof != NULL) UsbProcessSOFEvent = True;

	// initialize devices
	for (i = 0; i < USE_USB_HOST_DEVNUM; i++) UsbHostClrDev((u8)i);

	// initialize USB peripheral for host mode
	*USB_MAIN_CTRL = B0 | B1;	// enable controller, use host mode

	// SIE control: B9 SOF_EN, B10 KEEP_ALIVE_EN, B15 PULLDOWN_EN, B29 EP0_INT_1BUF
	*USB_SIE_CTRL = B9 | B10 | B15 | B29;

	// enable interrupt on: B0 HOST_CONN_DIS, B1 HOST RESUME, B2 SOF sent,
	//	B3 TRANS_COMPLETE, B4 BUFF STATUS, B5 ERROR_DATA_SEQ, B6 ERROR_RX_TIME, B10 STALL
	u32 n = B0 | B1 | B3 | B4 | B5 | B6 | B10;
	if (UsbProcessSOFEvent) n |= B2;
	*USB_INTE = n;

	// initialize class drivers
	for (i = 0; i < UsbHostDrvNum; i++) UsbHostDrv[i].init();

	// interrupt enable
	UsbIntEnable();

	// host mode is initialized
	UsbHostIsInit = True;
}

// ----------------------------------------------------------------------------
//                                  Transfer
// ----------------------------------------------------------------------------

// start host transfer
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
//  buf ... pointer to data buffer, or pointer to ring buffer
//  len ... length of data
//  use_ring ... use ring buffer (buf is pointer to sRing ring buffer, must be filled with 'len' data)
void UsbHostXferStart(u8 dev_addr, u8 dev_epinx, u8* buf, u16 len, Bool use_ring)
{
	// get endpoint (tries opposite direction as well)
	u8 ep = UsbHostDevEp(dev_addr, dev_epinx);
	if (ep == USB_DRVID_INVALID) return;

	// get descriptor
	sEndpoint* sep = &UsbEndpoints[ep];

	// direction has been flipped, re-initialize endpoint
	if (dev_epinx != sep->dev_epinx)
	{
		// re-initialize endpoint
		UsbHostEpInit(ep, dev_addr, dev_epinx, sep->pktmax, sep->xfer, 0);
	}

	// start new transfer
	UsbXferStart(ep, buf, len, use_ring);

	// if a normal transfer (non-interrupt) then initiate using SIE_CTRL register
	if (ep == 0)
	{
		// set device address and endpoint
		*USB_ADDR_ENDP = dev_addr | (USB_EP(dev_epinx) << 16); // device address and endpoint

		// SIE control: B0 START_TRANS, B9 SOF_EN, B10 KEEP_ALIVE_EN, B15 PULLDOWN_EN, B29 EP0_INT_1BUF
		u32 val = *sep->ep_ctrl & (B28 | B29 | B30);

		val |= B9 | B10 | B15; // | B28 | B30; //B29;
		val |= sep->rx ? B3 : B2;	// receive or send data
		if (UsbNeedPreamble(dev_addr)) val |= B6; // need preamble
		if ((sep->xfer == USB_XFER_ISO) || (sep->xfer == USB_XFER_INT)) val |= B8; // SOF_SYNC;
		*USB_SIE_CTRL = val;		// set SIE control, without start transaction
		UsbDelay12(); 			// short delay 12 system ticks
		*USB_SIE_CTRL = val | B0;	// set SIE control, start transaction
	}
}

// send setup packet (setup packet is in UsbSetupRequest buffer)
//  dev_addr ... device address
void UsbHostSetupSend(u8 dev_addr)
{
	// copy setup packet to packet buffer
	UsbMemcpy(USB_SETUP_PKT, &UsbSetupRequest, USB_SETUP_PKT_SIZE);

	// initialize endpoint 0, direction OUT
	sEndpoint* sep = &UsbEndpoints[0];
	UsbHostEpInit(0, dev_addr, USB_EPINX(0, USB_DIR_OUT), sep->pktmax, USB_XFER_CTRL, 0);
	sep->rem_len = USB_SETUP_PKT_SIZE; // remaining data
	sep->total_len = USB_SETUP_PKT_SIZE; // total length of data to transfer
	sep->xfer_len = 0;		// transferred data
	sep->active = True;		// transfer is active

	// set device address
	*USB_ADDR_ENDP = dev_addr;

	// SIE control: B0 START_TRANS, B1 SEND_SETUP, B9 SOF_EN, B10 KEEP_ALIVE_EN, B15 PULLDOWN_EN, B29 EP0_INT_1BUF
	u32 val = B1 | B9 | B10 | B15 | B29;
	if (UsbNeedPreamble(dev_addr)) val |= B6; // need preamble
	*USB_SIE_CTRL = val;		// set SIE control, without start transaction
	UsbDelay12(); 			// short delay 12 system ticks
	*USB_SIE_CTRL = val | B0;	// set SIE control, start transaction
}

// start control transfer with callback (UsbSetupRequest contains filled setup packet, UsbSetupDataXfer will contain data length)
//  dev_addr ... device address
//  buf ... pointer to data buffer
//  cb ... callback
void UsbHostCtrlXfer(u8 dev_addr, void* buf, pUsbHostSetupCompCB cb)
{
	// save setup
	UsbSetupDataBuff = (u8*)buf;		// pointer to data buffer
	UsbSetupDataLen = UsbSetupRequest.length; // length of data
	UsbSetupDataXfer = 0;			// already transferred data
	UsbHostSetupCompCB = cb;		// callback
	UsbHostSetupStage = USB_STAGE_SETUP;	// start setup stage

	// send setup packet
	UsbHostSetupSend(dev_addr);
}

#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
