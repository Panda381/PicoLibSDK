
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

#include "../../global.h"	// globals

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_HOST	// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#if USE_USB_HOST_CDC	// use USB CDC Communication Device Class, value = number of interfaces (host)

#include "../inc/sdk_timer.h"
#include "../usb_inc/sdk_usb_host.h"
#include "../usb_inc/sdk_usb_host_cdc.h"

// CDC interfaces
sUsbHostCdcInter UsbHostCdcInter[USE_USB_HOST_CDC];

// temporary save interface for UsbHostCdcCfg
u8 UsbHostCdcCfg_Itf;

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// check if device is mounted
//  cdc_inx ... CDC interface
Bool UsbHostCdcInxIsMounted(u8 cdc_inx)
{
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];
	return cdc->used && UsbIsMounted() && (cdc->ep_in != 0) && (cdc->ep_out != 0);
}

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostCdcFindItf(u8 dev_addr, u8 itf_num)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

		// check interface
		if (cdc->used && (cdc->dev_addr == dev_addr) && (cdc->itf_num == itf_num))
		{
			return cdc_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// ----------------------------------------------------------------------------
//                           Internal functions
// ----------------------------------------------------------------------------
// All functions are called from interrupt service.

// find interface by device address (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbHostCdcFindAddr(u8 dev_addr, u8 dev_epinx)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

		// check interface
		if (cdc->used && (cdc->dev_addr == dev_addr) &&
			((cdc->ep_in == dev_epinx) || (cdc->ep_out == dev_epinx)))
		{
			return cdc_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbHostCdcNewItf(u8 dev_addr, const sUsbDescItf* itf)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

		// check if interface is not used
		if (!cdc->used)
		{
			cdc->used = True; // interface is used
			cdc->dev_addr = dev_addr; // device address
			cdc->itf_num = itf->itf_num; // interface number
			cdc->subclass = itf->itf_subclass; // subclass
			cdc->protocol = itf->itf_protocol; // protocol
			return cdc_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// send data from ring buffer
//  cdc_inx ... CDC interface
void UsbHostCdcInxSend(u8 cdc_inx)
{
	// check if interface is mounted
	if (!UsbHostCdcInxIsMounted(cdc_inx)) return;

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// get endpoint
	u8 ep = UsbHostDevEp(cdc->dev_addr, cdc->ep_out);
	sEndpoint* sep = &UsbEndpoints[ep];

	// get packet size
	u16 pktmax = sep->pktmax;

	// check if any data to send
	u16 count = (u16)RingNum(&cdc->tx_ring); // count of data in ring buffer
	if (count == 0) return; // no data to send

	// ISO transfer - break transfer for next SOF frame (RP2040 does not raise IRQ after sending)
	if (sep->xfer == USB_XFER_ISO)
	{
		sep->active = False;
		*sep->buf_ctrl = 0;
		if (count < pktmax) return; // must send whole packet
	}

	// check if endpoint is busy
	if (UsbHostIsBusy(cdc->dev_addr, cdc->ep_out)) return; // transmission is active

	// send data (better to align size to whole packets to maximize performance)
	if (count > pktmax)
	{
		if (sep->xfer == USB_XFER_ISO) // limit ISO to 1 long packet
			count = pktmax;
		else
			count = count/pktmax*pktmax;
	}
	UsbHostXferStart(cdc->dev_addr, cdc->ep_out, (u8*)&cdc->tx_ring, count, True);
}

// send data of all interfaces
void UsbHostCdcAllSend()
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		UsbHostCdcInxSend(cdc_inx);
	}
}

// receive data to ring buffer
//  cdc_inx ... CDC interface
void UsbHostCdcInxRecv(u8 cdc_inx)
{
	// check if interface is mounted
	if (!UsbHostCdcInxIsMounted(cdc_inx)) return;

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// get endpoint
	u8 ep = UsbHostDevEp(cdc->dev_addr, cdc->ep_in);
	sEndpoint* sep = &UsbEndpoints[ep];

	// get packet size
	u16 pktmax = sep->pktmax;

	// get free space in receive ring buffer
	u16 space = (u16)RingFree(&cdc->rx_ring);
	if (space < pktmax) return; // no free space

	// check if endpoint is busy
	if (UsbHostIsBusy(cdc->dev_addr, cdc->ep_in)) return;

	// start receiving data packet ... we must use whole packet or its multiply, or some data could be lost
	if (space > pktmax)
	{
		if (sep->xfer == USB_XFER_ISO) // limit ISO to 1 long packet
			space = pktmax;
		else
			space = space/pktmax*pktmax;
	}
	UsbHostXferStart(cdc->dev_addr, cdc->ep_in, (u8*)&cdc->rx_ring, space, True);
}

// receive data of all interfaces
void UsbHostCdcAllRecv()
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		UsbHostCdcInxRecv(cdc_inx);
	}
}

