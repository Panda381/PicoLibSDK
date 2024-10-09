
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

#include "../../global.h"	// globals

#if USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)

#include "../inc/sdk_irq.h"
#include "../inc/sdk_reset.h"
#include "../inc/sdk_timer.h"
#include "../../_lib/inc/lib_ring.h"	// data ring buffer
#include "../usb_inc/sdk_usb_phy.h"
#include "../usb_inc/sdk_usb_dev.h" // devices
#include "../usb_inc/sdk_usb_host.h"

// Device: 32 descriptors of endpoints 0..15, separate IN and OUT (endpoint 0 and 1 = SETUP packets)
// Host: 16 descriptors of endpoints 0..15 (used for IN and OUT), 0=SETUP endpoint, 1..15=interrupt endpoints
sEndpoint UsbEndpoints[USB_ENDPOINT_NUM2]; // size 32*32=1024 bytes

// data of SETUP transfer
sUsbSetupPkt UsbSetupRequest; // (size 8 bytes) setup request packet, saved in SETUP stage
u8* UsbSetupDataBuff;	// pointer to setup request data buffer
u16 UsbSetupDataLen;		// total length of data in setup data buffer
u16 UsbSetupDataXfer;	// already transferred data in setup data buffer

// host mode (or device mode otherwise)
Bool UsbHostMode;

// device data
volatile u8 UsbDevCfgNum = 0;		// current active configuration (0 = not configured, >0 = device is mounted)

// USB is initialized
Bool UsbDevIsInit = False; // device is initialized
Bool UsbHostIsInit = False; // host is initialized
Bool UsbProcessSOFEvent;			// internal - some driver uses SOF function

// memory map (64 pages of 64-byte pages = 4 KB, bit 1 = page is used)
u64 UsbRamMap;

// ----------------------------------------------------------------------------
//                               Utilities
// ----------------------------------------------------------------------------

// unaligned memcpy (libc memcpy on RP2350 does unaligned access, but DPRAM does not support it)
void UsbMemcpy(void* dst, const void* src, u32 n)
{
	u8* d = (u8*)dst;
	const u8* s = (const u8*)src;
	while (n--) *d++ = *s++;
}

// interrupt enable
void UsbIntEnable()
{
	NVIC_IRQEnable(IRQ_USBCTRL);
}

// interrupt disable
void UsbIntDisable()
{
	NVIC_IRQDisable(IRQ_USBCTRL);
}

// connect device to USB bus
void UsbDevConnect()
{
	// enable pull-up resistor
	RegSet(USB_SIE_CTRL, B16);
}

// disconnect device from USB bus
void UsbDevDisconnect()
{
	// disable pull-up resistor
	RegClr(USB_SIE_CTRL, B16);
}

// get device speed (returns USB_SPEED_*)
u8 UsbGetDevSpeed()
{
	return (u8)((*USB_SIE_STATUS >> 8) & 0x03);
}

// claim endpoint by class driver (returns False if cannot be claimed - already claimed or still active)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
Bool UsbEpClaim(u8 epinx)
{
	Bool res;

	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// fast check
	if (sep->active || sep->claimed) return False;

	// lock spinlock
	SPINLOCK_LOCK(USB_SPIN);

	// check result (must not be active and must not be claimed)
	res = !sep->active && !sep->claimed;

	// claim endpoint
	if (res) sep->claimed = True;

	// unlock spinlock
	SPINLOCK_UNLOCK(USB_SPIN);

	return res;
}

// ----------------------------------------------------------------------------
//                              Transfer control
// ----------------------------------------------------------------------------

// delay 12 system ticks (needed before setting bit to start transmission)
NOINLINE void UsbDelay12()
{
	// delay 12 clock cycles
	nop2();
	nop2();
	nop2();
	nop2();
	nop2();
	nop2();
}

