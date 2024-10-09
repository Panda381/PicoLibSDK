
// ****************************************************************************
//
//                             TMDS Encoder (only RP2350)
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

#if USE_TMDS && !RP2040	// use TMDS encoder (sdk_tmds.h)

#ifndef _SDK_TMDS_H
#define _SDK_TMDS_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// TMDS hardware interface
//  (SIO does not support aliases for atomic access!)
typedef struct {
	io32	tmds_ctrl;		// 0x00: Control register for TMDS encoder
	io32	tmds_wdata;		// 0x04: W/O access to the TMDS colour data register
	io32	tmds_peek_single;	// 0x08: R/O Get the encoding of one pixel's worth of colour data, packed into a 32-bit value (3x10-bit symbols)
	io32	tmds_pop_single;	// 0x0C: R/O Get the encoding of one pixel's worth of colour data, packed into a 32-bit value
	io32	tmds_peek_double_l0;	// 0x10: R/O Get lane 0 of the encoding of two pixels' worth of colour data
	io32	tmds_pop_double_l0;	// 0x14: R/O Get lane 0 of the encoding of two pixels' worth of colour data
	io32	tmds_peek_double_l1;	// 0x18: R/O Get lane 1 of the encoding of two pixels' worth of colour data
	io32	tmds_pop_double_l1;	// 0x1C: R/O Get lane 1 of the encoding of two pixels' worth of colour data
	io32	tmds_peek_double_l2;	// 0x20: R/O Get lane 2 of the encoding of two pixels' worth of colour data
	io32	tmds_pop_double_l2;	// 0x24: R/O Get lane 2 of the encoding of two pixels' worth of colour data
} tmds_encode_hw_t;

#define tmds_encode_hw ((tmds_encode_hw_t*)(SIO_BASE + 0x01C0))
#define tmds_encode_ns_hw ((tmds_encode_hw_t*)(SIO_NONSEC_BASE + 0x01C0))

STATIC_ASSERT(sizeof(tmds_encode_hw_t) == 0x28, "Incorrect tmds_encode_hw_t!");

// pixel shift width
#define TMDS_SHIFT_NO	0	// do not shift colour data register
#define TMDS_SHIFT_1	1	// shift color data register by 1 bit (GRAY2)
#define TMDS_SHIFT_2	2	// shift color data register by 2 bits (GRAY4)
#define TMDS_SHIFT_4	3	// shift color data register by 4 bits (RGB121, GRAY16)
#define TMDS_SHIFT_8	4	// shift color data register by 8 bits (RGB332)
#define TMDS_SHIFT_16	5	// shift color data register by 16 bits (RGB565)

// clear DC balance
INLINE void TMDS_ClearDC(void) { tmds_encode_hw->tmds_ctrl |= B28; }

// enable pixel doubling - disable shift of 2nd encoder
INLINE void TMDS_Pix2NoShift(void) { tmds_encode_hw->tmds_ctrl |= B27; }

// disable pixel doubling - enable shift of 2nd encoder
INLINE void TMDS_Pix2Shift(void) { tmds_encode_hw->tmds_ctrl &= ~B27; }

// set pixel shift width TMDS_SHIFT_*
INLINE void TMDS_PixShift(int shift)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(B24+B25+B26)) | (shift << 24); }

// enable interleaving (symbols are packed into 5 chunks of 2 lanes times 2 bits, 30 bits total, each chunk 2 bits per lane)
INLINE void TMDS_InterEnable(void) { tmds_encode_hw->tmds_ctrl |= B23; }

// disable interleaving (each of 3 symbols appears as 10-bit field)
INLINE void TMDS_InterDisable(void) { tmds_encode_hw->tmds_ctrl &= ~B23; }

// set rotate of lane 0, blue (right-rotate 16 LSBs of the colour accumulator by 0-15 bits)
INLINE void TMDS_Lane0Rot(int rot)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(0x0f << 0)) | (rot << 0); }

// set rotate of lane 1, green (right-rotate 16 LSBs of the colour accumulator by 0-15 bits)
INLINE void TMDS_Lane1Rot(int rot)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(0x0f << 4)) | (rot << 4); }

// set rotate of lane 2, red (right-rotate 16 LSBs of the colour accumulator by 0-15 bits)
INLINE void TMDS_Lane2Rot(int rot)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(0x0f << 8)) | (rot << 8); }

// set bits of lane 0, blue (number of valid colour MSBs for lane 0; use value 1 to 8)
INLINE void TMDS_Lane0Bits(int bits)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(7 << 12)) | ((bits-1) << 12); }

// set bits of lane 1, green (number of valid colour MSBs for lane 1; use value 1 to 8)
INLINE void TMDS_Lane1Bits(int bits)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(7 << 15)) | ((bits-1) << 15); }

// set bits of lane 2, red (number of valid colour MSBs for lane 2; use value 1 to 8)
INLINE void TMDS_Lane2Bits(int bits)
	{ tmds_encode_hw->tmds_ctrl = (tmds_encode_hw->tmds_ctrl & ~(7 << 18)) | ((bits-1) << 18); }

// write data to colour register
INLINE void TMDS_Write(u32 data) { tmds_encode_hw->tmds_wdata = data; }

// peek encoded data as single value 3x10 bits, no shift colour register
INLINE u32 TMDS_Peek(void) { return tmds_encode_hw->tmds_peek_single; }

// pop encoded data as single value 3x10 bits, shift colour register
INLINE u32 TMDS_Pop(void) { return tmds_encode_hw->tmds_pop_single; }

// peek encoded data of lane 0 (blue) as two 10-bit symbols, no shift colour register
INLINE u32 TMDS_Peek0(void) { return tmds_encode_hw->tmds_peek_double_l0; }

// pop encoded data of lane 0 (blue) as two 10-bit symbols, shift colour register
INLINE u32 TMDS_Pop0(void) { return tmds_encode_hw->tmds_pop_double_l0; }

// peek encoded data of lane 1 (green) as two 10-bit symbols, no shift colour register
INLINE u32 TMDS_Peek1(void) { return tmds_encode_hw->tmds_peek_double_l1; }

// pop encoded data of lane 1 (green) as two 10-bit symbols, shift colour register
INLINE u32 TMDS_Pop1(void) { return tmds_encode_hw->tmds_pop_double_l1; }

// peek encoded data of lane 2 (red) as two 10-bit symbols, no shift colour register
INLINE u32 TMDS_Peek2(void) { return tmds_encode_hw->tmds_peek_double_l2; }

// pop encoded data of lane 2 (red) as two 10-bit symbols, shift colour register
INLINE u32 TMDS_Pop2(void) { return tmds_encode_hw->tmds_pop_double_l2; }

#ifdef __cplusplus
}
#endif

#endif // _SDK_TMDS_H

#endif // USE_TMDS && !RP2040
