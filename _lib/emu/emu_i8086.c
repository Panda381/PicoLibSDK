
// ****************************************************************************
//
//                      I8088/I8086/I80186/I80188 CPU Emulator
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

// 8086 uncodumented:
//  https://en.wikipedia.org/wiki/X86_instruction_listings#Undocumented_x86_instructions
//  https://www.righto.com/2023/07/undocumented-8086-instructions.html ... undocumented instructions
//  https://www.righto.com/2023/03/8086-multiplication-microcode.html ... multiplication
//  https://www.righto.com/2023/04/reverse-engineering-8086-divide-microcode.html ... division

#if USE_EMU_I8086		// use I8086/I8088/I80186/I80188 CPU emulator

// current CPU descriptor (NULL = not running)
volatile sI8086* I8086_Cpu = NULL;

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
		f = (u8)I8086_FLAGHW;
		if (par == 0) f |= I8086_PF; // even parity
		if (i >= 0x80) f |= I8086_SF; // negative
		if (i == 0) f |= I8086_ZF; // zero
		I8086_FlagParTab[i] = f;
	}
}

// reset processor
void I8086_Reset(sI8086* cpu)
{
	cpu->segpref = I8086_SEG_NO;	// index of segment prefix for current instruction (I8086_SEG_NO if no prefix)
	cpu->segpref_next = I8086_SEG_NO; // index of segment prefix for next instruction (I8086_SEG_NO if no prefix)
	cpu->reppref = I8086_REP_NO;	// REP prefix flag I8086_REP_* (internal flag - REP prefix has been applied ... IMUL and IDIV uses this flag to track sign)
	cpu->reppref_next = I8086_REP_NO; // next REP prefix flag
	cpu->waiting = 0;		// waiting if TEST pin is HIGH (WAIT instruction)
	cpu->tid = 0;			// temporary interrupt disable
	cpu->halted = 0;		// halted
	cpu->ip = 0;			// instruction pointer
	cpu->flags = I8086_FLAGDEF;	// flags
	cpu->es = 0;			// extra segment
	cpu->cs = 0xffff;		// code segment
	cpu->ss = 0;			// stack segment
	cpu->ds = 0;			// data segment
}

#include "emu_i8086_macro.h"

