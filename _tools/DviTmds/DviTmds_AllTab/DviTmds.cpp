// Generator of DVI TMDS 16-bit 2-symbol table
// Based on "Figure 3-5. T.M.D.S. Encode Algorithm" on page 29 of DVI 1.0 spec

// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include <stdio.h>
#include <malloc.h>
#include <string.h>

// base types
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;		// on 8-bit system use "signed int"
typedef unsigned short u16;		// on 8-bit system use "unsigned int"
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
typedef signed long long int s64;
typedef unsigned long long int u64;

/*
// control symbols
u32 CtrlSym[4]=
{
	0x354,	// 0b00: 0b1101010100, no sync
	0x0AB,	// 0b01: 0b0010101011, HSYNC
	0x154,	// 0b10: 0b0101010100, VSYNC
	0x2AB,	// 0b11: 0b1010101011, HSYNC + VSYNC
};
*/

// count of "1" bits, slow version
int PopCountSlow(u32 x)
{
	int n = 0;
	while (x != 0)
	{
		n += x & 1;
		x >>= 1;
	}
	return n;
}

// count of "1" bits, faster version (Brian Kernighan Algorithm)
int PopCountBrian(u32 x)
{
	int n = 0;
	while (x != 0)
	{
		n++;
		x = x & (x - 1);
	}
	return n;
}

// count of "1" bits, fast version (using "Hamming weight")
int PopCountFast(u32 x)
{
	x -= ((x >> 1) & 0x55555555);	// put count of each 2 bits into those 2 bits
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333); // put count of each 4 bits into those 4 bits
	x = (x + (x >> 4)) & 0x0F0F0F0F; // put count of each 8 bits into those 8 bits
	x += (x >> 8); // put count of each 16 bits into their lowest 8 bits
	x += (x >> 16); // put count of each 32 bits into their lowest 8 bits
	return x & 0x3F; // mask count of bits
}

// count of "1" bits, fast version with multiplication (using "Hamming weight")
int PopCountMul(u32 x)
{
	x -= ((x >> 1) & 0x55555555);	// put count of each 2 bits into those 2 bits
	x = (x & 0x33333333) + ((x >> 2) & 0x33333333); // put count of each 4 bits into those 4 bits
	x = (x + (x >> 4)) & 0x0F0F0F0F; // put count of each 8 bits into those 8 bits
	x *= 0x01010101; // horizontal sum of bytes
	return x >> 24;	// return just that top byte
}

