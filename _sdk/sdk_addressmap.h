
// ****************************************************************************
//
//                          Register address offsets
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

// Included from both C and assembly.

#ifndef _SDK_ADDRESSMAP_H
#define _SDK_ADDRESSMAP_H

// register aliases for atomic access (register block is 4 KB, total register space is 16 KB; RP2040 datasheet page 18)
#define REG_ALIAS_RW	(0UL << 12)	// address base 0x0000, normal read/modify/write access
#define REG_ALIAS_XOR	(1UL << 12)	// address base 0x1000, atomic XOR
#define REG_ALIAS_SET	(2UL << 12)	// address base 0x2000, atomic SET
#define REG_ALIAS_CLR	(3UL << 12)	// address base 0x3000, atomic CLR

// atomic xor/set/clr u32 register bits (cannot be used for access to SIO registers!)
#define RegXor(addr, mask) *(volatile u32*)((u32)(addr) | REG_ALIAS_XOR) = (mask)
#define RegSet(addr, mask) *(volatile u32*)((u32)(addr) | REG_ALIAS_SET) = (mask)
#define RegClr(addr, mask) *(volatile u32*)((u32)(addr) | REG_ALIAS_CLR) = (mask)

// write value masked to u32 register, atomic for different bits, non-atomic for the same bits
#define RegMask(addr, value, mask) RegXor((addr), (*(addr) ^ (value)) & (mask))

#if USE_ORIGSDK		// include interface of original SDK
#define hw_xor_bits(addr, mask) *(volatile u32*)((u32)(addr) | REG_ALIAS_XOR) = (mask)
#define hw_set_bits(addr, mask) *(volatile u32*)((u32)(addr) | REG_ALIAS_SET) = (mask)
#define hw_clear_bits(addr, mask) *(volatile u32*)((u32)(addr) | REG_ALIAS_CLR) = (mask)
#define hw_write_masked(addr, value, mask) RegXor((addr), (*(addr) ^ (value)) & (mask))
#endif // USE_ORIGSDK

// Register address offsets for atomic RMW aliases
#define REG_ALIAS_RW_BITS  (0x0u << 12)
#define REG_ALIAS_XOR_BITS (0x1u << 12)
#define REG_ALIAS_SET_BITS (0x2u << 12)
#define REG_ALIAS_CLR_BITS (0x3u << 12)

// register memory map - base address
#if RP2040		// 1=use MCU RP2040
#include "sdk_addressmap_rp2040.h"
#endif

#if RP2350		// 1=use MCU RP2350
#include "sdk_addressmap_rp2350.h"
#endif

#endif // _SDK_ADDRESSMAP_H
