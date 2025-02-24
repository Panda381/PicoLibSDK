
// ****************************************************************************
//
//                    HSTX High-speed serial transmit (only RP2350)
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

// HSTX balances all delays to GPIO outputs within 300 picoseconds.
// Maximum frequency for the HSTX clock is 150 MHz.
// Maximum output data rate is 300 Mb/s per pin (2 bits per clock cycle per pin).

#if USE_HSTX && !RP2040		// use HSTX (sdk_hstx.c, sdk_hstx.h)

#ifndef _SDK_HSTX_H
#define _SDK_HSTX_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

#define HSTX_GPIO_FIRST	12	// first GPIO pin with HSTX (bit 0)
#define HSTX_GPIO_LAST	19	// last GPIO pin with HSTX (bit 7)
#define HSTX_GPIO_NUM	8	// number of GPIO pins with HSTX

// HSTX commands (4 bits << 12)
#define HSTX_CMD_RAW		(0x0u << 12)	// write raw data
#define HSTX_CMD_RAW_REPEAT	(0x1u << 12)	// write raw data repeated
#define HSTX_CMD_TMDS		(0x2u << 12)	// write TMDS mark
#define HSTX_CMD_TMDS_REPEAT	(0x3u << 12)	// write TMDS mark repeated
#define HSTX_CMD_NOP		(0xfu << 12)	// no operation

// HSTX control interface
typedef struct {
	io32	csr;		// 0x00: HSTX Control Register
	io32	bit[8];		// 0x04: Data control register for output bit 0..7
	io32	expand_shift;	// 0x24: Configure the optional shifter inside the command expander
	io32	expand_tmds;	// 0x28: Configure the optional TMDS encoder inside the command expander
} hstx_ctrl_hw_t;

#define hstx_ctrl_hw ((hstx_ctrl_hw_t*)HSTX_CTRL_BASE)

STATIC_ASSERT(sizeof(hstx_ctrl_hw_t) == 0x2C, "Incorrect hstx_ctrl_hw_t!");

// HSTX FIFO interface
typedef struct {
	io32	stat;		// 0x00: FIFO status
	io32	fifo;		// 0x04: Write access to FIFO
} hstx_fifo_hw_t;

#define hstx_fifo_hw ((hstx_fifo_hw_t*)HSTX_FIFO_BASE)

STATIC_ASSERT(sizeof(hstx_fifo_hw_t) == 0x08, "Incorrect hstx_fifo_hw_t!");

// configure one pin to use HSTX (pin = 12..19; means bit 0..7)
void HSTX_InitGPIO(int pin);

// configure pins GPIO12..19 to use HSTX
// To use pin mask in range (first..last), use function RangeMask.
void HSTX_InitGPIOMask(u32 mask);

// configure all pins GPIO12..19 to use HSTX
INLINE void HSTX_InitGPIOAll() { HSTX_InitGPIOMask(RangeMask(HSTX_GPIO_FIRST, HSTX_GPIO_LAST)); }

// enable HSTX (start sending data)
INLINE void HSTX_Enable(void) { RegSet(&hstx_ctrl_hw->csr, B0); }

// disable HSTX (stop sending data)
INLINE void HSTX_Disable(void) { RegClr(&hstx_ctrl_hw->csr, B0); }

// enable command expander (commands will be inserted to data; HSTX must be disabled to change this flag)
INLINE void HSTX_CmdEnable(void) { RegSet(&hstx_ctrl_hw->csr, B1); }

// disable command expander (FIFO data passed directly out; HSTX must be disabled to change this flag)
INLINE void HSTX_CmdDisable(void) { RegClr(&hstx_ctrl_hw->csr, B1); }

// enable PIO connection (coupled mode)
// - SEL_P and SEL_N indices 24 to 31 will select bits from 8-bit PIO path
// - SEL_P and SEL_N indices 0 to 23 will index shift register as normal
// - PIO outputs are those same outputs as direct connect PIO to pins
// - HSTX must be clocked directly from sys_clk
INLINE void HSTX_PioEnable(void) { RegSet(&hstx_ctrl_hw->csr, B4); }

// disable PIO connection
// - SEL_P and SEL_N indices will index shift register as normal
INLINE void HSTX_PioDisable(void) { RegClr(&hstx_ctrl_hw->csr, B4); }

// select PIO for PIO coupled mode (pio = 0..2)
INLINE void HSTX_PioSel(int pio) { RegMask(&hstx_ctrl_hw->csr, pio << 5, B5+B6); }

// set shift - rotate shift register right after each cycle (shift = 0..31)
INLINE void HSTX_Shift(int shift) { RegMask(&hstx_ctrl_hw->csr, (shift & 0x1f) << 8, 0x1f << 8); }

// set shift left - rotate shift register left after each cycle (shift = 0..31)
INLINE void HSTX_ShiftLeft(int shift) { RegMask(&hstx_ctrl_hw->csr, ((32 - shift) & 0x1f) << 8, 0x1f << 8); }

// set number of shifts before refilling shift register from FIFO (num = 1..32)
INLINE void HSTX_NShifts(int num) { RegMask(&hstx_ctrl_hw->csr, (num & 0x1f) << 16, 0x1f << 16); }

