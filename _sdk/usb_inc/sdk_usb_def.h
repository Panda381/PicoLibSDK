
// ****************************************************************************
//
//                              USB Definitions
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

#ifndef _SDK_USB_DEF_H
#define _SDK_USB_DEF_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
//                           Constants and macros
// ----------------------------------------------------------------------------

#define USB_ENDPOINT_NUM	16		// total number of endpoints (0..15)
#define USB_ENDPOINT_NUM2	(USB_ENDPOINT_NUM*2) // total number of endpoints including direction (0..31)
#define USB_SETUP_PKT_SIZE	8		// size of setup packet
#define USB_PACKET_MAX		64		// max. size of normal packet
#define USB_ISO_PACKET_MAX	1023		// max. size of ISO packet ... limited to 64 bytes

#define USB_RAM_SIZE 4096			// size of USB dual-port RAM
#define USB_RAM_DATA_MAX (4096-0x180)		// max. size of USB dual-port RAM allocable data

// Used terms:
//  endpoint index (epinx) = endpoint index used internally in RP2040 registers (range device=0..31 or host=0..15)
//  endpoint address (epaddr) = endpoint address in format used by USB packets (endpoint 0..15 with direction B7=IN)
//  endpoint number (ep) = number of endpoint 0..15 (identical to endpoint interrupt index)
//  endpoint direction (dir) = packet direction USB_DIR_* (0=IN, 1=OUT)

#define USB_DIR_IN	0			// IN packet (from device to host)
#define USB_DIR_OUT	1			// OUT packet (from host to device)

#define USB_EPINX(ep, dir) (((ep) << 1) + (dir)) // convert endpoint number (0..15) and direction (USB_DIR_*) to endpoint index (in RP2040 format)
#define USB_DIR(epinx)	((epinx) & 1)		// get endpoint direction USB_DIR_* from endpoint index
#define USB_EP(epinx)	((epinx) >> 1)		// get endpoint number 0..15 from endpoint index (identical to endpoint interrupt index)
#define USB_EPINX_EPADDR(epinx) ( ((epinx) >> 1) | ((((epinx) & 1) ^ 1) << 7) ) // convert endpoint index (in RP2040 format) to endpoint address (in USB format)

#define USB_EPADDR(ep, dir) ((ep) + (((dir) ^ 1) << 7))	// convert endpoint number (0..15) and direction (USB_DIR_*) to endpoint address (in USB format)
#define USB_EPADDR_DIR(addr)	(((addr) >> 7) ^ 1) // get endpoint direction USB_DIR_* from endpoint address (in USB format)
#define USB_EPADDR_EP(addr)	((addr) & 0x0f)	// get endpoint number (0..15) from endpoint address (in USB format)
#define USB_EPADDR_EPINX(addr) ( (((addr) & 0x0f) << 1) | ((addr >> 7) ^ 1) )	// convert endpoint address (in USB format) to endpoint index (in RP2040 format)

// transfer type
#define USB_XFER_CTRL	0	// control packet
#define USB_XFER_ISO	1	// isochronous
#define USB_XFER_BULK	2	// bulk
#define USB_XFER_INT	3	// interrupt

// transfer result
#define USB_XRES_OK	0	// success
#define USB_XRES_FAIL	1	// failed
#define USB_XRES_STALL	2	// stalled

// device speed
#define USB_SPEED_OFF	0	// disconnected
#define USB_SPEED_LS	1	// low speed
#define USB_SPEED_FS	2	// full speed

#define USB_DRVID_INVALID 0xff	// driver ID is invalid

// setup stage
#define USB_STAGE_IDLE		0	// idle, no setup process
#define USB_STAGE_SETUP		1	// received SETUP packet with request
#define USB_STAGE_DATA		2	// DATA packet transferred
#define USB_STAGE_ACK		3	// transfer completed

// Reschedule USB interrupts (use bit value of some interrupt - interrupt must be enabled)
#define USB_RESCHEDULE_DRV	B4	// (BUFF_STATUS) request to reschedule from class driver
#define USB_RESCHEDULE_TIMER	B3	// (TRANS_COMPLETE) host: request to reschedule from timer
//#define USB_RESCHEDULE_	B18	// (ABORT_DONE) ... reserve, can be used too

// ----------------------------------------------------------------------------
//                            USB DPRAM registers
// ----------------------------------------------------------------------------
// Do not use atomic access to DPRAM, it does not work (RegSet, RegClr, RegXor).
// Conversely, don't use direct write to USB_BUFF_STATUS. Use atomic access RegClr.

//#define USBCTRL_DPRAM_BASE	0x50100000
#define USB_RAM			((volatile u8*)(USBCTRL_DPRAM_BASE + 0x00))	// base of USB dual-port RAM (size USB_RAM_SIZE bytes)
#define USB_RAM_DATA		((u8*)(USBCTRL_DPRAM_BASE + 0x180))		// start of allocable DPRAM data

//#define USBCTRL_BASE		0x50100000
#define USB_SETUP_PKT		((u8*)(USBCTRL_BASE + 0x00))			// setup packet (8 bytes = USB_SETUP_PKT_SIZE)

// device:
#define USB_EP_CTRL(epinx)	((volatile u32*)(USBCTRL_BASE + (epinx)*4))	// device: endpoint IN/OUT control register, epinx=2..31
// For endpoint 0 use SIE_CTRL register, bits 28..30.
//	bit 0..15 (16 bits): address base offset in DPSRAM of data buffer
//            (bits 0..5 are ignored, address must be aligned to 64 bytes)
//      bit 16 device: interrupt on NAK
//      bit 17 device: interrupt on Stall
//      bit 16..25 (10 bits) host: interval the host controller should
//             poll this endpoint (only interrupt endpoint),
//             specified in ms - 1 (value 9 means 10 ms).
//	bit 26..27 (2 bits): endpoint type 0=control, 1=ISO, 2=Bulk, 3=Interrupt
//	bit 28: enable interrupt for every 2 transferred buffers (valid for double buffered)
//	bit 29: enable interrupt for every transferred buffer
//	bit 30: 0=single buffer (64 bytes), 1=double buffer (2x64 bytes)
//	bit 31: endpoint enable
#define USB_EP_BUF(epinx)	((volatile u32*)(USBCTRL_BASE + (epinx)*4 + 0x80)) // device: endpoint IN/OUT buffer control, epinx=0..31
// Note: "available" and "stall" bits should be set after other data
// To send STALL on EP0, set both stall bits in buffer control and in STALL_ARM register.
//   buffer 0 or single buffer:
//	bit 0..9 (10 bits): transfer length
//	bit 10: available 1=can be used for transfer, 0=status from controller
//	bit 11: send STALL for device, STALL received for host
//	bit 12: reset buffer select to buffer 0, cleared at end of transfer (for device only)
//	bit 13: data PID 0=DATA0, 1=DATA1
//	bit 14: last buffer of transfer
//	bit 15: buffer is full
//   buffer 1 (only valid for double buffered):
//	bit 16..25 (10 bits): transfer length
//	bit 26: available 1=can be used for transfer, 0=status from controller
//	bit 27..28 (2 bits): double buffer offset for ISO mode (0=128, 1=256, 2=512, 3=1024)
//	bit 29: data PID 0=DATA0, 1=DATA1
//	bit 30: last buffer of transfer
//	bit 31: buffer is full
#define USB_EP0_BUF0		((u8*)(USBCTRL_BASE + 0x100))			// device: endpoint 0 fixed IN/OUT buffer 0 (size 64 bytes)
#define USB_EP0_BUF1		((u8*)(USBCTRL_BASE + 0x140))			// device: endpoint 0 fixed IN/OUT buffer 1 (size 64 bytes)

