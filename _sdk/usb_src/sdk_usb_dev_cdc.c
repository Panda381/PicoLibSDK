
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

#include "../../global.h"	// globals

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#if USE_USB_DEV_CDC	// use USB CDC Communication Device Class, value = number of interfaces (device)

#include "../usb_inc/sdk_usb_phy.h" // physical layer
#include "../usb_inc/sdk_usb_dev.h" // devices
#include "../usb_inc/sdk_usb_dev_cdc.h"

// CDC interfaces
sUsbDevCdcInter UsbDevCdcInter[USE_USB_DEV_CDC];

// === default descriptors

// buffer to load unique board string ID from flash memory
char BoardIdStr_cdc[9] = "12345";

// Strings
const char* UsbDevCdcDescStr[] =
{
				// 0: language code (not included here)
	"Raspberry Pi",		// 1: manufacturer
	"PicoPad",		// 2: product
	BoardIdStr_cdc,		// 3: board serial number
	"USB UART",		// 4: configuration descriptor
	"USB UART2",		// 5: configuration descriptor
	"USB UART3",		// 6: configuration descriptor
	"USB UART4",		// 7: configuration descriptor
};

// CDC device descriptor
const sUsbDescDev UsbDevCdcDescDev =
{
	18,			// 0: size of this descriptor in bytes (= 18)
	USB_DESC_DEVICE,	// 1: descriptor type (= USB_DESC_DEVICE)
	0x0200,			// 2: USB specification in BCD code (0x200 = 2.00)
	USB_CLASS_MISC,		// 4: device class code = Miscellaneous Device (class is defined by interface descriptor)
	2,			// 5: device subclass code = misc subclass "common"
	1,			// 6: device protocol code = IAD protocol
	USB_PACKET_MAX,		// 7: max. packet size for endpoint 0 (valid size 8, 16, 32, 64) = size of controll buffer
	0x2E8A,			// 8: vendor ID (0x2E8A = Raspberry Pi)
	USB_PID,		// 10: product ID (0x000A = Raspbery Pi Pico SDK)
	0x0100,			// 12: device release number in BCD code (0x0100 = 1.00)
	1,			// 14: index of manufacturer string descriptor (0=none)
	2,			// 15: index of product string descriptor (0=none)
	3,			// 16: index of serial number string descriptor (0=none)
	1,			// 17: number of possible configurations
};

#define USB_CDCD_ITF	0				// index of first interface - control
#define USB_CDCD_ITFD	1				// index of first interface - data
#define USB_CDCD_EPNOT	USB_EPADDR(1, USB_DIR_IN)	// notification endpoint
#define USB_CDCD_EPOUT	USB_EPADDR(2, USB_DIR_OUT)	// output endpoint
#define USB_CDCD_EPIN	USB_EPADDR(2, USB_DIR_IN)	// input endpoint

#define USB_CDCD_ITF2	2				// index of second interface - control
#define USB_CDCD_ITFD2	3				// index of second interface - data
#define USB_CDCD_EPNOT2	USB_EPADDR(3, USB_DIR_IN)	// notification endpoint
#define USB_CDCD_EPOUT2	USB_EPADDR(4, USB_DIR_OUT)	// output endpoint
#define USB_CDCD_EPIN2	USB_EPADDR(4, USB_DIR_IN)	// input endpoint

#define USB_CDCD_ITF3	4				// index of second interface - control
#define USB_CDCD_ITFD3	5				// index of second interface - data
#define USB_CDCD_EPNOT3	USB_EPADDR(5, USB_DIR_IN)	// notification endpoint
#define USB_CDCD_EPOUT3	USB_EPADDR(6, USB_DIR_OUT)	// output endpoint
#define USB_CDCD_EPIN3	USB_EPADDR(6, USB_DIR_IN)	// input endpoint

#define USB_CDCD_ITF4	6				// index of second interface - control
#define USB_CDCD_ITFD4	7				// index of second interface - data
#define USB_CDCD_EPNOT4	USB_EPADDR(7, USB_DIR_IN)	// notification endpoint
#define USB_CDCD_EPOUT4	USB_EPADDR(8, USB_DIR_OUT)	// output endpoint
#define USB_CDCD_EPIN4	USB_EPADDR(8, USB_DIR_IN)	// input endpoint

