
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

// interpolator register array
typedef struct {
	io32	accum[2];	// 0x00: Read/write access to accumulator
	io32	base[3];	// 0x08: Read/write access to BASE register
	io32	pop[3];		// 0x14: Read LANE result, and simultaneously write lane results to both accumulators (POP)
	io32	peek[3];	// 0x20: Read LANE result, without altering any internal state (PEEK)
	io32	ctrl[2];	// 0x2C: Control register for lane
	io32	add_raw[2];	// 0x34: Values written here are atomically added to ACCUM
	io32	base01;		// 0x3C: On write, the lower 16 bits go to BASE0, upper bits to BASE1 simultaneously
} interp_hw_t;

STATIC_ASSERT(sizeof(interp_hw_t) == 0x40, "Incorrect interp_hw_t!");

// SIO register array
typedef struct {
	io32	cpuid;			// 0x00: Processor core identifier
	io32	gpio_in;		// 0x04: Input value for GPIO pins 0..31
	io32	gpio_hi_in;		// 0x08: Input value for GPIO pins 32..47, QSPI pins and USB pins
	io32	_pad0;			// 0x0C:
	io32	gpio_out;		// 0x10: output value for GPIO pins 0..29
#if RP2040
	io32	gpio_set;		// 0x14: set output value for GPIO pins 0..29
	io32	gpio_clr;		// 0x18: clear output value for GPIO pins 0..29
	io32	gpio_togl;		// 0x1C: XOR output value for GPIO pins 0..29
	io32	gpio_oe;		// 0x20: output enable for GPIO pins 0..29
	io32	gpio_oe_set;		// 0x24: set output enable for GPIO pins 0..29
	io32	gpio_oe_clr;		// 0x28: clear output enable for GPIO pins 0..29
	io32	gpio_oe_togl;		// 0x2C: XOR output enable for GPIO pins 0..29
	io32	gpio_hi_out;		// 0x30: QSPI output value
	io32	gpio_hi_set;		// 0x34: QSPI output value set
	io32	gpio_hi_clr;		// 0x38: QSPI output value clear
	io32	gpio_hi_togl;		// 0x3C: QSPI output value XOR
	io32	gpio_hi_oe;		// 0x40: QSPI output enable
	io32	gpio_hi_oe_set;		// 0x44: QSPI output enable set
	io32	gpio_hi_oe_clr;		// 0x48: QSPI output enable clear
	io32	gpio_hi_oe_togl;	// 0x4C: QSPI output enable XOR
	io32	fifo_st;		// 0x50: Status register for inter-core FIFOs (mailboxes)
	io32	fifo_wr;		// 0x54: Write access to this core's TX FIFO
	io32	fifo_rd;		// 0x58: Read access to this core's RX FIFO
	io32	spinlock_st;		// 0x5C: Spinlock state
	io32	div_udividend;		// 0x60: Divider unsigned dividend
	io32	div_udivisor;		// 0x64: Divider unsigned divisor
	io32	div_sdividend;		// 0x68: Divider signed dividend
	io32	div_sdivisor;		// 0x6C: Divider signed divisor
	io32	div_quotient;		// 0x70: Divider result quotient
	io32	div_remainder;		// 0x74: Divider result remainder
	io32	div_csr;		// 0x78: Control and status register for divider
	io32	_pad1;			// 0x7C:
	interp_hw_t interp[2];		// 0x80: interpolator
	io32	spinlock[32];		// 0x100: spin locks
#else // RP2350
	io32	gpio_hi_out;		// 0x14: output value for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_set;		// 0x18: set output value for GPIO pins 0..29
	io32	gpio_hi_set;		// 0x1C: set output value for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_clr;		// 0x20: clear output value for GPIO pins 0..29
	io32	gpio_hi_clr;		// 0x24: clear output value for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_togl;		// 0x28: XOR output value for GPIO pins 0..29
	io32	gpio_hi_togl;		// 0x2C: XOR output value for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_oe;		// 0x30: output enable for GPIO pins 0..29
	io32	gpio_hi_oe;		// 0x34: output enable for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_oe_set;		// 0x38: set output enable for GPIO pins 0..29
	io32	gpio_hi_oe_set;		// 0x3C: set output enable for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_oe_clr;		// 0x40: clear output enable for GPIO pins 0..29
	io32	gpio_hi_oe_clr;		// 0x44: clear output enable for GPIO pins 32..47, QSPI pins and USB pins
	io32	gpio_oe_togl;		// 0x48: XOR output enable for GPIO pins 0..29
	io32	gpio_hi_oe_togl;	// 0x4C: XOR output enable for GPIO pins 32..47, QSPI pins and USB pins
	io32	fifo_st;		// 0x50: Status register for inter-core FIFOs (mailboxes)
	io32	fifo_wr;		// 0x54: Write access to this core's TX FIFO
	io32	fifo_rd;		// 0x58: Read access to this core's RX FIFO
	io32	spinlock_st;		// 0x5C: Spinlock state
	io32	_pad1[8];		// 0x60:
	interp_hw_t interp[2];		// 0x80: interpolator
	io32	spinlock[32];		// 0x100: spin locks
	io32	doorbell_out_set;	// 0x180: trigger doorbell interrupt on opposite core (read opposite doorbells)
	io32	doorbell_out_clr;	// 0x184: clear doorbell interrupt on opposite core (read opposite doorbells)
	io32	doorbell_in_set;	// 0x188: trigger doorbell interrupt on this core (read this doorbells)
	io32	doorbell_in_clr;	// 0x18C: clear doorbell interrupt on this code (read this doorbells)
	io32	peri_nonsec;		// 0x190: secure SIO: detach periphery from secure SIO and attach to non-secure SIO
	io32	_pad2[3];		// 0x194:
	io32	riscv_softirq;		// 0x1A0: control assertion of standard software interrupt on RISC-V cores
	io32	mtime_ctrl;		// 0x1A4: secure SIO: control register for RISC-V 64-bit machine-mode timer
	io32	_pad3[2];		// 0x1A8:
	io32	mtime;			// 0x1B0: read/write access to low half of RISC-V machine-mode timer (shared between cores)
	io32	mtimeh;			// 0x1B4: read/write access to high half of RISC-V machine-mode timer (shared between cores)
	io32	mtimecmp;		// 0x1B8: low half of RISC-V machine-mode timer comparator (core-local)
	io32	mtimecmph;		// 0x1BC: high half of RISC-V machine-mode timer comparator (core-local)
	io32	tmds_ctrl;		// 0x1C0: control TMDS encoder
	io32	tmds_wdata;		// 0x1C4: W/O access to TMDS colour data register
	io32	tmds_peek_single;	// 0x1C8: get encoding of one pixel's worth of colour data, no shift register
	io32	tmds_pop_single;	// 0x1CC: get encoding of one pixel's worth of colour data, shift register
	io32	tmds_peek_double_l0;	// 0x1D0: get lane 0 encoding of two pixels' worth of colour data, no shift register
	io32	tmds_pop_double_l0;	// 0x1D4: get lane 0 encoding of two pixels' worth of colour data, shift register
	io32	tmds_peek_double_l1;	// 0x1D8: get lane 1 encoding of two pixels' worth of colour data, no shift register
	io32	tmds_pop_double_l1;	// 0x1DC: get lane 1 encoding of two pixels' worth of colour data, shift register
	io32	tmds_peek_double_l2;	// 0x1E0: get lane 2 encoding of two pixels' worth of colour data, no shift register
	io32	tmds_pop_double_l2;	// 0x1E4: get lane 2 encoding of two pixels' worth of colour data, shift register
#endif
} sio_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(sio_hw_t) == 0x180, "Incorrect sio_hw_t!");
#else
STATIC_ASSERT(sizeof(sio_hw_t) == 0x1e8, "Incorrect sio_hw_t!");
#endif

#define sio_hw ((sio_hw_t*)SIO_BASE)		// base access is to secure SIO

#define interp_hw_array sio_hw->interp
#define interp0_hw (&interp_hw_array[0])
#define interp1_hw (&interp_hw_array[1])

#if !RP2040
#define sio_ns_hw ((sio_hw_t*)SIO_NONSEC_BASE)	// non-secure SIO
#define interp_hw_array_ns sio_ns_hw->interp
#define interp0_hw_ns (&interp_hw_array_ns[0])
#define interp1_hw_ns (&interp_hw_array_ns[1])
#endif

#ifdef __cplusplus
}
#endif

#endif // _SDK_SIO_H
