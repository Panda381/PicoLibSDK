
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

#include "../../global.h"	// globals

#if USE_ESCPKT	// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)

#include "../inc/lib_crc.h"
#include "../inc/lib_escpkt.h"

// initialize descriptor
//  esc ... pointer to sEscPkt descriptor
//  tx_ring ... pointer to transmission ring buffer descriptor (NULL = transmission not used)
//  rx_ring ... pointer to receiving ring buffer descriptor (NULL = receiving is not used)
void EscPktInit(sEscPkt* esc, sRing* tx_ring, sRing* rx_ring)
{
	esc->tx_ring = tx_ring;
	esc->rx_ring = rx_ring;
	esc->rcv_num = -1;
	esc->esc = False;
}

// receive packet, without waiting (returns length of received packet if result is >= 0, or -1 if packet not received)
//  esc ... pointer to sEscPkt descriptor
//  buf ... destination buffer (must be 2 bytes larger than longest packet, as reserve for CRC-16)
//  bufsize ... size of destination buffer (must be 2 bytes larger than longest packet, as reserve for CRC-16)
// If packet not received (-1 is returned), packet buffer may contain part of received data.
// Do not change buffer contents and repeat operation with the same settings until packet is received.
int EscPktReceive(sEscPkt* esc, u8* buf, int bufsize)
{
	u8 ch;
	int n;

	// no receiving ring buffer
	if (esc->rx_ring == NULL) return -1;

	// until there is some data
	while (RingRead8(esc->rx_ring, &ch))
	{
		switch (ch)
		{
		// NUL 0x00 ... (Null) ignored character, can be used as idle transmission
		case 0x00:
		//  SYN 0x16 ... (Synchronous Idle) used in synchronnous transmission to synchronise signal, or as ping character
		case 0x16:
			break;

		// STX 0x02 ... (Start of Text) start of packet data
		case 0x02:
			esc->rcv_num = 0; // start of receiving data
			esc->esc = False; // no escape
			break;

		// ETX 0x03 ... (End of Text) end of packet data
		case 0x03:
			n = esc->rcv_num; // number of bytes in receive buffer
			esc->rcv_num = -1; // start waiting for STX
			if (n >= 2) // minimal size of data, with reserve for CRC-16
			{
				// check CRC
				n -= 2; // length without CRC
				u16 crc = buf[n] + ((u16)buf[n+1] << 8); // get received CRC
				if (crc == Crc16AFast(buf, n)) return n; // packet is received OK
			}
			break;

		//  ESC 0x1B ... (Escape) escape character before charactets NUL, STX, ETX, SYN or ESC.
		//               Offset 0x40 (64) is added to the control character - they are changed
		//               to @, B, C, V or [ characters.
		case 0x1B:
			esc->esc = True; // set escape flag
			break;

		// receive another characters
		default:
			// if receiving data
			n = esc->rcv_num;
			if (n >= 0)
			{
				// buffer overflow
				if (n >= bufsize)
					esc->rcv_num = -1; // start waiting for STX
				else
				{
					// escape character
					if (esc->esc) ch -= 0x40; // shift offset
					esc->esc = False;

					// store character into buffer
					buf[n++] = ch;
					esc->rcv_num = n;
				}
			}
			break;
		}
	}

	// packet not received
	return -1;
}

// send packet, with waiting
//  esc ... pointer to sEscPkt descriptor
//  buf ... source buffer
//  len ... length of packet data in source buffer
void EscPktSend(sEscPkt* esc, const u8* buf, int len)
{
	int i;
	u8 ch;
	u16 crc;
	//Bool e = False;

	// no transmission ring buffer
	if (esc->tx_ring == NULL) return;

	// calculate CRC
	crc = Crc16AFast(buf, len);

	// send STX character
	RingWrite8Wait(esc->tx_ring, 0x02);

	// send data and CRC
	for (i = 0; i < len+2; i++)
	{
		// prepare next byte to send (data or CRC)
		ch = (i < len) ? buf[i] : ((i == len) ? (u8)crc : (u8)(crc >> 8));
	
		// need escape character
		if ((ch == 0x00) || (ch == 0x02) || (ch == 0x03) || (ch == 0x16) || (ch == 0x1B))
		{
			// send ESC character
			RingWrite8Wait(esc->tx_ring, 0x1B);

			// shift character offset
			ch += 0x40;
		}

		// send character
		RingWrite8Wait(esc->tx_ring, ch);
	}

	// send ETX character
	RingWrite8Wait(esc->tx_ring, 0x03);
}

#endif // USE_ESCPKT	// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)
