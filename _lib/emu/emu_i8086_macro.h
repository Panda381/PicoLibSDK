
// ****************************************************************************
//
//                  I8088/I8086/I80186/I80188 CPU Emulator - macro
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

// merge segment and offset into absolute address
#define I8086_ADDR(seg, off) (((u32)(seg) << 4) + (u16)(off))

// get DS segment (can be overridden)
INLINE u16 I8086_DS(sI8086* cpu)
{
	u16 seg = cpu->ds;
	u8 pref = cpu->segpref;
	if (pref < I8086_SEG_NO) seg = cpu->regseg[pref];
	return seg;
}

// read byte from memory
INLINE u8 I8086_GetByte(sI8086* cpu, u16 seg, u16 off)
{
	return cpu->readmem(I8086_ADDR(seg, off));
}

// write byte to memory
INLINE void I8086_SetByte(sI8086* cpu, u16 seg, u16 off, u8 num)
{
	cpu->writemem(I8086_ADDR(seg, off), num);
}

// read word from memory
u16 FASTCODE NOFLASH(I8086_GetWord)(sI8086* cpu, u16 seg, u16 off)
{
#if I8086_CPU_8088	// 1=use 8088 timings limitations
	cpu->sync.clock += I8086_CLOCKMUL*4; // penalty for 2 bytes
#else
	if ((off & 1) != 0) cpu->sync.clock += I8086_CLOCKMUL*4; // penalty for unaligned address
#endif
	u8 tmp = I8086_GetByte(cpu, seg, off);
	return ((u16)I8086_GetByte(cpu, seg, off+1) << 8) | tmp;
}

// write word to memory
void FASTCODE NOFLASH(I8086_SetWord)(sI8086* cpu, u16 seg, u16 off, u16 num)
{
#if I8086_CPU_8088	// 1=use 8088 timings limitations
	cpu->sync.clock += I8086_CLOCKMUL*4; // penalty for 2 bytes
#else
	if ((off & 1) != 0) cpu->sync.clock += I8086_CLOCKMUL*4; // penalty for unaligned address
#endif
	I8086_SetByte(cpu, seg, off, (u8)num);
	I8086_SetByte(cpu, seg, off+1, (u8)(num >> 8));
}

// load byte from program memory
u8 FASTCODE NOFLASH(I8086_ProgByte)(sI8086* cpu)
{
	// read program byte
	u8 n = I8086_GetByte(cpu, cpu->cs, cpu->ip);

	// compiler barrier (for the IP to be valid at the time of the callback instruction)
	cb();

	// shift instruction pointer
	cpu->ip++;

	return n;
}

// load word from program memory
u16 FASTCODE NOFLASH(I8086_ProgWord)(sI8086* cpu)
{
	u8 temp = I8086_ProgByte(cpu);
	// do not combine commands into one expression - the order of operations cannot be guaranteed
	return (temp | ((u16)I8086_ProgByte(cpu) << 8));
}

