
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

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)

#ifndef _SDK_USB_DEV_H
#define _SDK_USB_DEV_H

#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

// USB product ID:
//  Devices with different interface must have different product ID, or PC will report system error.
#define USB_PID  (0x4000 + USE_USB_DEV_CDC + (USE_USB_DEV_HID<<2) + (USE_USB_DEV_MIDI<<4) + \
	(USE_USB_DEV_VENDOR<<6) + (USE_USB_DEV_MSC<<8) + (USE_USB_DEV_VIDEO<<9) + (USE_USB_DEV_TMC<<10) + \
	(USE_USB_DEV_NET<<11) + (USE_USB_DEV_BTH<<12) + (USE_USB_DEV_AUDIO<<13) + (USE_USB_DEV_DFU<<14))

// setup buffer for device
#define USB_DEVSETUPBUFF_MAX	USB_PACKET_MAX	// size of setup buffer
// device: uses only USB_PACKET_MAX
extern ALIGNED u8 UsbDevSetupBuff[USB_DEVSETUPBUFF_MAX]; // size 64 bytes

// Device: mapping endpoint to device driver (USB_DRVID_INVALID = invalid driver)
extern u8 UsbDevEp2Drv[USB_ENDPOINT_NUM2]; // size 32 bytes

// Device: mapping interface number to device driver (USB_DRVID_INVALID = invalid)
extern u8 UsbDevItf2Drv[USB_ENDPOINT_NUM]; // size 16 bytes

// Device: process control transfer complete callback (returns False to stall; NULL = none)
// Used internally to call device 'ctrl' function.
typedef Bool (*pUsbDevSetupCompCB) (u8 stage);
extern pUsbDevSetupCompCB UsbDevSetupCompCB;

// pointer to application device setup descriptor
extern const sUsbDevSetupDesc* UsbDevSetupDesc;

// Device: device assigned address (>0 device is addressed)
extern volatile u8 UsbDevAddress;

// device data
extern volatile Bool UsbDevConnected;	// device is connected (= 1st SETUP packet has been received)
extern volatile Bool UsbDevSuspended;	// device is suspended
extern volatile Bool UsbDevSelfPowered;	// device is self powered (updated by UsbDevSetCfg)
extern volatile Bool UsbDevWakeupEn;		// remote wake up host by driver is enabled

// device class driver
typedef struct {
	void	(*init)();	// initialize class driver
	void	(*term)();	// terminate class driver
	void	(*reset)();	// reset class driver (device is unplugged)
	u16	(*open)(const sUsbDescItf* itf, u16 max_len); // open device class interface (returns size of used interface, 0=not supported)
	Bool	(*ctrl)(u8 stage); // process control transfer complete (returns False to stall)
	void	(*comp)(u8 epinx, u8 xres, u16 len); // process data transfer complete
	void	(*sof)(u16 sof); // receiving SOF (start of frame; NULL=not used)
	void	(*sched)();	// schedule driver (NULL=not used, raised with UsbRescheduleDrv())
} sUsbDevDrv;

// device class drivers
extern const sUsbDevDrv UsbDevDrv[];

// number of device drivers
extern const u8 UsbDevDrvNum;

// ----------------------------------------------------------------------------
//                              Utilities
// ----------------------------------------------------------------------------

// check if device is addressed (has assigned device address)
INLINE Bool UsbIsAddressed() { return UsbDevAddress > 0; }

// check if device is connected to host (= 1st SETUP packet has been received)
INLINE Bool UsbIsConnected() { return UsbDevConnected; }

// check if device is suspended
INLINE Bool UsbIsSuspended() { return UsbDevSuspended; }

// check if device is self powered
INLINE Bool UsbIsSelfPowered() { return UsbDevSelfPowered != 0; }

// enable SOF interrupt
void UsbDevSofEnable();

// disable SOF interrupt (if not used by drivers)
void UsbDevSofDisable();