#if USE_USB_DEV_CDC == 1 // 1 interface

// CDC configuration descriptor - 1 channel (9+66 = 75 bytes)
const u8 UsbDevCdcDescCfg[9+USB_TEMP_CDCD_LEN] =
{
	// (9) configuration descriptor (index=1, 2 interfaces, no string, length, 100 mA)
	USB_TEMP_CFG(1, 2, 0, 9+USB_TEMP_CDCD_LEN, 100),
	// (66) CDC device descriptors (index of interface, string = 4, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF, 4, USB_CDCD_EPNOT, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT, USB_CDCD_EPIN, USB_PACKET_MAX),
};

#elif USE_USB_DEV_CDC == 2 // 2 interfaces

// CDC configuration descriptor - 2 channels (9+2*66 = 141 bytes)
const u8 UsbDevCdcDescCfg[9+2*USB_TEMP_CDCD_LEN] =
{
	// (9) configuration descriptor (index=1, 4 interfaces, no string, length, 150 mA)
	USB_TEMP_CFG(1, 4, 0, 9+2*USB_TEMP_CDCD_LEN, 150),
	// (66) 1st CDC device descriptors (index of interface, string = 4, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF, 4, USB_CDCD_EPNOT, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT, USB_CDCD_EPIN, USB_PACKET_MAX),
	// (66) 2nd CDC device descriptors (index of interface, string = 5, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF2, 5, USB_CDCD_EPNOT2, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT2, USB_CDCD_EPIN2, USB_PACKET_MAX),
};

#elif USE_USB_DEV_CDC == 3 // 3 interfaces

// CDC configuration descriptor - 3 channels (9+3*66 = 207 bytes)
const u8 UsbDevCdcDescCfg[9+3*USB_TEMP_CDCD_LEN] =
{
	// (9) configuration descriptor (index=1, 6 interfaces, no string, length, 200 mA)
	USB_TEMP_CFG(1, 6, 0, 9+3*USB_TEMP_CDCD_LEN, 200),
	// (66) 1st CDC device descriptors (index of interface, string = 4, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF, 4, USB_CDCD_EPNOT, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT, USB_CDCD_EPIN, USB_PACKET_MAX),
	// (66) 2nd CDC device descriptors (index of interface, string = 5, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF2, 5, USB_CDCD_EPNOT2, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT2, USB_CDCD_EPIN2, USB_PACKET_MAX),
	// (66) 3rd CDC device descriptors (index of interface, string = 6, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF3, 6, USB_CDCD_EPNOT3, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT3, USB_CDCD_EPIN3, USB_PACKET_MAX),
};

#elif USE_USB_DEV_CDC == 4 // 4 interfaces

// CDC configuration descriptor - 4 channels (9+4*66 = 273 bytes)
const u8 UsbDevCdcDescCfg[9+4*USB_TEMP_CDCD_LEN] =
{
	// (9) configuration descriptor (index=1, 8 interfaces, no string, length, 250 mA)
	USB_TEMP_CFG(1, 8, 0, 9+4*USB_TEMP_CDCD_LEN, 250),
	// (66) 1st CDC device descriptors (index of interface, string = 4, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF, 4, USB_CDCD_EPNOT, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT, USB_CDCD_EPIN, USB_PACKET_MAX),
	// (66) 2nd CDC device descriptors (index of interface, string = 5, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF2, 5, USB_CDCD_EPNOT2, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT2, USB_CDCD_EPIN2, USB_PACKET_MAX),
	// (66) 3rd CDC device descriptors (index of interface, string = 6, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF3, 6, USB_CDCD_EPNOT3, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT3, USB_CDCD_EPIN3, USB_PACKET_MAX),
	// (66) 4th CDC device descriptors (index of interface, string = 7, notification ep, size 8, out ep, in ep, packet size)
	USB_TEMP_CDCD(USB_CDCD_ITF4, 7, USB_CDCD_EPNOT4, USB_SETUP_PKT_SIZE, USB_CDCD_EPOUT4, USB_CDCD_EPIN4, USB_PACKET_MAX),
};

