// Generator of constants of 'real' numbers.
// - Project of MS VC++ 2005 (for Windows)

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

#include "include.h"

const char* ChebProgTab;

// Chebyshev progress
void ChebProg(int permille)
{
	fprintf(stderr, "\rChebCoef %s: %d.%d%% ", ChebProgTab, permille/10, permille % 10);
}

// get number of leading zeros of u32 number
u8 clz(u32 val)
{
	u8 i = 32;
	if (val >= 0x10000) { i = 16; val >>= 16; }
	if (val >= 0x100) { i -= 8; val >>= 8; }
	if (val >= 0x10) { i -= 4; val >>= 4; }
	if (val >= 4) { i -= 2; val >>= 2; }
	if (val >= 2) { i -= 1; val >>= 1; }
	if (val >= 1) i--;
	return i;
}

// count trailing zeros
u8 ctz(u32 val)
{
	u8 i = 0;
	if ((val & 0xffff) == 0) { i = 16; val >>= 16; }
	if ((val & 0xff) == 0) { i += 8; val >>= 8; }
	if ((val & 0xf) == 0) { i += 4; val >>= 4; }
	if ((val & 3) == 0) { i += 2; val >>= 2; }
	if ((val & 1) == 0) { i += 1; }
	return i;
}

// get number of leading zeros of u64 number
u8 clz64(u64 num)
{
	if (num >= 0x100000000ULL)
		return clz((u32)(num >> 32));
	else
		return clz((u32)num) + 32;
}

u32 DivModU32(u32 a, u32 b, u32* rem)
{
	u32 c = a/b;
	*rem = a - c*b;
	return c;
}

// Unsigned multiply u32*u32 with result u64
u64 UMul(u32 a, u32 b)
{
	return (u64)a*b;
}

int main(int argc, char* argv[])
{
	// export constants
	ExportConst();
	return 0;
}