// host:
#define USB_EP_INTCTRL(ep)	((volatile u32*)(USBCTRL_BASE + (ep)*8))	// host: interrupt endpoint control register, ep=1..15 (bits as USB_EP_CTRL)
#define USB_EPX_BUF		((volatile u32*)(USBCTRL_BASE + 0x80))		// host: endpoint X buffer control (bits as USB_EP_BUF)
#define USB_EP_INTBUF(ep)	((volatile u32*)(USBCTRL_BASE + (ep)*8 + 0x80))	// host: interrupt endpoint buffer control, ep=1..15, or ep=0 for endpoint X buffer control (bits as USB_EP_BUF)
#define USB_EPX_CTRL		((volatile u32*)(USBCTRL_BASE + 0x100))		// host: endpoint X control register (bits as USB_EP_CTRL)

// ----------------------------------------------------------------------------
//                            USB control registers
// ----------------------------------------------------------------------------

// USB control registers
//#define USBCTRL_REGS_BASE	0x50110000
#define USB_ADDR_ENDP		((volatile u32*)(USBCTRL_REGS_BASE + 0x00))	// device address and endpoint control
//  bit 0..6 (7 bits): device address
//  bit 16..19 (4 bits): host: device endpoint
#define USB_ADDR_ENDPN(ep)	((volatile u32*)(USBCTRL_REGS_BASE + (ep)*4))	// host: interrupt endpoint, ep=1..15
//  bit 0..6 (7 bits): device address
//  bit 16..19 (4 bits): endpoint number of interrupt endpoint
//  bit 25: direction of interrupt endpoint 0=IN, 1=OUT
//  bit 26: interrupt endpoint requires preamble (low speed device on full speed hub)
#define USB_MAIN_CTRL		((volatile u32*)(USBCTRL_REGS_BASE + 0x40))	// main control register
//  bit 0: enable controller
//  bit 1: 0=device mode, 1=host mode
//  bit 31: reduced timings for simulation
#define USB_SOF_WR		((volatile u32*)(USBCTRL_REGS_BASE + 0x44))	// set "start of frame" in host controller (sent every 1 ms and incremented)
#define USB_SOF_RD		((volatile u32*)(USBCTRL_REGS_BASE + 0x48))	// read last "start of frame"
#define USB_SIE_CTRL		((volatile u32*)(USBCTRL_REGS_BASE + 0x4C))	// SIE control register
//  bit 0: host: start transaction
//  bit 1: host: send setup packet
//  bit 2: host: send transaction (OUT from host)
//  bit 3: host: receive transaction (IN to host)
//  bit 4: host: stop transaction
//  bit 6: host: preamble enable for low speed device on full speed hub
//  bit 8: host: sync, delay packet until adter SOF
//  bit 9: host: enable SOF generation (for full speed bus)
//  bit 10: host: enable keep alive packet (for low speed bus)
//  bit 11: host: enable VBUS
//  bit 12: device: remote wakeup
//  bit 13: host: reset bus
//  bit 15: host: enable pull-down resistors
//  bit 16: device: enable pull-up resistor
//  bit 17: device: pull-up strength (0 = 1K2, 1 = 2K3)
//  bit 18: power down bus transceiver
//  bit 24: direct control of DM
//  bit 25: direct control of DP
//  bit 26: direct bus drive enable
//  bit 27: device: set bit in NAK_STALL when EP0 sends NAK
//  bit 28: device: set bit in BUFF_STATUS for every 2 buffers completed on EP0
//  bit 29: device: set bit in BUFF_STATUS for every buffer completed on EP0
//  bit 30: device: EP0 is 0=single or 1=double buffered
//  bit 31: device: set bit in NAK_STALL when EP0 sends STALL
#define USB_SIE_STATUS		((volatile u32*)(USBCTRL_REGS_BASE + 0x50))	// SIE status register
//  bit 0: device: VBUS detected
//  bit 2..3 (2 bits): USB bus line state
//  bit 4: bus in suspended state
//  bit 8..9 (2 bits): host: device speed 0=disconnected, 1=low speed, 2=full speed
//  bit 10: VBUS over current detected
//  bit 11: host: device has initiated remote resume; device: host has initiated resume
//  bit 16: device: connected
//  bit 17: device: setup packet received
//  bit 18: transaction complete
//  bit 19: device: bus reset received
//  bit 24: CRC error on RX
//  bit 25: bit stuff error on RX
//  bit 26: RX overflow (incoming data too fast)
//  bit 27: RX timeout (no ACK)
//  bit 28: host: NAK received
//  bit 29: host: STALL received
//  bit 30: ACK received
//  bit 31: data sequence error
#define USB_INT_EP_CTRL		((volatile u32*)(USBCTRL_REGS_BASE + 0x54))	// interrupt endpoint control register
//  bit 1..15: host: enable interrupt endpoint 1..15
#define USB_BUFF_STATUS		((volatile u32*)(USBCTRL_REGS_BASE + 0x58))	// buffer status register
//  bit 0..31: 1=buffer is completed on endpoint with index 0..31 (even = IN, odd = OUT)
#define USB_BUFF_SHOULD_HANDLE	((volatile u32*)(USBCTRL_REGS_BASE + 0x5C))	// which of the double buffers should be handled
//  bit 0..31: 1=buffer should be handled on endpoint with index 0..31 (even = IN, odd = OUT) ... valid if using interrupt per 1 buffer
#define USB_ABORT		((volatile u32*)(USBCTRL_REGS_BASE + 0x60))	// device: set to ignore buffer control register (send NAK)
// Note RP2040-E2 error: do not use EP_ABORT
//  bit 0..31: 1=ignore buffer control register for endpoint with index 0..31 (even = IN, odd = OUT); NAK will be set for every access
#define USB_ABORT_DONE		((volatile u32*)(USBCTRL_REGS_BASE + 0x64))	// device: endpoint is idle
//  bit 0..31: read 1=endpoint is idle, it is safe modify buffer control register for endpoint with index 0..31 (even = IN, odd = OUT)
#define USB_STALL_ARM		((volatile u32*)(USBCTRL_REGS_BASE + 0x68))	// device: must be set to send STALL
// This bit must be set in conjunction with STALL bit in buffer control register to send STALL on EP0 (cleared after receiving SETUP packet)
//  bit 0: EP0_IN
//  bit 1: EP0_OUT
#define USB_NAK_POLL		((volatile u32*)(USBCTRL_REGS_BASE + 0x6C))	// host: wait time after NAK
//  bit 0..9 (10 bits): NAK polling interval for low speed device in [us]
//  bit 16..25 (10 bits): NAK polling interva for full speed device in [us]
#define USB_NAK_STALL		((volatile u32*)(USBCTRL_REGS_BASE + 0x70))	// device: NAK or STALL bits are sets
//  bit 0..31: read 1=set if ON_NAK or ON_STALL buts are set on endpoint with index 0..31 (even = IN, odd = OUT)
#define USB_MUXING		((volatile u32*)(USBCTRL_REGS_BASE + 0x74))	// where to connect USB controller
//  bit 0: TO_PHY (default connection)
//  bit 1: TO_EXTPHY
//  bit 2: TO_DIGITAL_PAD, connect D+/D- to GPIO15/16
//  bit 3: SOFTCON
#define USB_PWR			((volatile u32*)(USBCTRL_REGS_BASE + 0x78))	// override power signals
//  bit 0: VBUS enable
//  bit 1: enable "VBUS enable" override
//  bit 2: VBUS detect
//  bit 3: enable "VBUS detect" override
//  bit 4: OVERCURR detect
//  bit 5: enable "OVERCURR detect" override
#define USB_PHY_DIRECT		((volatile u32*)(USBCTRL_REGS_BASE + 0x7C))	// direct control of USB phy
//  bit 0: enable second DP pull-up resistor (0=Pu2, 1=Pu1+Pu2)
//  bit 1: DP pull-up enable
//  bit 2: DP pull-down enable
//  bit 4: enable second DM pull-up resistor (0=Pu2, 1=Pu1+Pu2)
//  bit 5: DM pull-up enable
//  bit 6: DM pull-down enable
//  bit 8: DP output enable
//  bit 9: DM output enable
//  bit 10: DP output
//  bit 11: DM output
//  bit 12: RX power down override (1=powered down)
//  bit 13: TX power down override (1=powered down)
//  bit 14: TX 0=low speed slew rate, 1=full speed slew rate
//  bit 15: TX 0=single ended mode, 1=differential drive mode
//  bit 16: read differential RX
//  bit 17: read DPP pin state
//  bit 18: read DPM pin state
//  bit 19: read DP over current
//  bit 20: read DM over current
//  bit 21: read DP over voltage
//  bit 22: read DM over voltage
#define USB_PHY_OVERRIDE	((volatile u32*)(USBCTRL_REGS_BASE + 0x80))	// override enable of USB phy direct
//  bit 0: DP pullup hisel override enable
//  bit 1: DM pullup hisel override enable
//  bit 2: DP pullup enable override enable
//  bit 3: DP pulldown enable override enable
//  bit 4: DM pulldown enable override enable
//  bit 5: TX DP output enable override enable
//  bit 6: TX DM output enable override enable
//  bit 7: TX DP override enable
//  bit 8: TX DM override enable
//  bit 9: RX PD override enable
//  bit 10: TX PD override enable
//  bit 11: TX slew override enable
//  bit 12: DM pullup override enable
//  bit 15: TX diff mode override enable
#define USB_PHY_TRIM		((volatile u32*)(USBCTRL_REGS_BASE + 0x84))	// adjust trim of USB phy pull down resistors
//  bit 0..4: adjust DP pulldown resistor
//  bit 8..12: adjust DM pulldown resistor
#define USB_INTR		((volatile u32*)(USBCTRL_REGS_BASE + 0x8C))	// raw interrupts
#define USB_INTE		((volatile u32*)(USBCTRL_REGS_BASE + 0x90))	// interrupt enable
#define USB_INTF		((volatile u32*)(USBCTRL_REGS_BASE + 0x94))	// interrupt force
#define USB_INTS		((volatile u32*)(USBCTRL_REGS_BASE + 0x98))	// interrupt status
//  bit 0: host: device is connected or disconnected (cleared by writing status speed)
//  bit 1: host: device wakes up the host (cleared by writing status resume)
//  bit 2: host: host sends SOF (cleared by reading SOF)
//  bit 3: transfer completed (write to clear)
//  bit 4: any bit of buffer status is set (clear by clearing all buffer status bits)
//  bit 5: SEQ error
//  bit 6: RX timeout
//  bit 7: RX overflow
//  bit 8: bit stuff error
//  bit 9: CRC error
//  bit 10: STALL received
//  bit 11: VBUS detect
//  bit 12: BUS reset
//  bit 13: device connection state changes (cleared by writing to status connected)
//  bit 14: device suspend state changes (cleared by writing to status suspended)
//  bit 15: device receives resume from host (cleared by writing to status resume)
//  bit 16: device: setup request
//  bit 17: device: device receives SOF (cleared by reading SOF)
//  bit 18: any bit of abort done is set (cleared by clearing all bits in abort done)
//  bit 19: any bit of stall NAK is set (cleared by clearing all bits in stall NAK)

