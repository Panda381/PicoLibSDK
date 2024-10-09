
// ****************************************************************************
//
//                               USB Mini-Port
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

#include "../../global.h"	// globals

#if USE_USBPORT		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)

#include "../inc/sdk_cpu.h"	// dmb()
#include "../inc/sdk_reset.h"
#include "../../_lib/inc/lib_crc.h"
#include "../usb_inc/sdk_usbport.h"

#define USBPORT_DEVADDR	0	// used device address
#define USBPORT_XFER	2	// used transfer type: 0 control, 1 isochronous, 2 bulk, 3 interrupt
#define USBPORT_INTER	0	// interrupt interval - 1

// ----------------------------------------------------------------------------
//                               Data
// ----------------------------------------------------------------------------

// selected USB port
u8 UsbPort = USBPORT_OFF;	// current selected port (0=not initialized)

// some transfers have started
Bool UsbPortStarted = 0;

// flags of active transfers
u16 UsbPortActive = 0;

// ----------------------------------------------------------------------------
//                              Initialize
// ----------------------------------------------------------------------------

// unaligned memcpy (libc memcpy on RP2350 does unaligned access, but DPRAM does not support it)
void UsbPortMemcpy(void* dst, const void* src, u32 n)
{
	u8* d = (u8*)dst;
	const u8* s = (const u8*)src;
	while (n--) *d++ = *s++;
}

// get DPRAM buffer address of the channel (64 bytes long + 64 bytes reserve after it)
//  chan ... channel 0..15
u8* UsbPortBuf(u8 chan)
{
	// host mode
	if (UsbPort == USBPORT_MASTER) return USB_RAM_DATA + chan*128;

	// device mode
	else return USB_EP0_BUF0 + chan*128;
}

// delay 12 system ticks (needed before setting bit to start transmission)
NOINLINE static void UsbPortDelay() { nop2(); nop2(); nop2(); nop2(); nop2(); nop2(); }

// initialize USB port
//  port ... use 1=USBPORT_MASTER or 2=USBPORT_SLAVE
void UsbPortInit(u8 port)
{
	// terminate USB port
	UsbPortTerm();	

	// current selected port (0=not initialized)
	if (port == 0) return;
	UsbPort = port;

	// set mux to onboard usb phy (B0: TO_PHY, B3: SOFTCON)
	*USB_MUXING = B0 | B3;		// select muxing to TO_PHY and SOFTCON

	// override power signals to detect VBUS signal, so the device thinks it is plugged into a host
	//  B2: VBUS detect, B3: enable "VBUS detect" override
	*USB_PWR = B2 | B3;		// override VBUS detect

	// device address and endpoint 0
	*USB_ADDR_ENDP = USBPORT_DEVADDR;

	// pre-set - first packet will start with DATA0
	u8 i;
	for (i = 0; i < USB_ENDPOINT_NUM2; i++) *USB_EP_BUF(i) = B13;

	// host mode
	if (port == USBPORT_MASTER)
	{
		// initialize USB peripheral for host mode
		*USB_MAIN_CTRL = B0 | B1;	// enable controller, use host mode

		// SIE control: B8 SOF_SYNC, B9 SOF_EN, B10 KEEP_ALIVE_EN, B15 PULLDOWN_EN, B29 EP0_INT_1BUF
		*USB_SIE_CTRL = B9 | B10 | B15 | B29;
	}

	// device mode
	else
	{
		// initialize USB peripheral for device mode
		*USB_MAIN_CTRL = B0;	// enable controller, use device mode

		// set BUFF_STATUS bit for every buffer EP0 completed, EP0 single buffered, B16 enable pull-up resistor
		*USB_SIE_CTRL = B16 | B29;
	}
}

// terminate USB port
void UsbPortTerm()
{
	// disconnect device from USB bus, disable pull-up resistor
	*USB_SIE_CTRL = 0;

	// disable interrupt endpoint register
	*USB_INT_EP_CTRL = 0;

	// current selected port (0=not initialized)
	UsbPort = USBPORT_OFF;

	// disable endpoints
	u8 i;
	for (i = 0; i < USB_ENDPOINT_NUM2; i++)
	{
		*USB_EP_CTRL(i) = 0; // i < 2 clears SETUP packet, it is OK
		*USB_EP_BUF(i) = 0;
	}
	*USB_EPX_CTRL = 0;
	*USB_BUFF_STATUS = 0;

	// hard reset USB periphery
	ResetPeriphery(RESET_USBCTRL);

	// clear flag of some transfers
	UsbPortStarted = 0;

	// clear flags of active transfers
	UsbPortActive = 0;
}