#else // >4 interfaces

#error "Unsupported number of CDC device interfaces"

#endif

// list of configurations
const sUsbDescCfg* UsbDescCdcCfgList[1] = { (const sUsbDescCfg*)&UsbDevCdcDescCfg, };

// application device setup descriptor
const sUsbDevSetupDesc UsbDevCdcSetupDesc =
{
	// descriptors
	.desc_dev = &UsbDevCdcDescDev,		// pointer to device descriptor
	.desc_cfg_list = UsbDescCdcCfgList,	// pointer to list of configuration descriptors
	.desc_cfg_num = 1,			// number of configuration descriptors in the list
	.desc_bos = NULL,			// binary device object store (BOS) descriptor (NULL = not used)
	.desc_list = NULL,			// list of additional descriptors (HID: list of report descriptors)

	// callbacks
	.mounted_cb = NULL,			// device is mounted callback (NULL = not used)
	.suspend_cb = NULL,			// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	.resume_cb = NULL,			// resume callback (NULL = not used)
	.str_cb = NULL,				// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	.str_list = UsbDevCdcDescStr,		// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	.str_num = 4,				// number of ASCIIZ strings in the list
};

// ----------------------------------------------------------------------------
//                        Common functions
// ----------------------------------------------------------------------------
// Can be called from both interrupt service and from application.

// check if device is mounted
//  cdc_inx ... CDC interface
Bool UsbDevCdcInxIsMounted(u8 cdc_inx)
{
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];
	return cdc->used && UsbIsMounted() && (cdc->ep_in != 0) && (cdc->ep_out != 0);
}

// find interface by interface number (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbDevCdcFindItf(u8 itf_num)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_DEV_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

		// check interface
		if (cdc->used && (cdc->itf_num == itf_num))
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

// find interface by endpoint (returns interface index, or USB_DRVID_INVALID if not found)
u8 UsbDevCdcFindAddr(u8 epinx)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_DEV_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

		// check interface
		if (cdc->used && ((cdc->ep_in == epinx) || (cdc->ep_out == epinx)))
		{
			return cdc_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// allocate new interface (returns interface index, or USB_DRVID_INVALID on error)
u8 UsbDevCdcNewItf(const sUsbDescItf* itf)
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_DEV_CDC; cdc_inx++)
	{
		// pointer to CDC interface
		sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

		// check if interface is not used
		if (!cdc->used)
		{
			cdc->used = True; // interface is used
			cdc->itf_num = itf->itf_num; // interface number
			return cdc_inx;
		}
	}
	return USB_DRVID_INVALID;
}

// send data from ring buffer
//  cdc_inx ... CDC interface
void UsbDevCdcInxSend(u8 cdc_inx)
{
	// check if interface is mounted
	if (!UsbDevCdcInxIsMounted(cdc_inx)) return;

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

	// get endpoint
	sEndpoint* sep = &UsbEndpoints[cdc->ep_in];

	// get packet size
	u16 pktmax = sep->pktmax;

	// check if any data to send
	u16 count = (u16)RingNum(&cdc->tx_ring); // count of data in ring buffer
	if (count == 0) return; // no data to send

	// ISO transfer - send whole packet
	if (sep->xfer == USB_XFER_ISO) if (count < pktmax) return; // must send whole packet

	// check if endpoint is busy
	if (UsbEpIsBusy(cdc->ep_in)) return; // transmission is active

	// send data (better to align size to whole packets to maximize performance)
	if (count > pktmax)
	{
		if (sep->xfer == USB_XFER_ISO) // limit ISO to 1 long packet
			count = pktmax;
		else
			count = count/pktmax*pktmax;
	}
	UsbXferStart(cdc->ep_in, &cdc->tx_ring, count, True);
}

// send data of all interfaces
void UsbDevCdcAllSend()
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_DEV_CDC; cdc_inx++)
	{
		UsbDevCdcInxSend(cdc_inx);
	}
}