// request host to wake up (only if suspended and if wakeup is enabled)
// In case of remote wake up, no RESUME signal will be received, we must resume by SOF packet.
void UsbDevWakeup();

// send STALL signal to the host
//  epinx ... endpoint index 0..31
void UsbDevSetStall(u8 epinx);

// clear STALL request
//  epinx ... endpoint index 0..31
void UsbDevClrStall(u8 epinx);

// ----------------------------------------------------------------------------
//                         Process SETUP control request
// ----------------------------------------------------------------------------

// get configuration descriptor by configuration number (returns NULL if not found)
const sUsbDescCfg* UsbDevGetDescCfg(u8 cfg);

// get configuration descriptor by configuration index (returns NULL if not found)
const sUsbDescCfg* UsbDevGetDescCfgInx(u8 cfginx);

// process "get descriptor" request (returns False to stall)
Bool UsbDevGetDesc();

// process "set configure" request (cfg = 1..., returns False = stall control endpoint)
// - Function finds interface and map it to driver.
Bool UsbDevSetCfg(u8 cfg);

// acknowledging at Status Stage
void UsbDevSetupAck();

// invoke class driver control request at SETUP stage, set continue to DATA stage (returns False to stall)
Bool UsbDevClassCtrl(const sUsbDevDrv* drv);

// start transfer data to/from control endpoint (in required SETUP direction)
// - In some special cases data to be sent can be in UsbDevSetupBuff buffer (ASCII text descriptor).
void UsbDevSetupStart(void* buffer, u16 len);

// process SETUP control request (start of SETUP transaction; returns False = stall control endpoint)
Bool UsbDevSetupReq();

// ----------------------------------------------------------------------------
//                            Transfer complete
// ----------------------------------------------------------------------------

// queue next transaction in Data Stage
// - In some special cases data to be sent can be in UsbDevSetupBuff buffer (ASCII text descriptor).
void UsbDevSetupNext();

// Process SETUP transfer complete (DATA or STATUS stage)
//  epinx ... endpoint index 0 or 1
//  xres ... transfer result USB_XRES_*
//  len ... transferred bytes
void UsbDevSetupComp(u8 epinx, u8 xres, u16 len);

// Process "transfer complete" event after transfer all buffers
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  xres ... transfer result USB_XRES_*
void UsbDevComp(u8 epinx, u8 xres);

// ----------------------------------------------------------------------------
//                            Interrupt service
// ----------------------------------------------------------------------------

// resume device (used internally from UsbDevIrq)
void UsbDevResume();

// USB device IRQ handler
void UsbDevIrq();

// ----------------------------------------------------------------------------
//                                Initialize
// ----------------------------------------------------------------------------
// Should be called from CPU0.

// reset all non-control endpoints
void UsbDevEpReset();

// reset configuration of USB device drivers (resets drivers and clears base variables; leaves SETUP data untouched)
void UsbDevCfgReset();

// reset SETUP data
void UsbDevSetupReset();

// reset USB device drivers (resets drivers and clears base variables, resets SETUP data)
void UsbDevReset();

// initialize device endpoint
//  epinx ... endpoint index 0..31
//  pktmax ... max. packet size
//  xfer ... transfer type USB_XFER_*
void UsbDevEpInit(u8 epinx, u16 pktmax, u8 xfer);

// initialize device endpoint by endpoint descriptor
//   desc ... endpoint request descriptor
void UsbDevEpOpen(const sUsbDescEp* desc);

// parse endpoint descriptors as IN/OUT pair (returns False to stall)
//  p_desc ... pointer to endpoint desciptors
//  ep_count ... number of endpoints
//  epinx_out ... pointer to endpoint index OUT
//  epinx_in ... pointer to endpoint index IN
Bool UsbDevOpenEpPair(const u8* p_desc, u8 ep_count, u8* epinx_out, u8* epinx_in);

// USB init in device mode
//  desc ... application device setup descriptor
void UsbDevInit(const sUsbDevSetupDesc* desc);

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_DEV_H

#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