typedef struct {
	io32	dev_addr_ctrl;		// 0x00: Device address and endpoint control
	io32	int_ep_addr_ctrl[15];	// 0x04: Interrupt endpoint
	io32	main_ctrl;		// 0x40: Main control register
	io32	sof_wr;			// 0x44: Set the SOF (Start of Frame) frame number in the host controller
	io32	sof_rd;			// 0x48: Read the last SOF (Start of Frame) frame number seen
	io32	sie_ctrl;		// 0x4C: SIE control register
	io32	sie_status;		// 0x50: SIE status register
	io32	int_ep_ctrl;		// 0x54: interrupt endpoint control register
	io32	buf_status;		// 0x58: Buffer status register
	io32	buf_cpu_should_handle;	// 0x5C: Which of the double buffers should be handled
	io32	abort;			// 0x60: Device only: Can be set to ignore the buffer control register for this endpoint in case you...
	io32	abort_done;		// 0x64: Device only: Used in conjunction with `EP_ABORT`
	io32	ep_stall_arm;		// 0x68: Device: this bit must be set in conjunction with the `STALL` bit in the buffer control register...
	io32	nak_poll;		// 0x6C: Used by the host controller
	io32	ep_nak_stall_status;	// 0x70: Device: bits are set when the `IRQ_ON_NAK` or `IRQ_ON_STALL` bits are set
	io32	muxing;			// 0x74: Where to connect the USB controller
	io32	pwr;			// 0x78: Overrides for the power signals in the event that the VBUS signals are not hooked up to GPIO
	io32	phy_direct;		// 0x7C: Note that most functions are driven directly from usb_fsls controller
	io32	phy_direct_override;	// 0x80: override
	io32	phy_trim;		// 0x84: Value to drive to USB PHY
	io32	_pad0;			// 0x88:
	io32	intr;			// 0x8C: Raw Interrupts
	io32	inte;			// 0x90: Interrupt Enable
	io32	intf;			// 0x94: Interrupt Force
	io32	ints;			// 0x98: Interrupt status after masking & forcing
} usb_hw_t;

