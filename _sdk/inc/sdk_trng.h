
// ****************************************************************************
//
//                  TRNG True Random Number Generator (only RP2350)
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

// Generation speed: 7.5 kb/s on 150 MHz
// Recommended setting (average generation 2 ms): chain length 0 or 1, sample count 20-25.
// Suitable alternative is function RoscRand32().

#if USE_TRNG && !RP2040		// use TRNG true random number generator (sdk_trng.h)

#ifndef _SDK_TRNG_H
#define _SDK_TRNG_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// TRNG hardware interface
typedef struct {
	io32	rng_imr;		// 0x00: Interrupt masking
	io32	rng_isr;		// 0x04: R/O RNG status register
	io32	rng_icr;		// 0x08: Interrupt/status bit clear Register
	io32	trng_config;		// 0x0C: Selecting the inverter-chain length
	io32	trng_valid;		// 0x10: R/O 192 bit collection indication
	io32	ehr_data[6];		// 0x14: R/O RNG collected bits (192 bits)
	io32	rnd_source_enable;	// 0x2C: Enable signal for the random source
	io32	sample_cnt1;		// 0x30: Counts clocks between sampling of random bit
	io32	autocorr_statistic;	// 0x34: Statistic about Autocorrelation test activations
	io32	trng_debug_control;	// 0x38: Debug register
	io32	_pad0;			// 0x3C:
	io32	trng_sw_reset;		// 0x40: Generate internal SW reset within the RNG block
	io32	_pad1[28];		// 0x44:
	io32	rng_debug_en_input;	// 0xB4: Enable the RNG debug mode
	io32	trng_busy;		// 0xB8: RNG Busy indication
	io32	rst_bits_counter;	// 0xBC: Reset the counter of collected bits in the RNG
	io32	rng_version;		// 0xC0: Displays the version settings of the TRNG
	io32	_pad[7];		// 0xC4:
	io32	rng_bist_cntr[3];	// 0xE0: Collected BIST results
} trng_hw_t;

#define trng_hw ((trng_hw_t*)(TRNG_BASE + 0x100))

STATIC_ASSERT(sizeof(trng_hw_t) == 0xEC, "Incorrect trng_hw_t!");

// Interrupt: TRNG_IRQ_IRQn

// enable interrupt on von Neumann error (= 32 consecutive collected bits are identical)
INLINE void TRNG_VnErrEnable(void) { RegClr(&trng_hw->rng_imr, B3); }

// disable interrupt on von Neumann error (= 32 consecutive collected bits are identical; default state)
INLINE void TRNG_VnErrDisable(void) { RegSet(&trng_hw->rng_imr, B3); }

// clear von Neumann error (= 32 consecutive collected bits are identical)
INLINE void TRNG_VnErrClr(void) { trng_hw->rng_icr = B3; }

// check von Neumann error (= 32 consecutive collected bits are identical)
INLINE Bool TRNG_VnErr(void) { return (trng_hw->rng_imr & B3) != 0; }

// enable interrupt on CRNGT error (= two consecutive blocks of 16 collected bits are equal)
INLINE void TRNG_CrngtErrEnable(void) { RegClr(&trng_hw->rng_imr, B2); }

// disable interrupt on CRNGT error (= two consecutive blocks of 16 collected bits are equal; default state)
INLINE void TRNG_CrngtErrDisable(void) { RegSet(&trng_hw->rng_imr, B2); }

// clear CRNGT error (= two consecutive blocks of 16 collected bits are equal)
INLINE void TRNG_CrngtClr(void) { trng_hw->rng_icr = B2; }

// check CRNGT error (= two consecutive blocks of 16 collected bits are equal)
INLINE Bool TRNG_CrngtErr(void) { return (trng_hw->rng_imr & B2) != 0; }

// enable interrupt on Autocorrelation error (= autocorrelation test failed four times in a row)
INLINE void TRNG_AutocorErrEnable(void) { RegClr(&trng_hw->rng_imr, B1); }

// disable interrupt on Autocorrelation error (= autocorrelation test failed four times in a row; default state)
INLINE void TRNG_AutocorErrDisable(void) { RegSet(&trng_hw->rng_imr, B1); }

// check Autocorrelation error (= autocorrelation test failed four times in a row)
// - cannot be cleared by SW
INLINE Bool TRNG_AutocorErr(void) { return (trng_hw->rng_imr & B1) != 0; }

// enable interrupt on 192 bits collected ready
INLINE void TRNG_EhrValidEnable(void) { RegClr(&trng_hw->rng_imr, B0); }

// disable interrupt on 192 bits collected ready (default state)
INLINE void TRNG_EhrValidDisable(void) { RegSet(&trng_hw->rng_imr, B0); }

// clear 192 bits collected ready flag
INLINE void TRNG_EhrValidClr(void) { trng_hw->rng_icr = B0; }

// check if 192 bits collected ready
INLINE Bool TRNG_EhrValid(void) { return (trng_hw->rng_imr & B0) != 0; }

// select number of inverters of the ROSC oscillator (0..3, higher values select longer inverter chain lengths)
INLINE void TRNG_SrcSel(int sel) { trng_hw->trng_config = sel; }

// check if collection of bits are completed
INLINE Bool TRNG_Valid(void) { return (trng_hw->trng_valid & B0) != 0; }

// get result data (index 0..5)
INLINE u32 TRNG_Data(int inx) { return trng_hw->ehr_data[inx]; }

// enable RNG source
INLINE void TRNG_SrcEnable(void) { trng_hw->rnd_source_enable = B0; }

// disable RNG source
INLINE void TRNG_SrcDisable(void) { trng_hw->rnd_source_enable = 0; }

// set count clocks between sampling of random bits (min. 17)
INLINE void TRNG_SampleCnt(int cnt) { trng_hw->sample_cnt1 = cnt; }

// get count of autorocelation test starts
INLINE int TRNG_AutocorrTrys(void) { return trng_hw->autocorr_statistic & 0x3fff; }

// reset count of autorocelation test starts
INLINE void TRNG_AutocorrTrysClr(void) { RegMask(&trng_hw->autocorr_statistic, 0, 0x3fff); }

// internal reset TRNG generator
INLINE void TRNG_SwReset(void) { trng_hw->trng_sw_reset = B0; }

// RNG is busy
INLINE Bool TRNG_Busy(void) { return (trng_hw->trng_busy & B0) != 0; }

// reset counter of collected bits
INLINE void TRNG_BitsRst(void) { trng_hw->rst_bits_counter = B0; }

// get RNG BIST counter (bist = 0..2)
INLINE int TRNG_BistCnt(int bist) { return trng_hw->rng_bist_cntr[bist] & 0x3fffff; }

#ifdef __cplusplus
}
#endif

#endif // _SDK_TRNG_H

#endif // USE_TRNG && !RP2040