// prepare transfer of one buffer (returns control word)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  bufid ... buffer ID 0 or 1
u32 UsbNextPrep(u8 epinx, u8 bufid)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// prepare remaining data size to transfer
	u32 val = sep->rem_len; // remaining length of data
	if (val > sep->pktmax) val = sep->pktmax; // limit to max. packet size

	// shift remaining length of data
	sep->rem_len -= val;

	// send: copy data from user buffer to USB data buffer
	if (!sep->rx)
	{
		// check buffers and length
		u8* data_buf = sep->data_buf; // data buffer in USB DPRAM (NULL = not enough DPRAM)
		void* user_buf = sep->user_buf; // user buffer (NULL = destroy data)
		if ((user_buf != NULL) && (data_buf != NULL) && (val > 0))
		{
			data_buf += bufid*USB_PACKET_MAX;

			// copy data
			if (sep->use_ring)
				// read data from ring buffer
				RingReadData((sRing*)user_buf, data_buf, val);
			else
				// use fixed buffer
				UsbMemcpy(data_buf, (u8*)user_buf + sep->xfer_len, val);
		}

		// mark buffer as full
		val |= B15; // set flag "buffer 0 is full"
	}

	// select PID (packet identification)
	if (sep->xfer != USB_XFER_ISO) val |= (u32)sep->next_pid << 13; // select PID = DATA0 or DATA1 (Isochronous mode uses always DATA0)
	sep->next_pid ^= 1;

	// host: set "last buffer" flag
	if (sep->rem_len == 0) val |= B14;

	return val;
}

// start transfer of next endpoint buffer
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
// It is safe to access different endpoints in a multitask environment.
void UsbNextBuf(u8 epinx)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// pointer to buffer control register
	volatile u32* buf_ctrl = sep->buf_ctrl;

	// reset buffer select to buffer 0
	*buf_ctrl = B12;

	// prepare buffer 0
	u32 val = UsbNextPrep(epinx, 0);

	// reset buffer select to buffer 0
	val |= B12;

	// when force single-buffer mode?
	// - in OUT endpoint of device mode, because host could send short packet < 64 bytes (lwip server)
	// - in interrupt endpoint of host mode
	Bool force_single = (!UsbHostMode && (USB_EPADDR_DIR(epinx) == USB_DIR_OUT)) ||
				(UsbHostMode && (epinx > 0));

	// get endpoint control (note: for device EP0 we use USB_SIE_CTRL)
	u32 epctrl = *sep->ep_ctrl;

	// prepare buffer 1
	if ((sep->rem_len > 0) && !force_single)
	{
		// prepare buffer 1
		val |= (UsbNextPrep(epinx, 1) | B10) << 16;

		// set endpoint control to double-buffer
		epctrl &= ~B29;		// clear single-buffer mode
		epctrl |= B30 | B28;	// set double-buffer mode
	}
	else
	{
		// set endpoint control to single-buffer
		epctrl |= B29;		// set single-buffer mode
		epctrl &= ~(B30 | B28);	// clear double-buffer mode
	}

	// set new endpoint control
	*sep->ep_ctrl = epctrl;

	// set buffer control - start transfer
	*buf_ctrl = val;
	UsbDelay12(); // short delay 12 system ticks
	*buf_ctrl = val | B10; // set "buffer available" flag
}

// start new transfer
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  buf ... pointer to user buffer or pointer to ring buffer
//  len ... length of data
//  use_ring ... use ring buffer (buf is pointer to sRing ring buffer, must be filled with 'len' data)
void UsbXferStart(u8 epinx, void* buf, u16 len, Bool use_ring)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// stop current transfer
	*sep->buf_ctrl = 0;

	// fill up
	sep->active = True;	// transfer is active (busy; i.e. endpoint is used to transfer)
	dmb();			// data memory barrier (to be able to safely test the 'active' flag)
	sep->rem_len = len;	// remaining length of data to transfer
	sep->total_len = len;	// total length of data to transfer
	sep->xfer_len = 0;	// length of already transferred data
	sep->user_buf = buf;	// pointer to user buffer, or pointer to sRing ring buffer
	sep->use_ring = use_ring; // use ring buffer

	// start transfer of next endpoint buffer
	UsbNextBuf(epinx);
}