#define usb_hw ((usb_hw_t*)USBCTRL_REGS_BASE)

STATIC_ASSERT(sizeof(usb_hw_t) == 0x9C, "Incorrect usb_hw_t!");

// ----------------------------------------------------------------------------
//                             USB endpoint descriptor
// ----------------------------------------------------------------------------

// USB endpoint descriptor
//  Device: The index into the UsbEndpoints array is also an index into the control registers
//          and can be used to derive the endpoint number (bits 1..4) and direction (bit 0).
//  Host: Host uses only endpoint descriptors with index 0..15. Endpoints 1..15 correspond to
//        interrupt number 1..15, endpoint 0 corresponds to control endpoint EPX.

typedef struct {
	Bool		used;		// this endpoint is used ("configured")
	volatile Bool	active;		// transfer is active (busy; i.e. endpoint is used to transfer) ... flag is controlled by PHY code
	volatile Bool	claimed;	// endpoint is claimed (i.e. endpoint is used by the driver) ... flag is controlled by the driver
	Bool		stalled;	// endpoint is stalled
	Bool		use_ring;	// use ring buffer instead of fixed buffer (user_buf is pointer to ring buffer)
	Bool		rx;		// receive direction of transfer
	u8		dev_addr;	// host: device address (0..127)
	u8		dev_epinx;	// host: device endpoint index (endpoint number and endpoint direction)
	u8		next_pid;	// next packet identification PID (0 or 1)
	u8		xfer;		// transfer type USB_XFER_*
	u16		pktmax;		// max. packet size (1023 for ISO packet, 64 otherwise)
	u16		total_len;	// total length to transfer
	u16		rem_len;	// remaining length of data to transfer (shifted before transfer in UsbNextBuf, set in UsbXferStart)
	u16		xfer_len;	// length of already transferred data (shifted after transfer in UsbXferCont, cleared in UsbXferStart)
	u8*		data_buf;	// pointer to USB data buffer in USB DPRAM (allocated with UsbRamAlloc)
	void*		user_buf;	// pointer to user buffer or pointer to ring buffer sRing
	volatile u32*	ep_ctrl;	// endpoint control register (part of USB_SIE_CTRL, USB_EP_CTRL, USB_EP_INTCTRL or USB_EPX_CTRL)
	volatile u32*	buf_ctrl;	// buffer control register (USB_EP_BUF, USB_EP_INTBUF or USB_EPX_BUF)
} sEndpoint;

// ----------------------------------------------------------------------------
//                               SETUP packet
// ----------------------------------------------------------------------------

// request code of setup packet
#define USB_REQ_GET_STATUS		0
#define USB_REQ_CLEAR_FEATURE		1
#define USB_REQ_SET_FEATURE		3
#define USB_REQ_SET_ADDRESS		5
#define USB_REQ_GET_DESCRIPTOR		6
#define USB_REQ_SET_DESCRIPTOR		7
#define USB_REQ_GET_CONFIGURATION	8
#define USB_REQ_SET_CONFIGURATION	9
#define USB_REQ_GET_INTERFACE		10
#define USB_REQ_SET_INTERFACE		11
#define USB_REQ_SYNCH_FRAME		12

// feature type
#define USB_REQ_FEATURE_EDPT_HALT	0
#define USB_REQ_FEATURE_REMOTE_WAKEUP	1
#define USB_REQ_FEATURE_TEST_MODE	2

// request type
#define USB_REQ_TYPE_STANDARD		0
#define USB_REQ_TYPE_CLASS		1
#define USB_REQ_TYPE_VENDOR		2
#define USB_REQ_TYPE_INVALID		3

// recipient
#define USB_REQ_RCPT_DEVICE		0
#define USB_REQ_RCPT_INTERFACE		1
#define USB_REQ_RCPT_ENDPOINT		2
#define USB_REQ_RCPT_OTHER		3

// setup packet (8 bytes = USB_SETUP_PKT_SIZE)
//   The SETUP packet always occurs at an aligned address, so it does
//   not need the PACKED attribute (access will be faster), but beware.
typedef struct /*PACKED*/ {
	u8	type;		// request type
				//  bit 7: direction 0=host to device (OUT), 1=device to host(IN)
				//  bit 5..6 (2 bits): request type 0=standard, 1=class, 2=vendor, 3=reserved
				//  bit 0..4 (5 bits): recipient 0=device, 1=interface, 2=endpoint, 3=other
	u8	request;	// request USB_REQ_*
	u16	value;		// value
	u16	index;		// index or offset
	u16	length;		// length in bytes to transfer data phase
} sUsbSetupPkt;

// ----------------------------------------------------------------------------
//                               Descriptors
// ----------------------------------------------------------------------------
// https://www.beyondlogic.org/usbnutshell/usb5.shtml#DeviceDescriptors