// load "modrm" byte and calculate effective address 
// ... undocumented: LEA with mod == 3 returns previous effective address
void FASTCODE NOFLASH(I8086_GetEA)(sI8086* cpu)
{
	// load modrm byte
	u8 modrm = I8086_ProgByte(cpu);
	cpu->modrm = modrm;

	// skip mode 3, it uses registers (leaves cpu->eoff unchanged)
	if(modrm >= 0xc0) return;

	u8 m = modrm & 7; // r/m array

	u16 off;
	u16 seg;
	u32 clk;

	// mod = 1, mod = 2
	if (modrm >= 0x40)
	{
		// get offset
		s16 n;
		if (modrm >= 0x80) // mod = 2
			n = (s16)I8086_ProgWord(cpu);
		else // mod = 1
			n = (s8)I8086_ProgByte(cpu);

		// get clock
		clk = cpu->sync.clock;

		switch (m)
		{
		// BX + SI + off
		case 0:
			off = cpu->bx + cpu->si + n;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*11;
			break;

		// BX + DI + off
		case 1:
			off = cpu->bx + cpu->di + n;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*12;
			break;

		// BP + SI + off
		case 2:
			off = cpu->bp + cpu->si + n;
			seg = cpu->ss;
			clk += I8086_CLOCKMUL*12;
			break;

		// BP + DI + off
		case 3:
			off = cpu->bp + cpu->di + n;
			seg = cpu->ss;
			clk += I8086_CLOCKMUL*11;
			break;

		// SI + off
		case 4:
			off = cpu->si + n;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*9;
			break;

		// DI + off
		case 5:
			off = cpu->di + n;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*9;
			break;

		// BP + off
		case 6:
			off = cpu->bp + n;
			seg = cpu->ss;
			clk += I8086_CLOCKMUL*9;
			break;

		// BX + off
		//case 7:
		default:
			off = cpu->bx + n;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*9;
			break;
		}
	}

	// mod = 0
	else
	{
		clk = cpu->sync.clock;

		switch (m)
		{
		// BX + SI
		case 0:
			off = cpu->bx + cpu->si;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*7;
			break;

		// BX + DI
		case 1:
			off = cpu->bx + cpu->di;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*8;
			break;

		// BP + SI
		case 2:
			off = cpu->bp + cpu->si;
			seg = cpu->ss;
			clk += I8086_CLOCKMUL*8;
			break;

		// BP + DI
		case 3:
			off = cpu->bp + cpu->di;
			seg = cpu->ss;
			clk += I8086_CLOCKMUL*7;
			break;

		// SI
		case 4:
			off = cpu->si;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*5;
			break;

		// DI
		case 5:
			off = cpu->di;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*5;
			break;

		// address16
		case 6:
			off = I8086_ProgWord(cpu);
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*6;
			break;

		// BX
		//case 7:
		default:
			off = cpu->bx;
			seg = cpu->ds;
			clk += I8086_CLOCKMUL*5;
			break;
		}
	}

	// save new clock
	cpu->sync.clock = clk;

	// override segment prefix
	u8 pref = cpu->segpref;
	if (pref < I8086_SEG_NO) seg = cpu->regseg[pref];

	// safe new effective address
	cpu->eseg = seg;
	cpu->eoff = off;
}

// get operation register 8-bit
INLINE u8 I8086_GetR8(sI8086* cpu)
{
	// 7 6 5  4  3  2 1 0    7 6 5 4 3 2  1  0
	// x x b5 b4 b3 x x x -> x x x x x b4 b3 b5
	u8 inx = cpu->modrm;
	inx = ((inx >> 2) & (B1|B2)) | ((inx >> 5) & B0);
	return cpu->reg8[inx];
}

// get operation register 16-bit
INLINE u16 I8086_GetR16(sI8086* cpu)
{
	return cpu->reg16[(cpu->modrm >> 3) & 7];
}

// get operation register/memory 8-bit
INLINE u8 I8086_GetRM8(sI8086* cpu)
{
	u8 inx = cpu->modrm;

	// use register
	if (inx >= 0xc0)
	{
		inx = ((inx << 1) & (B1|B2)) | ((inx >> 2) & B0);
		return cpu->reg8[inx];
	}

	// use memory
	return I8086_GetByte(cpu, cpu->eseg, cpu->eoff);
}

// get operation register/memory 16-bit
INLINE u16 I8086_GetRM16(sI8086* cpu)
{
	u8 inx = cpu->modrm;

	// use register
	if (inx >= 0xc0) return cpu->reg16[inx & 7];

	// use memory
	return I8086_GetWord(cpu, cpu->eseg, cpu->eoff);
}

// set operation register 8-bit
void FASTCODE NOFLASH(I8086_SetR8)(sI8086* cpu)
{
	// 7 6 5  4  3  2 1 0    7 6 5 4 3 2  1  0
	// x x b5 b4 b3 x x x -> x x x x x b4 b3 b5
	u8 inx = cpu->modrm;
	inx = ((inx >> 2) & (B1|B2)) | ((inx >> 5) & B0);
	cpu->reg8[inx] = cpu->dst8;
}

// set operation register 16-bit
void FASTCODE NOFLASH(I8086_SetR16)(sI8086* cpu)
{
	cpu->reg16[(cpu->modrm >> 3) & 7] = cpu->dst16;
}

// set operation register/memory 8-bit
void FASTCODE NOFLASH(I8086_SetRM8)(sI8086* cpu)
{
	u8 inx = cpu->modrm;
	u8 val = cpu->dst8;

	// use register
	if (inx >= 0xc0)
	{
		inx = ((inx << 1) & (B1|B2)) | ((inx >> 2) & B0);
		cpu->reg8[inx] = val;
	}

	// use memory
	else
		I8086_SetByte(cpu, cpu->eseg, cpu->eoff, val);
}