// initialize class driver
void UsbHostCdcInit()
{
	memset(UsbHostCdcInter, 0, sizeof(UsbHostCdcInter));

	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

		// setup ring buffers
		RingInit(&cdc->rx_ring, cdc->rx_ring_buf, USB_HOST_CDC_RX_BUFSIZE, USB_SPIN, NULL, NULL, 0); // RX buffer
		RingInit(&cdc->tx_ring, cdc->tx_ring_buf, USB_HOST_CDC_TX_BUFSIZE, USB_SPIN, NULL, NULL, 0); // TX buffer
	}
}

// terminate class driver
void UsbHostCdcTerm()
{

}

// open class interface (returns size of used interface, 0=not supported)
// - At this point, no communications are allowed yet.
u16 UsbHostCdcOpen(u8 dev_addr, const sUsbDescItf* itf, u16 max_len)
{
	// check interface class, check subclass "abstract control model" ... support only ACM subclass
	if ((itf->itf_class != USB_CLASS_CDC) || (itf->itf_subclass != 2)) return 0;

	// check protocol (only AT commands, not Ethernet)
	if (itf->itf_protocol > 6) return 0;

	// check descriptor size
	if (USB_DESC_LEN(itf) > max_len) return 0;

	// allocate new interface
	u8 cdc_inx = UsbHostCdcNewItf(dev_addr, itf);
	if (cdc_inx == USB_DRVID_INVALID) return 0;

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// skip interface descriptor
	u16 drv_len = USB_DESC_LEN(itf);
	const u8* p_desc = USB_DESC_NEXT(itf);

	// skip communication functional descriptor
	while ((USB_DESC_TYPE(p_desc) == USB_DESC_CS_INTERFACE) &&
		(drv_len + USB_DESC_LEN(p_desc) <= max_len))
	{
		// skip descriptor
		drv_len += USB_DESC_LEN(p_desc);
		p_desc = USB_DESC_NEXT(p_desc);
	}

	// skip notification endpoint descriptor
	if (	(itf->num_ep == 1) && // only 1 endpoint on this interface
		(USB_DESC_TYPE(p_desc) == USB_DESC_ENDPOINT) &&
		(drv_len + USB_DESC_LEN(p_desc) <= max_len))
	{
		// skip descriptor
		drv_len += USB_DESC_LEN(p_desc);
		p_desc = USB_DESC_NEXT(p_desc);
	}

	// communication data interface
	if ((USB_DESC_TYPE(p_desc) == USB_DESC_INTERFACE) &&
		(((const sUsbDescItf*)p_desc)->itf_class == USB_CLASS_CDC_DATA) &&
		(drv_len + USB_DESC_LEN(p_desc) + 2*sizeof(sUsbDescEp) <= max_len))
	{
		// skip interface descriptor
		drv_len += USB_DESC_LEN(p_desc);
		p_desc = USB_DESC_NEXT(p_desc);

		// open endpoint pair
		if (!UsbHostOpenEpPair(dev_addr, p_desc, 2, &cdc->ep_out, &cdc->ep_in, NULL, NULL)) return 0;

		// skip endpoint descriptors
		drv_len += 2*sizeof(sUsbDescEp);
	}

	return drv_len;
}

// continue config interface - config complete
void UsbHostCdcCfgComp(u8 dev_addr, u8 xres)
{
	// error
	if (xres != USB_XRES_OK) return;

	// get temporary interface
	u8 itf_num = UsbHostCdcCfg_Itf;

	// pointer to CDC interface
	u8 cdc_inx = UsbHostCdcFindItf(dev_addr, itf_num);
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// prepare for incoming data (do not use UsbHostCdcInxRecv, device is not mounted yet)
	u16 space = (u16)RingFree(&cdc->rx_ring);
	if ((space > 0) && !UsbHostIsBusy(cdc->dev_addr, cdc->ep_in)) 
	{
		// start receiving data
		u8 ep = UsbHostDevEp(cdc->dev_addr, cdc->ep_in);
		u16 pktmax = UsbEndpoints[ep].pktmax;
		space = space/pktmax*pktmax;
		UsbHostXferStart(dev_addr, cdc->ep_in, (u8*)&cdc->rx_ring, space, True);
	}

	// continue set config interface, use 1 more interface for data interface
	UsbHostCfgComp(dev_addr, itf_num+1);
}