// check if opposite device is connected
Bool UsbPortConnected()
{
	Bool con;

	// host mode
	if (UsbPort == USBPORT_MASTER) con = (*USB_SIE_STATUS & (B8+B9)) != 0; // device speed, 0=disconnected

	// device mode
	else if (UsbPort == USBPORT_SLAVE) con = (*USB_SIE_STATUS & B4) == 0; // suspended

	// not initialized
	else con = False;

	// not connected - reset transfers (to synchronise DATA0/DATA1 toggle)
	if (!con && UsbPortStarted) UsbPortInit(UsbPort);

	return con;
}

// ----------------------------------------------------------------------------
//                         Multi-channel transfer
// ----------------------------------------------------------------------------

// start sending or receiving packet
//  chan ... channel 1..15
//  buf ... buffer with data to send (ignored if receiving; buf can be equal ram buffer address)
//  len ... length of data to send, or max. size of data to receive (0..64 bytes)
//  send ... send data (or receive otherwise)
// Returns:
//   USBPORT_ERR_OK ... transfer started OK
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
static u8 UsbPortStart(u8 chan, const void* buf, int len, Bool send)
{
	// get channel state
	u8 res = UsbPortState(chan);
	if ((res != USBPORT_ERR_OK) && (res != USBPORT_ERR_COMP)) return res;

	// stop current transfer
	volatile u32* bc = USB_EP_BUF(chan*2);
	*bc = *bc & B13;
	bc++;
	*bc = *bc & B13;

	// clear complete status
	UsbPortClear(chan);

	// set first started flag
	UsbPortStarted = True;

	// set flag of active transfer
	UsbPortActive |= BIT(chan);

	// prepare DPRAM buffer address
	u8* ram = UsbPortBuf(chan);

	// prepare endpoint index for device mode (direction is opposite to the host)
	u8 epinx = USB_EPINX(chan, send ? USB_DIR_IN : USB_DIR_OUT);

	// host mode
	u32 val;
	if (UsbPort == USBPORT_MASTER)
	{
		// setup endpoint control register
		volatile u32* cr = (chan == 0) ? USB_EPX_CTRL : USB_EP_INTCTRL(chan);
		*cr =	(ram - USB_RAM) |	// offset of data buffer in DPRAM
			(USBPORT_XFER << 26) |	// transfer type
			B29 |			// enable interrupt for every transferred single-buffer
			(USBPORT_INTER << 16) |	// interval to poll this endpoint in [ms]
			B31;			// endpoint enable

		// setup interrupt endpoint, direction OUT
		if (chan > 0)
		{
			// set interrupt endpoint address and channel
			val = USBPORT_DEVADDR | (chan << 16);
			if (send) val |= B25; // send
			*USB_ADDR_ENDPN(chan) = val;

			// enable interrupt control register
			RegSet(USB_INT_EP_CTRL, BIT(chan));
		}
	}

	// device mode
	else
	{
		// setup endpoint control register (not for endpoint 0)
		if (chan > 0)
		{
			*USB_EP_CTRL(epinx) =
				(ram - USB_RAM) |	// offset of data buffer in DPRAM
				(USBPORT_XFER << 26) |	// transfer type
				B29 |			// enable interrupt for every transferred single-buffer
				B31;			// endpoint enable
		}
	}

	// copy data to DPRAM buffer
	if (send && (ram != buf)) UsbPortMemcpy(ram, buf, len);

	// pointer to buffer control register
	bc = (UsbPort == USBPORT_MASTER) ? USB_EP_INTBUF(chan) : USB_EP_BUF(epinx);

	// prepare next DATA toggle
	int pid = ((*bc >> 13) & 1) ^ 1;

	// setup buffer control register
	val =	len |			// length of data to send
		(pid << 13) |		// select DATA0 or DATA1
		B12;			// reset buffer select to buffer 0
	if (send) val |= B14 | B15;	// flag "last buffer" and "buffer 0 is full"

	// start transfer
	*bc = val;				// setup buffer
	UsbPortDelay();				// short delay
	*bc = val | B10;			// set "buffer available" flag

	// setup endpoint 0 for host mode
	if ((UsbPort == USBPORT_MASTER) && (chan == 0))
	{
		// SIE control: B0 START_TRANS, B2 send, B3 receive, B8 SOF_SYNC, B9 SOF_EN,
		//	 B10 KEEP_ALIVE_EN, B15 PULLDOWN_EN, B29 EP0_INT_1BUF
		u32 val = B9 | B10 | B15 | B29;
		val |= send ? B2 : B3;		// send or receive flag
		*USB_SIE_CTRL = val;		// setup register
		UsbPortDelay();			// short delay
		*USB_SIE_CTRL = val | B0;	// start transaction
	}

	// transfer OK
	return USBPORT_ERR_OK;
}