// USB class codes
// https://www.usb.org/defined-class-codes
#define USB_CLASS_UNSPECIFIED		0	// device, use class information in the interface descriptor
#define USB_CLASS_AUDIO			1	// Audio device
#define USB_CLASS_CDC			2	// Communication Device Class
#define USB_CLASS_HID			3	// Human Interface Device
#define USB_CLASS_PHYSICAL		5	// Physical Device Class
#define USB_CLASS_IMAGE			6	// Still Imaging Device Class
#define USB_CLASS_PRINTER		7	// Printer Device Class
#define USB_CLASS_MSC			8	// Mass Storage Class
#define USB_CLASS_HUB			9	// USB Hub (protocol 0=full speed, 1=hi-speed, 2=hi-speed multiple TTs)
#define USB_CLASS_CDC_DATA		10	// Communication Data Device Class
#define USB_CLASS_SMART_CARD		11	// Smart Card Device Class
#define USB_CLASS_SECURITY		13	// Content Security Device Class
#define USB_CLASS_VIDEO			14	// Video Device Class
#define USB_CLASS_HEALTHCARE		15	// Personal Healthcare Device Class
#define USB_CLASS_AUDIO_VIDEO		16	// Audio/Video Device Class (subclass 1=control, 2=video, 3=audio)
#define USB_CLASS_BILLBOARD		17	// Billboard Device Class
#define USB_CLASS_BRIDGE		18	// USB Type-C Bridge Device Class
#define USB_CLASS_BULK			19	// VESA USB Bulk Display Protocol Device Class
#define USB_CLASS_I3C			0x3C	// USB I3C Device Class
#define USB_CLASS_DIAGNOSTIC		0xDC	// Diagnostic Device Class
#define USB_CLASS_WIRELESS		0xE0	// Wireless Controller (including Bluetooth)
#define USB_CLASS_MISC			0xEF	// Miscellaneous Device (class is defined by interface descriptor)
#define USB_CLASS_APPLICATION		0xFE	// Application Specific Device Class (including 01h=device firmware upgrade)
#define USB_CLASS_VENDOR		0xFF	// Vendor specific Device Class

// USB Descriptor Types
#define USB_DESC_DEVICE			1
#define USB_DESC_CONFIGURATION		2
#define USB_DESC_STRING			3
#define USB_DESC_INTERFACE		4
#define USB_DESC_ENDPOINT		5
//#define USB_DESC_DEVICE_QUALIFIER	6	// high speed not supported
//#define USB_DESC_OTHER_SPEED_CONFIG	7	// high speed not supported
#define USB_DESC_INTERFACE_POWER	8
#define USB_DESC_OTG			9
#define USB_DESC_DEBUG			10
#define USB_DESC_INTERFACE_ASSOCIATION	11
#define USB_DESC_BOS			15
#define USB_DESC_DEVICE_CAPABILITY	16
#define USB_DESC_FUNCTIONAL		0x21
// Class Specific Descriptor
#define USB_DESC_CS_DEVICE		0x21
#define USB_DESC_CS_CONFIGURATION	0x22
#define USB_DESC_CS_STRING		0x23
#define USB_DESC_CS_INTERFACE		0x24
#define USB_DESC_CS_ENDPOINT		0x25
//#define USB_DESC_SUPERSPEED_ENDPOINT_COMPANION		0x30
//#define USB_DESC_SUPERSPEED_ISO_ENDPOINT_COMPANION	0x31

// device capability type
#define USB_CAPABILITY_WIRELESS_USB		1
#define USB_CAPABILITY_USB20_EXTENSION		2
#define USB_CAPABILITY_SUPERSPEED_USB		3
#define USB_CAPABILITY_CONTAINER_ID		4
#define USB_CAPABILITY_PLATFORM			5
#define USB_CAPABILITY_POWER_DELIVERY		6
#define USB_CAPABILITY_BATTERY_INFO		7
#define USB_CAPABILITY_PD_CONSUMER_PORT		8
#define USB_CAPABILITY_PD_PROVIDER_PORT		9
#define USB_CAPABILITY_SUPERSPEED_PLUS		10
#define USB_CAPABILITY_PRECESION_TIME_MEASUREMENT 11
#define USB_CAPABILITY_WIRELESS_USB_EXT		12
#define USB_CAPABILITY_BILLBOARD		13
#define USB_CAPABILITY_AUTHENTICATION		14
#define USB_CAPABILITY_BILLBOARD_EX		15
#define USB_CAPABILITY_CONFIGURATION_SUMMARY	16

// HID subclass (defined in device descriptor; class should be 0)
#define USB_HID_SUBCLASS_NONE	0	// no subclass
#define USB_HID_SUBCLASS_BOOT	1	// boot interface subclass - used when booting to BIOS

// HID interface protocol
#define USB_HID_ITF_NONE	0	// none (no subclass)
#define USB_HID_ITF_KEYB	1	// keyboard (boot subclass)
#define USB_HID_ITF_MOUSE	2	// mouse (boot subclass)

// HID descriptor type
#define USB_HID_DESC_HID	0x21	// HID descriptor
#define USB_HID_DESC_REP	0x22	// report descriptor
#define USB_HID_DESC_PHY	0x23	// physical descriptor

// - All descriptors must be "PACKED" to ensure unaligned access to u16.

// USB base descriptor
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes
	u8	type;		// 1: descriptor type
} sUsbDesc;

// get length of USB descriptor
#define USB_DESC_LEN(p_desc) ((const sUsbDesc*)(p_desc))->len

// get type of USB descriptor
#define USB_DESC_TYPE(p_desc) ((const sUsbDesc*)(p_desc))->type

// shift pointer to next descriptor (returns const u8*)
#define USB_DESC_NEXT(p_desc) ((const u8*)(p_desc) + USB_DESC_LEN(p_desc))

// USB device descriptor (18 bytes)
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 18)
	u8	type;		// 1: descriptor type (= USB_DESC_DEVICE)
	u16	usb;		// 2: USB specification in BCD code (2.10 = 0x210)
	u8	dev_class;	// 4: device class code USB_CLASS_*
	u8	dev_subclass;	// 5: device subclass code
	u8	dev_protocol;	// 6: device protocol code
	u8	pktmax;		// 7: max. packet size for endpoint 0 (valid size 8, 16, 32, 64)
	u16	vendor;		// 8: vendor ID
	u16	product;	// 10: product ID
	u16	device;		// 12: device release number in BCD code
	u8	i_manufact;	// 14: index of manufacturer string descriptor (0=none)
	u8	i_product;	// 15: index of product string descriptor (0=none)
	u8	i_serial;	// 16: index of serial number string descriptor (0=none)
	u8	config;		// 17: number of possible configurations
} sUsbDescDev;

