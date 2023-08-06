
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

// 15 simplex (or half-duplex) channels, each channel of speed 64 KB per second
// (total transfer max. 960 KB per second).

#if USE_USBPORT		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)

#ifndef _SDK_USBPORT_H
#define _SDK_USBPORT_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// ----------------------------------------------------------------------------
//                              Definitions
// ----------------------------------------------------------------------------

// base definitions
#define USB_ENDPOINT_NUM	16		// total number of endpoints (0..15)
#define USB_ENDPOINT_NUM2	(USB_ENDPOINT_NUM*2) // total number of endpoints including direction (0..31)
#define USB_PACKET_MAX		64		// max. size of normal packet

// selected USB port
#define USBPORT_OFF	0	// USB port is not initialized
#define USBPORT_MASTER	1	// master mode selected (host mode)
#define USBPORT_SLAVE	2	// slave mode selected (device mode)

// error codes (error code is greater than 64 to ensure distinction from packet length)
#define USBPORT_ERR_OK		0	// all OK
#define USBPORT_ERR_INIT	65	// driver is not initialized
#define USBPORT_ERR_CON		66	// opposite device is not connected
#define USBPORT_ERR_BUSY	67	// channel is busy (transfer is in progress)
#define USBPORT_ERR_COMP	68	// transfer is complete (it may also mean that it was never launched)
#define USBPORT_ERR_LEN		69	// incorrect length of formatted packet
#define USBPORT_ERR_CRC		70	// CRC error of received packet

// endpoint index
#define USB_DIR_IN	0	// IN packet (from device to host)
#define USB_DIR_OUT	1	// OUT packet (from host to device)
#define USB_EPINX(ep, dir) (((ep) << 1) + (dir)) // convert endpoint number (0..15) and direction (USB_DIR_*) to endpoint index (in RP2040 format)
#define USB_DIR(epinx)	((epinx) & 1)		// get endpoint direction USB_DIR_* from endpoint index
#define USB_EP(epinx)	((epinx) >> 1)		// get endpoint number 0..15 from endpoint index (identical to endpoint interrupt index)

// transfer type
#define USB_XFER_CTRL	0	// control packet
#define USB_XFER_ISO	1	// isochronous
#define USB_XFER_BULK	2	// bulk
#define USB_XFER_INT	3	// interrupt

//#define USBCTRL_DPRAM_BASE	0x50100000
#define USB_RAM			((volatile u8*)(USBCTRL_DPRAM_BASE + 0x00))	// base of USB dual-port RAM (size USB_RAM_SIZE bytes)
#define USB_RAM_DATA		((u8*)(USBCTRL_DPRAM_BASE + 0x180))		// start of allocable DPRAM data

//#define USBCTRL_BASE		0x50100000
#define USB_EP_CTRL(epinx)	((volatile u32*)(USBCTRL_BASE + (epinx)*4))	// device: endpoint IN/OUT control register, epinx=2..31
#define USB_EP_BUF(epinx)	((volatile u32*)(USBCTRL_BASE + (epinx)*4 + 0x80)) // device: endpoint IN/OUT buffer control, epinx=0..31
#define USB_EP0_BUF0		((u8*)(USBCTRL_BASE + 0x100))			// device: endpoint 0 fixed IN/OUT buffer 0 (size 64 bytes)
#define USB_EP_INTCTRL(ep)	((volatile u32*)(USBCTRL_BASE + (ep)*8))	// host: interrupt endpoint control register, ep=1..15 (bits as USB_EP_CTRL)
#define USB_EPX_BUF		((volatile u32*)(USBCTRL_BASE + 0x80))		// host: endpoint X buffer control (bits as USB_EP_BUF)
#define USB_EP_INTBUF(ep)	((volatile u32*)(USBCTRL_BASE + (ep)*8 + 0x80))	// host: interrupt endpoint buffer control, ep=1..15, or ep=0 for endpoint X buffer control (bits as USB_EP_BUF)
#define USB_EPX_CTRL		((volatile u32*)(USBCTRL_BASE + 0x100))		// host: endpoint X control register (bits as USB_EP_CTRL)

