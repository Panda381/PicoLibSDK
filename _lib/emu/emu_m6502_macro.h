
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

// load byte from RAM zero page
u8 INLINE M6502_ReadZp(sM6502* cpu, u8 addr) { return cpu->ramzp[addr]; }

// load word from RAM zero page
u16 INLINE M6502_ReadZpW(sM6502* cpu, u8 addr) { return cpu->ramzp[addr] | ((u16)cpu->ramzp[(u8)(addr+1)] << 8); }

// write byte to RAM zero page
void INLINE M6502_WriteZp(sM6502* cpu, u8 addr, u8 val) { cpu->ramzp[addr] = val; }

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
	cpu->stack[sp] = n;
}

// pop byte from the stack
u8 FASTCODE NOFLASH(M6502_Pop)(sM6502* cpu)
{
	u8 sp = cpu->sp + 1;
	cpu->sp = sp;
	return cpu->stack[sp];
}

// get 16-bit address of X-indexed indirect addressing ... (indirect,X) ... 16-bit address from zero page 8(imm + X)
u16 FASTCODE NOFLASH(M6502_IndX)(sM6502* cpu)
{
	u8 a = M6502_ProgByte(cpu);		// get short address from 2nd byte of the instruction
	a += cpu->x;				// add X offset, discarding carry
	return M6502_ReadZpW(cpu, a);		// get word from zero page
}

// get 16-bit address of Y-indirect indexed addressing, write ... (indirect),Y ... 16-bit address from zero page 8(imm) + Y
u16 FASTCODE NOFLASH(M6502_IndYW)(sM6502* cpu)
{
	u8 a = M6502_ProgByte(cpu);		// get short address from 2nd byte of the instruction
	u16 w = M6502_ReadZpW(cpu, a);		// get word from zero page
	return w + cpu->y;			// compose effective address, with Y offset
}

// get 16-bit address of Y-indirect indexed addressing, read ... (indirect),Y ... 16-bit address from zero page 8(imm) + Y
u16 FASTCODE NOFLASH(M6502_IndYR)(sM6502* cpu)
{
	u8 a = M6502_ProgByte(cpu);		// get short address from 2nd byte of the instruction
	u16 addr = M6502_ReadZpW(cpu, a);	// get word from zero page
	u16 addr2 = addr + cpu->y;		// add Y offset
	if (((addr ^ addr2) & 0x0100) != 0) cpu->sync.clock += M6502_CLOCKMUL; // time correction on page boundary
	return addr2;
}

// get 8-bit address of zero page X ... zeropage,X ... 8-bit zero page address 8(imm + X)
INLINE u8 M6502_ZeropageX(sM6502* cpu)
{
	return (u8)(M6502_ProgByte(cpu) + cpu->x);
}

// get 8-bit address of zero page Y ... zeropage,Y ... 8-bit zero page address 8(imm + Y)
INLINE u8 M6502_ZeropageY(sM6502* cpu)
{
	return (u8)(M6502_ProgByte(cpu) + cpu->y);
}

// get 16-bit address of absolute X, write ... absolute,X ... immediate 16-bit absolute address 16(imm) + X
INLINE u16 M6502_AbsoluteXW(sM6502* cpu)
{
	return (u16)(M6502_ProgWord(cpu) + cpu->x);
}

// get 16-bit address of absolute X, read ... absolute,X ... immediate 16-bit absolute address 16(imm) + X
u16 FASTCODE NOFLASH(M6502_AbsoluteXR)(sM6502* cpu)
{
	u16 addr = M6502_ProgWord(cpu);
	u16 addr2 = addr + cpu->x;
	if (((addr ^ addr2) & 0x0100) != 0) cpu->sync.clock += M6502_CLOCKMUL; // time correction on page boundary
	return addr2;
}

// get 16-bit address of absolute Y, write ... absolute,Y ... immediate 16-bit absolute address 16(imm) + Y
INLINE u16 M6502_AbsoluteYW(sM6502* cpu)
{
	return (u16)(M6502_ProgWord(cpu) + cpu->y);
}

// get 16-bit address of absolute Y, read ... absolute,Y ... immediate 16-bit absolute address 16(imm) + Y
u16 FASTCODE NOFLASH(M6502_AbsoluteYR)(sM6502* cpu)
{
	u16 addr = M6502_ProgWord(cpu);
	u16 addr2 = addr + cpu->y;
	if (((addr ^ addr2) & 0x0100) != 0) cpu->sync.clock += M6502_CLOCKMUL; // time correction on page boundary
	return addr2;
}

