
// ****************************************************************************
//
//                               USB physical layer
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

// Control transfer (on endpoint 0):
// 1) SETUP stage
//    - host: token packet, contains device address and direction OUT
//    - host: setup packet 8 bytes, DATA0 (= setup request)
//    - device: handshake packet DATA0 (acknowledge receiving setup packet)
// 2) DATA stage
//    - host: token packet, contains device address and data direction IN/OUT
//    - host or device: data packet, first packet is DATA1 (next packet alternates 1 and 0), other packets may follow
//    - device or host: handshake packet with acknowledgement
// 3) STATUS stage
//    - host: token packet, contains address and IN/OUT direction, which is reverse of direction in data stage
//    - device or host: data packet, zero length means OK, or contains NAK or STALL
//    - host or device: handshake packet with ACK/NAK/STALL acknowledgement

// Timeouts required by USB standard:
// - All requests must be processed within 5 seconds.
// - Standard device requests without a data stage must be completed in 50 ms.
// - Standard device requests with a data stage must start to return data 500 ms after request.
// - Status stage must complete within 50 ms after transmission of last data packet.
// - SetAddress command must process the command and return status within 50 ms.
//   The device then has 2 ms (or 20 ms?) to change address before next request is sent.

// Device states:
//   CONNECT - device connect to bus with UsbDevConnect, host will receive CONN ON
//   DISCONNECT - device disconnect from bus with UsbDevDisconnect, host will receive CONN OFF
//   UNPLUGGED - USB cabel unplugged, host will receive CONN OFF, device will receive SUSPEND
//   PLUGGED - USB cabel plugged, not detected
//   RESET - host sends reset signal to device, device will receive RESET (exit suspend state)
//   SUSPEND - host suspend device to low power mode (go sleep) - stop sending SOF, device receive SUSPEND
//   RESUME - host wake up device from sleep - start sending SOF, device receive RESUME
//   WAKEUP - device requires host to wake up, host receive HOST_RESUME

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)

#ifndef _SDK_USB_PHY_H
#define _SDK_USB_PHY_H

#include "sdk_usb_def.h"	// definitions
#include "../inc/sdk_cpu.h"	// dmb()

#ifdef __cplusplus
extern "C" {
#endif

// Device: 32 descriptors of endpoints 0..15, separate IN and OUT (endpoint 0 and 1 = SETUP packets)
// Host: 16 descriptors of endpoints 0..15 (used for IN and OUT), 0=SETUP endpoint, 1..15=interrupt endpoints
extern sEndpoint UsbEndpoints[USB_ENDPOINT_NUM2]; // size 32*32=1024 bytes

// data of SETUP transfer
extern sUsbSetupPkt UsbSetupRequest; // (size 8 bytes) setup request packet, saved in SETUP stage
extern u8* UsbSetupDataBuff;		// pointer to setup request data buffer
extern u16 UsbSetupDataLen;		// total length of data in setup data buffer
extern u16 UsbSetupDataXfer;		// already transferred data in setup data buffer

// host mode (or device mode otherwise)
extern Bool UsbHostMode;

// device data
extern volatile u8 UsbDevCfgNum;	// current active configuration (0 = not configured, >0 = device is mounted)

// USB is initialized
extern Bool UsbDevIsInit; // device is initialized
extern Bool UsbHostIsInit; // host is initialized
extern Bool UsbProcessSOFEvent;		// internal - some driver uses SOF function

// memory map (64 pages of 64-byte pages = 4 KB, bit 1 = page is used)
extern u64 UsbRamMap;

// ----------------------------------------------------------------------------
//                               Utilities
// ----------------------------------------------------------------------------

// unaligned memcpy (libc memcpy on RP2350 does unaligned access, but DPRAM does not support it)
void UsbMemcpy(void* dst, const void* src, u32 n);

// read SOF (start of frame number)
INLINE u16 UsbGetSof() { return (u16)(*USB_SOF_RD & 0x7ff); }

// check if device is mounted (configured)
INLINE Bool UsbIsMounted() { return (UsbDevCfgNum != 0); }

// check if endpoint is active (busy)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
INLINE Bool UsbEpIsBusy(u8 epinx) { return UsbEndpoints[epinx].active; }

// interrupt enable
void UsbIntEnable();

// interrupt disable
void UsbIntDisable();

// connect device to USB bus
void UsbDevConnect();

// disconnect device from USB bus
void UsbDevDisconnect();

// get device speed (returns USB_SPEED_*)
u8 UsbGetDevSpeed();

// claim endpoint by class driver (returns False if cannot be claimed - already claimed or still active)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
Bool UsbEpClaim(u8 epinx);

// unclaim endpoint by class driver
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
INLINE void UsbEpUnclaim(u8 epinx) { dmb(); UsbEndpoints[epinx].claimed = False; }

// request to reschedule interrupt from class driver
INLINE void UsbRescheduleDrv() { RegSet(USB_INTF, USB_RESCHEDULE_DRV); }

// host: request to reschedule interrupt from timer
INLINE void UsbRescheduleTimer() { RegSet(USB_INTF, USB_RESCHEDULE_TIMER); }

// ----------------------------------------------------------------------------
//                              Transfer control
// ----------------------------------------------------------------------------

// delay 12 system ticks (needed before setting bit to start transmission)
NOINLINE void UsbDelay12();

// prepare transfer of one buffer (returns control word)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  bufid ... buffer ID 0 or 1
u32 UsbNextPrep(u8 epinx, u8 bufid);

// start transfer of next endpoint buffer
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
// It is safe to access different endpoints in a multitask environment.
void UsbNextBuf(u8 epinx);

// start new transfer
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  buf ... pointer to user buffer or pointer to ring buffer
//  len ... length of data
//  use_ring ... use ring buffer (buf is pointer to sRing ring buffer, must be filled with 'len' data)
void UsbXferStart(u8 epinx, void* buf, u16 len, Bool use_ring);

// synchronize one completed buffer (returns number of transferred bytes)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  bufid ... buffer ID 0 or 1
u16 UsbXferSync(u8 epinx, u8 bufid);

// transfer continue (returns True if transfer is complete)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
Bool UsbXferCont(u8 epinx);

// ----------------------------------------------------------------------------
//                                Initialize
// ----------------------------------------------------------------------------

// allocate RAM buffer (returns NULL on error; size is aligned to 64 bytes)
u8* UsbRamAlloc(int size);

// deallocate RAM buffer previously allocated by UsbRamAlloc (addr can be NULL)
void UsbRamFree(u8* addr, int size);

// terminate endpoint
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
void UsbEpTerm(u8 epinx);

// bind endpoints to driver
//  ep2drv ... mapping array of size USB_ENDPOINT_NUM2
//  p_desc ... pointer to interface descriptor
//  desc_len ... max. length of descriptors
//  drv_id ... driver index 
void UsbEpBindDrv(u8* ep2drv, const u8* p_desc, u16 desc_len, u8 drv_id);

// USB terminate
void UsbTerm();

// Base USB init (common part for device and host)
void UsbPhyInit();

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_PHY_H

#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