// get channel state
//  chan ... channel 1..15
// Returns:
//   USBPORT_ERR_OK ... channel is waiting and not active
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
//   USBPORT_ERR_COMP ... transfer is complete (it may also mean that it was never launched)
u8 UsbPortState(u8 chan)
{
	// check if driver is initialized
	if (UsbPort == USBPORT_OFF) return USBPORT_ERR_INIT;

	// check if device is connected
	if (!UsbPortConnected()) return USBPORT_ERR_CON;

	// check buffer status, if previous transfer is complete
	Bool comp = ((*USB_BUFF_STATUS & (3 << (chan*2))) != 0);

	// clear flag of active transfer when transfer is complete
	if (comp) UsbPortActive &= ~BIT(chan);

	// channel is busy
	if ((UsbPortActive & BIT(chan)) != 0) return USBPORT_ERR_BUSY;

	// transfer is complete
	if (comp) return USBPORT_ERR_COMP;

	// all OK, channel is waiting and not active
	return USBPORT_ERR_OK;
}

// reset channel
//  Stop current transmission. Must be executed on both sides to ensure DATA0/DATA1 synchronization.
void UsbPortReset(u8 chan)
{
	// clear buffer registers for both directions to stop old transfers
	// pre-set - first packet will start with DATA0
	volatile u32* bc = USB_EP_BUF(chan*2);
	*bc = B13;
	bc++;
	*bc = B13;

	// clear active flag
	UsbPortActive &= ~BIT(chan);

	// reset buffer status
	UsbPortClear(chan);
}

// clear complete status of the channel
void UsbPortClear(u8 chan)
{
	// reset buffer status
	RegClr(USB_BUFF_STATUS, (3 << (chan*2)));
}


// check if transmit channel is ready to send next packet
Bool UsbPortSendChanReady(u8 chan)
{
	// get channel state
	u8 res = UsbPortState(chan);
	return (res == USBPORT_ERR_OK) || (res == USBPORT_ERR_COMP);
}

// send packet
//  chan ... channel 1..15
//  buf ... buffer with data to send
//  len ... length of data to send (0..64 bytes)
// Returns:
//   USBPORT_ERR_OK ... packet was sent OK
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
u8 UsbPortSendChan(u8 chan, const void* buf, int len)
{
	return UsbPortStart(chan, buf, len, True);
}

// check if next received packet is ready
//  chan ... channel 1..15
Bool UsbPortRecvChanReady(u8 chan)
{
	// get channel state
	u8 res = UsbPortState(chan);
	if ((res == USBPORT_ERR_INIT) || (res == USBPORT_ERR_CON) || (res == USBPORT_ERR_BUSY)) return False;

	// not busy and not complete - start new receive
	if (res == USBPORT_ERR_OK)
	{
		// start new receive
		UsbPortStart(chan, NULL, USB_PACKET_MAX, False);
		return False;
	}

	// complete
	return True;
}

// receive packet
//  chan ... channel 1..15
//  buf ... buffer to load received data
//  len ... max. size of data to receive
// If it returns number between 0 and 64, it is length of received packet.
// If it returns number greater than 64, it is following error code:
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
u8 UsbPortRecvChan(u8 chan, void* buf, int len)
{
	// get channel state
	u8 res = UsbPortState(chan);
	if ((res == USBPORT_ERR_INIT) || (res == USBPORT_ERR_CON) || (res == USBPORT_ERR_BUSY)) return res;

	// not busy and not complete - start new receive
	if (res == USBPORT_ERR_OK)
	{
		// start new receive
		res = UsbPortStart(chan, NULL, USB_PACKET_MAX, False);

		// ok, but now busy
		if (res == USBPORT_ERR_OK) return USBPORT_ERR_BUSY;
		return res;
	}

// --- transfer is complete

	// pointer to buffer control register
	volatile u32* bc = (UsbPort == USBPORT_MASTER) ? USB_EP_INTBUF(chan)
		: USB_EP_BUF(USB_EPINX(chan, USB_DIR_OUT));

	// get size of data
	int n = (*bc & 0x3ff);
	if (n > USB_PACKET_MAX) n = USB_PACKET_MAX;
	if (n > len) n = len;

	if (n > 0)
	{
		// prepare DPRAM buffer address
		u8* ram = UsbPortBuf(chan);

		// copy data
		if (ram != buf) UsbPortMemcpy(buf, ram, n);
	}

	// clear buffer status
	UsbPortClear(chan);

	// length of received packet
	return n;
}

