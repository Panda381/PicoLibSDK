
// ****************************************************************************
//
//                     USB Communication Device Class (host)
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
#if USE_USB_HOST	// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#if USE_USB_HOST_CDC	// use USB CDC Communication Device Class, value = number of interfaces (host)

#ifndef _SDK_USB_HOST_CDC_H
#define _SDK_USB_HOST_CDC_H

#include "../../_lib/inc/lib_ring.h"
//#include "sdk_usb_def.h"	// definitions
#include "sdk_usb_phy.h"	// physical layer
//#include "../usb_inc/sdk_usb_setuph.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef USB_HOST_CDC_RX_BUFSIZE
#define USB_HOST_CDC_RX_BUFSIZE	256	// USB CDC host RX buffer size
#endif

#ifndef USB_HOST_CDC_TX_BUFSIZE
#define USB_HOST_CDC_TX_BUFSIZE	256	// USB CDC host TX buffer size
#endif

// CDC host interface structure
typedef struct {
	Bool	used;		// interface is used
	u8	dev_addr;	// device address
	u8	itf_num;	// interface number (zero based)
	u8	subclass;	// interface subclass
	u8	protocol;	// interface protocol
	u8	ep_in;		// endpoint index for input (send data from device to host)
	u8	ep_out;		// endpoint index for output (receive data from host to device)

	u8	rx_ring_buf[USB_HOST_CDC_RX_BUFSIZE]; // receive buffer
	u8	tx_ring_buf[USB_HOST_CDC_TX_BUFSIZE]; // transmission buffer
	sRing	rx_ring;	// receive ring buffer
	sRing	tx_ring;	// transmission ring buffer
} sUsbHostCdcInter;

