
#include <stdio.h>

#define PRINT_ALL		0		// 1 = print all states, even if OK

// code small modifications
#define I8086_CPU_INTEL		0	// 1=use Intel alternative
#define I8086_CPU_AMD		1	// 1=use AMD alternative

#define I8086_CPU_80286_UP	1	// 1=use 80286 or later

#if I8086_CPU_INTEL && I8086_CPU_AMD
#error Do not use Intel and AMD at the same time
#endif

// declare types
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;		// on 8-bit system use "signed int"
typedef unsigned short u16;		// on 8-bit system use "unsigned int"
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
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

// count of array entries
#define count_of(a) (sizeof(a)/sizeof((a)[0]))

// flags
#define I8086_CF_BIT		0	// carry flag
#define I8086_PF_BIT		2	// parity flag (0=odd parity, 1=even parity)
#define I8086_AF_BIT		4	// auxiliary flag, carry from bit 3 to bit 4
#define I8086_ZF_BIT		6	// zero flag
#define I8086_SF_BIT		7	// sign flag
#define I8086_OF_BIT		11	// overflow flag

#define I8086_CF		B0	// (0x001) carry flag
#define I8086_PF		B2	// (0x004) parity flag (0=odd parity, 1=even parity)
#define I8086_AF		B4	// (0x010) auxiliary flag, carry from bit 3 to bit 4
#define I8086_ZF		B6	// (0x040) zero flag
#define I8086_SF		B7	// (0x080) sign flag
#define I8086_OF		B11	// (0x800) overflow flag

#define I8086_FLAGDEF		B1	// default flags
#define I8086_FLAGALL		(I8086_OF|I8086_SF|I8086_ZF|I8086_AF|I8086_PF|I8086_CF) // (0x8D5) arithmetic flags

// table of indexed flags
#define FLAG_NUM	64		// number of flag alternatives
u16 FlagInx[FLAG_NUM];

// flags table with parity (I8086_PF, I8086_ZF and I8086_SF flags) ... keep the table in RAM, for higher speed
u8 I8086_FlagParTab[256];

// initialize I8086 tables
void I8086_InitTab()
{
	int i, j, par;
	u8 f;
	for (i = 0; i < 256; i++)
	{
		// initialize parity table
		par = 0;
		j = i;
		while (j != 0)
		{
			par ^= (j & 1);
			j >>= 1;
		}

		// set flag
		f = I8086_FLAGDEF;
		if (par == 0) f |= I8086_PF; // even parity
		if (i >= 0x80) f |= I8086_SF; // negative
		if (i == 0) f |= I8086_ZF; // zero
		I8086_FlagParTab[i] = f;
	}
}

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

#include "test_8086_op16_c.h" // 16-bit operations in C
#include "test_8086_op16_e.h" // 16-bit operations in emulator
#include "test_8086_op8_c.h" // 8-bit operations in C
#include "test_8086_op8_e.h" // 8-bit operations in emulator

// print flags
void PrintFlags(u32 f)
{
	printf("C%u P%u A%u Z%u S%u O%u",
		(f & I8086_CF) ? 1 : 0,
		(f & I8086_PF) ? 1 : 0,
		(f & I8086_AF) ? 1 : 0,
		(f & I8086_ZF) ? 1 : 0,
		(f & I8086_SF) ? 1 : 0,
		(f & I8086_OF) ? 1 : 0);
}

// print AL, AH and flags
void Print8(u32 a, u32 f)
{
	printf(" al=0x%02X ah=0x%02X f=", a & 0xff, a >> 8);
	PrintFlags(f);
}

