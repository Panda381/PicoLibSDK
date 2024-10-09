
// ****************************************************************************
//
//                                   FIFO
//                             Inter-core FIFO
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

#if USE_FIFO	// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)

#ifndef _SDK_FIFO_H
#define _SDK_FIFO_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_sio.h"			// SIO registers

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_sio.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_sio.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// === Inter-core FIFO

// FIFO hardware registers
//  (SIO does not support aliases for atomic access!)
#define FIFO_STATE	((volatile u32*)(SIO_BASE+0x50)) // inter-core FIFO status
#define FIFO_WRITE	((volatile u32*)(SIO_BASE+0x54)) // inter-core TX FIFO, write access
#define FIFO_READ	((volatile u32*)(SIO_BASE+0x58)) // inter-core RX FIFO, read access

// FIFO hardware interface
typedef struct {
	io32	state;		// 0x00: inter-core FIFO status
	io32	write;		// 0x04: inter-core TX FIFO, write access
	io32	read;		// 0x08: inter-core RX FIFO, read access
} fifo_hw_t;

#define	fifo_hw	((fifo_hw_t*)(SIO_BASE+0x50))

STATIC_ASSERT(sizeof(fifo_hw_t) == 0x0C, "Incorrect fifo_hw_t!");

// inter-core FIFO status (register FIFO_STATE)
#define FIFO_ROE	B3	// sticky flag, RX FIFO was read when empty (underflow error; write to clear)
#define FIFO_WOF	B2	// sticky flag, TX FIFO was written when full (overflow error, write to clear)
#define FIFO_RDY	B1	// 1=this core's TX FIFO is not full (ready for write; read only)
#define FIFO_VLD	B0	// 1=this core's RX FIFO is not empty (ready for read; read only)

// check if inter-core RX FIFO is ready to read
INLINE Bool FifoReadReady(void) { return (fifo_hw->state & FIFO_VLD) != 0; }

// check if inter-core TX FIFO is ready to write
INLINE Bool FifoWriteReady(void) { return (fifo_hw->state & FIFO_RDY) != 0; }

// write message to other core (wait if not ready)
void FifoWrite(u32 data);

// write message to other core with time-out in [us] (wait if not ready, max. 71 minutes, returns False on time-out)
Bool FifoWriteTime(u32 data, u32 us);

// read message from other core (wait if not ready)
u32 FifoRead(void);

// read message from other core with time-out in [us] (wait if not ready, max. 71 minutes, returns False on time-out)
Bool FifoReadTime(u32* data, u32 us);

// FIFO hanshake - send code and wait for same response
void FifoHandshake(u32 code);

// get FIFO status (FIFO_STATE register)
INLINE u32 FifoState() { return fifo_hw->state; }

// flush all read messages
INLINE void FifoReadFlush(void) { while (FifoReadReady()) (void)fifo_hw->read; }

// clear FIFO interrupt request (clear FIFO_ROE and FIFO_WOF error flags)
INLINE void FifoClear(void) { fifo_hw->state = 0xff; }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Check the read FIFO to see if there is data available (sent by the other core)
INLINE bool multicore_fifo_rvalid(void) { return FifoReadReady(); }

// Check the write FIFO to see if it has space for more data
INLINE bool multicore_fifo_wready(void) { return FifoWriteReady(); }

// Push data on to the write FIFO (data to the other core).
INLINE void multicore_fifo_push_blocking(u32 data) { FifoWrite(data); }

// Push data on to the write FIFO (data to the other core) with timeout in [us].
INLINE bool multicore_fifo_push_timeout_us(u32 data, u64 timeout) { return FifoWriteTime(data, (u32)timeout); }

// Pop data from the read FIFO (data from the other core).
INLINE u32 multicore_fifo_pop_blocking(void) { return FifoRead(); }

// Pop data from the read FIFO (data from the other core) with timeout in [us].
INLINE bool multicore_fifo_pop_timeout_us(u64 timeout, u32* data) { return FifoReadTime(data, (u32)timeout); }

// Discard any data in the read FIFO
INLINE void multicore_fifo_drain(void) { FifoReadFlush(); }

// Clear FIFO interrupt
INLINE void multicore_fifo_clear_irq(void) { FifoClear(); }

// Get FIFO statuses
//  Bit | Description
//  ----|------------
//    3 | Sticky flag indicating the RX FIFO was read when empty (ROE). This read was ignored by the FIFO.
//    2 | Sticky flag indicating the TX FIFO was written when full (WOF). This write was ignored by the FIFO.
//    1 | Value is 1 if this core’s TX FIFO is not full (i.e. if FIFO_WR is ready for more data)
//    0 | Value is 1 if this core’s RX FIFO is not empty (i.e. if FIFO_RD is valid)
INLINE u32 multicore_fifo_get_status(void) { return FifoState(); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_FIFO_H

#endif // USE_FIFO	// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)