// receive data to ring buffer
//  cdc_inx ... CDC interface
void UsbDevCdcInxRecv(u8 cdc_inx)
{
	// check if interface is mounted
	if (!UsbDevCdcInxIsMounted(cdc_inx)) return;

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

	// get endpoint
	sEndpoint* sep = &UsbEndpoints[cdc->ep_out];

	// get packet size
	u16 pktmax = sep->pktmax;

	// get free space in receive ring buffer
	u16 space = (u16)RingFree(&cdc->rx_ring);
	if (space < pktmax) return; // no free space

	// check if endpoint is busy
	if (UsbEpIsBusy(cdc->ep_out)) return;

	// start receiving data packet ... we must use whole packet or its multiply, or some data could be lost
	if (space > pktmax)
	{
		if (sep->xfer == USB_XFER_ISO) // limit ISO to 1 long packet
			space = pktmax;
		else
			space = space/pktmax*pktmax;
	}
	UsbXferStart(cdc->ep_out, &cdc->rx_ring, space, True);
}

// receive data of all interfaces
void UsbDevCdcAllRecv()
{
	u8 cdc_inx;
	for (cdc_inx = 0; cdc_inx < USE_USB_DEV_CDC; cdc_inx++)
	{
		UsbDevCdcInxRecv(cdc_inx);
	}
}

// initialize class driver
void UsbDevCdcInit()
{
	memset(UsbDevCdcInter, 0, sizeof(UsbDevCdcInter));

	u8 i;
	for (i = 0; i < USE_USB_DEV_CDC; i++)
	{
		// pointer to CDC interface
		sUsbDevCdcInter* cdc = &UsbDevCdcInter[i];

		// default line coding
		cdc->line_coding.bit_rate = 115200;	// 115200 Baud
		cdc->line_coding.stop_bits = 0;		// 1 stop bit
		cdc->line_coding.parity = 0;		// no parity
		cdc->line_coding.data_bits = 8;		// 8 data bits

		// setup ring buffers
		RingInit(&cdc->rx_ring, cdc->rx_ring_buf, USB_DEV_CDC_RX_BUFSIZE, USB_SPIN, NULL, NULL, 0); // RX buffer
		RingInit(&cdc->tx_ring, cdc->tx_ring_buf, USB_DEV_CDC_TX_BUFSIZE, USB_SPIN, NULL, NULL, 0); // TX buffer
	}
}

// terminate class driver
void UsbDevCdcTerm()
{

}

// reset class driver
void UsbDevCdcReset()
{
	u8 i;
	for (i = 0; i < USE_USB_DEV_CDC; i++)
	{
		// pointer to CDC interface
		sUsbDevCdcInter* cdc = &UsbDevCdcInter[i];

		cdc->used = False;	// used
		cdc->itf_num = 0;	// interface number
		cdc->ep_in = 0;		// endpoint index for input
		cdc->ep_out = 0;	// endpoint index for output
		cdc->line_state = 0;	// linestate: bit 0 DTR (Data Terminal Ready), bit 1 RTS (Request To Send)

		RingClear(&cdc->rx_ring); // clear RX buffer
		RingClear(&cdc->tx_ring); // clear TX buffer
	}
}

// open device class interface (returns size of used interface, 0=not supported)
u16 UsbDevCdcOpen(const sUsbDescItf* itf, u16 max_len)
{
	// check interface class, check subclass "abstract control model"
	if ((itf->itf_class != USB_CLASS_CDC) || (itf->itf_subclass != 2)) return 0;

	// check protocol (only AT commands, not Ethernet)
	if (itf->itf_protocol > 6) return 0;

	// check descriptor size
	if (USB_DESC_LEN(itf) > max_len) return 0;

	// allocate new interface
	u8 cdc_inx = UsbDevCdcNewItf(itf);
	if (cdc_inx == USB_DRVID_INVALID) return 0;

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

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
		if (!UsbDevOpenEpPair(p_desc, 2, &cdc->ep_out, &cdc->ep_in)) return 0;

		// skip endpoint descriptors
		drv_len += 2*sizeof(sUsbDescEp);
	}

	// receive data to ring buffer
	UsbDevCdcInxRecv(cdc_inx);

	return drv_len;
}

