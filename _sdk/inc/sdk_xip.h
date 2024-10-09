
// ****************************************************************************
//
//                            XIP flash control
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

#ifndef _SDK_XIP_H
#define _SDK_XIP_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_xip.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_xip.h"	// constants of original SDK
#endif

#define XIP_STAT_FIFO_FULL XIP_STAT_FIFO_FULL_BITS
#define XIP_STAT_FIFO_EMPTY XIP_STAT_FIFO_EMPTY_BITS
#define XIP_STAT_FLUSH_RDY XIP_STAT_FLUSH_READY_BITS

#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// XIP hardware registers
#define XIP_CTRL	((volatile u32*)(XIP_CTRL_BASE + 0x00)) // control
#define XIP_FLUSH	((volatile u32*)(XIP_CTRL_BASE + 0x04)) // flush
#define XIP_STAT	((volatile u32*)(XIP_CTRL_BASE + 0x08)) // status
#define XIP_CTRL_HIT	((volatile u32*)(XIP_CTRL_BASE + 0x0C)) // cache hit counter
#define XIP_CTRL_ACC	((volatile u32*)(XIP_CTRL_BASE + 0x10)) // cache access counter
#define XIP_STREAM_ADDR	((volatile u32*)(XIP_CTRL_BASE + 0x14)) // FIFO stream address
#define XIP_STREAM_CTR	((volatile u32*)(XIP_CTRL_BASE + 0x18)) // FIFO stream control
#define XIP_STREAM_FIFO	((volatile u32*)(XIP_CTRL_BASE + 0x1C)) // FIFO stream data

#if !RP2040
#define XIP_STREAM	((volatile u32*)(XIP_AUX_BASE + 0x00))	// read XIP stream FIFO
#define XIP_DIRECT_TX	((volatile u32*)(XIP_AUX_BASE + 0x04))	// write to QMI direct-mode TX FIFO
#define XIP_DIRECT_RX	((volatile u32*)(XIP_AUX_BASE + 0x08))	// read from QMI direct-mode RX FIFO
#endif

// XIP hardware interface
typedef struct {
	io32	ctrl;		// 0x00: Cache control
#if RP2040
	io32	flush;		// 0x04: Cache Flush control
#else
	io32	_pad0;		// 0x04_
#endif
	io32	stat;		// 0x08: Cache Status
	io32	ctr_hit;	// 0x0C: Cache Hit counter
	io32	ctr_acc;	// 0x10: Cache Access counter
	io32	stream_addr;	// 0x14: FIFO stream address
	io32	stream_ctr;	// 0x18: FIFO stream control
	io32	stream_fifo;	// 0x1C: FIFO stream data
} xip_ctrl_hw_t;

#define xip_ctrl_hw ((xip_ctrl_hw_t*)XIP_CTRL_BASE)

STATIC_ASSERT(sizeof(xip_ctrl_hw_t) == 0x20, "Incorrect xip_ctrl_hw_t!");

#if !RP2040
typedef struct {
	io32	stream;		// 0x00: read XIP stream FIFO
	io32	qmi_direct_tx;	// 0x04: write to QMI direct-mode TX FIFO
	io32	qmi_direct_rx;	// 0x08: read from QMI direct-mode RX FIFO
} xip_aux_hw_t;

#define xip_aux_hw ((xip_aux_hw_t *)XIP_AUX_BASE)

STATIC_ASSERT(sizeof(xip_aux_hw_t) == 0x0C, "Incorrect xip_aux_hw_t!");
#endif

#if RP2040
// flush flash cache, no wait
INLINE void FlashCacheFlush() { xip_ctrl_hw->flush = 1; }

// flush flash cache and wait to complete
void FlashCacheFlushWait();
#endif

// enable flash cache (RP2350: cache for secure accesses)
INLINE void FlashCacheEnable() { RegSet(&xip_ctrl_hw->ctrl, B0); }

// disable flash cache (RP2350: cache for secure accesses)
INLINE void FlashCacheDisable() { RegClr(&xip_ctrl_hw->ctrl, B0); }

#ifdef __cplusplus
}
#endif

#endif // _SDK_XIP_H
