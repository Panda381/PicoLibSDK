
#include <stdio.h>
#include <windows.h>

#define M6502_CPU_65C02		1	// 1=use modifications of 65C02 and later

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;
typedef signed long int s32;
typedef unsigned long int u32;
typedef signed long long int s64;
typedef unsigned long long int u64;

typedef unsigned int uint;

typedef unsigned char Bool;
#define True 1
#define False 0

// bits
#define	B0 (1<<0)
#define	B1 (1<<1)
#define	B2 (1<<2)
#define	B3 (1<<3)
#define	B4 (1<<4)
#define	B5 (1<<5)
#define	B6 (1<<6)
#define	B7 (1<<7)
#define	B8 (1U<<8)
#define	B9 (1U<<9)
#define	B10 (1U<<10)
#define	B11 (1U<<11)
#define	B12 (1U<<12)
#define	B13 (1U<<13)
#define	B14 (1U<<14)
#define	B15 (1U<<15)
#define B16 (1UL<<16)

#define BIT(pos) (1UL<<(pos))

// flags I8086
#define I8086_CF		B0	// (0x001) carry flag
#define I8086_PF		B2	// (0x004) parity flag (0=odd parity, 1=even parity)
#define I8086_AF		B4	// (0x010) auxiliary flag, carry from bit 3 to bit 4
#define I8086_ZF		B6	// (0x040) zero flag
#define I8086_SF		B7	// (0x080) sign flag
#define I8086_OF		B11	// (0x800) overflow flag

#define I8086_FLAGALL		(I8086_OF|I8086_SF|I8086_ZF|I8086_AF|I8086_PF|I8086_CF) // (0x8D5) arithmetic flags

// flags M6502
#define M6502_N		B7		// (0x80) Negative, arithmetic operation was negative (bit 7 was set)
#define M6502_V		B6		// (0x40) Overflow, arithmetic operation overflow
#define M6502_D		B3		// (0x08) Decimal, use BCD binary coded decimal arithmetic
#define M6502_Z		B1		// (0x02) Zero, arithmetic operation was zero
#define M6502_C		B0		// (0x01) Carry, carry over

#define M6502_FLAGALL		(B0+B1+B6+B7) // all arithmetics flags

// load input flags
#define GET_FLAGS0					\
	__asm { mov eax,reg_flags }		\
	__asm { and eax,I8086_FLAGALL }	\
	__asm { pushf }					\
	__asm { pop bx }				\
	__asm { and ebx,~I8086_FLAGALL } \
	__asm { or ebx,eax }			\
	__asm { push bx }				\
	__asm { popf }

// load input flags and register AX
#define GET_FLAGS				\
	GET_FLAGS0					\
	__asm { mov eax,reg_ax }

// setup output flags with register AX
#define SET_FLAGS			\
	__asm { pushf }			\
	__asm { xor ebx,ebx }	\
	__asm { pop bx }		\
	__asm { mov cl,16 }		\
	__asm { shl ebx,cl }	\
	__asm { and eax,0xffff } \
	__asm { or eax,ebx }

// DAA instruction by C
u32 DAA_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { daa }
	SET_FLAGS;
}

// DAS instruction by C
u32 DAS_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { das }
	SET_FLAGS;
}

// 8-bit C-addition (AL+AH+C) by C
u32 ADC8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { adc al,ah }
	SET_FLAGS;
}

// 8-bit C-subtraction (AL-AH-C) by C
u32 SBB8_C(u32 reg_flags, u32 reg_ax)
{
	GET_FLAGS;
	__asm { sbb	al,ah }
	SET_FLAGS;
}

// ADC by hardware
u32 ADC_C(u32 reg_flags, u32 reg_ax)
{
	// ADC sum
	u32 f = 0;
	if ((reg_flags & M6502_C) != 0) f = I8086_CF;
	u32 k = ADC8_C(f, reg_ax);

	// DAA correction
	if ((reg_flags & M6502_D) != 0)
	{
		u8 a0 = (u8)k;
		k = DAA_C(k >> 16, (u16)k);
		k &= ~(I8086_OF << 16);
		u8 a = (u8)k;
		if ((a0 < 0x80) && (a >= 0x80)) k |= I8086_OF << 16;
	}

	reg_ax = (u16)k;
	k >>= 16;
	f = reg_flags & ~M6502_FLAGALL;
	if ((k & I8086_CF) != 0) f |= M6502_C;
	if ((k & I8086_ZF) != 0) f |= M6502_Z;
	if ((k & I8086_SF) != 0) f |= M6502_N;
	if ((k & I8086_OF) != 0) f |= M6502_V;

	return (f << 16) | reg_ax;
}