// CDC interfaces
extern sUsbHostCdcInter UsbHostCdcInter[USE_USB_HOST_CDC];

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// check if device is mounted
//  cdc_inx ... CDC interface
Bool UsbHostCdcInxIsMounted(u8 cdc_inx);
INLINE Bool UsbHostCdcIsMounted() { return UsbHostCdcInxIsMounted(0); }

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostCdcFindItf(u8 dev_addr, u8 itf_num);

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by device address (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostCdcFindAddr(u8 dev_addr, u8 dev_epinx);

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbHostCdcNewItf(u8 dev_addr, const sUsbDescItf* itf);

// send data from ring buffer
//  cdc_inx ... CDC interface
void UsbHostCdcInxSend(u8 cdc_inx);

// send data of all interfaces
void UsbHostCdcAllSend();

// receive data to ring buffer
//  cdc_inx ... CDC interface
void UsbHostCdcInxRecv(u8 cdc_inx);

// receive data of all interfaces
void UsbHostCdcAllRecv();

// set control line state
//  cdc_inx ... CDC interface
//  dtr ... DTR (Data Terminal Ready)
//  rts ... RTS (Request To Send)
void UsbHostCdcInxSetLine(u8 cdc_inx, Bool dtr, Bool rts);

// initialize class driver
void UsbHostCdcInit();

// terminate class driver
void UsbHostCdcTerm();

// open class interface (returns size of used interface, 0=not supported)
// - At this point, no communications are allowed yet.
u16 UsbHostCdcOpen(u8 dev_addr, const sUsbDescItf* itf, u16 max_len);

// set config interface
Bool UsbHostCdcCfg(u8 dev_addr, u8 itf_num);

// transfer complete callback
Bool UsbHostCdcComp(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len);

// close device
void UsbHostCdcClose(u8 dev_addr);

// schedule driver, synchronized packets in frames
void UsbHostCdcSof(u16 sof);

// schedule driver (raised with UsbRescheduleDrv())
//void UsbHostCdcSched();

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// get bytes available for reading from CDC interface
INLINE int UsbHostCdcInxReadReady(u8 cdc_inx) { return (int)RingNum(&UsbHostCdcInter[cdc_inx].rx_ring); }
INLINE int UsbHostCdcReadReady() { return UsbHostCdcInxReadReady(0); }

// read one character from CDC interface (returns 0 if not ready)
char UsbHostCdcInxReadChar(u8 cdc_inx);
INLINE char UsbHostCdcReadChar() { return UsbHostCdcInxReadChar(0); }

// read data from CDC interface (returns number of bytes)
int UsbHostCdcInxReadData(u8 cdc_inx, void* buf, int bufsize);
INLINE int UsbHostCdcReadData(void* buf, int bufsize) { return UsbHostCdcInxReadData(0, buf, bufsize); }

// flush received data in CDC interface
INLINE void UsbHostCdcInxReadFlush(u8 cdc_inx) { RingClear(&UsbHostCdcInter[cdc_inx].rx_ring); }
INLINE void UsbHostCdcReadFlush() { RingClear(&UsbHostCdcInter[0].rx_ring); }

// get bytes available for writing to CDC interface
INLINE int UsbHostCdcInxWriteReady(u8 cdc_inx) { return (int)RingFree(&UsbHostCdcInter[cdc_inx].tx_ring); }
INLINE int UsbHostCdcWriteReady() { return UsbHostCdcInxWriteReady(0); }

// write one character to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbHostCdcInxWriteChar(u8 cdc_inx, char ch);
INLINE Bool UsbHostCdcWriteChar(char ch) { return UsbHostCdcInxWriteChar(0, ch); }

// write data to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbHostCdcInxWriteData(u8 cdc_inx, const void* buf, int len);
INLINE Bool UsbHostCdcWriteData(const void* buf, int len) { return UsbHostCdcInxWriteData(0, buf, len); }

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_HOST_CDC_H

#endif // USE_USB_HOST_CDC	// use USB CDC Communication Device Class, value = number of interfaces (host)
#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)

// ============================================================================
//                      Common interface of host and device
// ============================================================================

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_HOST_CDC || USE_USB_DEV_CDC

#ifndef _SDK_USB_CDC_H
#define _SDK_USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

// check if device is mounted
//  cdc_inx ... CDC interface
Bool UsbCdcInxIsMounted(u8 cdc_inx);
INLINE Bool UsbCdcIsMounted() { return UsbCdcInxIsMounted(0); }

// get bytes available for reading from CDC interface
int UsbCdcInxReadReady(u8 cdc_inx);
INLINE int UsbCdcReadReady() { return UsbCdcInxReadReady(0); }

// read one character from CDC interface (returns 0 if not ready)
char UsbCdcInxReadChar(u8 cdc_inx);
INLINE char UsbCdcReadChar() { return UsbCdcInxReadChar(0); }

// read data from CDC interface (returns number of bytes)
int UsbCdcInxReadData(u8 cdc_inx, void* buf, int bufsize);
INLINE int UsbCdcReadData(void* buf, int bufsize) { return UsbCdcInxReadData(0, buf, bufsize); }

// flush received data in CDC interface
void UsbCdcInxReadFlush(u8 cdc_inx);
INLINE void UsbCdcReadFlush() { UsbCdcInxReadFlush(0); }

// get bytes available for writing to CDC interface
int UsbCdcInxWriteReady(u8 cdc_inx);
INLINE int UsbCdcWriteReady() { return UsbCdcInxWriteReady(0); }

// write one character to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbCdcInxWriteChar(u8 cdc_inx, char ch);
INLINE Bool UsbCdcWriteChar(char ch) { return UsbCdcInxWriteChar(0, ch); }

// write data to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbCdcInxWriteData(u8 cdc_inx, const void* buf, int len);
INLINE Bool UsbCdcWriteData(const void* buf, int len) { return UsbCdcInxWriteData(0, buf, len); }

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_CDC_H

#endif // USE_USB_HOST_CDC || USE_USB_DEV_CDC
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