// process control transfer complete (returns False to stall)
Bool UsbDevCdcCtrl(u8 stage)
{
	// setup request packet
	sUsbSetupPkt* setup = &UsbSetupRequest;

	// handle class request only
	if (((setup->type >> 5) & 3) != USB_REQ_TYPE_CLASS) return False;

	// find interface by interface number
	u8 cdc_inx = UsbDevCdcFindItf((u8)setup->index);
	if (cdc_inx == USB_DRVID_INVALID) return False; // interface not found

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

	switch (setup->request)
	{
	// set line coding
	case 0x20:
	// get line coding
	case 0x21:
		if (stage == USB_STAGE_SETUP)
		{
			UsbDevSetupStart(&cdc->line_coding, sizeof(sUsbDevCdcLineCoding));
		}
		break;

	// set control line state
	case 0x22:
		if (stage == USB_STAGE_SETUP)
		{
			// set new line state
			cdc->line_state = (u8)setup->value;

			// acknowledging at Status Stage
			UsbDevSetupAck();
		}
		break;

	// send break
	case 0x23:
		if (stage == USB_STAGE_SETUP)
		{
			// acknowledging at Status Stage
			UsbDevSetupAck();
		}
		break;

	// unsupported request
	default:
		return False;
	}

	return True;
}

// process data transfer complete
void UsbDevCdcComp(u8 epinx, u8 xres, u16 len)
{
	// find interface by endpoint
	u8 cdc_inx = UsbDevCdcFindAddr(epinx);
	if (cdc_inx == USB_DRVID_INVALID) return; // invalid endpoint

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

	// receive new data
	if (cdc->ep_out == epinx)
	{
		// receive next data
		UsbDevCdcInxRecv(cdc_inx);
	}

	// data sent to host
	else
	{
		// get packet size
		u16 pktmax = UsbEndpoints[epinx].pktmax;

		// if no more data and last packet was full data packet, mark end of transmission
		if (	(len > 0) && // last sent data was not 0
			(RingNum(&cdc->tx_ring) == 0) && // no new data to send
			(len == (len/pktmax*pktmax))) // last packet was full (was not marked as "last")
		{
			// set zero-length packet ZLP, to mark end of data
			UsbXferStart(epinx, NULL, 0, False);
		}

		// send next data
		else
			UsbDevCdcInxSend(cdc_inx);		
	}
}

// schedule driver, synchronized packets in frames
void UsbDevCdcSof(u16 sof)
{
	// receive data of all interfaces
	UsbDevCdcAllRecv();

	// send data of all interfaces
	UsbDevCdcAllSend();
}

/*
// schedule driver (raised with UsbRescheduleDrv())
void UsbDevCdcSched()
{
}
*/

// ----------------------------------------------------------------------------
//                        Application API functions
// ----------------------------------------------------------------------------
// All functions are called from application.

// read one character from CDC interface (returns 0 if not ready)
char UsbDevCdcInxReadChar(u8 cdc_inx)
{
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx]; // pointer to CDC interface
	return RingReadChar(&cdc->rx_ring); // receive character
}

// read data from CDC interface (returns number of bytes)
int UsbDevCdcInxReadData(u8 cdc_inx, void* buf, int bufsize)
{
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx]; // pointer to CDC interface
	return (int)RingReadData(&cdc->rx_ring, buf, bufsize); // read data
}

// write one character to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbDevCdcInxWriteChar(u8 cdc_inx, char ch)
{
	// device is not mounted
	if (!UsbDevCdcInxIsMounted(cdc_inx)) return False;

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

	// send character
	while (!RingWrite8(&cdc->tx_ring, ch))
	{
		// device is not mounted
		if (!UsbDevCdcInxIsMounted(cdc_inx)) return False;
	}
	return True;
}

// write data to CDC interface, wait until ready (returns False if device is not connected)
Bool UsbDevCdcInxWriteData(u8 cdc_inx, const void* buf, int len)
{
	char ch;
	const char* s = (const char*)buf;

	// pointer to CDC interface
	sUsbDevCdcInter* cdc = &UsbDevCdcInter[cdc_inx];

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
		if (!UsbDevCdcInxWriteChar(cdc_inx, ch)) return False;
	}
	return True;
}

#endif // USE_USB_DEV_CDC	// use USB CDC Communication Device Class, value = number of interfaces (device)
#endif // USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
