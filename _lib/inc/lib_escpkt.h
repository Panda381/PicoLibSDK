
// ****************************************************************************
//
//                           Escape Packet Protocol
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

// Control characters:
//  NUL 0x00 ... (Null) ignored character, can be used as idle transmission
//  STX 0x02 ... (Start of Text) start of packet data
//  ETX 0x03 ... (End of Text) end of packet data
//  SYN 0x16 ... (Synchronous Idle) used in synchronnous transmission to synchronise signal, or as ping character
//  ESC 0x1B ... (Escape) escape character before charactets NUL, STX, ETX, SYN or ESC.
//               Offset 0x40 (64) is added to the control character - they are changed
//               to @, B, C, V or [ characters.

#if USE_ESCPKT	// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)

#ifndef _LIB_ESCPKT_H
#define _LIB_ESCPKT_H

#include "lib_ring.h"

#ifdef __cplusplus
extern "C" {
#endif

// protocol descriptor
typedef struct {
	sRing*	tx_ring;	// pointer to transmission ring buffer descriptor (NULL = transmission not used)
	sRing*	rx_ring;	// pointer to receiving ring buffer descriptor (NULL = receiving is not used)
	int	rcv_num;	// number of bytes in receive buffer (-1 = waiting for STX)
	Bool	esc;		// flag of escape character received
} sEscPkt;

// initialize descriptor
//  esc ... pointer to sEscPkt descriptor
//  tx_ring ... pointer to transmission ring buffer descriptor (NULL = transmission not used)
//  rx_ring ... pointer to receiving ring buffer descriptor (NULL = receiving is not used)
void EscPktInit(sEscPkt* esc, sRing* tx_ring, sRing* rx_ring);

// receive packet, without waiting (returns length of received packet if result is >= 0, or -1 if packet not received)
//  esc ... pointer to sEscPkt descriptor
//  buf ... destination buffer (must be 2 bytes larger than longest packet, as reserve for CRC-16)
//  bufsize ... size of destination buffer (must be 2 bytes larger than longest packet, as reserve for CRC-16)
// If packet not received (-1 is returned), packet buffer may contain part of received data.
// Do not change buffer contents and repeat operation with the same settings until packet is received.
int EscPktReceive(sEscPkt* esc, u8* buf, int bufsize);

// send packet, with waiting
//  esc ... pointer to sEscPkt descriptor
//  buf ... source buffer
//  len ... length of packet data in source buffer
void EscPktSend(sEscPkt* esc, const u8* buf, int len);

#ifdef __cplusplus
}
#endif

#endif // _LIB_ESCPKT_H

#endif // USE_ESCPKT	// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)
