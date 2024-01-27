
// ****************************************************************************
//
//                          M6502/M65C02 CPU Emulator - macro
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

// load byte from program memory
u8 FASTCODE NOFLASH(M6502_ProgByte)(sM6502* cpu)
{
	// read program byte
	u8 n = cpu->readmem(cpu->pc);

	// compiler barrier (for the PC to be valid at the time of the callback instruction)
	cb();

	// shift program counter
	cpu->pc++;

	return n;
}

// load word from program memory
u16 FASTCODE NOFLASH(M6502_ProgWord)(sM6502* cpu)
{
	u8 temp = M6502_ProgByte(cpu);
	return (temp | ((u16)M6502_ProgByte(cpu) << 8));
}

// set N and Z flags
void FASTCODE NOFLASH(M6502_SetNZ)(sM6502* cpu, u8 n)
{
	u8 f = cpu->f & ~(M6502_N | M6502_Z);
	if (n == 0) f |= M6502_Z; // zero flag
	if ((s8)n < 0) f |= M6502_N; // negative flag
	cpu->f = f;
}

// push byte to the stack
void FASTCODE NOFLASH(M6502_Push)(sM6502* cpu, u8 n)
{
	u8 sp = cpu->sp;
	cpu->sp = sp - 1;
	cpu->writemem(sp + M6502_STACKBASE, n);
}

// pop byte from the stack
u8 FASTCODE NOFLASH(M6502_Pop)(sM6502* cpu)
{
	u8 sp = cpu->sp + 1;
	cpu->sp = sp;
	return cpu->readmem(sp + M6502_STACKBASE);
}

// get address of X-indexed indirect addressing ... (indirect,X)
u16 FASTCODE NOFLASH(M6502_IndX)(sM6502* cpu)
{
	u8 a = M6502_ProgByte(cpu); // get short address from 2nd byte of the instruction
	a += cpu->x; // add X offset, discarding carry
	u8 low = cpu->readmem(a); // load indirect address LOW from zero page
	u8 high = cpu->readmem((u8)(a+1)); // load indirect address HIGH from zero page
	return (u16)(low | ((u16)high << 8)); // compose effective address
}

// get address of Y-indirect indexed addressing ... (indirect),Y
u16 FASTCODE NOFLASH(M6502_IndY)(sM6502* cpu)
{
	u8 a = M6502_ProgByte(cpu); // get short address from 2nd byte of the instruction
	u8 low = cpu->readmem(a); // load indirect address LOW from zero page
	u8 high = cpu->readmem((u8)(a+1)); // load indirect address HIGH from zero page
	return (u16)(low | ((u16)high << 8)) + cpu->y; // compose effective address, with Y offset
}

// get address of zero page X ... zeropage,X
INLINE u8 M6502_ZeropageX(sM6502* cpu)
{
	return (u8)(M6502_ProgByte(cpu) + cpu->x);
}

// get address of zero page Y ... zeropage,Y
INLINE u8 M6502_ZeropageY(sM6502* cpu)
{
	return (u8)(M6502_ProgByte(cpu) + cpu->y);
}

// get address of absolute X ... absolute,X
INLINE u16 M6502_AbsoluteX(sM6502* cpu)
{
	return (u16)(M6502_ProgWord(cpu) + cpu->x);
}

// get address of absolute Y ... absolute,Y
INLINE u16 M6502_AbsoluteY(sM6502* cpu)
{
	return (u16)(M6502_ProgWord(cpu) + cpu->y);
}

// ORA (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ORA)(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 n = cpu->readmem(addr);

	// do operation
	n |= cpu->a;

	// store result to A
	cpu->a = n;

	// set flags
	M6502_SetNZ(cpu, n);
}

// AND (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_AND)(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 n = cpu->readmem(addr);

	// do operation
	n &= cpu->a;

	// store result to A
	cpu->a = n;

	// set flags
	M6502_SetNZ(cpu, n);
}

// EOR (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_EOR)(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 n = cpu->readmem(addr);

	// do operation
	n ^= cpu->a;

	// store result to A
	cpu->a = n;

	// set flags
	M6502_SetNZ(cpu, n);
}