// synchronize one completed buffer (returns number of transferred bytes)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
//  bufid ... buffer ID 0 or 1
u16 UsbXferSync(u8 epinx, u8 bufid)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// get register
	u32 bufctrl = *sep->buf_ctrl;
	if (bufid != 0) bufctrl >>= 16;

	// get number of transferred bytes
	int len = bufctrl & 0x3ff;

	// limit number of bytes
	if ((int)sep->xfer_len + len > (int)sep->total_len) len = (int)sep->total_len - (int)sep->xfer_len;

	// receive data - copy received data to user buffer
	if (sep->rx && (len > 0))
	{
		u8* data_buf = sep->data_buf; // data buffer in USB DPRAM (NULL = not enough DPRAM)
		void* user_buf = sep->user_buf; // user buffer (NULL = destroy data)
		if ((user_buf != NULL) && (data_buf != NULL))
		{
			data_buf += bufid*USB_PACKET_MAX;

			// copy data
			if (sep->use_ring)
				// write data to ring buffer
				RingWriteData((sRing*)user_buf, data_buf, len);
			else
				// use fixed buffer
				UsbMemcpy((u8*)user_buf + sep->xfer_len, data_buf, len);
		}
	}

	// shift transferred data
	sep->xfer_len += len;

	// short received data = end of transmission (less data may be received than requested)
	if (len < sep->pktmax)
	{
		sep->rem_len = 0;
		sep->total_len = sep->xfer_len;
	}

	return len;
}

// transfer continue (returns True if transfer is complete)
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
Bool UsbXferCont(u8 epinx)
{
	// synchronize buffer 0
	/*u16 len =*/ UsbXferSync(epinx, 0);

	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// synchronize buffer 1 (if some data remain and if we used double-buffer mode)
	if ((sep->xfer_len < sep->total_len) && ((*sep->ep_ctrl & B30) != 0)) UsbXferSync(epinx, 1);

	// end of transmission
	if (sep->rem_len == 0) return True;

	// start transfer of next endpoint buffer
	UsbNextBuf(epinx);

	// transfer not complete
	return False;
}

// ----------------------------------------------------------------------------
//                                Initialize
// ----------------------------------------------------------------------------

// allocate RAM buffer (returns NULL on error; size is aligned to 64 bytes)
u8* UsbRamAlloc(int size)
{
	// check minimal size
	if (size <= 0) return NULL;

	// align size to 64 bytes
	size = (size + 63) & ~63;

	// number of pages
	int pages = size / 64;

	// find stream of free pages
	u64 mask, mask2, map;
	map = UsbRamMap; // RAM map
	mask = 1;
	int i, j;
	for (i = 0; i <= 64 - pages; i++)
	{
		mask2 = mask;

		// check this stream
		for (j = 0; j < pages; j++)
		{
			// check this page
			if ((map & mask2) != 0) break;

			// shift mask
			mask2 <<= 1;
		}

		// page stream has been found OK
		if (j == pages)
		{
			// mark this stream as used
			mask2 = mask;
			for (j = 0; j < pages; j++)
			{
				map |= mask2;
				mask2 <<= 1;
			}

			// store new RAM map
			UsbRamMap = map;

			// return result address
			return (u8*)(USB_RAM + i*64);
		}


		// shift mask
		mask <<= 1;
	}

	// memory error
	return NULL;
}

// deallocate RAM buffer previously allocated by UsbRamAlloc (addr can be NULL)
void UsbRamFree(u8* addr, int size)
{
	// check address
	if ((addr == NULL) || (size <= 0)) return;

	// align size to 64 bytes
	size = (size + 63) & ~63;

	// number of pages
	int pages = size / 64;

	// start index
	int i = (addr - USB_RAM)/64;
	u32 mask = 1ull << i;

	// mark this stream as used
	u32 map = UsbRamMap; // RAM map
	int j;
	for (j = 0; j < pages; j++)
	{
		map &= ~mask;
		mask <<= 1;
	}

	// store new RAM map
	UsbRamMap = map;
}

