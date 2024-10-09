
// ****************************************************************************
//
//                         SHA-256 accelerator (only RP2350)
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

// Bit stream message must be terminated with bit "1", zero bits and 64-bit block indicating length of message.
// Number of zero bits must ensure padding block to modulo 512 bits.
// Example, message = text "abc" (beware of the order of the bits):
//  01100001 01100010 01100011  1  00000000...00000000000 00000000 00000000 00000000 00011000
//  |---message (24 bits)----|  1  |--- 423 bits '0' ---| |---------64 bit len (=24)--------|

// How to use:
// - Add ending mark '1', padding zero bits and 64-bit length (beware order of bits) 
//   to end of message, so the result size will be multiple of 64 bytes.
// - Start new checksum SHA256_Start()
// - Write message using DMA or by software, wait if not ready
// - Read result with SHA256_GetResult()

#if USE_SHA256 && !RP2040		// use SHA-256 accelerator (sdk_sha256.c, sdk_sha256.h)

#ifndef _SDK_SHA256_H
#define _SDK_SHA256_H

#include "../sdk_addressmap.h"		// Register address offsets

#ifdef __cplusplus
extern "C" {
#endif

// SHA-256 hardware interface
typedef struct {
	io32	csr;		// 0x00: Control and status register
	io32	wdata;		// 0x04: Write data register
	io32	sum[8];		// 0x08: 256-bit checksum resulz
} sha256_hw_t;

#define sha256_hw ((sha256_hw_t*)SHA256_BASE)

STATIC_ASSERT(sizeof(sha256_hw_t) == 0x28, "Incorrect sha256_hw_t!");

// data size
#define SHA256_DATA_8	0	// data size 8 bits
#define SHA256_DATA_16	1	// data size 16 bits
#define SHA256_DATA_32	2	// data size 32 bits

// start SHA-256 core for a new checksum
// - SUM registers are initialised and internal counters are cleared
// - must be written before initiating DMA transfer, core will request 16 transfers of 32-bit
INLINE void SHA256_Start(void) { RegSet(&sha256_hw->csr, B0); }

// SHA-256 core is ready to accept mode data
INLINE Bool SHA256_Ready(void) { return (sha256_hw->csr & B1) != 0; }

// SHA-256 checksum in registers SUM is valid
INLINE Bool SHA256_Valid(void) { return (sha256_hw->csr & B2) != 0; }

// check error - write when not ready
INLINE Bool SHA256_ErrWrite(void) { return (sha256_hw->csr & B4) != 0; }

// clear error of write when not ready
INLINE void SHA256_ErrWriteClr(void) { RegSet(&sha256_hw->csr, B4); }

// set data size SHA256_DATA_*
INLINE void SHA256_DataSize(int size) { RegMask(&sha256_hw->csr, size << 8, 3 << 8); }

// enable byte swapping of 32-bit values (convert CPU little-endian to SHA-256 big-endian; default state)
INLINE void SHA256_BswapEnable() { RegSet(&sha256_hw->csr, B12); }

// disable byte swapping of 32-bit values (data are pre-formated in big-endian)
INLINE void SHA256_BswapDisable() { RegClr(&sha256_hw->csr, B12); }

// write data (write 16 words of 32-bits)
INLINE void SHA256_Write(u32 data) { sha256_hw->wdata = data; }

// get address of the data register to write using DMA
INLINE volatile u32* SHA256_DataAddr(void) { return &sha256_hw->wdata; }

// get result data (inx = 0..7)
INLINE u32 SHA256_Sum(int inx) { return sha256_hw->sum[inx]; }

// wait until hardware is ready to accept more data
INLINE void SHA256_WaitReady(void) { while (!SHA256_Ready()) {} }

// wait until checksum is valid
INLINE void SHA256_WaitValid(void) { while (!SHA256_Valid()) {} }

// read 256-bit result, 8 words of 32-bit (must be valid; bigend = get result as big-endian)
INLINE void SHA256_GetResult(u32* dst, Bool bigend)
{
	int i;
	for (i = 0; i < 8; i++)
	{
		u32 d = sha256_hw->sum[i];
		if (bigend) d = Endian(d);
		dst[i] = d;
	}
}

#ifdef __cplusplus
}
#endif

#endif // _SDK_SHA256_H

#endif // USE_SHA256 && !RP2040