// ADC (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ADC)(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 val = cpu->readmem(addr);

	// get flags and accumulator
	u8 f0 = cpu->f;
	u8 a = cpu->a;

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

	// store result
	cpu->a = a;
	cpu->f = f;
}

// SBC (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_SBC)(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 val = cpu->readmem(addr);

	// get flags and accumulator
	u8 f0 = cpu->f;
	u8 a = cpu->a;

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

	// store result
	cpu->a = a;
	cpu->f = f;
}

// ASL
u8 FASTCODE NOFLASH(M6502_ASL)(sM6502* cpu, u8 val)
{
	// shift operation
	u8 n = val << 1;

	// set flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);
	if (n == 0) f |= M6502_Z; // zero flag
	if ((s8)n < 0) f |= M6502_N; // negative flag
	if ((val & B7) != 0) f |= M6502_C; // carry flag
	cpu->f = f;

	return n;
}

// ASL memory (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ASL_M)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// ASL val
	n = M6502_ASL(cpu, n);

	// write new content to memory
	cpu->writemem(addr, n);
}

// ROL
u8 FASTCODE NOFLASH(M6502_ROL)(sM6502* cpu, u8 val)
{
	// shift operation
	u8 n = val << 1;
	u8 f = cpu->f;
	n |= f & M6502_C; // M6502_C is bit 0

	// set flags
	f &= ~(M6502_N | M6502_Z | M6502_C);
	if (n == 0) f |= M6502_Z; // zero flag
	if ((s8)n < 0) f |= M6502_N; // negative flag
	if ((val & B7) != 0) f |= M6502_C; // carry flag
	cpu->f = f;

	return n;
}

// ROL memory (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ROL_M)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// ROL val
	n = M6502_ROL(cpu, n);

	// write new content to memory
	cpu->writemem(addr, n);
}

// LSR
u8 FASTCODE NOFLASH(M6502_LSR)(sM6502* cpu, u8 val)
{
	// shift operation
	u8 n = val >> 1;

	// set flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);
	if (n == 0) f |= M6502_Z; // zero flag
	f |= val & M6502_C; // carry flag
	cpu->f = f;

	return n;
}

// LSR memory (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_LSR_M)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// LSR val
	n = M6502_LSR(cpu, n);

	// write new content to memory
	cpu->writemem(addr, n);
}

// ROR
u8 FASTCODE NOFLASH(M6502_ROR)(sM6502* cpu, u8 val)
{
	// shift operation
	u8 n = val >> 1;
	u8 f = cpu->f;
	n |= f << 7; // M6502_C is bit 0

	// set flags
	f &= ~(M6502_N | M6502_Z | M6502_C);
	if (n == 0) f |= M6502_Z; // zero flag
	if ((s8)n < 0) f |= M6502_N; // negative flag
	f |= val & M6502_C; // carry flag
	cpu->f = f;

	return n;
}

// ROR memory (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ROR_M)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// ROR val
	n = M6502_ROR(cpu, n);

	// write new content to memory
	cpu->writemem(addr, n);
}

// SLO (addr = source address of 2nd operand) ... ASL + ORA instruction (undocumented)
void FASTCODE NOFLASH(M6502_SLO)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 val = cpu->readmem(addr);

	// shift operation
	u8 n = val << 1;

	// OR operation with A
	u8 a = cpu->a | n;

	// store result to A
	cpu->a = a;

	// set flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);
	if (a == 0) f |= M6502_Z; // zero flag
	if ((s8)a < 0) f |= M6502_N; // negative flag
	if ((val & B7) != 0) f |= M6502_C; // carry flag
	cpu->f = f;

	// write new content to memory
	cpu->writemem(addr, n);
}