// terminate endpoint
//  epinx ... endpoint index 0..31 (device) or 0..15 (host)
void UsbEpTerm(u8 epinx)
{
	// address of endpoint descriptor
	sEndpoint* sep = &UsbEndpoints[epinx];

	// if configured
	if (sep->used)
	{
		// clear buffer control
		*sep->buf_ctrl = 0;

		// clear control register
		if ((sep->ep_ctrl != NULL) && (sep->ep_ctrl != USB_SIE_CTRL)) *sep->ep_ctrl = 0;

		// deallocate memory
		UsbRamFree(sep->data_buf, sep->pktmax);

		// clear endpoint descriptor
		memset(sep, 0, sizeof(sEndpoint));
	}
}

// bind endpoints to driver
//  ep2drv ... mapping array of size USB_ENDPOINT_NUM2
//  p_desc ... pointer to interface descriptor
//  desc_len ... max. length of descriptors
//  drv_id ... driver index 
void UsbEpBindDrv(u8* ep2drv, const u8* p_desc, u16 desc_len, u8 drv_id)
{
	// end of descriptors
	const u8* p_end = p_desc + desc_len;

	// search interfaces
	while (p_desc < p_end)
	{
		// check if endpoint descriptor has been found
		if (USB_DESC_TYPE(p_desc) == USB_DESC_ENDPOINT)
		{
			// get endpoint address in USB format
			u8 ep_addr = ((const sUsbDescEp*)p_desc)->ep_addr;

			// convert endpoint address to endpoint index
			u8 ep_inx = USB_EPADDR_EPINX(ep_addr);

			// set remapping endpoint to driver
			ep2drv[ep_inx] = drv_id;
		}

		// shift address
		p_desc = USB_DESC_NEXT(p_desc);
	}
}

// USB terminate
void UsbTerm()
{
	// disable interrupts
	*USB_INTE = 0;
	*USB_INT_EP_CTRL = 0;

	// interrupt disable
	UsbIntDisable();

	// disconnect device from USB bus
	UsbDevDisconnect();

	// disable endpoints
	u8 i;
	for (i = 0; i < USB_ENDPOINT_NUM2; i++)
	{
		*USB_EP_CTRL(i) = 0; // epinx < 2 clears SETUP packet, it is OK
		*USB_EP_BUF(i) = 0;
	}
	*USB_EPX_CTRL = 0;

#if USE_USB_DEV		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
	// terminate class drivers, if device was initialized
	if (UsbDevIsInit) for (i = 0; i < UsbDevDrvNum; i++) UsbDevDrv[i].term();
#endif

#if USE_USB_HOST		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
	// terminate class drivers, if host was initialized
	if (UsbHostIsInit) for (i = 0; i < UsbHostDrvNum; i++) UsbHostDrv[i].term();
#endif

	// USB is not initialized
	UsbDevCfgNum = 0; // not mounted
	UsbDevIsInit = False; // device not initialized
	UsbHostIsInit = False; // host not initialized

	// hard reset USB periphery
	ResetPeriphery(RESET_USBCTRL);

	// reset base variables
	UsbDevCfgNum = 0;		// current active configuration (0 = not configured)
}

// Base USB init (common part for device and host)
void UsbPhyInit()
{
	// USB terminate (and hard reset USB periphery)
	UsbTerm();

	// clear any previous state
	memset((void*)USBCTRL_REGS_BASE, 0, 0x9c); // clear USB registers
	memset((void*)USBCTRL_DPRAM_BASE, 0, 0x180); // clear buffer registers

	// clear endpoint descriptors
	memset((void*)UsbEndpoints, 0, sizeof(UsbEndpoints));

	// memory map (64 pages of 64-byte pages = 4 KB, bit 1 = page is used), 6 pages 0x000..0x17F are reserved
	UsbRamMap = 0x3full;

	// set mux to onboard usb phy (B0: TO_PHY, B3: SOFTCON)
	*USB_MUXING = B0 | B3;		// select muxing to TO_PHY and SOFTCON

	// override power signals to detect VBUS signal, so the device thinks it is plugged into a host
	//  B2: VBUS detect, B3: enable "VBUS detect" override
	*USB_PWR = B2 | B3;		// override VBUS detect
}

#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
