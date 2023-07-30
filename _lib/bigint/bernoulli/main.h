
// ****************************************************************************
//
//                                 Main code
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

// get number of leading zeros of u32 number
u8 clz(u32 num);

// count trailing zeros
u8 ctz(u32 val);

// get number of leading zeros of u64 number
u8 clz64(u64 num);

// Divide modulo unsigned U32
u32 DivModU32(u32 a, u32 b, u32* rem);

// Unsigned multiply u32*u32 with result u64
u64 UMul(u32 a, u32 b);

// memory allocator
INLINE void* MemAlloc(u32 size) { return malloc(size); }
INLINE void MemFree(void* mem) { free(mem); }
INLINE void* MemResize(void* addr, u32 size) { return realloc(addr, size); }