//#define USBCTRL_REGS_BASE	0x50110000
#define USB_ADDR_ENDP		((volatile u32*)(USBCTRL_REGS_BASE + 0x00))	// device address and endpoint control
#define USB_ADDR_ENDPN(ep)	((volatile u32*)(USBCTRL_REGS_BASE + (ep)*4))	// host: interrupt endpoint, ep=1..15
#define USB_MAIN_CTRL		((volatile u32*)(USBCTRL_REGS_BASE + 0x40))	// main control register
#define USB_SIE_CTRL		((volatile u32*)(USBCTRL_REGS_BASE + 0x4C))	// SIE control register
#define USB_SIE_STATUS		((volatile u32*)(USBCTRL_REGS_BASE + 0x50))	// SIE status register
#define USB_INT_EP_CTRL		((volatile u32*)(USBCTRL_REGS_BASE + 0x54))	// interrupt endpoint control register
#define USB_BUFF_STATUS		((volatile u32*)(USBCTRL_REGS_BASE + 0x58))	// buffer status register
#define USB_MUXING		((volatile u32*)(USBCTRL_REGS_BASE + 0x74))	// where to connect USB controller
#define USB_PWR			((volatile u32*)(USBCTRL_REGS_BASE + 0x78))	// override power signals
#define USB_INTR		((volatile u32*)(USBCTRL_REGS_BASE + 0x8C))	// raw interrupts

// ----------------------------------------------------------------------------
//                               Data
// ----------------------------------------------------------------------------

// selected USB port
extern u8 UsbPort;		// current selected port (0=not initialized)

// ----------------------------------------------------------------------------
//                              Initialize
// ----------------------------------------------------------------------------

// get current selected USB port (0=not initialized, 1=master, 2=slave)
INLINE u8 UsbPortGet() { return UsbPort; }

// check if USB port is initialized
INLINE Bool UsbPortIsInit() { return UsbPort != USBPORT_OFF; }

// check if master mode is selected (host mode of USB)
INLINE Bool UsbPortIsMaster() { return UsbPort == USBPORT_MASTER; }

// check if slave mode is selected (device mode of USB)
INLINE Bool UsbPortIsSlave() { return UsbPort == USBPORT_SLAVE; }

// get DPRAM buffer address of the channel (64 bytes long + 64 bytes reserve after it)
//  chan ... channel 0..15
u8* UsbPortBuf(u8 chan);

// initialize USB port
//  port ... use 1=USBPORT_MASTER or 2=USBPORT_SLAVE
void UsbPortInit(u8 port);

// terminate USB port
void UsbPortTerm();

// check if opposite device is connected
Bool UsbPortConnected();

// ----------------------------------------------------------------------------
//                         Multi-channel transfer
// ----------------------------------------------------------------------------
// After receiving packet, next packet will start receiving automatically.
// To swap receiving and transmitting, reset channel on both sides using UsbPortReset.

// get channel state
//  chan ... channel 1..15
// Returns:
//   USBPORT_ERR_OK ... channel is waiting and not active
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
//   USBPORT_ERR_COMP ... transfer is complete (it may also mean that it was never launched)
u8 UsbPortState(u8 chan);

// reset channel
//  Stop current transmission. Must be executed on both sides to ensure DATA0/DATA1 synchronization.
void UsbPortReset(u8 chan);

// clear complete status of the channel
void UsbPortClear(u8 chan);

// check if transmit channel is ready to send next packet
Bool UsbPortSendChanReady(u8 chan);

// send packet
//  chan ... channel 1..15
//  buf ... buffer with data to send
//  len ... length of data to send (0..64 bytes)
// Returns:
//   USBPORT_ERR_OK ... packet was sent OK
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
u8 UsbPortSendChan(u8 chan, const void* buf, int len);

// check if next received packet is ready
//  chan ... channel 1..15
Bool UsbPortRecvChanReady(u8 chan);

// receive packet
//  chan ... channel 1..15
//  buf ... buffer to load received data
//  len ... max. size of data to receive
// If it returns number between 0 and 64, it is length of received packet.
// If it returns number greater than 64, it is following error code:
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
u8 UsbPortRecvChan(u8 chan, void* buf, int len);

// ----------------------------------------------------------------------------
//                      Transfer of formatted packet
// ----------------------------------------------------------------------------
// After receiving packet, next packet will start receiving automatically.
// To swap receiving and transmitting, reset channel on both sides using UsbPortReset.

// Recommended to support hello packet, which will be sent after connection
// by host to device on channel 1, and by device to host on channel 2.

// common packet (4 to 64 bytes)
// Pay attention to the alignment of entries in the packet and the alignment
// of the entire packet structure in memory to ensure compatibility and portability.
// If you cannot keep the alignment, use the PACKED attribute of the packet.
typedef struct {
	u16	crc;	// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: (1) total packet length (4 to 64 bytes)
	u8	cmd;	// 3: (1) command (USBPORT_CMD_*)
	u8	data[60]; // 4: packet data (start of data array is u32 aligned)
} sUsbPortPkt;

// ping packet (4 bytes) ... can be used instead of SOF frame USB packets
#define USBPORT_CMD_PING	0	// command: ping packet
typedef struct {
	u16	crc;	// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: (1) total packet length (= 4)
	u8	cmd;	// 3: (1) command (= USBPORT_CMD_PING)
} sUsbPortPktPing;