// USB configuration descriptor (9 bytes)
// USB other speed configuration descriptor (9 bytes)
// - follow interface descriptors
// - follow endpoint descriptors
// - follow class descriptors
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 9)
	u8	type;		// 1: descriptor type (= USB_DESC_CONFIGURATION or USB_DESC_OTHER_SPEED_CONFIG)
	u16	totallen;	// 2: total length in bytes of data returned (includes configuration, interface, endpoint and class descriptors)
	u8	itf_num;	// 4: number of interfaces supported by this configuration
	u8	config;		// 5: value to select this configuration (1-based)
	u8	i_config;	// 6: index of string descriptor describing this configuration
	u8	attrib;		// 7: attributes
				//	bit 7: must be 1 (historical reasons)
				//	bit 6: self powered from USB bus (set maxpower > 0)
				//	bit 5: remote wakeup supported
				//	bit 0..4: must be 0
	u8	maxpower;	// 8: max. power consumption from USB bus in 2mA units
} sUsbDescCfg;

// USB string descriptor (string descriptor with index 0 contains list of supported languages)
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 2 + 2*strlen)
	u8	type;		// 1: descriptor type (=USB_DESC_STRING)
	u16	str[];		// 2: UTF-16 string (Unicode) or list of supported languages
} sUsbDescStr;

// USB interface descriptor (9 bytes)
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 9)
	u8	type;		// 1: descriptor type (= USB_DESC_INTERFACE)
	u8	itf_num;	// 2: number of this interface (0-based)
	u8	alter;		// 3: alternative setting
	u8	num_ep;		// 4: number of endpoints used by this interface (excluding EP0; 0 = use only default control pipe)
	u8	itf_class;	// 5: interface class code
	u8	itf_subclass;	// 6: interface subclass code
	u8	itf_protocol;	// 7: interface protocol code
	u8	i_interface;	// 8: index of string descriptor
} sUsbDescItf;

// USB endpoint descriptor (7 bytes)
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 7)
	u8	type;		// 1: descriptor type (= USB_DESC_ENDPOINT)
	u8	ep_addr;	// 2: endpoint address bit 0..3 = endpoint number 0..15, bit 4..6 = must be 0, bit 7 direction 0=OUT, 1=IN
	u8	attr;		// 3: attributes
				//	bit 0..1: transfer type USB_XFER_* 0=control, 1=isochronous, 2=bulk, 3=interrupt
				//	bit 2..3: synchronization (for ISO mode) 0=no, 1=asynchronous, 2=adaptive, 3=synchronous
				//	bit 4..5: usage type (for ISO mode) 0=data, 1=feedback, 2=explicit feedback
	u16	pktmax;		// 4: max packet size
				//	bit 0..10: max. packet size
				//	bit 11..12: additional transactions per microframe 0..2 (= 1 to 3 transactions per microframe)
	u8	interval;	// 6: interval for polling (in frame counts), for ISO must be 1, ignored for bulk and control
} sUsbDescEp;

// USB interface association descriptor (8 bytes)
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 8)
	u8	type;		// 1: descriptor type (= USB_DESC_INTERFACE_ASSOCIATION)
	u8	itf_first;	// 2: index of first associated interface
	u8	itf_count;	// 3: number of associated interfaces
	u8	itf_class;	// 4: interface class code
	u8	itf_subclass;	// 5: interface subclass code
	u8	itf_protocol;	// 6: interface protocol code
	u8	i_interface;	// 7: index of string descriptor
} sUsbDescItfAssoc;

// USB binary device object store (BOS) descriptor (5 bytes)
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 5)
	u8	type;		// 1: descriptor type (= USB_DESC_BOS)
	u16	totallen;	// 2: total length in bytes of data returned
	u8	num_caps;	// 4: number of device capability descriptors in the BOS
} sUsbDescBos;

// USB BOS device capability descriptor
typedef struct PACKED {
	u8	len;		// 0: size of this descriptor in bytes (= 3 + capnum)
	u8	type;		// 1: descriptor type (= USB_DESC_DEVICE_CAPABILITY)
	u8	cap_type;	// 2: device capability type
	u8	cap_data[];	// 3: device capability data
} sUsbDescBosCap;

// HID descriptor (9 bytes)
// - class type for HID class device is not defined by device descriptor, but by interface descriptor.
// - subclass of device descriptor is used to identify boot device
typedef struct PACKED {
	u8	len;		// 0: (1) size of this descriptor in bytes (= 9)
	u8	type;		// 1: (1) descriptor type (= USB_HID_DESC_*)
	u16	bcd_hid;	// 2: (2) HID class specification release in BCD (0x0110 means 1.10)
	u8	country;	// 4: (1) country code USB_HID_COUNTRY_* (0 = not localized)
	u8	desc_num;	// 5: (1) number of class descriptors (>= 1, minimal 1 report descriptor)
// array of class descriptors (every descriptor is 3 bytes)
	u8	rep_type;	// 6: (1) type of class descriptor
	u16	rep_len;	// 7: (2) size of class descriptor
} sUsbDescHid;

// ----------------------------------------------------------------------------
//                            Descriptor templates
// ----------------------------------------------------------------------------

#define U16_U8_LE(nn)	((u8)(nn)), ((u8)((nn)>>8))	// convert u16 to 2x u8 entries, little endian format (Intel)
#define U32_U8_LE(nn)	((u8)(nn)), ((u8)((nn)>>8)), ((u8)((nn)>>16)), ((u8)((nn)>>24)) // convert u32 to 4x u8 entries

// configuration descriptor template (9 bytes)
//  cfg ... index of this configuration (1-based)
//  itf ... number of interfaces supported by this configuration
//  str ... index of string descriptor describing this configuration (1-based, 0=none)
//  len ... total length in bytes of data returned (includes configuration, interface, endpoint and class descriptors)
//  pwr ... max. power consumption from USB bus in [mA] (0 = device is self powered)
#define USB_TEMP_CFG(cfg, itf, str, len, pwr) \
	9,				/* 0: size of this descriptor in bytes */ \
	USB_DESC_CONFIGURATION,		/* 1: descriptor type */ \
	U16_U8_LE(len),			/* 2: total length in bytes of data returned (includes descriptors) */ \
	(itf),				/* 4: number of interfaces supported by this configuration */ \
	(cfg),				/* 5: value to select this configuration (1-based) */ \
	(str),				/* 6: index of string descriptor describing this configuration */ \
	(((pwr) <= 1) ? B6 : 0) | B7,	/* 7: attributes, bit 6: self powered from USB bus */ \
	(pwr)/2				/* 8: max. power consumption from USB bus in 2mA units */