// SBC by hardware
u32 SBC_C(u32 reg_flags, u32 reg_ax)
{
	// SBB sum (carry is inverted)
	u32 f = 0;
	if ((reg_flags & M6502_C) == 0) f = I8086_CF;
	u32 k = SBB8_C(f, reg_ax);

	// DAS correction
	if ((reg_flags & M6502_D) != 0)
	{
		u8 a0 = (u8)k;
		k = DAS_C(k >> 16, (u16)k);
		k &= ~(I8086_OF << 16);
		u8 a = (u8)k;
		if ((a0 >= 0x80) && (a < 0x80)) k |= I8086_OF << 16;
	}

	reg_ax = (u16)k;
	k >>= 16;
	f = reg_flags & ~M6502_FLAGALL;
	if ((k & I8086_CF) == 0) f |= M6502_C; // carry is inverted
	if ((k & I8086_ZF) != 0) f |= M6502_Z;
	if ((k & I8086_SF) != 0) f |= M6502_N;
	if ((k & I8086_OF) != 0) f |= M6502_V;

	return (f << 16) | reg_ax;
}

// ADC by emulator
u32 ADC_E(u32 reg_flags, u32 reg_ax)
{
	// get operands
	u8 f0 = (u8)reg_flags;
	u8 a = (u8)reg_ax;
	u8 val = (u8)(reg_ax >> 8);

	// get old carry (0 or 1)
	u8 c = f0 & M6502_C;

	// clear flags
	u8 f = f0 & ~M6502_FLAGALL;

	// sum ADC = a + val + carry
	u16 res = a + val + c;		// result
	u16 xor = a ^ val ^ res;	// XOR

	// overflow flag
#if 1 // set to 0 to use alternative method (works with the same result)
	f |= ((xor ^ (xor >> 1)) >> 1) & M6502_V;
#else
	if ((~(a ^ val) & (a ^ res) & 0x80) != 0) f |= M6502_V;
#endif

#if !M6502_CPU_65C02		// 1=use modifications of 65C02 and later
	// check zero ... 6502 tests Zero flag before decimal correction
	if ((u8)res == 0) f |= M6502_Z;
#endif

	// decimal correction
	if ((f0 & M6502_D) != 0)
	{
		// save old value
		u8 a0 = (u8)res;

		// clear flags
#if M6502_CPU_65C02		// 1=use modifications of 65C02 and later
		f &= ~M6502_FLAGALL;
#else
		f &= ~M6502_FLAGALL | M6502_Z; // 6502 leaves Zero flag testes after binary operation
#endif

#if 1 // set to 0 to use alternative method (works with the same result)

		// low nibble correction (>9 or half-carry)
		if (((res & 0x0f) > 9) || ((xor & B4) != 0)) res += 6;

		// high nibble correction
		if (res > 0x9f) res += 0x60;

#else

		// high nibble correction
		if (res > 0x99) res += 0x60;

		// low nibble correction (>9 or half-carry)
		if (((res & 0x0f) > 9) || ((xor & B4) != 0)) res += 6;
#endif

		// check overflow
		if ((a0 < 0x80) && ((u8)res >= 0x80)) f |= M6502_V;
	}

	// check carry
	if (res > 0xff) f |= M6502_C;

	// result
	a = (u8)res;

#if M6502_CPU_65C02		// 1=use modifications of 65C02 and later
	// check zero ... 6502 tests Zero flag before decimal correction
	if (a == 0) f |= M6502_Z;
#endif

	// check negative
	if ((s8)a < 0) f |= M6502_N;

	return (f << 16) | (reg_ax & 0xff00) | a;
}