// print AX and flags
void Print16(u32 a, u32 f)
{
	printf(" ax=0x%04X f=", a);
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

// print error, 16-bit
void PrintErr16(u32 a, u32 b, u32 f, u32 a_c, u32 f_c, u32 a_e, u32 f_e)
{
	// print input state
	printf(" bx=0x%04X", b);
	Print16(a, f);

	// output C state
	printf("/ C:");
	Print16(a_c, f_c);

	// output E state
	printf("/ E:");
	Print16(a_e, f_e);

	printf(" ERROR\n");
}

// print error, 16-bit
void PrintErr16B(u32 a, u32 f, u32 a_c, u32 f_c, u32 a_e, u32 f_e)
{
	// print input state
	Print16(a, f);

	// output C state
	printf("/ C:");
	Print16(a_c, f_c);

	// output E state
	printf("/ E:");
	Print16(a_e, f_e);

	printf(" ERROR\n");
}

// 16-bit samples
u16 Samp16[] = { 0x0000, 0x0001, 0x007F, 0x0080, 0x0081, 0x00FF, 0x0100, 0x0101,
	0x017F, 0x0180, 0x0181, 0x01FF, 0x0200, 0x0201, 0x7F00, 0x7F01, 0x7F7F,
	0x7F80, 0x7F81, 0x7FFF, 0x8000, 0x8001, 0xFF7F, 0xFF80, 0xFF81, 0xFFFF };

#define SAMP16_NUM count_of(Samp16)

#include "test_8086_test16.h" // 16-bit tests
#include "test_8086_test8.h" // 8-bit tests

int main(int argc, char* argv[])
{
	int i;
	u16 f;

	// initialize I8086 tables
	I8086_InitTab();

	// generate table of indexed flags
	for (i = 0; i < FLAG_NUM; i++)
	{
		f = 0;
		if (i & B0) f |= I8086_CF;
		if (i & B1) f |= I8086_PF;
		if (i & B2) f |= I8086_AF;
		if (i & B3) f |= I8086_ZF;
		if (i & B4) f |= I8086_SF;
		if (i & B5) f |= I8086_OF;
		FlagInx[i] = f;
	}

	printf("=== Emulator 8086 test ===\n");

	Test_ADD8();	// test 8-bit addition
	Test_ADD16();	// test 16-bit addition
	Test_ADC8();	// test 8-bit C-addition
	Test_ADC16();	// test 16-bit C-addition
	Test_SUB8();	// test 8-bit subtraction
	Test_SUB16();	// test 16-bit subtraction
	Test_SBB8();	// test 8-bit C-subtraction
	Test_SBB16();	// test 16-bit C-subtraction
	Test_AND8();	// test 8-bit AND
	Test_AND16();	// test 16-bit AND
	Test_OR8();		// test 8-bit OR
	Test_OR16();	// test 16-bit OR
	Test_XOR8();	// test 8-bit XOR
	Test_XOR16();	// test 16-bit XOR
	Test_CMP8();	// test 8-bit compare
	Test_CMP16();	// test 16-bit compare
	Test_TEST8();	// test 8-bit test
	Test_TEST16();	// test 16-bit test

	Test_INC8();	// test 8-bit increment
	Test_INC16();	// test 16-bit increment
	Test_DEC8();	// test 8-bit decrement
	Test_DEC16();	// test 16-bit decrement

	Test_JMP();		// test jumps

	Test_DAA();		// test DAA instruction
	Test_DAS();		// test DAS instruction
	Test_AAA();		// test AAA instruction
	Test_AAS();		// test AAS instruction

	Test_AAD();		// test AAD instruction
	Test_AAM();		// test AAM instruction

	Test_ROL8();	// test 8-bit ROL,1
	Test_ROR8();	// test 8-bit ROR,1
	Test_RCL8();	// test 8-bit RCL,1
	Test_RCR8();	// test 8-bit RCR,1
	Test_SHL8();	// test 8-bit SHL,1
	Test_SHR8();	// test 8-bit SHR,1
	Test_SAR8();	// test 8-bit SAR,1

	Test_ROL16();	// test 16-bit ROL,1
	Test_ROR16();	// test 16-bit ROR,1
	Test_RCL16();	// test 16-bit RCL,1
	Test_RCR16();	// test 16-bit RCR,1
	Test_SHL16();	// test 16-bit SHL,1
	Test_SHR16();	// test 16-bit SHR,1
	Test_SAR16();	// test 16-bit SAR,1

	Test_ROL8N();	// test 8-bit ROL,CL
	Test_ROR8N();	// test 8-bit ROR,CL
	Test_RCL8N();	// test 8-bit RCL,CL
	Test_RCR8N();	// test 8-bit RCR,CL
	Test_SHL8N();	// test 8-bit SHL,CL
	Test_SHR8N();	// test 8-bit SHR,CL
	Test_SAR8N();	// test 8-bit SAR,CL

	Test_ROL16N();	// test 16-bit ROL,CL
	Test_ROR16N();	// test 16-bit ROR,CL
	Test_RCL16N();	// test 16-bit RCL,CL
	Test_RCR16N();	// test 16-bit RCR,CL
	Test_SHL16N();	// test 16-bit SHL,CL
	Test_SHR16N();	// test 16-bit SHR,CL
	Test_SAR16N();	// test 16-bit SAR,CL

	Test_MUL8();	// test 8-bit MUL
	Test_IMUL8();	// test 8-bit IMUL
	Test_DIV8();	// test 8-bit DIV
	Test_IDIV8();	// test 8-bit IDIV

	Test_MUL16();	// test 16-bit MUL
	Test_IMUL16();	// test 16-bit IMUL
	Test_DIV16();	// test 16-bit DIV
	Test_IDIV16();	// test 16-bit IDIV

	printf("======= completed ========\n");

	return 0;
}