// interface descriptor template (9 bytes)
//  itf ... number of this interface (0-base)
//  alt ... alternative setting
//  epnum ... number of endpoints used by this interface
//  cls ... interface class code
//  subcls ... interface subclass code
//  prot ... interface protocol code
//  str ... index of string descriptor
#define USB_TEMP_ITF(itf, alt, epnum, cls, subcls, prot, str) \
	9,				/* 0: size of this descriptor in bytes */ \
	USB_DESC_INTERFACE,		/* 1: descriptor type */ \
	(itf),				/* 2: number of this interface (0-based) */ \
	(alt),				/* 3: alternative setting */ \
	(epnum),			/* 4: number of endpoints used by this interface */ \
	(cls),				/* 5: interface class code */ \
	(subcls),			/* 6: interface subclass code */ \
	(prot),				/* 7: interface protocol code */ \
	(str)				/* 8: index of string descriptor */

// endpoint descriptor template (7 bytes)
//  ep ... endpoint address, bit 0..3 = endpoint number 0..15, bit 7 direction 0=OUT 1=IN
//  attr ... attributes
//		bit 0..1: transfer type USB_XFER_* 0=control, 1=isochronous, 2=bulk, 3=interrupt
//		bit 2..3: synchronization (for ISO mode) 0=no, 1=asynchronous, 2=adaptive, 3=synchronous
//		bit 4..5: usage type (for ISO mode) 0=data, 1=feedback, 2=explicit feedback
//  pktmax ... max packet size
//  poll ... interval for polling (in frame counts), for ISO must be 1, ignored for bulk and control
#define USB_TEMP_EP(ep, attr, pktmax, poll) \
	7,				/* 0: size of this descriptor in bytes */ \
	USB_DESC_ENDPOINT,		/* 1: descriptor type */ \
	(ep),				/* 2: endpoint address */ \
	(attr),				/* 3: attributes */ \
	U16_U8_LE(pktmax),		/* 4: max packet size */ \
	(poll)				/* 6: interval for polling (in frame counts) */

// interface association descriptor template (8 bytes)
//  itf ... index of first associated interface (0-based)
//  num ... number of associated interfaces
//  cls ... interface class code
//  subcls ... interface subclass code
//  prot ... interface protocol code
//  str ... index of string descriptor
#define USB_TEMP_ASSOC(itf, num, cls, subcls, prot, str) \
	8,				/* 0: size of this descriptor in bytes */ \
	USB_DESC_INTERFACE_ASSOCIATION,	/* 1: descriptor type */ \
	(itf),				/* 2: index of first associated interface */ \
	(num),				/* 3: number of associated interfaces */ \
	(cls),				/* 4: interface class code */ \
	(subcls),			/* 5: interface subclass code */ \
	(prot),				/* 6: interface protocol code */ \
	(str)				/* 7: index of string descriptor */

// BOS descriptor template
//  len ... total length in bytes of data returned
//  capsnum ... number of device capability descriptors in the BOS
#define USB_TEMP_BOS(len, capsnum) \
	5,				/* 0: size of this descriptor in bytes */ \
	USB_DESC_BOS,			/* 1: descriptor type */ \
	U16_U8_LE(len),			/* 2: total length in bytes of data returned */ \
	(capsnum)			/* 4: number of device capability descriptors in the BOS */

// HID descriptor template
//  num ... number or class report descriptors (>= 1, minimal 1 report descriptor)
#define USB_TEMP_HID(len) \
	9,				/* 0: size of this descriptor in bytes */ \
	USB_HID_DESC_HID,		/* 1: descriptor type */ \
	U16_U8_LE(0x0111),		/* 2: HID class specification release in BCD = 1.11 */ \
	0,				/* 4: country code, 0 = not localized */ \
	1,				/* 5: number of class descriptors */ \
	USB_HID_DESC_REP,		/* 6: type of class descriptor */ \
	U16_U8_LE(len)			/* 7: size of class descriptor */
	
// CDC device descriptor template (66 bytes)
//  itf ... index of first associated interface (0-based)
//  str ... string descriptor (0=none)
//  epnot ... notification endpoint
//  notmax .. notification max packet size
//  epout .. OUT endpoint (direction from host to device)
//  epin ... IN endpoint (direction from devise to host)
//  epmax ... IN/OUT max packet size
#define USB_TEMP_CDCD(itf, str, epnot, notmax, epout, epin, epmax) \
	/* (8) interface association: index of interface, 2 interfaces, CDC class, abstract subclass, protocol none, no string */ \
	USB_TEMP_ASSOC((itf), 2, USB_CLASS_CDC, 2, 0, 0), \
	/* (9) CDC control interface: index of interface, no alternate, 1 endpoint, CDC class, abstract subclass, protocol none, string */ \
	USB_TEMP_ITF((itf), 0, 1, USB_CLASS_CDC, 2, 0, (str)), \
	/* (5) class specific: CDC header, version 1.20 */ \
	5, USB_DESC_CS_INTERFACE, 0, U16_U8_LE(0x0120), \
	/* (5) class specific: CDC call, interface + 1 */ \
	5, USB_DESC_CS_INTERFACE, 1, 0, ((u8)(itf)+1), \
	/* (4) class specific: CDC abstract control, supports line coding */ \
	4, USB_DESC_CS_INTERFACE, 2, 2, \
	/* (5) class specific: CDC union */ \
	5, USB_DESC_CS_INTERFACE, 6, (itf), ((u8)(itf)+1), \
	/* (7) endpoint notification: endpoint address, transfer type interrupt, max packet size, polling interval 16 */ \
	USB_TEMP_EP((epnot), USB_XFER_INT, (notmax), 16), \
	/* (9) CDC data interface: index of interface, no alternate, 2 endpoints, CDC DATA class, no subclass, no protocol, no string */ \
	USB_TEMP_ITF(((u8)(itf)+1), 0, 2, USB_CLASS_CDC_DATA, 0, 0, 0), \
	/* (7) endpoint OUT: endpoint address, transfer type bulk, max packet size, no polling interval */ \
	USB_TEMP_EP((epout), USB_XFER_BULK, (epmax), 0), \
	/* (7) endpoint IN: endpoint address, transfer type bulk, max packet size, no polling interval */ \
	USB_TEMP_EP((epin), USB_XFER_BULK, (epmax), 0)

// length of CDC device descriptor template (= 66 bytes)
#define USB_TEMP_CDCD_LEN (8+9+5+5+4+5+7+9+7+7)

