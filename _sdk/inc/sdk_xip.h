
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
#include "orig/orig_xip.h"	// constants of original SDK

#define XIP_STAT_FIFO_FULL XIP_STAT_FIFO_FULL_BITS
#define XIP_STAT_FIFO_EMPTY XIP_STAT_FIFO_EMPTY_BITS
#define XIP_STAT_FLUSH_RDY XIP_STAT_FLUSH_READY_BITS

#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// XIP hardware registers
//#define XIP_CTRL_BASE		0x14000000
#define XIP_CTRL	((volatile u32*)(XIP_CTRL_BASE + 0x00)) // control
#define XIP_FLUSH	((volatile u32*)(XIP_CTRL_BASE + 0x04)) // flush
#define XIP_STAT	((volatile u32*)(XIP_CTRL_BASE + 0x08)) // status
#define XIP_CTRL_HIT	((volatile u32*)(XIP_CTRL_BASE + 0x0C)) // cache hit counter
#define XIP_CTRL_ACC	((volatile u32*)(XIP_CTRL_BASE + 0x10)) // cache access counter
#define XIP_STREAM_ADDR	((volatile u32*)(XIP_CTRL_BASE + 0x14)) // FIFO stream address
#define XIP_STREAM_CTR	((volatile u32*)(XIP_CTRL_BASE + 0x18)) // FIFO stream control
#define XIP_STREAM_FIFO	((volatile u32*)(XIP_CTRL_BASE + 0x1C)) // FIFO stream data

// XIP hardware interface
typedef struct {
	io32	ctrl;		// 0x00: Cache control
	io32	flush;		// 0x04: Cache Flush control
	io32	stat;		// 0x08: Cache Status
	io32	ctr_hit;	// 0x0C: Cache Hit counter
	io32	ctr_acc;	// 0x10: Cache Access counter
	io32	stream_addr;	// 0x14: FIFO stream address
	io32	stream_ctr;	// 0x18: FIFO stream control
	io32	stream_fifo;	// 0x1C: FIFO stream data
} xip_ctrl_hw_t;

#define xip_ctrl_hw ((xip_ctrl_hw_t*)XIP_CTRL_BASE)

STATIC_ASSERT(sizeof(xip_ctrl_hw_t) == 0x20, "Incorrect xip_ctrl_hw_t!");

// flush flash cache, does not wait
INLINE void FlashCacheFlush() { xip_ctrl_hw->flush = 1; }

// flush flash cache and wait to complete
void FlashCacheFlushWait();

// enable flash cache
INLINE void FlashCacheEnable() { RegSet(&xip_ctrl_hw->ctrl, B0); }

// disable flash cache
INLINE void FlashCacheDisable() { RegClr(&xip_ctrl_hw->ctrl, B0); }

#ifdef __cplusplus
}
#endif

#endif // _SDK_XIP_H