// ORA zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_ORA_Zp)(sM6502* cpu, u8 addr)
{
	// load 2nd operand
	u8 n = M6502_ReadZp(cpu, addr);

	// do operation
	n |= cpu->a;

	// store result to A
	cpu->a = n;

	// set flags
	M6502_SetNZ(cpu, n);
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

// AND zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_AND_Zp)(sM6502* cpu, u8 addr)
{
	// load 2nd operand
	u8 n = M6502_ReadZp(cpu, addr);

	// do operation
	n &= cpu->a;

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

// EOR zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_EOR_Zp)(sM6502* cpu, u8 addr)
{
	// load 2nd operand
	u8 n = M6502_ReadZp(cpu, addr);

	// do operation
	n ^= cpu->a;

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

// ADC
void FASTCODE NOFLASH(M6502_ADC_)(sM6502* cpu, u8 val)
{
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


#if !M6502_CPU_2A03	// 1=use NES CPU Ricoh 2A93 modification (does not contain support for decimal)

	// decimal correction (not supported by Ricoh 2A93)
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

#endif // M6502_CPU_2A03


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

// ADC zeropage (addr = source address of 2nd operand)
INLINE void M6502_ADC_Zp(sM6502* cpu, u8 addr)
{
	// load 2nd operand
	u8 val = M6502_ReadZp(cpu, addr);

	// ADC
	M6502_ADC_(cpu, val);
}

// ADC (addr = source address of 2nd operand)
INLINE void M6502_ADC(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 val = cpu->readmem(addr);

	// ADC
	M6502_ADC_(cpu, val);
}

// SBC
void FASTCODE NOFLASH(M6502_SBC_)(sM6502* cpu, u8 val)
{
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


#if !M6502_CPU_2A03	// 1=use NES CPU Ricoh 2A93 modification (does not contain support for decimal)

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

#endif // M6502_CPU_2A03


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

// SBC zeropage (addr = source address of 2nd operand)
INLINE void M6502_SBC_Zp(sM6502* cpu, u8 addr)
{
	// load 2nd operand
	u8 val = M6502_ReadZp(cpu, addr);

	// SBC
	M6502_SBC_(cpu, val);
}

// SBC (addr = source address of 2nd operand)
INLINE void M6502_SBC(sM6502* cpu, u16 addr)
{
	// load 2nd operand
	u8 val = cpu->readmem(addr);

	// SBC
	M6502_SBC_(cpu, val);
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

// ASL zeropage (addr = source address of 2nd operand)
INLINE void M6502_ASL_Zp(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// ASL val
	n = M6502_ASL(cpu, n);

	// write new content to memory
	M6502_WriteZp(cpu, addr,  n);
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

// ROL zeropage (addr = source address of 2nd operand)
INLINE void M6502_ROL_Zp(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// ROL val
	n = M6502_ROL(cpu, n);

	// write new content to memory
	M6502_WriteZp(cpu, addr,  n);
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

// LSR zeropage (addr = source address of 2nd operand)
INLINE void M6502_LSR_Zp(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// LSR val
	n = M6502_LSR(cpu, n);

	// write new content to memory
	M6502_WriteZp(cpu, addr,  n);
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

// ROR zeropage (addr = source address of 2nd operand)
INLINE void M6502_ROR_Zp(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// ROR val
	n = M6502_ROR(cpu, n);

	// write new content to memory
	M6502_WriteZp(cpu, addr,  n);
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

// BIT zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_BIT_Zp)(sM6502* cpu, u8 addr)
{
	// load 2nd operand
	u8 n = M6502_ReadZp(cpu, addr);

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

// DEC zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_DEC_Zp)(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// do operation
	n--;

	// set flags
	M6502_SetNZ(cpu, n);

	// write new content to memory
	M6502_WriteZp(cpu, addr,  n);
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

// INC zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_INC_Zp)(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// do operation
	n++;

	// set flags
	M6502_SetNZ(cpu, n);

	// write new content to memory
	M6502_WriteZp(cpu, addr,  n);
}

// CMP val,M (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_CMP)(sM6502* cpu, u8 val, u16 addr)
{
	// clear flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);

	// read content of memory
	u8 n = cpu->readmem(addr);

	// compare
	u16 res = (u16)val - n;

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

// CMP val,M zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_CMP_Zp)(sM6502* cpu, u8 val, u8 addr)
{
	// clear flags
	u8 f = cpu->f & ~(M6502_N | M6502_Z | M6502_C);

	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

	// compare
	u16 res = (u16)val - n;

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

// LAX zeropage (addr = source address of 2nd operand)
void FASTCODE NOFLASH(M6502_LAX_Zp)(sM6502* cpu, u8 addr)
{
	// read content of memory
	u8 n = M6502_ReadZp(cpu, addr);

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
	M6502_Push(cpu, (cpu->f | M6502_E) & ~M6502_B); // push flags (Break flag is 0)

	// load destination address
	u8 pcl = cpu->readmem(addr); // load jump address LOW
	cpu->pch = cpu->readmem(addr+1); // load jump address HIGH
	cpu->pcl = pcl;
	cpu->f |= M6502_I; // disable interrupt
#if M6502_CPU_2A03 || M6502_CPU_65C02
	cpu->f &= ~M6502_D; // disable decimal
#endif

	// shift time
	cpu->sync.clock += M6502_CLOCKMUL*7;
}

// conditional branch if condition is True
void FASTCODE NOFLASH(M6502_BRA)(sM6502* cpu, Bool cc)
{
	if (cc) // if condition is True
	{
		s8 rel = (s8)M6502_ProgByte(cpu); // read relative offset
		u16 pc = cpu->pc;		// current address
		u16 pc2 = pc + rel;		// new address
		cpu->pc = pc2;			// set new PC
		cpu->sync.clock += M6502_CLOCKMUL*3; // 3 clock cycles if jump
		if (((pc ^ pc2) & 0x0100) != 0) cpu->sync.clock += M6502_CLOCKMUL; // time correction on page boundary
	}
	else
	{
		cpu->pc++;			// skip byte with relative offset
		cpu->sync.clock += M6502_CLOCKMUL*2; // 2 clock cycles if no jump
	}
}
