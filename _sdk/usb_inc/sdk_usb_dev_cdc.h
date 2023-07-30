
// ****************************************************************************
//
//                     USB Communication Device Class (device)
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
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_CDC	// use USB CDC Communication Device Class, value = number of interfaces (device)

#ifndef _SDK_USB_DEV_CDC_H
#define _SDK_USB_DEV_CDC_H

#include "../../_lib/inc/lib_ring.h"
#include "sdk_usb_def.h"	// definitions

#ifdef __cplusplus
extern "C" {
#endif

// size of buffers must be greater than USB_PACKET_MAX
#ifndef USB_DEV_CDC_RX_BUFSIZE
#define USB_DEV_CDC_RX_BUFSIZE	256	// USB CDC device RX buffer size
#endif

#ifndef USB_DEV_CDC_TX_BUFSIZE
#define USB_DEV_CDC_TX_BUFSIZE	256	// USB CDC device TX buffer size
#endif

// CRC line coding (7 bytes)
typedef struct PACKED {
	u32	bit_rate;	// bit rate (Baud)
	u8	stop_bits;	// number of stop bits 0=1 stop bit, 1=1.5 stop bit, 2=2 stop bits
	u8	parity;		// parity: 0=none, 1=odd, 2=even, 3=mark, 4=space
	u8	data_bits;	// data bits 5, 6, 7, 8 or 16
} sUsbDevCdcLineCoding;

// CDC device interface structure (size 8+8+256+256+32+32=592 bytes)
typedef struct {
	Bool	used;		// interface is used
	u8	itf_num;	// interface number (zero based)
	u8	ep_in;		// endpoint index for input (send data from device to host)
	u8	ep_out;		// endpoint index for output (receive data from host to device)

	u8	line_state;	// linestate: bit 0 DTR (Data Terminal Ready), bit 1 RTS (Request To Send)
				//  DTR must be set to enable send data

	sUsbDevCdcLineCoding	line_coding; // line coding

	u8	rx_ring_buf[USB_DEV_CDC_RX_BUFSIZE]; // receive buffer
	u8	tx_ring_buf[USB_DEV_CDC_TX_BUFSIZE]; // transmission buffer
	sRing	rx_ring;	// receive ring buffer
	sRing	tx_ring;	// transmission ring buffer
} sUsbDevCdcInter;

// CDC interfaces
extern sUsbDevCdcInter UsbDevCdcInter[USE_USB_DEV_CDC];

// application device setup descriptor
extern const sUsbDevSetupDesc UsbDevCdcSetupDesc;

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// check if device is mounted
//  cdc_inx ... CDC interface
Bool UsbDevCdcInxIsMounted(u8 cdc_inx);
INLINE Bool UsbDevCdcIsMounted() { return UsbDevCdcInxIsMounted(0); }

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbDevCdcFindItf(u8 itf_num);

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by endpoint (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbDevCdcFindAddr(u8 epinx);

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbDevCdcNewItf(const sUsbDescItf* itf);

// send data from ring buffer
//  cdc_inx ... CDC interface
void UsbDevCdcInxSend(u8 cdc_inx);

// send data of all interfaces
void UsbDevCdcAllSend();

// receive data to ring buffer
//  cdc_inx ... CDC interface
void UsbDevCdcInxRecv(u8 cdc_inx);

// receive data of all interfaces
void UsbDevCdcAllRecv();

// initialize class driver
void UsbDevCdcInit();

// terminate class driver
void UsbDevCdcTerm();

// reset class driver
void UsbDevCdcReset();

// open device class interface (returns size of used interface, 0=not supported)
u16 UsbDevCdcOpen(const sUsbDescItf* itf, u16 max_len);

// process control transfer complete (returns False to stall)
Bool UsbDevCdcCtrl(u8 stage);

// process data transfer complete
void UsbDevCdcComp(u8 epinx, u8 xres, u16 len);

// schedule driver
void UsbDevCdcSof(u16 sof);

// schedule driver (raised with UsbRescheduleDrv())
//void UsbDevCdcSched();

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// get bytes available for reading from CDC interface
INLINE int UsbDevCdcInxReadReady(u8 cdc_inx) { return (int)RingNum(&UsbDevCdcInter[cdc_inx].rx_ring); }
INLINE int UsbDevCdcReadReady() { return UsbDevCdcInxReadReady(0); }

// read one character from CDC interface (returns 0 if not ready)
char UsbDevCdcInxReadChar(u8 cdc_inx);
INLINE char UsbDevCdcReadChar() { return UsbDevCdcInxReadChar(0); }

// read data from CDC interface (returns number of bytes)
int UsbDevCdcInxReadData(u8 cdc_inx, void* buf, int bufsize);
INLINE int UsbDevCdcReadData(void* buf, int bufsize) { return UsbDevCdcInxReadData(0, buf, bufsize); }

// flush received data in CDC interface
INLINE void UsbDevCdcInxReadFlush(u8 cdc_inx) { RingClear(&UsbDevCdcInter[cdc_inx].rx_ring); }
INLINE void UsbDevCdcReadFlush() { RingClear(&UsbDevCdcInter[0].rx_ring); }

// get bytes available for writing to CDC interface
INLINE int UsbDevCdcInxWriteReady(u8 cdc_inx) { return (int)RingFree(&UsbDevCdcInter[cdc_inx].tx_ring); }
INLINE int UsbDevCdcWriteReady() { return UsbDevCdcInxWriteReady(0); }

// write one character to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbDevCdcInxWriteChar(u8 cdc_inx, char ch);
INLINE Bool UsbDevCdcWriteChar(char ch) { return UsbDevCdcInxWriteChar(0, ch); }

// write data to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbDevCdcInxWriteData(u8 cdc_inx, const void* buf, int len);
INLINE Bool UsbDevCdcWriteData(const void* buf, int len) { return UsbDevCdcInxWriteData(0, buf, len); }

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_DEV_CDC_H

#endif // USE_USB_DEV_CDC	// use USB CDC Communication Device Class, value = number of interfaces (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