// execute program (start or continue, until "stop" request)
// 8086: Size of code of this function and subfunctions: 12752 code + 1356 jump table = 14108 bytes
// 80186: Size of code of this function and subfunctions: 14472 code + 1412 jump table = 15884 bytes
// CPU loading at 4.77 MHz on 252 MHz: used 17-44%, max. 30-130%
// CPU loading at 12 MHz on 252 MHz: used 46-999%, max. 98-999%
void FASTCODE NOFLASH(I8086_Exec)(sI8086* cpu)
{
	u8 op, m;
	u32 irq;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// check interrupt request
		irq = EmuInterGet(&cpu->sync);
		if (((irq & I8086_IRQ_ALL) != 0) && (cpu->tid == 0) && ((cpu->flags & I8086_IF) != 0))
		{
			// continue after HALT
			if (cpu->halted != 0)
			{
				cpu->ip++;
				cpu->halted = 0;
			}

			// find INT
			// IRQ0..IRQ7: raise INT 08h..INT 0Fh
			// IRQ8..IRQ15: raise INT 70h..INT 77h
			m = 8;
			op = 0;
			while ((irq & 1) == 0)
			{
				irq >>= 1;
				op++;
				m++;
				if (m == 0x10) m = 0x1b;
				if (m == 0x1d) m = 0x23;
				if (m == 0x24) m = 0x70;
			}
			
			// clear interrupt request
			EmuInterClrBit(&cpu->sync, op);

			// raise INT
			I8086_INT(cpu, m);
		}

		// shift segment pregix
		cpu->segpref = cpu->segpref_next;
		cpu->segpref_next = I8086_SEG_NO;

		// shift REP prefix
		cpu->reppref = cpu->reppref_next;
		cpu->reppref_next = I8086_REP_NO;

		// temporary interrupt disable
		if (cpu->tid > 0) cpu->tid--;

		// get next instruction
		op = I8086_ProgByte(cpu);

		// switch base operation code
		switch (op)
		{
		// ADD mem/reg,reg (byte)
		case 0x00:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_ADD8(cpu);	// ADD
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// ADD mem/reg,reg (word)
		case 0x01:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_ADD16(cpu);	// ADD
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// ADD reg,mem/reg (byte)
		case 0x02:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_ADD8(cpu);	// ADD
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// ADD reg,mem/reg (word)
		case 0x03:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_ADD16(cpu);	// ADD
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// ADD AL,kk
		case 0x04:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_ADD8(cpu);	// ADD
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// ADD AX,kk
		case 0x05: // + word kk
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_ADD16(cpu);	// ADD
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// PUSH ES
		case 0x06:
		// PUSH CS
		case 0x0E:
		// PUSH SS
		case 0x16:
		// PUSH DS
		case 0x1E:
			I8086_PUSH(cpu, cpu->regseg[op >> 3]);
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// POP CS ... undocumented
		case 0x0F:
		// POP SS
		case 0x17:
			cpu->tid = 1; // temporary interrupt disable
		// POP ES
		case 0x07:
		// POP DS
		case 0x1F:
			cpu->regseg[op >> 3] = I8086_POP(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// OR mem/reg,reg (byte)
		case 0x08:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_OR8(cpu);		// OR
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// OR mem/reg,reg (word)
		case 0x09:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_OR16(cpu);	// OR
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// OR reg,mem/reg (byte)
		case 0x0A:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_OR8(cpu);		// OR
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// OR reg,mem/reg (word)
		case 0x0B:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_OR16(cpu);	// OR
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// OR AL,kk
		case 0x0C:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_OR8(cpu);		// OR
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// OR AX,kk
		case 0x0D:
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_OR16(cpu);	// OR
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// ADC mem/reg,reg (byte)
		case 0x10:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_ADC8(cpu);	// ADC
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// ADC mem/reg,reg (word)
		case 0x11:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_ADC16(cpu);	// ADC
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// ADC reg,mem/reg (byte)
		case 0x12:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_ADC8(cpu);	// ADC
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// ADC reg,mem/reg (word)
		case 0x13:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_ADC16(cpu);	// ADC
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// ADC AL,kk
		case 0x14:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_ADC8(cpu);	// ADC
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// ADC AX,kk
		case 0x15:
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_ADC16(cpu);	// ADC
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SBB mem/reg,reg (byte)
		case 0x18:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_SBB8(cpu);	// SBB
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// SBB mem/reg,reg (word)
		case 0x19:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_SBB16(cpu);	// SBB
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// SBB reg,mem/reg (byte)
		case 0x1A:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_SBB8(cpu);	// SBB
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// SBB reg,mem/reg (word)
		case 0x1B:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_SBB16(cpu);	// SBB
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// SBB AL,kk
		case 0x1C:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_SBB8(cpu);	// SBB
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SBB AX,kk
		case 0x1D: // + word kk
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_SBB16(cpu);	// SBB
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// AND mem/reg,reg (byte)
		case 0x20:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_AND8(cpu);	// AND
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// AND mem/reg,reg (word)
		case 0x21:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_AND16(cpu);	// AND
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// AND reg,mem/reg (byte)
		case 0x22:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_AND8(cpu);	// AND
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// AND reg,mem/reg (word)
		case 0x23:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_AND16(cpu);	// AND
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// AND AL,kk
		case 0x24:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_AND8(cpu);	// AND
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// AND AX,kk
		case 0x25:
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_AND16(cpu);	// AND
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SEG ES
		case 0x26:
			cpu->tid = 1; // temporary interrupt disable
			cpu->reppref_next = cpu->reppref; // hold next REP prefix
			cpu->segpref_next = I8086_SEG_ES;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// DAA
		case 0x27:
			{ // ... code verified on contemporary PC hardware
				u16 f = cpu->flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF);
				u8 a = cpu->al;
#if I8086_CPU_AMD
				u8 a0 = a;
#endif

				if ((a > 0x99) || ((f & I8086_CF) != 0))
				{
					a += 0x60;
					f |= I8086_CF;
				}

				if (((a & 0x0f) > 9) || ((f & I8086_AF) != 0))
				{
					a += 6;
					f |= I8086_AF;
				}

#if I8086_CPU_AMD
				if ((a0 < 0x80) && (a >= 0x80)) f |= I8086_OF;
#endif

				cpu->al = a;
				f |= I8086_FlagParTab[a];
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SUB mem/reg,reg (byte)
		case 0x28:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_SUB8(cpu);	// SUB
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// SUB mem/reg,reg (word)
		case 0x29:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_SUB16(cpu);	// SUB
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// SUB reg,mem/reg (byte)
		case 0x2A:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_SUB8(cpu);	// SUB
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// SUB reg,mem/reg (word)
		case 0x2B:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_SUB16(cpu);	// SUB
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// SUB AL,kk
		case 0x2C:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_SUB8(cpu);	// SUB
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SUB AX,kk
		case 0x2D: // + word kk
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_SUB16(cpu);	// SUB
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SEG CS
		case 0x2E:
			cpu->tid = 1; // temporary interrupt disable
			cpu->reppref_next = cpu->reppref; // hold next REP prefix
			cpu->segpref_next = I8086_SEG_CS;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// DAS
		case 0x2F:
			{ // ... code verified on contemporary PC hardware
				u16 f = cpu->flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF);
				u8 a = cpu->al;
#if I8086_CPU_AMD
				u8 a0 = a;
#endif

				if ((a > 0x99) || ((f & I8086_CF) != 0))
				{
					a -= 0x60;
					f |= I8086_CF;
				}

				if (((a & 0x0f) > 9) || ((f & I8086_AF) != 0))
				{
					if (a < 6) f |= I8086_CF;
					a -= 6;
					f |= I8086_AF;
				}

#if I8086_CPU_AMD
				if ((a0 >= 0x80) && (a < 0x80)) f |= I8086_OF;
#endif

				cpu->al = a;
				f |= I8086_FlagParTab[a];
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// XOR mem/reg,reg (byte)
		case 0x30:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_XOR8(cpu);	// XOR
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// XOR mem/reg,reg (word)
		case 0x31:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_XOR16(cpu);	// XOR
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
			break;

		// XOR reg,mem/reg (byte)
		case 0x32:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_XOR8(cpu);	// XOR
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// XOR reg,mem/reg (word)
		case 0x33:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_XOR16(cpu);	// XOR
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// XOR AL,kk
		case 0x34:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_XOR8(cpu);	// XOR
			cpu->al = cpu->dst8;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// XOR AX,kk
		case 0x35:
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_XOR16(cpu);	// XOR
			cpu->ax = cpu->dst16;	// save result
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SEG SS
		case 0x36:
			cpu->tid = 1; // temporary interrupt disable
			cpu->reppref_next = cpu->reppref; // hold next REP prefix
			cpu->segpref_next = I8086_SEG_SS;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// AAA
		case 0x37:
			{ // ... code verified on contemporary PC hardware
				u16 f = cpu->flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF);
				u8 al = cpu->al;
				u8 ah = cpu->ah;

				if (((al & 0x0f) > 9) || ((f & I8086_AF) != 0))
				{
					al += 6;

					// === Implementation difference:
					//   On 8086/8088, correction "6" is added to AL register only.
					//   On 80286 and later, correction "6" is added do AX register (so add carry +1 to AH).
#if I8086_CPU_80186 || I8086_CPU_AMD	// use 80186 or later, or use AMD
					if (al < 6)
					{
						ah++;
#if I8086_CPU_AMD
						if (ah == 0x80) f |= I8086_OF;
#endif // AMD
					}
#endif // 80286 || AMD

					ah++;

#if I8086_CPU_AMD
					if (ah == 0x80) f |= I8086_OF;
#endif

					f |= I8086_AF | I8086_CF;
				}
				else
					f &= ~(I8086_AF | I8086_CF);

#if I8086_CPU_AMD
				if ((al == 0) && (ah == 0)) f |= I8086_ZF;
				f |= I8086_FlagParTab[al] & I8086_PF;
				if (ah >= 0x80) f |= I8086_SF;
#endif

				al &= 0x0f;

#if I8086_CPU_INTEL
				if (al == 0) f |= I8086_ZF;
				f |= I8086_FlagParTab[al] & I8086_PF;
#endif

				cpu->al = al;
				cpu->ah = ah;
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// CMP mem/reg,reg (byte)
		case 0x38:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_SUB8(cpu);	// SUB
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// CMP mem/reg,reg (word)
		case 0x39:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_SUB16(cpu);	// SUB
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// CMP reg,mem/reg (byte)
		case 0x3A:
			I8086_OpenR_MR8(cpu);	// open instruction
			I8086_SUB8(cpu);	// SUB
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// CMP reg,mem/reg (word)
		case 0x3B:
			I8086_OpenR_MR16(cpu);	// open instruction
			I8086_SUB16(cpu);	// SUB
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// CMP AL,kk
		case 0x3C:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_SUB8(cpu);	// SUB
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// CMP AX,kk
		case 0x3D:
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_SUB16(cpu);	// SUB
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// SEG DS
		case 0x3E:
			cpu->tid = 1; // temporary interrupt disable
			cpu->reppref_next = cpu->reppref; // hold next REP prefix
			cpu->segpref_next = I8086_SEG_DS;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// AAS
		case 0x3F:
			{ // ... code verified on contemporary PC hardware
				u16 f = cpu->flags & ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF);
				u8 al = cpu->al;
				u8 ah = cpu->ah;

				if (((al & 0x0f) > 9) || ((f & I8086_AF) != 0))
				{
					al -= 6;

					// === Implementation difference:
					//   On 8086/8088, correction "6" is subtracted from AL register only.
					//   On 80286 and later, correction "6" is subtracted from AX register (so subtract borrow -1 from AH).
#if I8086_CPU_80186 || I8086_CPU_AMD	// use 80186 or later, or use AMD
					if (al >= 0xfa)
					{
						ah--;
#if I8086_CPU_AMD
						if (ah == 0x7f) f |= I8086_OF;
#endif // AMD
					}
#endif // 80286 || AMD

					ah--;

#if I8086_CPU_AMD
					if (ah == 0x7f) f |= I8086_OF;
#endif

					f |= I8086_AF | I8086_CF;
				}
				else
					f &= ~(I8086_AF | I8086_CF);

#if I8086_CPU_AMD
				if ((al == 0) && (ah == 0)) f |= I8086_ZF;
				if (ah >= 0x80) f |= I8086_SF;
				f |= I8086_FlagParTab[al] & I8086_PF;
#endif

				al &= 0x0f;

#if I8086_CPU_INTEL
				if (al == 0) f |= I8086_ZF;
				f |= I8086_FlagParTab[al] & I8086_PF;
#endif

				cpu->al = al;
				cpu->ah = ah;
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// INC AX
		case 0x40:
		// INC CX
		case 0x41:
		// INC DX
		case 0x42:
		// INC BX
		case 0x43:
		// INC SP
		case 0x44:
		// INC BP
		case 0x45:
		// INC SI
		case 0x46:
		// INC DI
		case 0x47:
			op -= 0x40;
			cpu->dst16 = cpu->reg16[op]; // destination operand
			I8086_INC16(cpu);	// INC
			cpu->reg16[op] = cpu->dst16; // save result
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// DEC AX
		case 0x48:
		// DEC CX
		case 0x49:
		// DEC DX
		case 0x4A:
		// DEC BX
		case 0x4B:
		// DEC SP
		case 0x4C:
		// DEC BP
		case 0x4D:
		// DEC SI
		case 0x4E:
		// DEC DI
		case 0x4F:
			op -= 0x48;
			cpu->dst16 = cpu->reg16[op]; // destination operand
			I8086_DEC16(cpu);	// INC
			cpu->reg16[op] = cpu->dst16; // save result
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// PUSH SP
		case 0x54:
#if I8086_CPU_80186	// 1=use 80186 or later ... Note: This difference occurs from the 80286 processor onwards.
			I8086_PUSH(cpu, cpu->sp);
#else
			I8086_PUSH(cpu, cpu->sp - 2);
#endif
			cpu->sync.clock += I8086_CLOCKMUL*11;
			break;

		// PUSH AX
		case 0x50:
		// PUSH CX
		case 0x51:
		// PUSH DX
		case 0x52:
		// PUSH BX
		case 0x53:
		// PUSH BP
		case 0x55:
		// PUSH SI
		case 0x56:
		// PUSH DI
		case 0x57:
			op -= 0x50;
			I8086_PUSH(cpu, cpu->reg16[op]); // PUSH
			cpu->sync.clock += I8086_CLOCKMUL*11;
			break;

		// POP AX
		case 0x58:
		// POP CX
		case 0x59:
		// POP DX
		case 0x5A:
		// POP BX
		case 0x5B:
		// POP SP
		case 0x5C:
		// POP BP
		case 0x5D:
		// POP SI
		case 0x5E:
		// POP DI
		case 0x5F:
			op -= 0x58;
			cpu->reg16[op] = I8086_POP(cpu); // POP
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

#if I8086_CPU_80186	// 1=use 80186 or later

		// PUSHA ... 80186 and later
		case 0x60:
			{
				u16 sp = cpu->sp;
				I8086_PUSH(cpu, cpu->ax);
				I8086_PUSH(cpu, cpu->cx);
				I8086_PUSH(cpu, cpu->dx);
				I8086_PUSH(cpu, cpu->bx);
				I8086_PUSH(cpu, sp);
				I8086_PUSH(cpu, cpu->bp);
				I8086_PUSH(cpu, cpu->si);
				I8086_PUSH(cpu, cpu->di);
			}
			cpu->sync.clock += I8086_CLOCKMUL*36;
			break;

		// POPA ... 80186 and later
		case 0x61:
			cpu->di = I8086_POP(cpu);
			cpu->si = I8086_POP(cpu);
			cpu->bp = I8086_POP(cpu);
			I8086_POP(cpu); // pop and ignore SP
			cpu->bx = I8086_POP(cpu);
			cpu->dx = I8086_POP(cpu);
			cpu->cx = I8086_POP(cpu);
			cpu->ax = I8086_POP(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*51;
			break;

		// BOUND reg,mem/reg ... 80186 and later
		case 0x62:
			{
				I8086_GetEA(cpu); // calculate effective address
				u16 low = I8086_GetRM16(cpu); // get lower limit
				u16 up = I8086_GetWord(cpu, cpu->eseg, cpu->eoff+2); // get upper limit
				u16 inx = I8086_GetR16(cpu); // get operation register (index)
				if ((inx < low) || (inx > up)) I8086_INT(cpu, 5); // error bound limits overflow
				cpu->sync.clock += I8086_CLOCKMUL*34;
			}
			break;

		// PUSH kk (word) ... 80186 and later
		case 0x68:
			{
				u16 n = I8086_ProgWord(cpu);
				I8086_PUSH(cpu, n);
			}
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// IMUL reg,mem/reg,kk (word) ... 80186 and later
		case 0x69:
			{
				I8086_GetEA(cpu); // calculate effective address
				s32 n1 = (s32)(s16)(u16)I8086_GetRM16(cpu); // get 1st operand
				s32 n2 = (s32)(s16)(u16)I8086_ProgWord(cpu); // get 2nd operand
				s32 res = n1 * n2; // result
				cpu->dst16 = (u16)(s16)res;
				I8086_SetR16(cpu);	// save result
				u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear flags, zero stays unchanged
				if ((res < -32768) || (res > 32767)) f |= I8086_CF | I8086_OF; // add carry and overflow if sign extension of AX has been changed
				cpu->flags = f;			// save new flags
			}
			cpu->sync.clock += I8086_CLOCKMUL*30;
			break;

		// PUSH kk (byte) ... 80186 and later
		case 0x6A:
			{
				u16 n = (u16)(s16)(s8)I8086_ProgByte(cpu);
				I8086_PUSH(cpu, n);
			}
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// IMUL reg,mem/reg,kk (signed byte) ... 80186 and later
		case 0x6B:
			{
				I8086_GetEA(cpu); // calculate effective address
				s32 n1 = (s32)(s16)(u16)I8086_GetRM16(cpu); // get 1st operand
				s32 n2 = (s32)(s8)(u8)I8086_ProgByte(cpu); // get 2nd operand
				s32 res = n1 * n2; // result
				cpu->dst16 = (u16)(s16)res;
				I8086_SetR16(cpu);	// save result
				u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear flags, zero stays unchanged
				if ((res < -32768) || (res > 32767)) f |= I8086_CF | I8086_OF; // add carry and overflow if sign extension of AX has been changed
				cpu->flags = f;			// save new flags
			}
			cpu->sync.clock += I8086_CLOCKMUL*23;
			break;

		// INSB ... 80186 and later
		case 0x6C:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// input byte
				u8 n = cpu->readport8(cpu->dx);

				// write byte
				I8086_SetByte(cpu, cpu->es, cpu->di, n);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->di++; // increment
				else
					cpu->di--; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*8;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*14;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// INSW ... 80186 and later
		case 0x6D:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// input byte
				u16 n = cpu->readport16(cpu->dx);

				// write word
				I8086_SetWord(cpu, cpu->es, cpu->di, n);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->di += 2; // increment
				else
					cpu->di -= 2; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*8;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*14;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// OUTSB ... 80186 and later
		case 0x6E:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read byte
				u8 n = I8086_GetByte(cpu, I8086_DS(cpu), cpu->si);

				// send byte
				cpu->writeport8(cpu->dx, n);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->si++; // increment
				else
					cpu->si--; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*8;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*14;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// OUTSW ... 80186 and later
		case 0x6F:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read word
				u16 n = I8086_GetWord(cpu, I8086_DS(cpu), cpu->si);

				// send word
				cpu->writeport16(cpu->dx, n);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->si += 2; // increment
				else
					cpu->si -= 2; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*8;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*14;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

#endif // I8086_CPU_80186

		// JO rel
		case 0x70:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x60: // JO rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_OF) != 0);
			break;

		// JNO rel
		case 0x71:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x61: // JNO rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_OF) == 0);
			break;

		// JB rel, JC rel, JNAE rel
		case 0x72:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x62: // JB rel, JC rel, JNAE rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_CF) != 0);
			break;

		// JNB rel, JAE rel
		case 0x73:
		case 0x63: // JNB rel, JAE rel ... undocumented alias
			I8086_SJUMP(cpu, (cpu->flags & I8086_CF) == 0);
			break;

		// JE rel, JZ rel
		case 0x74:
		case 0x64: // JE rel, JZ rel ... undocumented alias
			I8086_SJUMP(cpu, (cpu->flags & I8086_ZF) != 0);
			break;

		// JNE rel, JNZ rel
		case 0x75:
		case 0x65: // JNE rel, JNZ rel ... undocumented alias
			I8086_SJUMP(cpu, (cpu->flags & I8086_ZF) == 0);
			break;

		// JBE rel, JNA rel
		case 0x76:
		case 0x66: // JBE rel, JNA rel ... undocumented alias
			I8086_SJUMP(cpu, (cpu->flags & (I8086_CF | I8086_ZF)) != 0);
			break;

		// JNBE rel, JA rel
		case 0x77:
		case 0x67: // JNBE rel, JA rel ... undocumented alias
			I8086_SJUMP(cpu, (cpu->flags & (I8086_CF | I8086_ZF)) == 0);
			break;

		// JS rel
		case 0x78:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x68: // JS rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_SF) != 0);
			break;

		// JNS rel
		case 0x79:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x69: // JNS rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_SF) == 0);
			break;

		// JP rel, JPE rel
		case 0x7A:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x6A: // JP rel, JPE rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_PF) != 0);
			break;

		// JNP rel, JPO rel
		case 0x7B:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x6B: // JNP rel, JPO rel ... undocumented alias
#endif
			I8086_SJUMP(cpu, (cpu->flags & I8086_PF) == 0);
			break;

		// JL rel, JNGE rel
		case 0x7C:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x6C: // JL rel, JNGE rel ... undocumented alias
#endif
			{
				u16 f = cpu->flags;
				f = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT)); // shift sign flag to position of overflow flag (= 4 shifts), and compare
				I8086_SJUMP(cpu, (f & I8086_OF) != 0); // condition: overflow and not negative, or negative and not overflow
			}
			break;

		// JNL rel, JGE rel
		case 0x7D:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x6D: // JNL rel, JGE rel ... undocumented alias
#endif
			{
				u16 f = cpu->flags;
				f = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT)); // shift sign flag to position of overflow flag (= 4 shifts), and compare
				I8086_SJUMP(cpu, (f & I8086_OF) == 0); // condition: overflow and negative, or not overflow and not negative
			}
			break;

		// JLE rel, JNG rel
		case 0x7E:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x6E: // JLE rel, JNG rel ... undocumented alias
