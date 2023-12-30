
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

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_HOST	// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)

#ifndef _SDK_USB_HOST_H
#define _SDK_USB_HOST_H

#include "sdk_usb_def.h"	// definitions
#include "sdk_usb_phy.h"

#ifdef __cplusplus
extern "C" {
#endif

// host enumeration state
enum {
	USB_HOST_ENUM_IDLE = 0,		// idle, no enumeration
	USB_HOST_ENUM_RESET_START,	// (WAIT) start reset signal
	USB_HOST_ENUM_RESET_STOP,	// (WAIT) stop reset signal (reset signal: both D-/D+ goes to low SE0 for 10 ms)
	USB_HOST_ENUM_START,		// (WAIT) wait to start of enumeration
	USB_HOST_ENUM_SET_ADDR,		// set address
	USB_HOST_ENUM_SET_ADDRW,	// delay after set address
	USB_HOST_ENUM_DEV_DESC,		// (WAIT) get device descriptor
	USB_HOST_ENUM_CFG_DESC,		// get base configuration descriptor
	USB_HOST_ENUM_CFG_FULL,		// get full configuration
	USB_HOST_ENUM_SET_CFG,		// set config
	USB_HOST_ENUM_STOP,		// stop enumeration
};

// setup buffer for host
#define USB_HOSTSETUPBUFF_MAX	320	// size of setup buffer
// host: must be big enough to hold all descriptors from device of one configuration (minimum is 9 bytes; 4 COM ports require 273 bytes)
extern ALIGNED u8 UsbHostSetupBuff[USB_HOSTSETUPBUFF_MAX];

// host device descriptors (index corresponds to the device address, including address 0)
extern sUsbHostDev UsbHostDev[USE_USB_HOST_DEVNUM];

// host SETUP complete callback (returns False if stall; NULL = none)
//  dev_addr ... device address
//  xres ... transfer result USB_XRES_*
// Used internally (if OK, UsbSetupDataXfer contains length of data, UsbSetupRequest contains request packet)
typedef void (*pUsbHostSetupCompCB) (u8 dev_addr, u8 xres);
extern pUsbHostSetupCompCB UsbHostSetupCompCB;

// host setup stage
extern u8 UsbHostSetupStage; // setup stage USB_STAGE_*

// host enumeration
extern volatile u8 UsbHostEnumState; // current state of device enumeration (0 = not enumerating)
extern volatile u8 UsbHostEnumTry; // counter of attempts to enumerate
extern u8 UsbHostEnumAddr; // address of enumerated device
extern volatile u32 UsbHostEnumNext; // time of next enumeration step, in [us]

// host class driver
typedef struct {
	void	(*init)();	// initialize class driver
	void	(*term)();	// terminate class driver
	u16	(*open)(u8 dev_addr, const sUsbDescItf* itf, u16 max_len); // open class interface (returns size of used interface, 0=not supported)
				// During open() no communications are allowed yet.
	Bool	(*cfg)(u8 dev_addr, u8 itf_num); // configure interface (NULL = not used)
					// Function cfg() can setup device. After it, function UsbHostCfgComp
					// will be called, with highest interface number of the driver.
	Bool	(*comp)(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len); // transfer complete callback (dev_epinx = device endpoint index 0..31)
	void	(*close)(u8 dev_addr);	// close device
	void	(*sof)(u16 sof); // sending SOF (start of frame; NULL=not used)
	void	(*sched)();	// schedule driver (NULL=not used, raised with UsbRescheduleDrv())
} sUsbHostDrv;

// host class drivers
extern const sUsbHostDrv UsbHostDrv[];

// number of host drivers
extern const u8 UsbHostDrvNum;

// ----------------------------------------------------------------------------
//                            Utilities
// ----------------------------------------------------------------------------

// enable SOF interrupt
void UsbDevSofEnable();

// disable SOF interrupt (if not used by drivers)
void UsbDevSofDisable();

// check if device address is valid
//  dev_addr ... device address
INLINE Bool UsbHostCheckAddr(u8 dev_addr) { return (dev_addr < USE_USB_HOST_DEVNUM); }

// get device structure
//  dev_addr ... device address
// Returns dev0 if the address is invalid, as a fallback treatment,
// since in most cases the address is already checked. To check
// the validity of the address, use UsbHostCheckAddr function.
sUsbHostDev* UsbHostGetDev(u8 dev_addr);

// get hub (or root) device speed (returns USB_SPEED_FS or USB_SPEED_LS)
//  dev_addr ... device address
INLINE u8 UsbGetHubSpeed(u8 dev_addr) { return UsbHostGetDev(dev_addr)->hub_speed; }

// check if device needs preamble
//  dev_addr ... device address
INLINE Bool UsbNeedPreamble(u8 dev_addr) { return UsbGetDevSpeed() != UsbGetHubSpeed(dev_addr); }

// get endpoint from device address and device endpoint index
// (returns USB_DRVID_INVALID = not found; tries opposite direction as well)
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
u8 UsbHostDevEp(u8 dev_addr, u8 dev_epinx);

// check if transfer is busy
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
Bool UsbHostIsBusy(u8 dev_addr, u8 dev_epinx);

// claim host endpoint by class driver (returns False if cannot be claimed - already claimed or still active)
//  dev_addr ... device address
//  dev_epainx ... device endpoint index 0..31
Bool UsbHostEpClaim(u8 dev_addr, u8 dev_epinx);

// unclaim host endpoint by class driver
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
void UsbHostEpUnclaim(u8 dev_addr, u8 dev_epinx);

// ----------------------------------------------------------------------------
//                            Transfer complete
// ----------------------------------------------------------------------------

// invoke SETUP transfer complete callback and idle
// If OK, UsbSetupDataXfer contains length of data, UsbSetupRequest contains request packet.
//  dev_addr ... device address
//  xres ... transfer result USB_XRES_*
void UsbHostSetupIdle(u8 dev_addr, u8 xres);

// process SETUP transfer complete on endpoint 0
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
//  xres ... transfer result USB_XRES_*
//  len ... length of transferred data
void UsbHostSetupComp(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len);

// proces "transfer completed" event after transfer all buffers
//  ep ... endpoint 0..15
//  xres ... transfer result USB_XRES_*
void UsbHostComp(u8 ep, u8 xres);

// ----------------------------------------------------------------------------
//                            Enumeration
// ----------------------------------------------------------------------------

// parse configuration descriptor (returns False on error)
//  dev_addr ... device address
Bool UsbHostParseCfg(u8 dev_addr, const u8* p_desc);

// stop unfinished enumeration
void UsbHostEnumStop();

// get descriptor from device
//  dev_addr ... device address
//  desc_type ... descriptor type
//  inx ... descriptor index
//  lang ... language ID
//  buf ... data buffer to receive descriptor
//  len ... length of data
//  cb ... callback
void UsbHostGetDesc(u8 dev_addr, u8 desc_type, u8 inx, u16 lang, void* buf, u16 len, pUsbHostSetupCompCB cb);

// set config complete
//  dev_addr ... device address
//  itf_num ... current interface number (Completion continues on the following interface, starting with 0xff)
// This function can be called from drv->cfg functions, to continue to next driver.
void UsbHostCfgComp(u8 dev_addr, u8 itf_num);

// process enumeration
//  dev_addr ... device address
//  xres ... transfer result USB_XRES_*
void UsbHostEnum(u8 dev_addr, u8 xres);

// ----------------------------------------------------------------------------
//                            Interrupt service
// ----------------------------------------------------------------------------

// USB host timer
//  - called from SysTick_Handler every 5 ms
void UsbHostOnTime();

// remove connected device
//  hub_addr ... hub address, 0 = roothub
//  hub_port ... hub port, 0 = all devices on downstream hub
void UsbHostDevRemove(u8 hub_addr, u8 hub_port);

// USB host IRQ handler
void UsbHostIrq();

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
void UsbHostEpInit(u8 ep, u8 dev_addr, u8 dev_epinx, u16 pktmax, u8 xfer, u8 inter);

// allocate endpoint (returns ep0 for transfers USB_XFER_CTRL)
u8 UsbHostEpAlloc(u8 xfer);

// initialize host endpoint by endpoint descriptor (returns endpoint number 0..15)
//  dev_addr ... device address
//  desc ... endpoint descriptor
u8 UsbHostEpOpen(u8 dev_addr, const sUsbDescEp* desc);

// initialize host control endpoint 0 to transfer data
//  dev_addr ... device address
//  pktmax ... size of data
void UsbHostEp0Open(u8 dev_addr, u16 pktmax);

// close all opened endpoints belong to this device
//  dev_addr ... device address
void UsbHostDevClose(u8 dev_addr);

// clear device
void UsbHostClrDev(u8 dev_addr);

// parse 2 endpoint descriptors as IN/OUT pairs (returns False on error)
//  dev_addr ... device address
//  p_desc ... pointer to endpoint desciptors
//  ep_count ... number of endpoints
//  epinx_out ... pointer to endpoint index OUT
//  epinx_in ... pointer to endpoint index IN
//  epout_max ... max. size of packet OUT
//  epin_max ... max. size of packet IN
Bool UsbHostOpenEpPair(u8 dev_addr, const u8* p_desc, u8 ep_count, u8* epinx_out, u8* epinx_in, u16* epout_max, u16* epin_max);

// USB init in host mode
void UsbHostInit();

// ----------------------------------------------------------------------------
//                                  Transfer
// ----------------------------------------------------------------------------

// start host transfer
//  dev_addr ... device address
//  dev_epinx ... device endpoint index 0..31
//  buf ... pointer to data buffer, or pointer to ring buffer
//  len ... length of data
//  use_ring ... use ring buffer (buf is pointer to sRing ring buffer, must be filled with 'len' data)
void UsbHostXferStart(u8 dev_addr, u8 dev_epinx, u8* buf, u16 len, Bool use_ring);

// send setup packet (setup packet is in UsbSetupRequest buffer)
//  dev_addr ... device address
void UsbHostSetupSend(u8 dev_addr);

// start control transfer with callback (UsbSetupRequest contains filled setup packet, UsbSetupDataXfer will contain data length)
//  dev_addr ... device address
//  buf ... pointer to data buffer
//  cb ... callback
void UsbHostCtrlXfer(u8 dev_addr, void* buf, pUsbHostSetupCompCB cb);

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_HOST_H

#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