// set config interface
Bool UsbHostCdcCfg(u8 dev_addr, u8 itf_num)
{
	// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
	u8 cdc_inx = UsbHostCdcFindItf(dev_addr, itf_num);
	if (cdc_inx == USB_DRVID_INVALID) return False;

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// pointer to setup packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// prepare setup packet
	setup->type = 1 | (1 << 5); // recipient = interface, type = class, direction = out
	setup->request = 0x22; // request = set control line state
	setup->value = B0; // linestate: bit 0 DTR (Data Terminal Ready), bit 1 RTS (Request To Send)
	setup->index = itf_num; // interface number
	setup->length = 0; // no data

	// send data
	UsbHostCdcCfg_Itf = itf_num; // temporary save interface
	UsbHostCtrlXfer(dev_addr, NULL, UsbHostCdcCfgComp);
	return True;
}

// transfer complete callback
Bool UsbHostCdcComp(u8 dev_addr, u8 dev_epinx, u8 xres, u16 len)
{
	// check result OK
	if (xres != USB_XRES_OK) return False;

	// find interface by device address
	u8 cdc_inx = UsbHostCdcFindAddr(dev_addr, dev_epinx);
	if (cdc_inx == USB_DRVID_INVALID) return False; // invalid address

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx]; // pointer to CDC interface

	// receive data
	if (dev_epinx == cdc->ep_in)
	{
		// receive next data
		UsbHostCdcInxRecv(cdc_inx);
	}

	// send data
	else
	{
		// get endpoint
		u8 ep = UsbHostDevEp(dev_addr, dev_epinx);
		if (ep == USB_DRVID_INVALID) return False;

		// get packet size
		u16 pktmax = UsbEndpoints[ep].pktmax;

		// send zero-length packet if transferred bytes where not multiply of packet size and where not zero (= mark end of data)
		if (	(len > 0) && // last sent data was not 0
			(RingNum(&cdc->tx_ring) == 0) && // no new data to send
			(len == (len/pktmax*pktmax))) // last packet was full (was not marked as "last")
		{
			// set zero-length packet ZLP, to mark end of data
			UsbHostXferStart(dev_addr, dev_epinx, NULL, 0, False);
		}

		// send next data ... with ISO, next packet will be sent in SOF service
		else
		{
			if (UsbEndpoints[ep].xfer != USB_XFER_ISO)
				UsbHostCdcInxSend(cdc_inx);
		}
	}

	return True;
}

// close device
void UsbHostCdcClose(u8 dev_addr)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_HOST_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx]; // pointer to CDC interface

		// deinit interface
		if (cdc->used && (cdc->dev_addr == dev_addr))
		{
			cdc->used = False;
			cdc->dev_addr = 0;
			cdc->itf_num = 0;
			cdc->ep_in = 0;
			cdc->ep_out = 0;
			RingClear(&cdc->rx_ring); // RX buffer
			RingClear(&cdc->tx_ring); // TX buffer
		}
	}
}

// schedule driver, synchronize packets with frames
void UsbHostCdcSof(u16 sof)
{
	// receive data of all interfaces
	UsbHostCdcAllRecv();

	// send data of all interfaces
	UsbHostCdcAllSend();
}

/*
// schedule driver (raised with UsbRescheduleDrv())
void UsbHostCdcSched()
{
}
*/

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// read one character from CDC interface (returns 0 if not ready)
char UsbHostCdcInxReadChar(u8 cdc_inx)
{
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx]; // pointer to CDC interface
	return RingReadChar(&cdc->rx_ring); // receive character
}

// read data from CDC interface (returns number of bytes)
int UsbHostCdcInxReadData(u8 cdc_inx, void* buf, int bufsize)
{
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx]; // pointer to CDC interface
	return (int)RingReadData(&cdc->rx_ring, buf, bufsize); // read data
}

// write one character to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbHostCdcInxWriteChar(u8 cdc_inx, char ch)
{
	// device is not mounted
	if (!UsbHostCdcInxIsMounted(cdc_inx)) return False;

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// send character
	while (!RingWrite8(&cdc->tx_ring, ch))
	{
		// device is not mounted
		if (!UsbHostCdcInxIsMounted(cdc_inx)) return False;
	}
	return True;
}