// HID device descriptor template - IN direction (from device to host; 25 bytes)
//  itf ... index of interface (0-based)
//  str ... string descriptor (0=none)
//  boot ... boot protocol USB_HID_ITF_NONE=0, USB_HID_ITF_KEYB=1, USB_HID_ITF_MOUSE=2
//  rep_len .. report descriptor length
//  epin ... IN endpoint (direction from devise to host)
//  epmax ... max packet size
//  inter ... polling interval
#define USB_TEMP_HIDD(itf, str, boot, rep_len, epin, epmax, inter) \
	/* (9) HID interface: index of interface, no alternate, 1 endpoint, HID class, subclass, protocol, string */ \
	USB_TEMP_ITF((itf), 0, 1, USB_CLASS_HID, (((boot)!=USB_HID_ITF_NONE)?USB_HID_SUBCLASS_BOOT:USB_HID_SUBCLASS_NONE), (boot), (str)), \
	/* (9) HID descriptor: report descriptor length */ \
	USB_TEMP_HID((rep_len)), \
	/* (7) endpoint IN: endpoint address, transfer type interrupt, max packet size, polling interval */ \
	USB_TEMP_EP((epin), USB_XFER_INT, (epmax), (inter))

// length of HID device descriptor template IN  (= 25)
#define USB_TEMP_HIDD_LEN (9+9+7)

// HID device descriptor template - IN/OUT direction (from device to host and from host to device; 32 bytes)
//  itf ... index of interface (0-based)
//  str ... string descriptor (0=none)
//  boot ... boot protocol USB_HID_ITF_NONE=0, USB_HID_ITF_KEYB=1, USB_HID_ITF_MOUSE=2
//  rep_len .. report descriptor length
//  epout .. OUT endpoint (direction from host to device)
//  epin ... IN endpoint (direction from devise to host)
//  epmax ... max packet size
//  inter ... polling interval
#define USB_TEMP_HIDD_INOUT(itf, str, boot, rep_len, epout, epin, epmax, inter) \
	/* (9) HID interface: index of interface, no alternate, 2 endpoints, HID class, subclass, protocol, string */ \
	USB_TEMP_ITF((itf), 0, 2, USB_CLASS_HID, (((boot)!=USB_HID_ITF_NONE)?USB_HID_SUBCLASS_BOOT:USB_HID_SUBCLASS_NONE), (boot), (str)), \
	/* (9) HID descriptor: report descriptor length */ \
	USB_TEMP_HID((rep_len)), \
	/* (7) endpoint OUT: endpoint address, transfer type interrupt, max packet size, polling interval */ \
	USB_TEMP_EP((epout), USB_XFER_INT, (epmax), (inter)), \
	/* (7) endpoint IN: endpoint address, transfer type interrupt, max packet size, polling interval */ \
	USB_TEMP_EP((epin), USB_XFER_INT, (epmax), (inter))

// length of HID device descriptor template IN  (= 32)
#define USB_TEMP_HIDD_INOUT_LEN (9+9+7+7)

// ----------------------------------------------------------------------------
//                        Host variables of one device
// ----------------------------------------------------------------------------

// device flags
#define USB_HOST_FLAG_CONN	B0	// device is connected (after setting valid device address)
#define USB_HOST_FLAG_CONF	B1	// device is configured
#define USB_HOST_FLAG_SUSP	B2	// device is suspended

// host variables of device (size 13 + 16 + 32 = 61 bytes)
typedef struct {

	// device address
	u8	hub_addr;	// HUB address (0 = ROOT hub, local host)
	u8	hub_port;	// HUB port (0 = all devices of downstream hub)
	u8	hub_speed;	// device speed (USB_SPEED_FS or USB_SPEED_LS)

	// device state
	volatile u8 flag;	// device flags USB_HOST_FLAG_*

	// device descriptor
	u16	vendor;		// vendor ID
	u16	product;	// product ID
	u8	i_manufact;	// manufacturer text index
	u8	i_product;	// product text index
	u8	i_serial;	// serial text index
	u8	pktmax;		// max. packet size for endpoint 0 (valid size 8, 16, 32, 64)
	
	// mapping
	u8	itf2drv[USB_ENDPOINT_NUM]; // mapping interface number to device driver (USB_DRVID_INVALID = invalid; 16 bytes)
	u8	ep2drv[USB_ENDPOINT_NUM2]; // mapping device endpoint index to device driver (USB_DRVID_INVALID = invalid driver; 32 bytes)

} sUsbHostDev;

#define USB_HOST_HUB_INX	(1 + USE_USB_HOST)	// base index of hub devices
#define USE_USB_HOST_DEVNUM	(USB_HOST_HUB_INX + USE_USB_HOST_HUB) // total number of host devices (including hub and device 0)

// ----------------------------------------------------------------------------
//                 Application device setup descriptor
// ----------------------------------------------------------------------------

// application callback: device is mounted (interface has been mapped)
typedef void (*pUsbDevMountedCB)();

// application callback: suspend
//   Suspend: within 7 ms, device must draw current less than 2.5 mA from bus
//            Suspend signal can be invoked after the device is connected before host starts sending SOF.
typedef void (*pUsbDevSuspendCB)();

// application callback: resume
typedef void (*pUsbDevResumeCB)();

// application callback: get string descriptor (NULL invalid; if return NULL = use str_list instead)
//  inx ... requred index (descriptor with inx=0 contains list of supported languages)
//  langid ... language ID
typedef const sUsbDescStr* (*pUsbDevGetDescStrCB)(u8 inx, u16 langid);

// application device setup descriptor
typedef struct {

	// descriptors
	const sUsbDescDev*	desc_dev;	// pointer to device descriptor
	const sUsbDescCfg**	desc_cfg_list;	// pointer to list of configuration descriptors
	int			desc_cfg_num;	// number of configuration descriptors in the list
	const sUsbDescBos*	desc_bos;	// binary device object store (BOS) descriptor (NULL = not used)
	const void**		desc_list;	// list of additional descriptors (HID: list of report descriptors)

	// callbacks
	pUsbDevMountedCB	mounted_cb;	// device is mounted callback (NULL = not used)
	pUsbDevSuspendCB	suspend_cb;	// suspend callback (NULL = not used; within 7 ms device must draw current less than 2.5 mA from bus)
	pUsbDevResumeCB		resume_cb;	// resume callback (NULL = not used)
	pUsbDevGetDescStrCB	str_cb;		// get string descriptor callback (NULL = not used)

	// ASCII strings, used default English language 0x0409 (used when str_cb returns NULL)
	const char**		str_list;	// list of ASCIIZ strings (index 0..; NULL = not used; index is 1 less than the index in str_cb)
	int			str_num;	// number of ASCIIZ strings in the list

} sUsbDevSetupDesc;

#ifdef __cplusplus
}
#endif

#endif // _SDK_USB_DEF_H

#endif // USE_USB		// use USB (sdk_usb_*.c, sdk_usb_*.h)