// set operation register/memory 16-bit
void FASTCODE NOFLASH(I8086_SetRM16)(sI8086* cpu)
{
	u8 inx = cpu->modrm;
	u16 val = cpu->dst16;

	// use register
	if (inx >= 0xc0)
		cpu->reg16[inx & 7] = val;

	// use memory
	else
		I8086_SetWord(cpu, cpu->eseg, cpu->eoff, val);
}

// open instruction MR<-R byte
void FASTCODE NOFLASH(I8086_OpenMR_R8)(sI8086* cpu)
{
	I8086_GetEA(cpu); // calculate effective address
	cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory (destination)
	cpu->src8 = I8086_GetR8(cpu); // get operation register (source)
}

// open instruction MR<-R word
void FASTCODE NOFLASH(I8086_OpenMR_R16)(sI8086* cpu)
{
	I8086_GetEA(cpu); // calculate effective address
	cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (destination)
	cpu->src16 = I8086_GetR16(cpu); // get operation register (source)
}

// open instruction R<-MR byte
void FASTCODE NOFLASH(I8086_OpenR_MR8)(sI8086* cpu)
{
	I8086_GetEA(cpu); // calculate effective address
	cpu->dst8 = I8086_GetR8(cpu); // get operation register (destination)
	cpu->src8 = I8086_GetRM8(cpu); // get operation register/memory (source)
}

// open instruction R<-MR word
void FASTCODE NOFLASH(I8086_OpenR_MR16)(sI8086* cpu)
{
	I8086_GetEA(cpu); // calculate effective address
	cpu->dst16 = I8086_GetR16(cpu); // get operation register (destination)
	cpu->src16 = I8086_GetRM16(cpu); // get operation register/memory (source)
}

// PUSH 16
void FASTCODE NOFLASH(I8086_PUSH)(sI8086* cpu, u16 val)
{
	u16 sp = cpu->sp - 2;
	cpu->sp = sp;
	I8086_SetWord(cpu, cpu->ss, sp, val);
}

// POP 16
u16 FASTCODE NOFLASH(I8086_POP)(sI8086* cpu)
{
	u16 sp = cpu->sp;
	cpu->sp = sp + 2;
	return I8086_GetWord(cpu, cpu->ss, sp);
}