// write data to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbHostCdcInxWriteData(u8 cdc_inx, const void* buf, int len)
{
	char ch;
	const char* s = (const char*)buf;

	// pointer to CDC interface
	sUsbHostCdcInter* cdc = &UsbHostCdcInter[cdc_inx];

	// write first part of data, without waiting
	u32 n = RingWriteData(&cdc->tx_ring, s, len);
	s += n;
	len -= n;

	// send rest of data with waiting
	for (; len > 0; len--)
	{
		// get next character
		ch = *s++;

		// send character (returns False if device is not connected)
		if (!UsbHostCdcInxWriteChar(cdc_inx, ch)) return False;
	}
	return True;
}

#endif // USE_USB_HOST_CDC	// use USB CDC Communication Device Class, value = number of interfaces (host)
#endif // USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)


// ============================================================================
//                      Common interface of host and device
// ============================================================================

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_HOST_CDC || USE_USB_DEV_CDC

#include "../usb_inc/sdk_usb_dev_cdc.h"

// check if device is mounted
//  cdc_inx ... CDC interface
Bool UsbCdcInxIsMounted(u8 cdc_inx)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxIsMounted(cdc_inx);
	else
		return UsbDevCdcInxIsMounted(cdc_inx);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxIsMounted(cdc_inx);
#else
	return UsbDevCdcInxIsMounted(cdc_inx);
#endif
}

// get bytes available for reading from CDC interface
int UsbCdcInxReadReady(u8 cdc_inx)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxReadReady(cdc_inx);
	else
		return UsbDevCdcInxReadReady(cdc_inx);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxReadReady(cdc_inx);
#else
	return UsbDevCdcInxReadReady(cdc_inx);
#endif
}

// read one character from CDC interface (returns 0 if not ready)
char UsbCdcInxReadChar(u8 cdc_inx)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxReadChar(cdc_inx);
	else
		return UsbDevCdcInxReadChar(cdc_inx);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxReadChar(cdc_inx);
#else
	return UsbDevCdcInxReadChar(cdc_inx);
#endif
}

// read data from CDC interface (returns number of bytes)
int UsbCdcInxReadData(u8 cdc_inx, void* buf, int bufsize)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxReadData(cdc_inx, buf, bufsize);
	else
		return UsbDevCdcInxReadData(cdc_inx, buf, bufsize);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxReadData(cdc_inx, buf, bufsize);
#else
	return UsbDevCdcInxReadData(cdc_inx, buf, bufsize);
#endif
}

// flush received data in CDC interface
void UsbCdcInxReadFlush(u8 cdc_inx)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		UsbHostCdcInxReadFlush(cdc_inx);
	else
		UsbDevCdcInxReadFlush(cdc_inx);
#elif USE_USB_HOST_CDC
	UsbHostCdcInxReadFlush(cdc_inx);
#else
	UsbDevCdcInxReadFlush(cdc_inx);
#endif
}

// get bytes available for writing to CDC interface
int UsbCdcInxWriteReady(u8 cdc_inx)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxWriteReady(cdc_inx);
	else
		return UsbDevCdcInxWriteReady(cdc_inx);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxWriteReady(cdc_inx);
#else
	return UsbDevCdcInxWriteReady(cdc_inx);
#endif
}

// write one character to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbCdcInxWriteChar(u8 cdc_inx, char ch)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxWriteChar(cdc_inx, ch);
	else
		return UsbDevCdcInxWriteChar(cdc_inx, ch);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxWriteChar(cdc_inx, ch);
#else
	return UsbDevCdcInxWriteChar(cdc_inx, ch);
#endif
}

// write data to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbCdcInxWriteData(u8 cdc_inx, const void* buf, int len)
{
#if USE_USB_HOST_CDC && USE_USB_DEV_CDC
	if (UsbHostMode)
		return UsbHostCdcInxWriteData(cdc_inx, buf, len);
	else
		return UsbDevCdcInxWriteData(cdc_inx, buf, len);
#elif USE_USB_HOST_CDC
	return UsbHostCdcInxWriteData(cdc_inx, buf, len);
#else
	return UsbDevCdcInxWriteData(cdc_inx, buf, len);
#endif
}

#endif // USE_USB_HOST_CDC || USE_USB_DEV_CDC
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
