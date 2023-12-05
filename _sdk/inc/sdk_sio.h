
// ****************************************************************************
//
//                              SIO registers
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

#ifndef _SDK_SIO_H
#define _SDK_SIO_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// interpolator register array (size 0x40)
typedef struct {
	io32	accum[2];	// 0x00: Read/write access to accumulator 0
	io32	base[3];	// 0x08: Read/write access to BASE0 register
	io32	pop[3];		// 0x14: Read LANE0 result, and simultaneously write lane results to both accumulators (POP)
	io32	peek[3];	// 0x20: Read LANE0 result, without altering any internal state (PEEK)
	io32	ctrl[2];	// 0x2C: Control register for lane 0
	io32	add_raw[2];	// 0x34: Values written here are atomically added to ACCUM0
	io32	base01;		// 0x3C: On write, the lower 16 bits go to BASE0, upper bits to BASE1 simultaneously
} interp_hw_t;

STATIC_ASSERT(sizeof(interp_hw_t) == 0x40, "Incorrect interp_hw_t!");

// SIO register array (size 0x100)
typedef struct {
	io32	cpuid;		// 0x00: Processor core identifier
	io32	gpio_in;	// 0x04: Input value for GPIO pins
	io32	gpio_hi_in;	// 0x08: Input value for QSPI pins
	io32	_pad0;		// 0x0C:
	io32	gpio_out;	// 0x10: GPIO output value
	io32	gpio_set;	// 0x14: GPIO output value set
	io32	gpio_clr;	// 0x18: GPIO output value clear
	io32	gpio_togl;	// 0x1C: GPIO output value XOR
	io32	gpio_oe;	// 0x20: GPIO output enable
	io32	gpio_oe_set;	// 0x24: GPIO output enable set
	io32	gpio_oe_clr;	// 0x28: GPIO output enable clear
	io32	gpio_oe_togl;	// 0x2C: GPIO output enable XOR
	io32	gpio_hi_out;	// 0x30: QSPI output value
	io32	gpio_hi_set;	// 0x34: QSPI output value set
	io32	gpio_hi_clr;	// 0x38: QSPI output value clear
	io32	gpio_hi_togl;	// 0x3C: QSPI output value XOR
	io32	gpio_hi_oe;	// 0x40: QSPI output enable
	io32	gpio_hi_oe_set;	// 0x44: QSPI output enable set
	io32	gpio_hi_oe_clr;	// 0x48: QSPI output enable clear
	io32	gpio_hi_oe_togl; // 0x4C: QSPI output enable XOR
	io32	fifo_st;	// 0x50: Status register for inter-core FIFOs (mailboxes)
	io32	fifo_wr;	// 0x54: Write access to this core's TX FIFO
	io32	fifo_rd;	// 0x58: Read access to this core's RX FIFO
	io32	spinlock_st;	// 0x5C: Spinlock state
	io32	div_udividend;	// 0x60: Divider unsigned dividend
	io32	div_udivisor;	// 0x64: Divider unsigned divisor
	io32	div_sdividend;	// 0x68: Divider signed dividend
	io32	div_sdivisor;	// 0x6C: Divider signed divisor
	io32	div_quotient;	// 0x70: Divider result quotient
	io32	div_remainder;	// 0x74: Divider result remainder
	io32	div_csr;	// 0x78: Control and status register for divider
	io32	_pad1;		// 0x7C:
	interp_hw_t interp[2];	// 0x80: interpolator
} sio_hw_t;

#define sio_hw ((sio_hw_t*)SIO_BASE)

#define interp_hw_array sio_hw->interp
#define interp0_hw (&interp_hw_array[0])
#define interp1_hw (&interp_hw_array[1])

STATIC_ASSERT(sizeof(sio_hw_t) == 0x100, "Incorrect sio_hw_t!");

#ifdef __cplusplus
}
#endif

#endif // _SDK_SIO_H