// set initial phase of the generated clock in number of half periods of clock cycle (phase = 0..15)
INLINE void HSTX_Phase(int phase) { RegMask(&hstx_ctrl_hw->csr, phase << 24, 0x0f << 24); }

// set clock period of the generated clock in number of clock cycles (clk = 1..16)
INLINE void HSTX_Clk(int clk) { RegMask(&hstx_ctrl_hw->csr, (clk & 0x0f) << 28, 0x0f << 28); }

// select data bit 0..31 for the first half of clock cycle for output bit 0..7
//   bit ..... output bit 0..7 (= GPIO 12..19)
//   sel_p ... select data bit from shift register 0..31
INLINE void HSTX_SelP(int bit, int sel_p) { RegMask(&hstx_ctrl_hw->bit[bit], sel_p, 0x1f); }

// select data bit 0..31 for the second half of clock cycle for output bit 0..7
//   bit ..... output bit 0..7 (= GPIO 12..19)
//   sel_n ... select data bit from shift register 0..31
INLINE void HSTX_SelN(int bit, int sel_n) { RegMask(&hstx_ctrl_hw->bit[bit], sel_n << 8, 0x1f << 8); }

// set invert output bit 0..7
INLINE void HSTX_Inv(int bit) { RegSet(&hstx_ctrl_hw->bit[bit], B16); }

// set non-invert output bit 0..7 (default state)
INLINE void HSTX_NotInv(int bit) { RegClr(&hstx_ctrl_hw->bit[bit], B16); }

// set output of bit 0..7 from clock generator (sel_p and sel_n are ignored, inv can be used)
INLINE void HSTX_OutClk(int bit) { RegSet(&hstx_ctrl_hw->bit[bit], B17); }

// set output of bit 0..7 from shift register (default state)
INLINE void HSTX_OutShift(int bit) { RegClr(&hstx_ctrl_hw->bit[bit], B17); }

// set number of bits to right-rotate shift register by each time data is pushed to the output shifter, when command is raw data (0..31)
INLINE void HSTX_RawShift(int shift) { RegMask(&hstx_ctrl_hw->expand_shift, shift, 0x1f); }

// set number of times of shifts before refilling from FIFO, when command is raw data (1..32)
INLINE void HSTX_RawNShifts(int shifts) { RegMask(&hstx_ctrl_hw->expand_shift, (shifts & 0x1f) << 8, 0x1f << 8); }

// set number of bits to right-rotate shift register by each time data is pushed to the output shifter, when command is encoded data (0..31)
INLINE void HSTX_EncShift(int shift) { RegMask(&hstx_ctrl_hw->expand_shift, shift << 16, 0x1f << 16); }

// set number of times of shifts before refilling from FIFO, when command is encoded data (1..32)
INLINE void HSTX_EncNShifts(int shifts) { RegMask(&hstx_ctrl_hw->expand_shift, (shifts & 0x1f) << 24, 0x1f << 24); }

// set right-rotate TMDS shifter data of lane 0 (0..31)
INLINE void HSTX_L0Rot(int rot) { RegMask(&hstx_ctrl_hw->expand_tmds, rot, 0x1f); }

// set number of data bits of TMDS lane 0 (1..8)
INLINE void HSTX_L0Bits(int bits) { RegMask(&hstx_ctrl_hw->expand_tmds, (bits-1) << 5, 0x0f << 5); }

// set right-rotate TMDS shifter data of lane 1 (0..31)
INLINE void HSTX_L1Rot(int rot) { RegMask(&hstx_ctrl_hw->expand_tmds, rot << 8, 0x1f << 8); }

// set number of data bits of TMDS lane 1 (1..8)
INLINE void HSTX_L1Bits(int bits) { RegMask(&hstx_ctrl_hw->expand_tmds, (bits-1) << 13, 0x0f << 13); }

// set right-rotate TMDS shifter data of lane 2 (0..31)
INLINE void HSTX_L2Rot(int rot) { RegMask(&hstx_ctrl_hw->expand_tmds, rot << 16, 0x1f << 16); }

// set number of data bits of TMDS lane 2 (1..8)
INLINE void HSTX_L2Bits(int bits) { RegMask(&hstx_ctrl_hw->expand_tmds, (bits-1) << 21, 0x0f << 21); }

// check if HSTX FIFO is overflow (write when full)
INLINE Bool HSTX_FifoIsOver(void) { return (hstx_fifo_hw->stat & B10) != 0; }

// clear HSTX FIFO overflow flag
INLINE void HSTX_FifoClrOver(void) { hstx_fifo_hw->stat = B10; }

// check if HSTX FIFO is empty
INLINE Bool HSTX_FifoIsEmpty(void) { return (hstx_fifo_hw->stat & B9) != 0; }

// check if HSTX FIFO is full
INLINE Bool HSTX_FifoIsFull(void) { return (hstx_fifo_hw->stat & B8) != 0; }

// get current level of HSTX FIFO (0..7)
INLINE int HSTX_FifoLevel(void) { return hstx_fifo_hw->stat & 7; }

// write data to HSTX FIFO
INLINE void HSTX_FifoWrite(u32 data) { hstx_fifo_hw->fifo = data; }

#ifdef __cplusplus
}
#endif

#endif // _SDK_HSTX_H

#endif // USE_HSTX && !RP2040