// ADD 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ADD8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u16 res = dst + src;		// result
	u16 xor = dst ^ src ^ res;	// XOR
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= res >> 8;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	cpu->dst8 = dst;		// save result of operation
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// ADD 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ADD16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u32 res = dst + src;		// result
	u32 xor = dst ^ src ^ res;	// XOR
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= res >> 16;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	cpu->dst16 = dst;		// save result of operation
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// ADC 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ADC8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u16 f = cpu->flags;		// get flags
	u16 res = dst + src + (f & I8086_CF); // result
	u16 xor = dst ^ src ^ res;	// XOR
	f &= ~I8086_FLAGALL;		// clear flags
	f |= res >> 8;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	cpu->dst8 = dst;		// save result of operation
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// ADC 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ADC16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u16 f = cpu->flags;		// get flags
	u32 res = dst + src + (f & I8086_CF); // result
	u32 xor = dst ^ src ^ res;	// XOR
	f &= ~I8086_FLAGALL;		// clear flags
	f |= res >> 16;			// I8086_CF carry flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	cpu->dst16 = dst;		// save result of operation
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// SUB 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SUB8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u16 res = dst - src;		// result
	u16 xor = dst ^ src ^ res;	// XOR
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= (res >> 8) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	cpu->dst8 = dst;		// save result of operation
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// SUB 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SUB16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u32 res = dst - src;		// result
	u32 xor = dst ^ src ^ res;	// XOR
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= (res >> 16) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	cpu->dst16 = dst;		// save result of operation
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// SBB 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SBB8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u16 f = cpu->flags;		// get flags
	u16 res = dst - src - (f & I8086_CF); // result
	u16 xor = dst ^ src ^ res;	// XOR
	f &= ~I8086_FLAGALL;		// clear flags
	f |= (res >> 8) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
	dst = (u8)res;
	cpu->dst8 = dst;		// save result of operation
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// SBB 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SBB16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u16 f = cpu->flags;		// get flags
	u32 res = dst - src - (f & I8086_CF); // result
	u32 xor = dst ^ src ^ res;	// XOR
	f &= ~I8086_FLAGALL;		// clear flags
	f |= (res >> 16) & I8086_CF;	// I8086_CF borrow flag
	f |= xor & I8086_AF;		// half-carry flag
	f |= ((xor ^ (xor >> 1)) >> 4) & I8086_OF; // overflow flag
	dst = (u16)res;
	cpu->dst16 = dst;		// save result of operation
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// AND 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_AND8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u8 res = dst & src;		// result
	cpu->dst8 = res;		// save result of operation
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// AND 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_AND16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u16 res = dst & src;		// result
	cpu->dst16 = res;		// save result of operation
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// OR 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_OR8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u8 res = dst | src;		// result
	cpu->dst8 = res;		// save result of operation
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// OR 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_OR16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u16 res = dst | src;		// result
	cpu->dst16 = res;		// save result of operation
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// XOR 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_XOR8)(sI8086* cpu)
{
	u8 dst = cpu->dst8;		// destination operand
	u8 src = cpu->src8;		// source operand
	u8 res = dst ^ src;		// result
	cpu->dst8 = res;		// save result of operation
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[res];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// XOR 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_XOR16)(sI8086* cpu)
{
	u16 dst = cpu->dst16;		// destination operand
	u16 src = cpu->src16;		// source operand
	u16 res = dst ^ src;		// result
	cpu->dst16 = res;		// save result of operation
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_FlagParTab[(u8)res] & I8086_PF; // add parity flag
	if (res == 0) f |= I8086_ZF;	// add zero flag
	f |= (res >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// INC 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_INC8)(sI8086* cpu)
{
	u8 dst = cpu->dst8 + 1;		// destination operand
	u16 f = cpu->flags & ((~I8086_FLAGALL) | I8086_CF); // clear flags (except carry)
	if ((dst & 0x0f) == 0) f |= I8086_AF; // half-carry flag
	if (dst == 0x80) f |= I8086_OF;	// overflow flag
	cpu->dst8 = dst;		// save result of operation
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// INC 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_INC16)(sI8086* cpu)
{
	u16 dst = cpu->dst16 + 1;	// destination operand
	u16 f = cpu->flags & ((~I8086_FLAGALL) | I8086_CF); // clear flags (except carry)
	if ((dst & 0x0f) == 0) f |= I8086_AF; // half-carry flag
	if (dst == 0x8000) f |= I8086_OF; // overflow flag
	cpu->dst16 = dst;		// save result of operation
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// DEC 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_DEC8)(sI8086* cpu)
{
	u8 dst = cpu->dst8 - 1;		// destination operand
	u16 f = cpu->flags & ((~I8086_FLAGALL) | I8086_CF); // clear flags (except carry)
	if ((dst & 0x0f) == 0x0f) f |= I8086_AF; // half-carry flag
	if (dst == 0x7f) f |= I8086_OF;	// overflow flag
	cpu->dst8 = dst;		// save result of operation
	f |= I8086_FlagParTab[dst];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// DEC 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_DEC16)(sI8086* cpu)
{
	u16 dst = cpu->dst16 - 1;	// destination operand
	u16 f = cpu->flags & ((~I8086_FLAGALL) | I8086_CF); // clear flags (except carry)
	if ((dst & 0x0f) == 0x0f) f |= I8086_AF; // half-carry flag
	if (dst == 0x7fff) f |= I8086_OF; // overflow flag
	cpu->dst16 = dst;		// save result of operation
	f |= I8086_FlagParTab[(u8)dst] & I8086_PF; // add parity flag
	if (dst == 0) f |= I8086_ZF;	// add zero flag
	f |= (dst >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// condition short jump
// ... jump conditions verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SJUMP)(sI8086* cpu, Bool cond)
{
	s8 rel = (s8)I8086_ProgByte(cpu); // load jump offset
	if (cond)
	{
		cpu->ip = cpu->ip + (s8)rel;
		cpu->sync.clock += I8086_CLOCKMUL*(16-4);
	}
	cpu->sync.clock += I8086_CLOCKMUL*4;
}

// ROL 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ROL8)(sI8086* cpu)
{
	u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear carry and overflow
	u8 n = cpu->dst8;		// get data
	u8 n2 = n >> 7;			// carry
	f |= n2;			// add carry flag
	n2 |= n << 1;			// ROL shift
	cpu->dst8 = n2;			// save result of operation
	n = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n << I8086_OF_BIT);	// set overflow flag
	cpu->flags = f;			// save new flags
}

// ROR 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ROR8)(sI8086* cpu)
{
	u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear carry and overflow
	u8 n = cpu->dst8;		// get data
	f |= n & I8086_CF;		// add carry flag
	u8 n2 = (n >> 1) | (n << 7);	// ROR shift
	cpu->dst8 = n2;			// save result of operation
	n = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n << I8086_OF_BIT);	// set overflow flag
	cpu->flags = f;			// save new flags
}

// RCL 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_RCL8)(sI8086* cpu)
{
	u16 f = cpu->flags;
	u8 n = cpu->dst8;		// get data
	u8 n2 = f & I8086_CF;		// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	f |= n >> 7;			// add new carry flag
	n2 |= n << 1;			// RCL shift
	cpu->dst8 = n2;			// save result of operation
	n = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n << I8086_OF_BIT);	// set overflow flag
	cpu->flags = f;			// save new flags
}

// RCR 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_RCR8)(sI8086* cpu)
{
	u16 f = cpu->flags;
	u8 n = cpu->dst8;		// get data
	u8 n2 = (f & I8086_CF) << 7;	// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	u8 n3 = (n ^ n2) >> 7;		// bit 0 = overflow flag (difference of the bits)
	f |= ((u16)n3 << I8086_OF_BIT);	// set overflow flag (in case of RCR instruction, OF flag is checked BEFORE rotation)
	f |= n & I8086_CF;		// add new carry flag
	n2 |= n >> 1;			// RCR shift
	cpu->dst8 = n2;			// save result of operation
	cpu->flags = f;			// save new flags
}

// SAL 8, SHL 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SHL8)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	u8 n = cpu->dst8;		// get data
	f |= n >> 7;			// I8086_CF carry flag
	f |= ((u16)(n ^ (n >> 1)) << 5) & I8086_OF; // overflow flag
	n <<= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	cpu->dst8 = n;			// save result of operation
	f |= I8086_FlagParTab[n];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// SHR 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SHR8)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	u8 n = cpu->dst8;		// get data
	f |= n & I8086_CF;		// I8086_CF carry flag
	f |= ((u16)n << 4) & I8086_OF;	// overflow flag
	n >>= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	cpu->dst8 = n;			// save result of operation
	f |= I8086_FlagParTab[n];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// SAR 8
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SAR8)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	u8 n = cpu->dst8;		// get data
	f |= n & I8086_CF;		// I8086_CF carry flag
	n = (u8)(((s8)n)>>1);		// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	cpu->dst8 = n;			// save result of operation
	f |= I8086_FlagParTab[n];	// add parity, sign and zero flag
	cpu->flags = f;			// save new flags
}

// SETMO 8 (Set Minus One) ... undocumented
void FASTCODE NOFLASH(I8086_SETMO8)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_PF | I8086_SF;
	cpu->dst8 = 0xff;
	cpu->flags = f;			// save new flags
}

// ROL 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ROL16)(sI8086* cpu)
{
	u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear carry and overflow
	u16 n = cpu->dst16;		// get data
	u16 n2 = n >> 15;		// carry
	f |= n2;			// add carry flag
	n2 |= n << 1;			// ROL shift
	cpu->dst16 = n2;		// save result of operation
	n = (n ^ n2) >> 15;		// bit 0 = overflow flag (difference of the bits)
	f |= (n << I8086_OF_BIT);	// set overflow flag
	cpu->flags = f;			// save new flags
}

// ROR 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_ROR16)(sI8086* cpu)
{
	u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear carry and overflow
	u16 n = cpu->dst16;		// get data
	f |= n & I8086_CF;		// add carry flag
	u16 n2 = (n >> 1) | (n << 15);	// ROR shift
	cpu->dst16 = n2;		// save result of operation
	n = (n ^ n2) >> 15;		// bit 0 = overflow flag (difference of the bits)
	f |= (n << I8086_OF_BIT);	// set overflow flag
	cpu->flags = f;			// save new flags
}

// RCL 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_RCL16)(sI8086* cpu)
{
	u16 f = cpu->flags;
	u16 n = cpu->dst16;		// get data
	u16 n2 = f & I8086_CF;		// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	f |= n >> 15;			// add new carry flag
	n2 |= n << 1;			// RCL shift
	cpu->dst16 = n2;		// save result of operation
	n = (n ^ n2) >> 15;		// bit 0 = overflow flag (difference of the bits)
	f |= (n << I8086_OF_BIT);	// set overflow flag
	cpu->flags = f;			// save new flags
}

// RCR 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_RCR16)(sI8086* cpu)
{
	u16 f = cpu->flags;
	u16 n = cpu->dst16;		// get data
	u16 n2 = (f & I8086_CF) << 15;	// old carry flag
	f &= ~(I8086_CF | I8086_OF);	// clear carry and overflow
	u16 n3 = (n ^ n2) >> 15;	// bit 0 = overflow flag (difference of the bits)
	f |= (n3 << I8086_OF_BIT);	// set overflow flag (in case of RCR instruction, OF flag is checked BEFORE rotation)
	f |= n & I8086_CF;		// add new carry flag
	n2 |= n >> 1;			// RCR shift
	cpu->dst16 = n2;		// save result of operation
	cpu->flags = f;			// save new flags
}

// SAL 16, SHL 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SHL16)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	u16 n = cpu->dst16;		// get data
	f |= n >> 15;			// I8086_CF carry flag
	f |= ((n ^ (n >> 1)) >> 3) & I8086_OF; // overflow flag
	n <<= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	cpu->dst16 = n;			// save result of operation
	f |= I8086_FlagParTab[(u8)n] & I8086_PF; // add parity flag
	if (n == 0) f |= I8086_ZF;	// add zero flag
	f |= (n >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// SHR 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SHR16)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	u16 n = cpu->dst16;		// get data
	f |= n & I8086_CF;		// I8086_CF carry flag
	f |= (n >> 4) & I8086_OF;	// overflow flag
	n >>= 1;			// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	cpu->dst16 = n;			// save result of operation
	f |= I8086_FlagParTab[(u8)n] & I8086_PF; // add parity flag
	if (n == 0) f |= I8086_ZF;	// add zero flag
	f |= (n >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// SAR 16
// ... code verified on contemporary PC hardware
void FASTCODE NOFLASH(I8086_SAR16)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	u16 n = cpu->dst16;		// get data
	f |= n & I8086_CF;		// I8086_CF carry flag
	n = (u16)(((s16)n)>>1);		// result

#if I8086_CPU_AMD
	f |= I8086_AF;			// half-carry flag is always set
#endif

	cpu->dst16 = n;			// save result of operation
	f |= I8086_FlagParTab[(u8)n] & I8086_PF; // add parity flag
	if (n == 0) f |= I8086_ZF;	// add zero flag
	f |= (n >> 8) & I8086_SF;	// add sign flag
	cpu->flags = f;			// save new flags
}

// SETMO 16 (Set Minus One) ... undocumented
void FASTCODE NOFLASH(I8086_SETMO16)(sI8086* cpu)
{
	u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
	f |= I8086_PF | I8086_SF;
	cpu->dst16 = 0xffff;
	cpu->flags = f;			// save new flags
}

// Interrupt
//   INT 0 ... divide by zero
//   INT 1 ... single step (I8086_TF trap flag is set)
//   INT 2 ... non-maskable NMI interrupt
//   INT 3 ... debug breakpoint (INT 3 instruction with opcode 0xCC)
//   INT 4 ... overflow (INTO instruction with opcode 0xCE)
//   INT 5 ... BOUND limits overflow (on 80186 or later)
void FASTCODE NOFLASH(I8086_INT)(sI8086* cpu, u8 type)
{
	u16 off = (u16)type << 2; // vector offset
	I8086_PUSH(cpu, (cpu->flags & I8086_FLAGALLCTRL) | I8086_FLAGHW); // push flags
	cpu->flags &= ~(I8086_TF | I8086_IF); // clear trap flag, disable interrupts
	I8086_PUSH(cpu, cpu->cs); // push current program segment
	cpu->cs = cpu->readmem(off+2) | (cpu->readmem(off+3)<<8); // load new program segment
	I8086_PUSH(cpu, cpu->ip); // push current instruction pointer
	cpu->ip = cpu->readmem(off) | (cpu->readmem(off+1)<<8); // load new instruction pointer
	cpu->sync.clock += I8086_CLOCKMUL*51;
}