#endif
			{
				u16 f = cpu->flags;
				u16 f2 = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT)); // shift sign flag to position of overflow flag (= 4 shifts), and compare
				I8086_SJUMP(cpu, ((f2 & I8086_OF) != 0) || ((f & I8086_ZF) != 0)); // condition: overflow and not negative, or negative and not overflow, or zero
			}
			break;

		// JNLE rel, JG rel
		case 0x7F:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0x6F: // JNLE rel, JG rel ... undocumented alias
#endif
			{
				u16 f = cpu->flags;
				u16 f2 = f ^ (f << (I8086_OF_BIT - I8086_SF_BIT)); // shift sign flag to position of overflow flag (= 4 shifts), and compare
				I8086_SJUMP(cpu, ((f2 & I8086_OF) == 0) && ((f & I8086_ZF) == 0)); // condition: (overflow and negative, or not negative and not overflow), not not zero
			}
			break;

		// op mem/reg,kk8 (byte)
		case 0x80: // byte unsigned
		case 0x82: // byte signed .... some sources say it is undocumented opcode, it is redundant to opcode 0x80
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory (destination)
			cpu->src8 = I8086_ProgByte(cpu); // immediate source operand

			op = (cpu->modrm >> 3) & 7; // required operation
			switch (op)
			{
			case 0: I8086_ADD8(cpu); break;
			case 1: I8086_OR8(cpu); break;
			case 2: I8086_ADC8(cpu); break;
			case 3: I8086_SBB8(cpu); break;
			case 4: I8086_AND8(cpu); break;
			case 6: I8086_XOR8(cpu); break;
			default: /* case 5, case 7 */ I8086_SUB8(cpu); break;
			}

			if (op != 7) I8086_SetRM8(cpu);	// save result

			// reg,im = 4, CMP mem,im = 10, others mem,im = 17
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4)
				: ((op == 7) ? (I8086_CLOCKMUL*10) : (I8086_CLOCKMUL*17));
			break;

		// op mem/reg,kk16 (word)
		case 0x81:
			I8086_GetEA(cpu); // calculate effective address
			cpu->src16 = I8086_ProgWord(cpu); // immediate source operand
			cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (destination)

			op = (cpu->modrm >> 3) & 7; // required operation
			switch (op)
			{
			case 0: I8086_ADD16(cpu); break;
			case 1: I8086_OR16(cpu); break;
			case 2: I8086_ADC16(cpu); break;
			case 3: I8086_SBB16(cpu); break;
			case 4: I8086_AND16(cpu); break;
			case 6: I8086_XOR16(cpu); break;
			default: /* case 5, case 7 */ I8086_SUB16(cpu); break;
			}

			if (op != 7) I8086_SetRM16(cpu); // save result

			// reg,im = 4, CMP mem,im = 10, others mem,im = 17
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4)
				: ((op == 7) ? (I8086_CLOCKMUL*10) : (I8086_CLOCKMUL*17));
			break;

		// op mem16/reg16,kk8 (byte signed extended to word)
		case 0x83:
			I8086_GetEA(cpu); // calculate effective address
			cpu->src16 = (u16)(s16)(s8)I8086_ProgByte(cpu); // immediate source operand
			cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (destination)

			op = (cpu->modrm >> 3) & 7; // required operation
			switch (op)
			{
			case 0: I8086_ADD16(cpu); break;
			case 1: I8086_OR16(cpu); break;
			case 2: I8086_ADC16(cpu); break;
			case 3: I8086_SBB16(cpu); break;
			case 4: I8086_AND16(cpu); break;
			case 6: I8086_XOR16(cpu); break;
			default: /* case 5, case 7 */ I8086_SUB16(cpu); break;
			}

			if (op != 7) I8086_SetRM16(cpu); // save result

			// reg,im = 4, CMP mem,im = 10, others mem,im = 17
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4)
				: ((op == 7) ? (I8086_CLOCKMUL*10) : (I8086_CLOCKMUL*17));
			break;

		// TEST mem/reg,reg (byte)
		case 0x84:
			I8086_OpenMR_R8(cpu);	// open instruction
			I8086_AND8(cpu);	// AND
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// TEST mem/reg,reg (word)
		case 0x85:
			I8086_OpenMR_R16(cpu);	// open instruction
			I8086_AND16(cpu);	// AND
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*9);
			break;

		// XCHG reg,mem/reg (byte)
		case 0x86:
			I8086_OpenR_MR8(cpu);	// open instruction (dst8 <- REG, src8 <- MEM)
			I8086_SetRM8(cpu);	// save REG from dst8 to MEM
			cpu->dst8 = cpu->src8;	// dst8 <- MEM
			I8086_SetR8(cpu);	// save MEM from dst8 to REG
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4) : (I8086_CLOCKMUL*17);
			break;

		// XCHG reg,mem/reg (word)
		case 0x87:
			I8086_OpenR_MR16(cpu);	// open instruction (dst16 <- REG, src16 <- MEM)
			I8086_SetRM16(cpu);	// save REG from dst16 to MEM
			cpu->dst16 = cpu->src16; // dst16 <- MEM
			I8086_SetR16(cpu);	// save MEM from dst16 to REG
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4) : (I8086_CLOCKMUL*17);
			break;

		// MOV mem/reg,reg (byte)
		case 0x88:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst8 = I8086_GetR8(cpu); // get operation register
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*9);
			break;

		// MOV mem/reg,reg (word)
		case 0x89:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst16 = I8086_GetR16(cpu); // get operation register
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*9);
			break;

		// MOV reg,mem/reg (byte)
		case 0x8A:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory
			I8086_SetR8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*8);
			break;

		// MOV reg,mem/reg (word)
		case 0x8B:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (source)
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*8);
			break;

		// MOV mem/reg,segreg
		case 0x8C:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst16 = cpu->regseg[(cpu->modrm >> 3) & 3]; // ... documented bit 5 = 0, undocumented bit 5 = 1 (has same meaning: bit 5 is ignored)
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*9);
			break;

		// LEA reg,addr
		case 0x8D:
			// ... undocumented: LEA with mod == 3 returns previous effective address
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst16 = cpu->eoff; // effective address (= offset)
			I8086_SetR16(cpu);	// save result
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// MOV segreg,mem/reg
		case 0x8E:
			I8086_GetEA(cpu); // calculate effective address
			cpu->tid = 1; // temporary interrupt disable
			cpu->regseg[(cpu->modrm >> 3) & 3] = I8086_GetRM16(cpu);  // ... documented bit 5 = 0, undocumented bit 5 = 1 (has same meaning: bit 5 is ignored)
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*8);
			break;

		// POP mem/reg
		case 0x8F:
			// ... documented reg=0, undocumented reg ignored)
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst16 = I8086_POP(cpu); // pop word
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += I8086_CLOCKMUL*17;
			break;

		// XCHG AX,CX
		case 0x91:
		// XCHG AX,DX
		case 0x92:
		// XCHG AX,BX
		case 0x93:
		// XCHG AX,SP
		case 0x94:
		// XCHG AX,BP
		case 0x95:
		// XCHG AX,SI
		case 0x96:
		// XCHG AX,DI
		case 0x97:
			{
				op -= 0x90;
				u16 tmp = cpu->reg16[op];
				cpu->reg16[op] = cpu->ax;
				cpu->ax = tmp;
			}
			// - continue to case 0x90:
		// NOP ... = instruction XCHG AX,AX
		case 0x90:
			cpu->sync.clock += I8086_CLOCKMUL*3;
			break;

		// CBW
		case 0x98:
			cpu->ax = (u16)(s16)(s8)cpu->al;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// CWD
		case 0x99:
			cpu->dx = ((s16)cpu->ax < 0) ? 0xffff : 0;
			cpu->sync.clock += I8086_CLOCKMUL*5;
			break;

		// CALL far addr (direct intersegment)
		case 0x9A:
			{
				u16 off = I8086_ProgWord(cpu); // load offset
				u16 seg = I8086_ProgWord(cpu); // load segment
				I8086_PUSH(cpu, cpu->cs); // push current program segment
				I8086_PUSH(cpu, cpu->ip); // push current instruction pointer
				cpu->ip = off; // set new instruction pointer
				cpu->cs = seg; // set new program segment
			}
			cpu->sync.clock += I8086_CLOCKMUL*28;
			break;

		// WAIT
		case 0x9B:
			if (cpu->test != 0) // TEST pin is not active, wait back loop
			{
				cpu->ip--; // roll back to WAIT instruction
				cpu->waiting = 1;
				cpu->sync.clock += I8086_CLOCKMUL*5;
			}
			else
				cpu->waiting = 0; // not waiting
			cpu->sync.clock += I8086_CLOCKMUL*3;
			break;

		// PUSHF
		case 0x9C:
			I8086_PUSH(cpu, (cpu->flags & I8086_FLAGALLCTRL) | I8086_FLAGHW);
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// POPF
		case 0x9D:
			{
				u16 f = I8086_POP(cpu);
				f = (f & I8086_FLAGALLCTRL) | I8086_FLAGHW;
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// SAHF
		case 0x9E:
			cpu->flags = (cpu->flags & 0xff00) | ((cpu->ah & I8086_FLAGALLCTRL) | I8086_FLAGHW);
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// LAHF
		case 0x9F:
			cpu->ah = (u8)((cpu->flags & I8086_FLAGALLCTRL) | I8086_FLAGHW);
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// MOV AL,addr
		case 0xA0:
			cpu->al = I8086_GetByte(cpu, I8086_DS(cpu), I8086_ProgWord(cpu));
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// MOV AX,addr
		case 0xA1:
			cpu->ax = I8086_GetWord(cpu, I8086_DS(cpu), I8086_ProgWord(cpu));
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// MOV addr,AL
		case 0xA2:
			I8086_SetByte(cpu, I8086_DS(cpu), I8086_ProgWord(cpu), cpu->al);
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// MOV addr,AX
		case 0xA3:
			I8086_SetWord(cpu, I8086_DS(cpu), I8086_ProgWord(cpu), cpu->ax);
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// MOVSB
		case 0xA4:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read byte
				u8 n = I8086_GetByte(cpu, I8086_DS(cpu), cpu->si);

				// write byte
				I8086_SetByte(cpu, cpu->es, cpu->di, n);

				// shift pointers
				if ((cpu->flags & I8086_DF) == 0)
				{
					// increment
					cpu->si++;
					cpu->di++;
				}
				else
				{
					// decrement
					cpu->si--;
					cpu->di--;
				}

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*17;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*18;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// MOVSW
		case 0xA5:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read word
				u16 n = I8086_GetWord(cpu, I8086_DS(cpu), cpu->si);

				// write word
				I8086_SetWord(cpu, cpu->es, cpu->di, n);

				// shift pointers
				if ((cpu->flags & I8086_DF) == 0)
				{
					// increment
					cpu->si += 2;
					cpu->di += 2;
				}
				else
				{
					// decrement
					cpu->si -= 2;
					cpu->di -= 2;
				}

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*17;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*18;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// CMPSB
		case 0xA6:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read first byte
				cpu->dst8 = I8086_GetByte(cpu, I8086_DS(cpu), cpu->si);

				// read second byte
				cpu->src8 = I8086_GetByte(cpu, cpu->es, cpu->di);

				// shift pointers
				if ((cpu->flags & I8086_DF) == 0)
				{
					// increment
					cpu->si++;
					cpu->di++;
				}
				else
				{
					// decrement
					cpu->si--;
					cpu->di--;
				}

				// compare operands (performing subtraction)
				I8086_SUB8(cpu);

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					u16 cx = cpu->cx - 1;
					cpu->cx = cx;

					// check stop condition
					Bool stop = (cx == 0);

					if (cpu->reppref == I8086_REP_Z)
					{
						// repeat if zero (stop if not zero)
						if ((cpu->flags & I8086_ZF) == 0) stop = True;
					}
					else
					{
						// repeat if not zero (stop if zero)
						if ((cpu->flags & I8086_ZF) != 0) stop = True;
					}

					// repeat
					if (!stop)
					{
						cpu->tid = 1; // temporary interrupt disable
						cpu->reppref_next = cpu->reppref; // hold next REP prefix
						cpu->segpref_next = cpu->segpref; // hold next segment prefix
						cpu->ip--; // return to the instruction
					}

					// stop repeating, add time of last step
					else
						cpu->sync.clock += I8086_CLOCKMUL*9;
				}

				// timing
				cpu->sync.clock += I8086_CLOCKMUL*22;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// CMPSW
		case 0xA7:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read first byte
				cpu->dst16 = I8086_GetWord(cpu, I8086_DS(cpu), cpu->si);

				// read second byte
				cpu->src16 = I8086_GetWord(cpu, cpu->es, cpu->di);

				// shift pointers
				if ((cpu->flags & I8086_DF) == 0)
				{
					// increment
					cpu->si += 2;
					cpu->di += 2;
				}
				else
				{
					// decrement
					cpu->si -= 2;
					cpu->di -= 2;
				}

				// compare operands (performing subtraction)
				I8086_SUB16(cpu);

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					u16 cx = cpu->cx - 1;
					cpu->cx = cx;

					// check stop condition
					Bool stop = (cx == 0);

					if (cpu->reppref == I8086_REP_Z)
					{
						// repeat if zero (stop if not zero)
						if ((cpu->flags & I8086_ZF) == 0) stop = True;
					}
					else
					{
						// repeat if not zero (stop if zero)
						if ((cpu->flags & I8086_ZF) != 0) stop = True;
					}

					// repeat
					if (!stop)
					{
						cpu->tid = 1; // temporary interrupt disable
						cpu->reppref_next = cpu->reppref; // hold next REP prefix
						cpu->segpref_next = cpu->segpref; // hold next segment prefix
						cpu->ip--; // return to the instruction
					}

					// stop repeating, add time of last step
					else
						cpu->sync.clock += I8086_CLOCKMUL*9;
				}

				// timing
				cpu->sync.clock += I8086_CLOCKMUL*22;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// TEST AL,kk
		case 0xA8:
			cpu->dst8 = cpu->al;	// destination operand
			cpu->src8 = I8086_ProgByte(cpu); // source operand
			I8086_AND8(cpu);	// AND
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// TEST AX,kk
		case 0xA9:
			cpu->dst16 = cpu->ax;	// destination operand
			cpu->src16 = I8086_ProgWord(cpu); // source operand
			I8086_AND16(cpu);	// AND
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// STOSB
		case 0xAA:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// write byte
				I8086_SetByte(cpu, cpu->es, cpu->di, cpu->al);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->di++; // increment
				else
					cpu->di--; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*10;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*11;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// STOSW
		case 0xAB:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// write word
				I8086_SetWord(cpu, cpu->es, cpu->di, cpu->ax);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->di += 2; // increment
				else
					cpu->di -= 2; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*10;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*11;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// LODSB
		case 0xAC:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read byte
				cpu->al = I8086_GetByte(cpu, I8086_DS(cpu), cpu->si);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->si++; // increment
				else
					cpu->si--; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*13;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*12;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// LODSW
		case 0xAD:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// read word
				cpu->ax = I8086_GetWord(cpu, I8086_DS(cpu), cpu->si);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->si += 2; // increment
				else
					cpu->si -= 2; // decrement

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					cpu->cx--;

					// repeat
					cpu->tid = 1; // temporary interrupt disable
					cpu->reppref_next = cpu->reppref; // hold next REP prefix
					cpu->segpref_next = cpu->segpref; // hold next segment prefix
					cpu->ip--; // return to the instruction

					// count loop step
					cpu->sync.clock += I8086_CLOCKMUL*13;
				}

				// without repeating
				else
					cpu->sync.clock += I8086_CLOCKMUL*12;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// SCASB
		case 0xAE:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// prepare first byte
				cpu->dst8 = cpu->al;

				// read second byte
				cpu->src8 = I8086_GetByte(cpu, cpu->es, cpu->di);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->di++; // increment
				else
					cpu->di--; // decrement

				// compare operands (performing subtraction)
				I8086_SUB8(cpu);

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					u16 cx = cpu->cx - 1;
					cpu->cx = cx;

					// check stop condition
					Bool stop = (cx == 0);

					if (cpu->reppref == I8086_REP_Z)
					{
						// repeat if zero (stop if not zero)
						if ((cpu->flags & I8086_ZF) == 0) stop = True;
					}
					else
					{
						// repeat if not zero (stop if zero)
						if ((cpu->flags & I8086_ZF) != 0) stop = True;
					}

					// repeat
					if (!stop)
					{
						cpu->tid = 1; // temporary interrupt disable
						cpu->reppref_next = cpu->reppref; // hold next REP prefix
						cpu->segpref_next = cpu->segpref; // hold next segment prefix
						cpu->ip--; // return to the instruction
					}

					// stop repeating, add time of last step
					else
						cpu->sync.clock += I8086_CLOCKMUL*9;
				}

				// timing
				cpu->sync.clock += I8086_CLOCKMUL*15;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// SCASW
		case 0xAF:
			if ((cpu->reppref == I8086_REP_NO) || (cpu->cx > 0))
			{
				// prepare first word
				cpu->dst16 = cpu->ax;

				// read second word
				cpu->src8 = I8086_GetWord(cpu, cpu->es, cpu->di);

				// shift pointer
				if ((cpu->flags & I8086_DF) == 0)
					cpu->di += 2; // increment
				else
					cpu->di -= 2; // decrement

				// compare operands (performing subtraction)
				I8086_SUB8(cpu);

				// repeat
				if (cpu->reppref != I8086_REP_NO)
				{
					// decrement counter
					u16 cx = cpu->cx - 1;
					cpu->cx = cx;

					// check stop condition
					Bool stop = (cx == 0);

					if (cpu->reppref == I8086_REP_Z)
					{
						// repeat if zero (stop if not zero)
						if ((cpu->flags & I8086_ZF) == 0) stop = True;
					}
					else
					{
						// repeat if not zero (stop if zero)
						if ((cpu->flags & I8086_ZF) != 0) stop = True;
					}

					// repeat
					if (!stop)
					{
						cpu->tid = 1; // temporary interrupt disable
						cpu->reppref_next = cpu->reppref; // hold next REP prefix
						cpu->segpref_next = cpu->segpref; // hold next segment prefix
						cpu->ip--; // return to the instruction
					}

					// stop repeating, add time of last step
					else
						cpu->sync.clock += I8086_CLOCKMUL*9;
				}

				// timing
				cpu->sync.clock += I8086_CLOCKMUL*15;
			}

			// stop repeating if CX = 0 (does nothing, only skip)
			else
				cpu->sync.clock += I8086_CLOCKMUL*9;
			break;

		// MOV AL,kk
		case 0xB0:
		// MOV CL,kk
		case 0xB1:
		// MOV DL,kk
		case 0xB2:
		// MOV BL,kk
		case 0xB3:
		// MOV AH,kk
		case 0xB4:
		// MOV CH,kk
		case 0xB5:
		// MOV DH,kk
		case 0xB6:
		// MOV BH,kk
		case 0xB7:
			op -= 0xb0;
			op = ((op << 1) & (B1|B2)) | ((op >> 2) & B0);
			cpu->reg8[op] = I8086_ProgByte(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// MOV AX,kk
		case 0xB8:
		// MOV CX,kk
		case 0xB9:
		// MOV DX,kk
		case 0xBA:
		// MOV BX,kk
		case 0xBB:
		// MOV SP,kk
		case 0xBC:
		// MOV BP,kk
		case 0xBD:
		// MOV SI,kk
		case 0xBE:
		// MOV DI,kk
		case 0xBF:
			op -= 0xb8;
			cpu->reg16[op] = I8086_ProgWord(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

#if I8086_CPU_80186	// 1=use 80186 or later

		// op mem/reg,kk (byte) ... 80186 and later
		case 0xC0:
			{
				I8086_GetEA(cpu); // calculate effective address

				u32 cc = (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*5) : (I8086_CLOCKMUL*17);
				u8 c = I8086_ProgByte(cpu) & 0x1f;
				cc += c;
				cpu->sync.clock += cc;

				cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory (destination)

				if (c != 0)
				{
				// Overflow flag OF is undefined for number of shifts greater than 1.
				// AMD keeps OF according to the last shift, but Intel does not.
					op = (cpu->modrm >> 3) & 7; // required operation
					switch (op)
					{
					case 0: do { I8086_ROL8(cpu); c--; } while (c != 0); break;
					case 1: do { I8086_ROR8(cpu); c--; } while (c != 0); break;
					case 2: do { I8086_RCL8(cpu); c--; } while (c != 0); break;
					case 3: do { I8086_RCR8(cpu); c--; } while (c != 0); break;
					case 4: do { I8086_SHL8(cpu); c--; } while (c != 0); break; // SAL8
					case 5: do { I8086_SHR8(cpu); c--; } while (c != 0); break;
					// mod 6: SETMOC (Set Minus One with C - returns 0xff if CL!=0, else unchanged... undocumented (flags: CF 0, PF 1, AF 0, ZF 0, SF 1, OF 0)
					case 6: I8086_SETMO8(cpu); break;
					default: /* case 7 */ do { I8086_SAR8(cpu); c--; } while (c != 0); break;
					}
				}

				I8086_SetRM8(cpu);	// save result
			}
			break;

		// op mem/reg,kk (word) ... 80186 and later
		case 0xC1:
			{
				I8086_GetEA(cpu); // calculate effective address

				u32 cc = (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*5) : (I8086_CLOCKMUL*17);
				u8 c = I8086_ProgByte(cpu) & 0x1f;
				cc += c;
				cpu->sync.clock += cc;

				cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (destination)

				if (c != 0)
				{
				// Overflow flag OF is undefined for number of shifts greater than 1.
				// AMD keeps OF according to the last shift, but Intel does not.
					op = (cpu->modrm >> 3) & 7; // required operation
					switch (op)
					{
					case 0: do { I8086_ROL16(cpu); c--; } while (c != 0); break;
					case 1: do { I8086_ROR16(cpu); c--; } while (c != 0); break;
					case 2: do { I8086_RCL16(cpu); c--; } while (c != 0); break;
					case 3: do { I8086_RCR16(cpu); c--; } while (c != 0); break;
					case 4: do { I8086_SHL16(cpu); c--; } while (c != 0); break; // SAL16
					case 5: do { I8086_SHR16(cpu); c--; } while (c != 0); break;
					// mod 6: SETMOC (Set Minus One with C - returns 0xffff if CL!=0, else unchanged... undocumented (flags: CF 0, PF 1, AF 0, ZF 0, SF 1, OF 0)
					case 6: I8086_SETMO16(cpu); break;
					default: /* case 7 */ do { I8086_SAR16(cpu); c--; } while (c != 0); break;
					}
				}

				I8086_SetRM16(cpu);	// save result
			}
			break;

#endif // I8086_CPU_80186

		// RET kk (near, intrasegment)
		case 0xC2:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0xC0: // RET kk ... undocumented alias
#endif
			{
				u16 n = I8086_ProgWord(cpu);
				cpu->ip = I8086_POP(cpu);
				cpu->sp += n;
			}
			cpu->sync.clock += I8086_CLOCKMUL*12;
			break;

		// RET (near, intrasegment)
		case 0xC3:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0xC1: // RET ... undocumented alias
#endif
			cpu->ip = I8086_POP(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// LES reg,addr
		case 0xC4:
			{
				// load address
				I8086_GetEA(cpu); // calculate effective address

				// load offset
				if (cpu->modrm < 0xc0) // ... undocumented: if source operand is register, use last value
					cpu->dst16 = I8086_GetRM16(cpu); // get operand
				I8086_SetR16(cpu); // save offset into register

				// load segment (undocumented: if source operand is register, use last effective address)
				cpu->es = I8086_GetWord(cpu, cpu->eseg, cpu->eoff+2);
			}
			cpu->sync.clock += I8086_CLOCKMUL*16;
			break;

		// LDS reg,addr
		case 0xC5:
			{
				// load address
				I8086_GetEA(cpu); // calculate effective address

				// load offset
				if (cpu->modrm < 0xc0) // ... undocumented: if source operand is register, use last value
					cpu->dst16 = I8086_GetRM16(cpu); // get operand
				I8086_SetR16(cpu); // save offset into register

				// load segment (undocumented: if source operand is register, use last effective address)
				cpu->ds = I8086_GetWord(cpu, cpu->eseg, cpu->eoff+2);
			}
			cpu->sync.clock += I8086_CLOCKMUL*16;
			break;

		// MOV mem/reg,kk (byte)
		case 0xC6:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst8 = I8086_ProgByte(cpu); // get immediate byte
			I8086_SetRM8(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4) : (I8086_CLOCKMUL*10);
			break;

		// MOV mem/reg,kk (word)
		case 0xC7:
			I8086_GetEA(cpu); // calculate effective address
			cpu->dst16 = I8086_ProgWord(cpu); // get immediate word
			I8086_SetRM16(cpu);	// save result
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*4) : (I8086_CLOCKMUL*10);
			break;

#if I8086_CPU_80186	// 1=use 80186 or later

		// ENTER bytes,level
		case 0xC8:
			{
				u16 n = I8086_ProgWord(cpu); // space of local variables
				u8 level = I8086_ProgByte(cpu); // nesting level
				if (level == 0)
					cpu->sync.clock += I8086_CLOCKMUL*15;
				else if (level == 1)
					cpu->sync.clock += I8086_CLOCKMUL*25;
				else
					cpu->sync.clock += I8086_CLOCKMUL*(6 + 16*level);

				u16 bp = cpu->bp;
				I8086_PUSH(cpu, bp);
				u16 newbp = cpu->sp;

				// copy stack frame pointers
				if (level > 0)
				{
					for (level--; level > 0; level--)
					{
						bp -= 2;
						u16 k = I8086_GetWord(cpu, cpu->ss, bp);
						I8086_PUSH(cpu, k);
					}
					I8086_PUSH(cpu, newbp);
				}
				cpu->bp = newbp;
				cpu->sp -= n;
			}
			break;

		// LEAVE
		case 0xC9:
			cpu->sp = cpu->bp;
			cpu->bp = I8086_POP(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

#endif // I8086_CPU_80186

		// RETF kk (far, intersegment)
		case 0xCA:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0xC8: // RETF kk ... undocumented alias
#endif
			{
				u16 n = I8086_ProgWord(cpu);
				cpu->ip = I8086_POP(cpu);
				cpu->cs = I8086_POP(cpu);
				cpu->sp += n;
			}
			cpu->sync.clock += I8086_CLOCKMUL*17;
			break;

		// RETF (far, intersegment)
		case 0xCB:
#if !I8086_CPU_80186	// 1=use 80186 or later
		case 0xC9: // RETF .. undocumented alias
#endif
			cpu->ip = I8086_POP(cpu);
			cpu->cs = I8086_POP(cpu);
			cpu->sync.clock += I8086_CLOCKMUL*18;
			break;

		// INT 3
		case 0xCC:
			I8086_INT(cpu, 3);
			cpu->sync.clock++;
			break;

		// INT type
		case 0xCD:
			I8086_INT(cpu, I8086_ProgByte(cpu));
			break;

		// INTO
		case 0xCE:
			if ((cpu->flags & I8086_OF) != 0) // check overflow flag
			{
				I8086_INT(cpu, 4);
				cpu->sync.clock += 2;
			}
			else
				cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// IRET
		case 0xCF:
			{
				cpu->ip = I8086_POP(cpu);
				cpu->cs = I8086_POP(cpu);
				u16 f = I8086_POP(cpu);
				f = (f & I8086_FLAGALLCTRL) | I8086_FLAGHW;
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*24;
			break;

		// op mem/reg,1 (byte)
		case 0xD0:
			{
				I8086_GetEA(cpu); // calculate effective address
				cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory (destination)

				op = (cpu->modrm >> 3) & 7; // required operation
				switch (op)
				{
				case 0: I8086_ROL8(cpu); break;
				case 1: I8086_ROR8(cpu); break;
				case 2: I8086_RCL8(cpu); break;
				case 3: I8086_RCR8(cpu); break;
				case 4: I8086_SHL8(cpu); break; // SAL8
				case 5: I8086_SHR8(cpu); break;
				// mod 6: SETMO (Set Minus One - returns 0xff ... undocumented (flags: CF 0, PF 1, AF 0, ZF 0, SF 1, OF 0)
				case 6: I8086_SETMO8(cpu); break;
				default: /* case 7 */ I8086_SAR8(cpu); break;
				}

				I8086_SetRM8(cpu);	// save result
			}
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*15);
			break;

		// op mem/reg,1 (word)
		case 0xD1:
			{
				I8086_GetEA(cpu); // calculate effective address
				cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (destination)

				op = (cpu->modrm >> 3) & 7; // required operation
				switch (op)
				{
				case 0: I8086_ROL16(cpu); break;
				case 1: I8086_ROR16(cpu); break;
				case 2: I8086_RCL16(cpu); break;
				case 3: I8086_RCR16(cpu); break;
				case 4: I8086_SHL16(cpu); break; // SAL16
				case 5: I8086_SHR16(cpu); break;
				// mod 6: SETMO (Set Minus One - returns 0xffff ... undocumented (flags: CF 0, PF 1, AF 0, ZF 0, SF 1, OF 0)
				case 6: I8086_SETMO16(cpu); break;
				default: /* case 7 */ I8086_SAR16(cpu); break;
				}

				I8086_SetRM16(cpu);	// save result
			}
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*15);
			break;

		// op mem/reg,CL (byte)
		case 0xD2:
			{
				I8086_GetEA(cpu); // calculate effective address

				u32 cc = (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*8) : (I8086_CLOCKMUL*20);
#if I8086_CPU_80186	// 1=use 80186 or later
				u8 c = cpu->cl & 0x1f;
#else
				u8 c = cpu->cl;
#endif
				cc += c * 4;
				cpu->sync.clock += cc;

				cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory (destination)

				if (c != 0)
				{
				// Overflow flag OF is undefined for number of shifts greater than 1.
				// AMD keeps OF according to the last shift, but Intel does not.
					op = (cpu->modrm >> 3) & 7; // required operation
					switch (op)
					{
					case 0: do { I8086_ROL8(cpu); c--; } while (c != 0); break;
					case 1: do { I8086_ROR8(cpu); c--; } while (c != 0); break;
					case 2: do { I8086_RCL8(cpu); c--; } while (c != 0); break;
					case 3: do { I8086_RCR8(cpu); c--; } while (c != 0); break;
					case 4: do { I8086_SHL8(cpu); c--; } while (c != 0); break; // SAL8
					case 5: do { I8086_SHR8(cpu); c--; } while (c != 0); break;
					// mod 6: SETMOC (Set Minus One with C - returns 0xff if CL!=0, else unchanged... undocumented (flags: CF 0, PF 1, AF 0, ZF 0, SF 1, OF 0)
					case 6: I8086_SETMO8(cpu); break;
					default: /* case 7 */ do { I8086_SAR8(cpu); c--; } while (c != 0); break;
					}
				}

				I8086_SetRM8(cpu);	// save result
			}
			break;

		// op mem/reg,CL (word)
		case 0xD3:
			{
				I8086_GetEA(cpu); // calculate effective address

				u32 cc = (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*8) : (I8086_CLOCKMUL*20);
#if I8086_CPU_80186	// 1=use 80186 or later
				u8 c = cpu->cl & 0x1f;
#else
				u8 c = cpu->cl;
#endif
				cc += c * 4;
				cpu->sync.clock += cc;

				cpu->dst16 = I8086_GetRM16(cpu); // get operation register/memory (destination)

				if (c != 0)
				{
				// Overflow flag OF is undefined for number of shifts greater than 1.
				// AMD keeps OF according to the last shift, but Intel does not.
					op = (cpu->modrm >> 3) & 7; // required operation
					switch (op)
					{
					case 0: do { I8086_ROL16(cpu); c--; } while (c != 0); break;
					case 1: do { I8086_ROR16(cpu); c--; } while (c != 0); break;
					case 2: do { I8086_RCL16(cpu); c--; } while (c != 0); break;
					case 3: do { I8086_RCR16(cpu); c--; } while (c != 0); break;
					case 4: do { I8086_SHL16(cpu); c--; } while (c != 0); break; // SAL16
					case 5: do { I8086_SHR16(cpu); c--; } while (c != 0); break;
					// mod 6: SETMOC (Set Minus One with C - returns 0xffff if CL!=0, else unchanged... undocumented (flags: CF 0, PF 1, AF 0, ZF 0, SF 1, OF 0)
					case 6: I8086_SETMO16(cpu); break;
					default: /* case 7 */ do { I8086_SAR16(cpu); c--; } while (c != 0); break;
					}
				}

				I8086_SetRM16(cpu);	// save result
			}
			break;

		// AAM
		case 0xD4:
			// ... undocumented - other values than 0x0A will use other base than 10
			{ // ... code verified on contemporary PC hardware
				u16 f = cpu->flags & ~I8086_FLAGALL; // CF, OF and AF stays 0
				u8 al = cpu->al;
				u8 ah = cpu->ah;

				u8 base = I8086_ProgByte(cpu); // load base (default 0x0A = 10)

				ah = al / base;			// AL / 10
				al -= ah * base;		// remainder
				f |= I8086_FlagParTab[al];	// add parity, sign and zero flag

				cpu->al = al;
				cpu->ah = ah;
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*83;
			break;

		// AAD
		case 0xD5:
			// ... undocumented - other values than 0x0A will use other base than 10
			{ // ... code verified on contemporary PC hardware
				u16 f = cpu->flags & ~I8086_FLAGALL;
				u8 al = cpu->al;
				u8 ah = cpu->ah;

				u8 base = I8086_ProgByte(cpu); // load base (default 0x0A = 10)

				ah *= base;			// AH * 10
				u16 res = al + ah;		// AL + AH*10
				u16 xor = al ^ ah ^ res;	// XOR
				f |= res >> 8;			// I8086_CF carry flag
				f |= xor & I8086_AF;		// half-carry flag
				f |= ((xor ^ (xor >> 1)) << 4) & I8086_OF; // overflow flag
				al = (u8)res;
				f |= I8086_FlagParTab[al];	// add parity, sign and zero flag
				ah = 0;

				cpu->al = al;
				cpu->ah = ah;
				cpu->flags = f;
			}
			cpu->sync.clock += I8086_CLOCKMUL*60;
			break;

		// SALC ... undocumented
		case 0xD6:
			cpu->al = ((cpu->flags & I8086_CF) != 0) ? 0xff : 0;
			cpu->sync.clock += I8086_CLOCKMUL*4;
			break;

		// XLAT
		case 0xD7:
			cpu->al = I8086_GetByte(cpu, I8086_DS(cpu), cpu->bx + cpu->al);
			cpu->sync.clock += I8086_CLOCKMUL*11;
			break;

		// ESC mem/reg
		case 0xD8:
		case 0xD9:
		case 0xDA:
		case 0xDB:
		case 0xDC:
		case 0xDD:
		case 0xDE:
		case 0xDF:
			I8086_GetEA(cpu); // calculate effective address
			cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*2) : (I8086_CLOCKMUL*8);
			break;

		// LOOPNE/LOOPNZ rel
		case 0xE0:
			{
				s8 rel = (s8)I8086_ProgByte(cpu); // load jump offset
				u16 cx = cpu->cx - 1; // get loop counter and decrement
				cpu->cx = cx; // save new loop counter
				if ((cx != 0) && ((cpu->flags & I8086_ZF) == 0))
				{
					cpu->ip = cpu->ip + (s8)rel;
					cpu->sync.clock += I8086_CLOCKMUL*(19-5);
				}
				cpu->sync.clock += I8086_CLOCKMUL*5;
			}
			break;

		// LOOPE/LOOPZ rel
		case 0xE1:
			{
				s8 rel = (s8)I8086_ProgByte(cpu); // load jump offset
				u16 cx = cpu->cx - 1; // get loop counter and decrement
				cpu->cx = cx; // save new loop counter
				if ((cx != 0) && ((cpu->flags & I8086_ZF) != 0))
				{
					cpu->ip = cpu->ip + (s8)rel;
					cpu->sync.clock += I8086_CLOCKMUL*(18-6);
				}
				cpu->sync.clock += I8086_CLOCKMUL*6;
			}
			break;

		// LOOP rel
		case 0xE2:
			{
				s8 rel = (s8)I8086_ProgByte(cpu); // load jump offset
				u16 cx = cpu->cx - 1; // get loop counter and decrement
				cpu->cx = cx; // save new loop counter
				if (cx != 0)
				{
					cpu->ip = cpu->ip + (s8)rel;
					cpu->sync.clock += I8086_CLOCKMUL*(17-5);
				}
				cpu->sync.clock += I8086_CLOCKMUL*5;
			}
			break;

		// JCXZ rel
		case 0xE3:
			{
				s8 rel = (s8)I8086_ProgByte(cpu); // load jump offset
				if (cpu->cx == 0) // if CX is zero, then jump
				{
					cpu->ip = cpu->ip + (s8)rel;
					cpu->sync.clock += I8086_CLOCKMUL*(18-6);
				}
				cpu->sync.clock += I8086_CLOCKMUL*6;
			}
			break;

		// IN AL,kk
		case 0xE4:
			cpu->al = cpu->readport8(I8086_ProgByte(cpu));
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// IN AX,kk
		case 0xE5:
			cpu->ax = cpu->readport16(I8086_ProgByte(cpu));
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// OUT kk,AL
		case 0xE6:
			cpu->writeport8(I8086_ProgByte(cpu), cpu->al);
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// OUT kk,AX
		case 0xE7:
			cpu->writeport16(I8086_ProgByte(cpu), cpu->ax);
			cpu->sync.clock += I8086_CLOCKMUL*10;
			break;

		// CALL near disp16 (direct within segment, IP relative)
		case 0xE8:
			{
				s16 off = (s16)I8086_ProgWord(cpu); // load offset
				I8086_PUSH(cpu, cpu->ip); // push current instruction pointer
				cpu->ip = cpu->ip + (s16)off; // set new instruction pointer
			}
			cpu->sync.clock += I8086_CLOCKMUL*19;
			break;

		// JMP near disp16 (direct within segment, IP relative)
		case 0xE9:
			{
				s16 off = (s16)I8086_ProgWord(cpu); // load offset
				cpu->ip = cpu->ip + (s16)off; // set new instruction pointer
			}
			cpu->sync.clock += I8086_CLOCKMUL*15;
			break;

		// JMP far addr (direct intersegment)
		case 0xEA:
			{
				u16 off = I8086_ProgWord(cpu); // load offset
				u16 seg = I8086_ProgWord(cpu); // load segment
				cpu->ip = off; // set new instruction pointer
				cpu->cs = seg; // set new program segment
			}
			cpu->sync.clock += I8086_CLOCKMUL*15;
			break;

		// JMP short rel (direct within segment - short)
		case 0xEB:
			{
				s8 rel = (s8)I8086_ProgByte(cpu); // load jump offset
				cpu->ip = cpu->ip + (s8)rel;
			}
			cpu->sync.clock += I8086_CLOCKMUL*15;
			break;

		// IN AL,DX
		case 0xEC:
			cpu->al = cpu->readport8(cpu->dx);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// IN AX,DX
		case 0xED:
			cpu->ax = cpu->readport16(cpu->dx);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// OUT DX,AL
		case 0xEE:
			cpu->writeport8(cpu->dx, cpu->al);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// OUT DX,AX
		case 0xEF:
			cpu->writeport16(cpu->dx, cpu->ax);
			cpu->sync.clock += I8086_CLOCKMUL*8;
			break;

		// LOCK
		case 0xF0:
		case 0xF1: // LOCK ... undocumented alias
			cpu->tid = 1; // temporary interrupt disable
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// REPNE or REPNZ
		case 0xF2:
			cpu->tid = 1; // temporary interrupt disable
			cpu->segpref_next = cpu->segpref; // hold next SEG prefix
			cpu->reppref_next = I8086_REP_NZ;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// REP or REPE or REPZ
		case 0xF3:
			cpu->tid = 1; // temporary interrupt disable
			cpu->segpref_next = cpu->segpref; // hold next SEG prefix
			cpu->reppref_next = I8086_REP_Z;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// HLT
		case 0xF4:
			cpu->ip--;
			cpu->halted = 1;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// CMC
		case 0xF5:
			cpu->flags ^= I8086_CF;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// op mem/reg (byte)
		case 0xF6:
			{
				u16 ip0 = cpu->ip; // save fot case of divide zero interrupt

				I8086_GetEA(cpu); // calculate effective address
				u8 n = I8086_GetRM8(cpu); // get operation register/memory

				op = (cpu->modrm >> 3) & 7; // required operation (shift>>3 to better optimise jump table)
				switch (op)
				{
				// TEST mem/reg,kk
				case 0:
				case 1: // ... undocumented alias
					{
						cpu->dst8 = n; // 1st operand
						cpu->src8 = I8086_ProgByte(cpu); // load immediate 1st operand
						I8086_AND8(cpu); // AND to save flags, but not save result
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*5) : (I8086_CLOCKMUL*11);
					}
					break;

				// NOT mem/reg
				case 2:
					{
						cpu->dst8 = ~n;		// invert operand
						I8086_SetRM8(cpu);	// save result
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
					}
					break;

				// NEG mem/reg
				case 3:
					{
						cpu->dst8 = 0;		// 1st operand is 0
						cpu->src8 = n;		// 2nd operand
						I8086_SUB8(cpu);	// subtract 0 - n
						I8086_SetRM8(cpu);	// save result
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
					}
					break;

				// MUL mem/reg
				case 4:
					{
						u16 res = (u16)n * (u16)cpu->al; // AX = AL * operand, unsigned
						if (cpu->reppref != I8086_REP_NO) res = - res; // undocumented: REP prefix negate result of MUL, IMUL and IDIV (not DIV)
						cpu->ax = res; // save result

#if I8086_CPU_INTEL
						u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
						f |= I8086_FlagParTab[(u8)res] & ~I8086_ZF; // add parity and sign, zero stays cleared
#else
						u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear flags, zero stays unchanged
#endif

						u8 ah = (u8)(res >> 8); // HIGH part of the result
						if (ah != 0) f |= I8086_CF | I8086_OF; // add carry and overflow
						cpu->flags = f;			// save new flags
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*74) : (I8086_CLOCKMUL*80);
					}
					break;

				// IMUL mem/reg
				case 5:
					{
						s16 al = (s16)(s8)cpu->al;
						s16 res = (s16)(s8)n * al; // AX = AL * operand, signed
						if (cpu->reppref != I8086_REP_NO) res = - res; // undocumented: REP prefix negate result of MUL, IMUL and IDIV (not DIV)
						cpu->ax = res; // save result

#if I8086_CPU_INTEL
						u16 f = cpu->flags & ~I8086_FLAGALL; // get flags
						f |= I8086_FlagParTab[(u8)res] & (I8086_PF | I8086_SF); // add parity flag and sign, zero stays cleared
#else
						u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear flags, zero stays unchanged
#endif

						if ((res < -128) || (res > 127)) f |= I8086_CF | I8086_OF; // add carry and overflow if sign extension of AL has been changed
						cpu->flags = f;			// save new flags
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*89) : (I8086_CLOCKMUL*95);
					}
					break;

				// DIV mem/reg
				case 6:
					{
						u16 ax = cpu->ax; // 1st operand
						if ((u8)(ax >> 8) >= n) // check overflow
						{
#if I8086_CPU_80186	// 1=use 80186 or later
							cpu->ip = ip0 - 1;
#endif
							I8086_INT(cpu, 0); // interrupt divide by zero ((ax >> 8) is >= n)
						}
						else
						{
							// divide
							u8 res = (u8)(ax / n); // get quotient (1st operand = numerator, 2nd operand = denominator)
							cpu->al = res; // quotient
							cpu->ah = (u8)(ax - res*n); // get remainder

#if I8086_CPU_AMD
							u16 f = cpu->flags;
							f |= I8086_AF;			// half-carry flag is always set
							f &= ~(I8086_PF | I8086_ZF | I8086_SF);
							cpu->flags = f;
#endif

#if I8086_CPU_INTEL && !I8086_CPU_80186
							cpu->flags &= ~(I8086_CF | I8086_OF); // clear flags (only 8088/8086)
#endif

						}

						// clocks
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*85) : (I8086_CLOCKMUL*90);
					}
					break;

				// IDIV mem/reg
				default: // case 7:
					if (n == 0) // check zero division
					{
#if I8086_CPU_80186	// 1=use 80186 or later
						cpu->ip = ip0 - 1;
#endif
						I8086_INT(cpu, 0); // interrupt divide by zero ((ax >> 8) is >= n)
					}
					else
					{
						s16 ax = cpu->ax; // 1st operand
						s8 nn = n;

						// divide
						s16 res = ax / nn; // get quotient (1st operand = numerator, 2nd operand = denominator)
						s16 rem = ax - res*nn; // get remainder

						// set sign
						if (cpu->reppref != I8086_REP_NO) // undocumented: REP prefix negate result of MUL, IMUL and IDIV (not DIV)
						{
							res = -res;
							rem = -rem;
						}

						// check overflow
						if ((res < -128) || (res > 127))
						{
#if I8086_CPU_80186	// 1=use 80186 or later
							cpu->ip = ip0 - 1;
#endif
							I8086_INT(cpu, 0); // interrupt divide by zero ((ax >> 8) is >= n)
						}
						else
						{
							cpu->al = (u8)res; // quotient
							cpu->ah = (u8)rem; // remainder
						}

#if I8086_CPU_AMD
						u16 f = cpu->flags;
						f |= I8086_AF;			// half-carry flag is always set
						f &= ~(I8086_PF | I8086_ZF | I8086_SF);
						cpu->flags = f;
#endif

#if I8086_CPU_INTEL && !I8086_CPU_80186
						cpu->flags &= ~(I8086_CF | I8086_OF); // clear flags (only 8088/8086)
#endif
						// clocks
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*107) : (I8086_CLOCKMUL*113);
					}
					break;
				}
			}
			break;

		// op mem/reg (word)
		case 0xF7:
			{
				u16 ip0 = cpu->ip; // save fot case of divide zero interrupt

				I8086_GetEA(cpu); // calculate effective address
				u16 n = I8086_GetRM16(cpu); // get operation register/memory

				op = (cpu->modrm >> 3) & 7; // required operation (shift>>3 to better optimise jump table)
				switch (op)
				{
				// TEST mem/reg,kk
				case 0:
				case 1: // ... undocumented alias
					{
						cpu->dst16 = n; // 1st operand
						cpu->src16 = I8086_ProgWord(cpu); // load immediate 1st operand
						I8086_AND16(cpu); // AND to save flags, but not save result
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*5) : (I8086_CLOCKMUL*11);
					}
					break;

				// NOT mem/reg
				case 2:
					{
						cpu->dst16 = ~n;	// invert operand
						I8086_SetRM16(cpu);	// save result
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
					}
					break;

				// NEG mem/reg
				case 3:
					{
						cpu->dst16 = 0;		// 1st operand is 0
						cpu->src16 = n;		// 2nd operand
						I8086_SUB16(cpu);	// subtract 0 - n
						I8086_SetRM16(cpu);	// save result
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*16);
					}
					break;

				// MUL mem/reg
				case 4:
					{
						u32 res = (u32)n * (u32)cpu->ax; // DX:AX = AX * operand, unsigned
						if (cpu->reppref != I8086_REP_NO) res = - res; // undocumented: REP prefix negate result of MUL, IMUL and IDIV (not DIV)
						u16 ax = (u16)res; // result LOW
						u16 dx = (u16)(res >> 16); // result HIGHT
						cpu->ax = ax; // save result LOW
						cpu->dx = dx; // save result HIGH

#if I8086_CPU_INTEL
						u16 f = cpu->flags & ~I8086_FLAGALL; // clear flags
						f |= I8086_FlagParTab[(u8)ax] & ~(I8086_ZF | I8086_SF); // add parity
						if ((s16)ax < 0) f |= I8086_SF;
#else
						u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear flags, zero stays unchanged
#endif

						if (dx != 0) f |= I8086_CF | I8086_OF; // add carry and overflow
						cpu->flags = f;			// save new flags
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*126) : (I8086_CLOCKMUL*132);
					}
					break;

				// IMUL mem/reg
				case 5:
					{
						s32 ax0 = (s32)(s16)cpu->ax;
						s32 res = (s32)(s16)n * ax0; // DX:AX = AX * operand, signed
						if (cpu->reppref != I8086_REP_NO) res = - res; // undocumented: REP prefix negate result of MUL, IMUL and IDIV (not DIV)
						u16 ax = (u16)res; // result LOW
						u16 dx = (u16)(res >> 16); // result HIGHT
						cpu->ax = ax; // save result LOW
						cpu->dx = dx; // save result HIGH

#if I8086_CPU_INTEL
						u16 f = cpu->flags & ~I8086_FLAGALL; // get flags
						f |= I8086_FlagParTab[(u8)ax] & ~(I8086_ZF | I8086_SF); // add parity
						if ((s16)ax < 0) f |= I8086_SF;
#else
						u16 f = cpu->flags & ~(I8086_CF | I8086_OF); // clear flags, zero stays unchanged
#endif

						if ((res < -32768) || (res > 32767)) f |= I8086_CF | I8086_OF; // add carry and overflow if sign extension of AX has been changed
						cpu->flags = f;			// save new flags
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*141) : (I8086_CLOCKMUL*147);
					}
					break;

				// DIV mem/reg
				case 6:
					{
						u16 dx = cpu->dx;
						if (dx >= n) // check overflow
						{
#if I8086_CPU_80186	// 1=use 80186 or later
							cpu->ip = ip0 - 1;
#endif
							I8086_INT(cpu, 0); // interrupt divide by zero
						}
						else
						{
							u32 dxax = ((u32)dx << 16) | cpu->ax; // 1st operand

							// divide
							u16 res = (u16)(dxax / n); // get quotient (1st operand = numerator, 2nd operand = denominator)
							cpu->ax = res; // quotient
							cpu->dx = (u16)(dxax - res*n); // get remainder

#if I8086_CPU_AMD
							u16 f = cpu->flags;
							f |= I8086_AF;			// half-carry flag is always set
							f &= ~(I8086_PF | I8086_ZF | I8086_SF);
							cpu->flags = f;
#endif

#if I8086_CPU_INTEL
#if I8086_CPU_80186
							u16 f = cpu->flags;
							f &= ~(I8086_CF | I8086_OF | I8086_AF | I8086_SF); // clear flags
							f |= (I8086_PF | I8086_ZF);
							cpu->flags = f;
#else
							cpu->flags &= ~(I8086_CF | I8086_OF); // clear flags
#endif
#endif
						}

						// clocks
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*153) : (I8086_CLOCKMUL*159);
					}
					break;

				// IDIV mem/reg
				default: // case 7:
					if (n == 0) // check zero division
					{
#if I8086_CPU_80186	// 1=use 80186 or later
						cpu->ip = ip0 - 1;
#endif
						I8086_INT(cpu, 0); // interrupt divide by zero ((ax >> 8) is >= n)
					}
					else
					{
						s32 dxax = (s32)(((u32)cpu->dx << 16) | cpu->ax); // 1st operand
						s16 nn = n;

						// divide
						s32 res = dxax / nn; // get quotient (1st operand = numerator, 2nd operand = denominator)
						s32 rem = dxax - res*nn; // get remainder

						// set sign
						if (cpu->reppref != I8086_REP_NO) // undocumented: REP prefix negate result of MUL, IMUL and IDIV (not DIV)
						{
							res = -res;
							rem = -rem;
						}

						// check overflow
						if ((res < -32768) || (res > 32767))
						{
#if I8086_CPU_80186	// 1=use 80186 or later
							cpu->ip = ip0 - 1;
#endif
							I8086_INT(cpu, 0); // interrupt divide by zero ((ax >> 8) is >= n)
						}
						else
						{
							cpu->ax = (u16)res; // quotient
							cpu->dx = (u16)rem; // remainder
						}

#if I8086_CPU_AMD // carry is undefined
						u16 f = cpu->flags;
						f |= I8086_AF;			// half-carry flag is always set
						f &= ~(I8086_PF | I8086_ZF | I8086_SF | I8086_OF);
						cpu->flags = f;
#endif

#if I8086_CPU_INTEL && !I8086_CPU_80186
						cpu->flags &= ~(I8086_CF | I8086_OF); // clear flags (only 8088/8086)
#endif

						// clocks
						cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*107) : (I8086_CLOCKMUL*113);
					}
					break;
				}
			}
			break;

		// CLC
		case 0xF8:
			cpu->flags &= ~I8086_CF;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// STC
		case 0xF9:
			cpu->flags |= I8086_CF;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// CLI
		case 0xFA:
			cpu->flags &= ~I8086_IF;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// STI
		case 0xFB:
			cpu->flags |= I8086_IF;
			cpu->tid = 1; // temporary interrupt disable
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// CLD
		case 0xFC:
			cpu->flags &= ~I8086_DF;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// STD
		case 0xFD:
			cpu->flags |= I8086_DF;
			cpu->sync.clock += I8086_CLOCKMUL*2;
			break;

		// op mem/reg (byte)
		case 0xFE:
			{
				I8086_GetEA(cpu); // calculate effective address
				cpu->dst8 = I8086_GetRM8(cpu); // get operation register/memory

				op = (cpu->modrm >> 3) & 7; // required operation (shift>>3 to better optimise jump table)
				switch (op)
				{
				// INC mem/reg
				case 0:
					I8086_INC8(cpu);	// INC
					break;

				// DEC mem/reg
				case 1:
					I8086_DEC8(cpu);	// DEC
					break;

				default:
					// mod 2: CALL mem/reg ... undocumented, destination address is corrupted (reads only 1 byte)
					// mod 3: CALL mem ... undocumented, destination address is corrupted (reads only 1 byte)
					// mod 4: JMP mem/reg ... undocumented, destination address is corrupted (reads only 1 byte)
					// mod 5: JMP mem ... undocumented, destination address is corrupted (reads only 1 byte)
					// mod 6: PUSH mem ... undocumented, one byte is corrupted (reads only 1 byte)
					// mod 7: PUSH mem ... undocumented (as mod 6), onebyte is corrupted (reads only 1 byte)
					break;
				}

				// save result
				I8086_SetRM8(cpu);	// save result

				// setup clock
				cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*15);
			}
			break;

		// op mem/reg (word)
		case 0xFF:
			{
				I8086_GetEA(cpu); // calculate effective address
				u16 n = I8086_GetRM16(cpu); // get operation register/memory

				op = (cpu->modrm >> 3) & 7; // required operation (shift>>3 to better optimise jump table)
				switch (op)
				{
				// INC mem/reg
				case 0:
					cpu->dst16 = n;
					I8086_INC16(cpu);	// INC
					I8086_SetRM16(cpu);	// save result
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*15);
					break;

				// DEC mem/reg
				case 1:
					cpu->dst16 = n;
					I8086_DEC16(cpu);	// DEC
					I8086_SetRM16(cpu);	// save result
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*3) : (I8086_CLOCKMUL*15);
					break;

				// CALL near mem/reg
				case 2:
					I8086_PUSH(cpu, cpu->ip); // push current instruction pointer
					cpu->ip = n; // set new instruction pointer
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*16) : (I8086_CLOCKMUL*21);
					break;

				// CALL far mem
				case 3:
					{
						u16 cs = cpu->cs;
						if (cpu->modrm < 0xc0) // ... undocumented: if source operand is register, use last value
							cpu->cs = I8086_GetWord(cpu, cpu->eseg, cpu->eoff+2);
						else
							cpu->cs = cpu->dst16; // use old value
						I8086_PUSH(cpu, cs); // push current program segment
						I8086_PUSH(cpu, cpu->ip); // push current instruction pointer
						cpu->ip = n; // new program counter
					}
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*26) : (I8086_CLOCKMUL*37);
					break;

				// JMP near mem/reg
				case 4:
					cpu->ip = n; // set new instruction pointer
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*11) : (I8086_CLOCKMUL*18);
					break;

				// JMP far mem
				case 5:
					{
						if (cpu->modrm < 0xc0) // ... undocumented: if source operand is register, use last value
							cpu->cs = I8086_GetWord(cpu, cpu->eseg, cpu->eoff+2);
						else
							cpu->cs = cpu->dst16; // use old value
						cpu->ip = n; // new program counter
					}
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*12) : (I8086_CLOCKMUL*24);
					break;

				// PUSH mem
				default: // case 6: case 7: ... mod 7 is undocumented, as mod 6
					I8086_PUSH(cpu, n);
					cpu->sync.clock += (cpu->modrm >= 0xc0) ? (I8086_CLOCKMUL*11) : (I8086_CLOCKMUL*16);
					break;
				}
			}
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute I8086 emulator on CPU 1
static void I8086_ExecCore1()
{
	// start time synchronization
	I8086_SyncStart((sI8086*)I8086_Cpu);

	// execute
	I8086_Exec((sI8086*)I8086_Cpu);

	// clear descriptor
	I8086_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*I8086_CLOCKMUL is recommended to maintain.
u32 I8086_Start(sI8086* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8086_Cpu != NULL) I8086_Stop(pwm);

	// initialize time synchronization
	freq = I8086_SyncInit(cpu, pwm, freq);

	// reset processor
	I8086_Reset(cpu);

	// current CPU descriptor
	I8086_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I8086_ExecCore1);
	else
		Core1Exec(I8086_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I8086_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I8086_ExecCore1);
#else
	Core1Exec(I8086_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 I8086_Cont(sI8086* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (I8086_Cpu != NULL) I8086_Stop(pwm);

	// initialize time synchronization
	freq = I8086_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	I8086_Cpu = cpu;
	dsb();

	// execute program
#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (DispHstxRunning)
		DispHstxCore1Exec(I8086_ExecCore1);
	else
		Core1Exec(I8086_ExecCore1);
#elif USE_DISPHSTX			// 1=use HSTX Display driver
	DispHstxCore1Exec(I8086_ExecCore1);
#elif USE_MINIVGA			// use mini-VGA display
	VgaCore1Exec(I8086_ExecCore1);
#else
	Core1Exec(I8086_ExecCore1);
#endif

	return freq;
}

// stop emulation
//  pwm ... index of used PWM slice (0..7)
void I8086_Stop(int pwm)
{
	// get CPU descriptor
	sI8086* cpu = (sI8086*)I8086_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (I8086_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	I8086_SyncTerm(pwm);

#if !USE_MINIVGA && !USE_DISPHSTX	// use mini-VGA display

#if USE_DISPHSTXMINI	// 1=use HSTX Display Mini driver
	if (!DispHstxRunning)
#endif

	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_I8086