// count of "1" bits, tabled version
const u8 NumBitsTab[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
int PopCountTab(u32 x)
{
	if (x == 0) return 0;
	return NumBitsTab[x & 0x0f] + PopCountTab(x>>4);
}

// encode one TMDS symbol (8 bits -> 10 bits)
int Balance = 0;
u16 Encode1(u8 d)
{
	int i;

	// odd symbol?
	int q = d & 1;

	// count of "1" bits
	int popcnt = PopCountMul(d);

	// popcount = 4(even) .. 7 bits
	if ((popcnt > 4) || ((popcnt == 4) && ((d & 1) == 0)))
	{
		for (i = 0; i < 7; i++) q |= (~((q >> i) ^ (d >> (i + 1))) & 1) << (i + 1);
	}

	// popcount = 0 .. 4(odd) bits
	else
	{
		for (i = 0; i < 7; i++) q |= (((q >> i) ^ (d >> (i + 1))) & 1) << (i + 1);
		q |= 0x100;
	}

	// byte balance (= N1(q) - N0(q) = N1(q) - (8 - N1(q)))
	int bal = 2*PopCountMul(q & 0xff) - 8;

	// DC balance correction
	int mask = 0x2ff;
	u16 out = 0;
	if ((Balance == 0) || (bal == 0))
	{
		out = q;
		if ((q & 0x100) == 0)
		{
			out ^= mask;
			Balance -= bal;
		}
		else
		{
			Balance += bal;
		}
	}
	else if (((Balance > 0) && (bal > 0)) || ((Balance < 0) && (bal < 0)))
	{
		out = q ^ mask;
		Balance += ((q & 0x100) >> 7) - bal;
	}
	else
	{
		out = q;
		Balance += bal - ((~q & 0x100) >> 7);
	}
	return out;
}

// main function
int main(int argc, char* argv[])
{
	// checking PopCount() algorithm of all versions ("population count")
#if 0
	int a, b, c, d, e;
	u32 i = 1;
	do {
		a = PopCountSlow(i);
		b = PopCountBrian(i);
		c = PopCountFast(i);
		d = PopCountMul(i);
		e = PopCountTab(i);

		if ((a != b) || (a != c) || (a != d) || (a != e))
		{
			printf("Error on %u: a=%d b=%d c=%d d=%d e=%d\n", i, a, b, c, d, e);
			return 1;
		}

		if ((i & 0xfffff) == 0)
		{
			printf("%.1f%%   \r", (double)i*100/0x100000000ULL);
		}

		i++;
	} while (i != 0);
	printf("All OK     \n");
#endif

	// checking speed of all PopCount() versions
#if 0
	u32 i2;
	u32 t1;
	volatile int a2;

	// PC AMD: 197 sec
	printf("PopCountSlow: ");
	t1 = ::GetTickCount();
	i2 = 0;
	do { a2 = PopCountSlow(i); i++; } while (i != 0);
	printf("%d sec\n", (::GetTickCount() - t1)/1000);

	// PC AMD: 128 sec
	printf("PopCountBrian: ");
	t1 = ::GetTickCount();
	i2 = 0;
	do { a2 = PopCountBrian(i); i++; } while (i != 0);
	printf("%d sec\n", (::GetTickCount() - t1)/1000);

	// PC AMD: 22 sec
	printf("PopCountFast: ");
	t1 = ::GetTickCount();
	i2 = 0;
	do { a2 = PopCountFast(i); i++; } while (i != 0);
	printf("%d sec\n", (::GetTickCount() - t1)/1000);

	// PC AMD: 18 sec
	printf("PopCountMul: ");
	t1 = ::GetTickCount();
	i2 = 0;
	do { a2 = PopCountMul(i); i++; } while (i != 0);
	printf("%d sec\n", (::GetTickCount() - t1)/1000);

	// PC AMD: 216 sec
	printf("PopCountTab: ");
	t1 = ::GetTickCount();
	i2 = 0;
	do { a2 = PopCountTab(i); i++; } while (i != 0);
	printf("%d sec\n", (::GetTickCount() - t1)/1000);
#endif

	// generate table
	int d, sym0, sym1;
	Balance = 0;
	for (d = 0; d < 256; d += 4)
	{
		sym0 = Encode1(d);
		sym1 = Encode1(d + 1);
		printf("\t0x%05x,\t// %d: (0x%03x << 10) | 0x%03x\n", sym0 | (sym1 << 10), d/4, sym1, sym0);
	}

	printf("\n\n");

	// even symbols
	Balance = 0;
	for (d = 0; d < 256; d += 4)
	{
		sym0 = Encode1(d);
		sym1 = Encode1(d + 1);
		printf("\t0x%03x,\t// %d: %d%d %d%d%d%d %d%d%d%d (%d-%d)", sym0, d/4, (sym0>>9)&1, (sym0>>8)&1,
			(sym0>>7)&1, (sym0>>6)&1, (sym0>>5)&1, (sym0>>4)&1,
			(sym0>>3)&1, (sym0>>2)&1, (sym0>>1)&1, (sym0>>0)&1,
			PopCountMul(sym0), 10 - PopCountMul(sym0));
		if (PopCountMul(sym0) == 5) printf(" ! DC-balanced");
		printf("\n");
	}

	printf("\n\n");

	// odd symbols
	Balance = 0;
	for (d = 0; d < 256; d += 4)
	{
		sym0 = Encode1(d);
		sym1 = Encode1(d + 1);
		printf("\t0x%03x,\t// %d: %d%d %d%d%d%d %d%d%d%d (%d-%d)", sym1, d/4, (sym1>>9)&1, (sym1>>8)&1,
			(sym1>>7)&1, (sym1>>6)&1, (sym1>>5)&1, (sym1>>4)&1,
			(sym1>>3)&1, (sym1>>2)&1, (sym1>>1)&1, (sym1>>0)&1,
			PopCountMul(sym1), 10 - PopCountMul(sym1));
		if (PopCountMul(sym1) == 5) printf(" ! DC-balanced");
		printf("\n");
	}

	printf("\n\n");

	// all symbols
	Balance = 0;
	for (d = 0; d < 256; d++)
	{
		sym0 = Encode1(d);
		printf("\t0x%03x, %d,\t// %d (%d): %d%d %d%d%d%d %d%d%d%d (%d-%d)", sym0, PopCountMul(sym0), d, d/4, (sym0>>9)&1, (sym0>>8)&1,
			(sym0>>7)&1, (sym0>>6)&1, (sym0>>5)&1, (sym0>>4)&1,
			(sym0>>3)&1, (sym0>>2)&1, (sym0>>1)&1, (sym0>>0)&1,
			PopCountMul(sym0), 10 - PopCountMul(sym0));
		if (PopCountMul(sym0) == 5) printf(" ! DC-balanced");
		printf("\n");
	}

	return 0;
}