// RLA (addr = source address of 2nd operand) ... ROL + AND instruction (undocumented)
void FASTCODE NOFLASH(M6502_RLA)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 val = cpu->readmem(addr);

	// shift operation
	u8 n = val << 1;
	u8 f = cpu->f;
	n |= f & M6502_C; // M6502_C is bit 0

	// AND operation with A
	u8 a = cpu->a & n;

	// store result to A
	cpu->a = a;

	// set flags
	f &= ~(M6502_N | M6502_Z | M6502_C);
	if (a == 0) f |= M6502_Z; // zero flag
	if ((s8)a < 0) f |= M6502_N; // negative flag
	if ((val & B7) != 0) f |= M6502_C; // carry flag
	cpu->f = f;

	// write new content to memory
	cpu->writemem(addr, n);
}

// SRE (addr = source address of 2nd operand) ... LSR + EOR instruction (undocumented)
void FASTCODE NOFLASH(M6502_SRE)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 val = cpu->readmem(addr);

	// shift operation
	u8 n = val >> 1;

	// XOR operation with A
	u8 a = cpu->a ^ n;

	// store result to A
	cpu->a = a;

	// set flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);
	if (a == 0) f |= M6502_Z; // zero flag
	if ((s8)a < 0) f |= M6502_N; // negative flag
	f |= val & M6502_C; // carry flag
	cpu->f = f;

	// write new content to memory
	cpu->writemem(addr, n);
}

// RRA (addr = source address of 2nd operand) ... ROR + ADC instruction (undocumented)
void FASTCODE NOFLASH(M6502_RRA)(sM6502* cpu, u16 addr)
{
	// do ROR operation
	M6502_ROR_M(cpu, addr);

	// do ADC operation
	M6502_ADC(cpu, addr);
}

// BIT (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_BIT)(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 n = cpu->readmem(addr);

	// get flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_V);

	// get N and V flags
	f |= n & (M6502_N | M6502_V);

	// and with A
	n &= cpu->a;

	// zero flag
	if (n == 0) f |= M6502_Z;

	cpu->f = f;
}

// DEC (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_DEC)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// do operation
	n--;

	// set flags
	M6502_SetNZ(cpu, n);

	// write new content to memory
	cpu->writemem(addr, n);
}

// INC (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_INC)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// do operation
	n++;

	// set flags
	M6502_SetNZ(cpu, n);

	// write new content to memory
	cpu->writemem(addr, n);
}

// CMP val,M (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_CMP)(sM6502* cpu, u8 val, u16 addr)
{
	// clear flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);

	// read content of memory
	u8 n = cpu->readmem(addr);

	// compare
	u16 res = val - n;

	// check carry (carry is inverted)
	if (res < 0x100) f |= M6502_C;

	// check zero
	n = (u8)res;
	if (n == 0) f |= M6502_Z;

	// check negative
	if ((s8)n < 0) f |= M6502_N;

	// store flags
	cpu->f = f;
}

// LAX (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_LAX)(sM6502* cpu, u16 addr)
{
	// read content of memory
	u8 n = cpu->readmem(addr);

	// store A and X
	cpu->a = n;
	cpu->x = n;

	// set flags
	M6502_SetNZ(cpu, n);
}

// DCP (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_DCP)(sM6502* cpu, u16 addr)
{
	M6502_DEC(cpu, addr);
	M6502_CMP(cpu, cpu->a, addr);
}

// ISC (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ISC)(sM6502* cpu, u16 addr)
{
	M6502_INC(cpu, addr);
	M6502_SBC(cpu, addr);
}

// IRQ interrupt (addr = vector address)
void FASTCODE NOFLASH(M6502_IRQ)(sM6502* cpu, u16 addr)
{
	// push PC and flags
	M6502_Push(cpu, cpu->pch); // push PC pointer HIGH
	M6502_Push(cpu, cpu->pcl); // push PC pointer LOW
	M6502_Push(cpu, cpu->f | M6502_E); // push flags (Break flag is 0)

	// load destination address
	u8 pcl = cpu->readmem(addr); // load jump address LOW
	cpu->pch = cpu->readmem(addr+1); // load jump address HIGH
	cpu->pcl = pcl;
	cpu->f |= M6502_I; // disable interrupt
}