// SBC by emulator
u32 SBC_E(u32 reg_flags, u32 reg_ax)
{
	// get operands
	u8 f0 = (u8)reg_flags;
	u8 a = (u8)reg_ax;
	u8 val = (u8)(reg_ax >> 8);

	// get old carry inverted (1 or 0)
	u8 c = (~f0) & M6502_C;

	// clear flags
	u8 f = f0 & ~M6502_FLAGALL;

	// sum SBB = a - val - ~carry
	u16 res = a - val - c;		// result
	u16 xor = a ^ val ^ res;	// XOR
#if 1 // set to 0 to use alternative method (works with the same result)
	f |= ((xor ^ (xor >> 1)) >> 1) & M6502_V; // overflow flag
#else
	if (((a ^ val) & (a ^ res) & 0x80) != 0) f |= M6502_V; // overflow flag ... another alternative, also working the same way
#endif

#if !M6502_CPU_65C02		// 1=use modifications of 65C02 and later
	// check zero ... 6502 tests Zero flag before decimal correction
	if ((u8)res == 0) f |= M6502_Z;
#endif

	// decimal correction
	if ((f0 & M6502_D) != 0)
	{
		// save old value
		u8 a0 = (u8)res;

		// clear flags
#if M6502_CPU_65C02		// 1=use modifications of 65C02 and later
		f &= ~M6502_FLAGALL;
#else
		f &= ~M6502_FLAGALL | M6502_Z; // 6502 leaves Zero flag testes after binary operation
#endif

		// high nibble correction
		if (res > 0x99) { res -= 0x60; f |= M6502_C; }

		// low nibble correction (>9 or half-carry)
		if (((res & 0x0f) > 9) || ((xor & B4) != 0)) res -= 6;

		// check overflow
		if ((a0 >= 0x80) && ((u8)res < 0x80)) f |= M6502_V;
	}

	// check carry
	if (res > 0xff) f |= M6502_C;

	// invert carry
	f ^= M6502_C;

	// result
	a = (u8)res;

#if M6502_CPU_65C02		// 1=use modifications of 65C02 and later
	// check zero ... 6502 tests Zero flag before decimal correction
	if (a == 0) f |= M6502_Z;
#endif

	// check negative
	if ((s8)a < 0) f |= M6502_N;

	return (f << 16) | (reg_ax & 0xff00) | a;
}

// print flags
void PrintFlags(u32 f)
{
	printf("C%u Z%u D%u V%u N%u",
		(f & M6502_C) ? 1 : 0,
		(f & M6502_Z) ? 1 : 0,
		(f & M6502_D) ? 1 : 0,
		(f & M6502_V) ? 1 : 0,
		(f & M6502_N) ? 1 : 0);
}

// print AL, AH and flags
void Print8(u32 a, u32 f)
{
	printf(" al=0x%02X ah=0x%02X f=", a & 0xff, a >> 8);
	PrintFlags(f);
}

// print error, 8-bit
void PrintErr8(u32 a, u32 f, u32 a_c, u32 f_c, u32 a_e, u32 f_e)
{
	// print input state
	Print8(a, f);

	// output C state
	printf("/ C:");
	Print8(a_c, f_c);

	// output E state
	printf("/ E:");
	Print8(a_e, f_e);

	printf(" ERROR\n");
}

u8 FlagInx[4] = { 0, M6502_C, M6502_D, M6502_D | M6502_C };

// test ADC
void TestADC()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("ADC check: ");

	// loop variants
	for (fi = 0; fi < 4; fi++)
	{
		f = FlagInx[fi];

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = ADC_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & (M6502_FLAGALL | M6502_D);

			// check E code
			k = ADC_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & (M6502_FLAGALL | M6502_D);

			// compare result
			if ((a_c != a_e) || (f_c != f_e))
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("ADC");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

// test SBC
void TestSBC()
{
	u32 f, fi, a, k, f_c, f_e, a_c, a_e;
	u32 err = 0;
	printf("SBC check: ");

	// loop variants
	for (fi = 0; fi < 4; fi++)
	{
		f = FlagInx[fi];

		for (a = 0; a < 0x10000; a++)
		{
			// check C code
			k = SBC_C(f, a);
			a_c = k & 0xffff;
			f_c = (k >> 16) & (M6502_FLAGALL | M6502_D);

			// check E code
			k = SBC_E(f, a);
			a_e = k & 0xffff;
			f_e = (k >> 16) & (M6502_FLAGALL | M6502_D);

			// compare result
			if ((a_c != a_e) || (f_c != f_e))
			{
				// error
				if (err == 0) printf("\n");
				err++;

				// print error
				printf("SBC");
				PrintErr8(a, f, a_c, f_c, a_e, f_e);
			}
		}
	}

	// all OK
	if (err == 0) printf(" ALL OK\n");
}

int main(int argc, char **argv)
{
	// test ADC
	TestADC();

	// test SBC
	TestSBC();

	return 0;
}