// hello packet (14 to 64 bytes) ... first packet that should be sent after connection
#define USBPORT_CMD_HELLO	1	// command: hello packet
typedef struct {
	u16	crc;	// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: (1) total packet length (14 to 64 bytes)
	u8	cmd;	// 3: (1) command (= USBPORT_CMD_HELLO)
	u32	uid;	// 4: (4) program unique identification number
	u16	ver;	// 8: (2) protocol version in hundreths (100 = v1.00)
	u16	var;	// 10: (2) program variant
	u8	infolen; // 12: (1) length of info string 0..50 (without trailing zero)
	char	info[51]; // 13: (1..51) info string (typically ASCIIZ program name; with trailing zero)
} sUsbPortPktHello;

// requirements (usually 8 bytes) ... mask of reqired packets to get from opposite side
#define USBPORT_CMD_REQ		2	// command: requirements
typedef struct {
	u16	crc;	// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: (1) total packet length (usually 8)
	u8	cmd;	// 3: (1) command (= USBPORT_CMD_REQ)
	u32	req;	// 4: (usually 4) bit mask of required packets from opposite side (B1 = hello packet required)
} sUsbPortPktReq;

// set random seed (usually 12 bytes) ... usually sent by master on start to setup game
#define USBPORT_CMD_SEED	3	// command: set seed of random generator
typedef struct {
	u16	crc;	// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: (1) total packet length (usually 12)
	u8	cmd;	// 3: (1) command (= USBPORT_CMD_SEED)
	u32	seedL;	// 4: (4) seed low
	u32	seedH;	// 8: (4) seed high
} sUsbPortPktSeed;

// quit the game (4 bytes) ... cancel connection
#define USBPORT_CMD_QUIT	4	// command: quit the game
typedef struct {
	u16	crc;	// 0: (2) checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: (1) total packet length (= 4)
	u8	cmd;	// 3: (1) command (= USBPORT_CMD_QUIT)
} sUsbPortPktQuit;

// test pattern (8 to 64 bytes) ... used to check connection quality
#define USBPORT_CMD_TEST	5	// command: test pattern
#define USBPORT_TEST_DATAMAX	14	// max. number of u32 entries
typedef struct {
	u16	crc;	// 0: checksum Crc16AFast (CRC-16 CCITT normal)
	u8	len;	// 2: total packet length (8 to 64 bytes)
	u8	cmd;	// 3: command (USBPORT_CMD_TEST)
	u32	cnt;	// 4: sequence counter (to check dropped packets)
	u32	data[USBPORT_TEST_DATAMAX]; // 8: test pattern (random data, max. 56 bytes)
} sUsbPortPktTest;

// first application packet (packets 0..9 are reserved for system common packets)
#define USBPORT_CMD_APP		10	// first application packet

// check if transmit channel is ready to send next packet
INLINE Bool UsbPortSendPktReady(u8 chan) { return UsbPortSendChanReady(chan); }

// send packet
//  chan ... channel 1..15
//  pkt ... buffer with sUsbPortPkt packet to send (crc will be calculated in temporary buffer, len must be filled)
// Returns:
//   USBPORT_ERR_OK ... packet was sent OK
//   USBPORT_ERR_INIT ... driver is not initialized
//   USBPORT_ERR_CON ... opposite device is not connected
//   USBPORT_ERR_BUSY ... channel is busy (transfer is in progress)
//   USBPORT_ERR_LEN ... incorrect length of formatted packet
u8 UsbPortSendPkt(u8 chan, const void* pkt);

// check if next received packet is ready
//  chan ... channel 1..15
INLINE Bool UsbPortRecvPktReady(u8 chan) { return UsbPortRecvChanReady(chan); }

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
u8 UsbPortRecvPkt(u8 chan, void* pkt);

// ----------------------------------------------------------------------------
//                      Single-channel simple transfer
// ----------------------------------------------------------------------------

// channels
#define USBPORT_OUT	1	// direction OUT from host to device
#define USBPORT_IN	2	// direction IN from device to host

// check if transmit channel is ready to send next packet
Bool UsbPortSendReady();

// send packet (returns False on transfer error)
//  pkt ... buffer with sUsbPortPkt packet to send (crc will be calculated, len must be filled)
Bool UsbPortSend(const void* pkt);

// check if next received packet is ready
Bool UsbPortRecvReady();

// receive packet (returns False if packet not received)
//  pkt ... buffer of size 64 bytes to receive sUsbPortPkt packet
Bool UsbPortRecv(void* pkt);

#ifdef __cplusplus
}
#endif

#endif // _SDK_USBPORT_H

#endif // USE_USBPORT		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