// ----------------------------------------------------------------------------
//                      Transfer of formatted packet
// ----------------------------------------------------------------------------

// send packet
//  chan ... channel 1..15
//  pkt ... buffer with sUsbPortPkt packet to send (crc will be calculated in temporary buffer, len must be filled)
// Returns:
//   USBPORT_ERR_OK ... packet was sent OK
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
//   USBPORT_ERR_LEN ... incorrect length of formatted packet
u8 UsbPortSendPkt(u8 chan, const void* pkt)
{
	// get channel state
	u8 res = UsbPortState(chan);
	if ((res != USBPORT_ERR_OK) && (res != USBPORT_ERR_COMP)) return res;

	// check length
	const sUsbPortPkt* p = (const sUsbPortPkt*)pkt;
	u8 len = p->len;
	if ((len < 4) || (len > 64)) return USBPORT_ERR_LEN;

	// prepare DPRAM buffer address
	sUsbPortPkt* ram = (sUsbPortPkt*)UsbPortBuf(chan);

	// copy packet to DPRAM buffer
	UsbPortMemcpy(ram, p, len);

	// calculate CRC
	ram->crc = Crc16AFast((u8*)ram+2, len-2);

	// send packet
	return UsbPortSendChan(chan, ram, len);
}

// receive packet
//  chan ... channel 1..15
//  pkt ... buffer of size 64 bytes to receive sUsbPortPkt packet
// Packet will appear in destination buffer even if length is wrong or if CRC is wrong.
// If real length of received packet is wrong, it will be stored in length entry of packet.
// Returns:
//   USBPORT_ERR_OK ... packet was received OK
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
//   USBPORT_ERR_LEN ... incorrect length of formatted packet
//   USBPORT_ERR_CRC ... CRC error of received packet
u8 UsbPortRecvPkt(u8 chan, void* pkt)
{
	// receive packet
	u8 len0 = UsbPortRecvChan(chan, pkt, USB_PACKET_MAX);
	if (len0 > 64) return len0; // error

	// check length
	sUsbPortPkt* p = (sUsbPortPkt*)pkt;
	u8 len = p->len;
	if ((len < 4) || (len != len0))
	{
		p->len = len0;
		return USBPORT_ERR_LEN;
	}

	// calculate CRC
	u16 crc = Crc16AFast((u8*)pkt+2, len-2);

	// check CRC
	if (crc != p->crc) return USBPORT_ERR_CRC;

	return USBPORT_ERR_OK;
}

// ----------------------------------------------------------------------------
//                      Single-channel simple transfer
// ----------------------------------------------------------------------------

// check if transmit channel is ready to send next packet
Bool UsbPortSendReady()
{
	// prepare channel to send
	u8 chan = (UsbPort == USBPORT_MASTER) ? USBPORT_OUT : USBPORT_IN;

	// check channel
	return UsbPortSendChanReady(chan);
}

// send packet (returns False on transfer error)
//  pkt ... buffer with sUsbPortPkt packet to send (crc will be calculated, len must be filled)
Bool UsbPortSend(const void* pkt)
{
	// prepare channel to send
	u8 chan = (UsbPort == USBPORT_MASTER) ? USBPORT_OUT : USBPORT_IN;

	// send packet
	u8 res = UsbPortSendPkt(chan, pkt);

	// check result
	return res == USBPORT_ERR_OK;
}

// check if next received packet is ready
Bool UsbPortRecvReady()
{
	// prepare channel to receive
	u8 chan = (UsbPort == USBPORT_MASTER) ? USBPORT_IN : USBPORT_OUT;

	// check channel
	return UsbPortRecvChanReady(chan);
}

// receive packet (returns False if packet not received)
//  pkt ... buffer of size 64 bytes to receive sUsbPortPkt packet
Bool UsbPortRecv(void* pkt)
{
	// prepare channel to receive
	u8 chan = (UsbPort == USBPORT_MASTER) ? USBPORT_IN : USBPORT_OUT;

	// receive packet
	u8 res = UsbPortRecvPkt(chan, pkt);

	// check result
	return res == USBPORT_ERR_OK;
}

#endif // USE_USBPORT		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
