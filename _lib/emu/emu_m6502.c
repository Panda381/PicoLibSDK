
// ****************************************************************************
//
//                           M6502/M65C02 CPU Emulator
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

#if USE_EMU_M6502			// use M6502 CPU emulator

// current CPU descriptor (NULL = not running)
volatile sM6502* M6502_Cpu = NULL;

// reset processor
void M6502_Reset(sM6502* cpu)
{
	cpu->pc = 0;		// program counter
	cpu->sp = 0xff;		// stack pointer
	cpu->a = 0;		// accumulator
	cpu->f = M6502_FLAGDEF;	// flags (processor status register)
	cpu->x = 0x80;		// index register X
	cpu->y = 0;		// index register Y
	cpu->intreq = 0;	// 1=interrupt request
	cpu->nmireq = 0;	// 1=NMI request
	cpu->resreq = 1;	// 1=RESET request
}

#include "emu_m6502_macro.h"

// execute program (start or continue, until "stop" request)
// Size of code of this function: 7568 code + 1024 jump table = 8592 bytes
// CPU loading at 1.77 MHz on 177 MHz: used 42-49%, max. 42-62%
void FASTCODE NOFLASH(M6502_Exec)(sM6502* cpu)
{
	u8 op;

	// clear stop flag
	cpu->stop = 0;

	// program loop
	while (cpu->stop == 0)
	{
		// RESET
		if (cpu->resreq != 0)
		{
			// clear reset request
			cpu->resreq = 0;

			// call interrupt
			M6502_IRQ(cpu, M6502_VECT_RES);
		}

		// NMI
		if (cpu->nmireq != 0)
		{
			// clear NMI request
			cpu->nmireq = 0;

			// call interrupt
			M6502_IRQ(cpu, M6502_VECT_NMI);
		}

		// IRQ
		if ((cpu->intreq != 0) && ((cpu->f & M6502_I) == 0))
		{
			// clear NMI request
			cpu->intreq = 0;

			// call interrupt
			M6502_IRQ(cpu, M6502_VECT_IRQ);
		}

		// get next instruction
		op = M6502_ProgByte(cpu);

		// switch base operation code
		switch (op)
		{
		// BRK implied
		case 0x00:
			{
				cpu->pc++; // skip padding byte
				M6502_Push(cpu, cpu->pch); // push PC pointer HIGH
				M6502_Push(cpu, cpu->pcl); // push PC pointer LOW
				M6502_Push(cpu, cpu->f | (M6502_E | M6502_B)); // push flags (Break flag is 1)
				u8 pcl = cpu->readmem(M6502_VECT_IRQ); // load jump address LOW
				cpu->pch = cpu->readmem(M6502_VECT_IRQ+1); // load jump address HIGH
				cpu->pcl = pcl;
				cpu->f |= M6502_I; // disable interrupt
#if M6502_CPU_2A03 || M6502_CPU_65C02
				cpu->f &= ~M6502_D; // disable decimal
#endif
			}
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// ORA (indirect,X)
		case 0x01:
			M6502_ORA(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// JAM ... undocumented
		case 0x02:
		case 0x12:
		case 0x22:
		case 0x32:
		case 0x42:
		case 0x52:
		case 0x62:
		case 0x72:
		case 0x92:
		case 0xB2:
		case 0xD2:
		case 0xF2:
			cpu->pc--; // freeze CPU
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// SLO (indirect,X) ... undocumented
		case 0x03:
			M6502_SLO(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// NOP absolute ... undocumented
		case 0x0C:
		case 0x1C: // NOP absolute,X ... undocumented
		case 0x3C: // NOP absolute,X ... undocumented
		case 0x5C: // NOP absolute,X ... undocumented
		case 0x7C: // NOP absolute,X ... undocumented
		case 0xDC: // NOP absolute,X ... undocumented
		case 0xFC: // NOP absolute,X ... undocumented
			cpu->pc++; // skip 3rd byte of the instruction
		// NOP zeropage,X ... undocumented
		case 0x14:
		case 0x34: // NOP zeropage,X ... undocumented
		case 0x54: // NOP zeropage,X ... undocumented
		case 0x74: // NOP zeropage,X ... undocumented
		case 0xD4: // NOP zeropage,X ... undocumented
		case 0xF4: // NOP zeropage,X ... undocumented
			cpu->sync.clock += M6502_CLOCKMUL; // clock 4
		// NOP zeropage ... undocumented
		case 0x04:
		case 0x44: // NOP zeropage ... undocumented
		case 0x64: // NOP zeropage ... undocumented
			cpu->sync.clock += M6502_CLOCKMUL; // clock 3
		// NOP #immediate ... undocumented
		case 0x80:
		case 0x82: // NOP #immediate ... undocumented
		case 0x89: // NOP #immediate ... undocumented
		case 0xC2: // NOP #immediate ... undocumented
		case 0xE2: // NOP #immediate ... undocumented
			cpu->pc++; // skip 2nd byte of the instruction
		// NOP implied
		case 0xEA:
		case 0x1A: // NOP implied ... undocumented
		case 0x3A: // NOP implied ... undocumented
		case 0x5A: // NOP implied ... undocumented
		case 0x7A: // NOP implied ... undocumented
		case 0xDA: // NOP implied ... undocumented
		case 0xFA: // NOP implied ... undocumented
			cpu->sync.clock += M6502_CLOCKMUL*2; // clock 2
			break;

		// ORA zeropage
		case 0x05:
			M6502_ORA_Zp(cpu, M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// ASL zeropage
		case 0x06:
			M6502_ASL_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SLO zeropage ... undocumented
		case 0x07:
			M6502_SLO(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// PHP implied
		case 0x08:
			M6502_Push(cpu, cpu->f | (M6502_E | M6502_B));
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// ORA #immediate
		case 0x09:
			M6502_ORA(cpu, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ASL A
		case 0x0A:
			cpu->a = M6502_ASL(cpu, cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ANC #immediate ... undocumented
		case 0x0B:
		case 0x2B:
			{
				M6502_AND(cpu, cpu->pc);	// 16-bit address
				cpu->pc++;
				u8 f = cpu->f | M6502_C;
				if ((cpu->a & B7) == 0) f ^= M6502_C;
				cpu->f = f;
			}			
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ORA absolute
		case 0x0D:
			M6502_ORA(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ASL absolute
		case 0x0E:
			M6502_ASL_M(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SLO absolute ... undocumented
		case 0x0F:
			M6502_SLO(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// BPL relative
		case 0x10:
			M6502_BRA(cpu, (cpu->f & M6502_N) == 0);
			break;

		// ORA (indirect),Y
		case 0x11:
			M6502_ORA(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SLO (indirect),Y ... undocumented
		case 0x13:
			M6502_SLO(cpu, M6502_IndYW(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// ORA zeropage,X
		case 0x15:
			M6502_ORA_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ASL zeropage,X
		case 0x16:
			M6502_ASL_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SLO zeropage,X ... undocumented
		case 0x17:
			M6502_SLO(cpu, M6502_ZeropageX(cpu));	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// CLC implied
		case 0x18:
			cpu->f &= ~M6502_C;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ORA absolute,Y
		case 0x19:
			M6502_ORA(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// SLO absolute,Y ... undocumented
		case 0x1B:
			M6502_SLO(cpu, M6502_AbsoluteYW(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// ORA absolute,X
		case 0x1D:
			M6502_ORA(cpu, M6502_AbsoluteXR(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ASL absolute,X
		case 0x1E:
			M6502_ASL_M(cpu, M6502_AbsoluteXW(cpu)); // immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// SLO absolute,X ... undocumented
		case 0x1F:
			M6502_SLO(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// JSR absolute
		case 0x20:
			{
				// read destination address LOW
				u8 pcl = M6502_ProgByte(cpu);

				// push this PC pointer HIGH
				M6502_Push(cpu, cpu->pch);

				// push this PC pointer LOW
				M6502_Push(cpu, cpu->pcl);

				// read destination address HIGH
				cpu->pch = M6502_ProgByte(cpu);
				cpu->pcl = pcl;
			}
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// AND (indirect,X)
		case 0x21:
			M6502_AND(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// RLA (indirect,X) ... undocumented
		case 0x23:
			M6502_RLA(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// BIT zeropage
		case 0x24:
			M6502_BIT_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// AND zeropage
		case 0x25:
			M6502_AND_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// ROL zeropage
		case 0x26:
			M6502_ROL_Zp(cpu, M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// RLA zeropage ... undocumented
		case 0x27:
			M6502_RLA(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// PLP implied
		case 0x28:
			cpu->f = (M6502_Pop(cpu) & ~M6502_B) | M6502_E;
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// AND #immediate
		case 0x29:
			M6502_AND(cpu, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ROL A
		case 0x2A:
			cpu->a = M6502_ROL(cpu, cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// BIT absolute
		case 0x2C:
			M6502_BIT(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// AND absolute
		case 0x2D:
			M6502_AND(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ROL absolute
		case 0x2E:
			M6502_ROL_M(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// RLA absolute ... undocumented
		case 0x2F:
			M6502_RLA(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// BMI relative
		case 0x30:
			M6502_BRA(cpu, (cpu->f & M6502_N) != 0);
			break;

		// AND (indirect),Y
		case 0x31:
			M6502_AND(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// RLA (indirect),Y ... undocumented
		case 0x33:
			M6502_RLA(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// AND zeropage,X
		case 0x35:
			M6502_AND_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ROL zeropage,X
		case 0x36:
			M6502_ROL_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// RLA zeropage,X ... undocumented
		case 0x37:
			M6502_RLA(cpu, M6502_ZeropageX(cpu));	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SEC implied
		case 0x38:
			cpu->f |= M6502_C;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// AND absolute,Y
		case 0x39:
			M6502_AND(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// RLA absolute,Y ... undocumented
		case 0x3B:
			M6502_RLA(cpu, M6502_AbsoluteYW(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// AND absolute,X
		case 0x3D:
			M6502_AND(cpu, M6502_AbsoluteXR(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ROL absolute,X
		case 0x3E:
			M6502_ROL_M(cpu, M6502_AbsoluteXW(cpu)); // immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// RLA absolute,X ... undocumented
		case 0x3F:
			M6502_RLA(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// RTI implied
		case 0x40:
			{
				u8 f = (M6502_Pop(cpu) & ~M6502_B) | M6502_E;
				u8 pcl = M6502_Pop(cpu);
				u8 pch = M6502_Pop(cpu);
				cpu->f = f;
				u16 pc = pcl | ((u16)pch << 8);
			}
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// EOR (indirect,X)
		case 0x41:
			M6502_EOR(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SRE (indirect,X) ... undocumented
		case 0x43:
			M6502_SRE(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// EOR zeropage
		case 0x45:
			M6502_EOR_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// LSR zeropage
		case 0x46:
			M6502_LSR_Zp(cpu, M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SRE zeropage ... undocumented
		case 0x47:
			M6502_SRE(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// PHA implied
		case 0x48:
			M6502_Push(cpu, cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// EOR #immediate
		case 0x49:
			M6502_ORA(cpu, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LSR A
		case 0x4A:
			cpu->a = M6502_LSR(cpu, cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ALR #immediate ... ASR #immediate ... undocumented
		case 0x4B:
			cpu->a = M6502_LSR(cpu, M6502_ProgByte(cpu) & cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// JMP absolute
		case 0x4C:
			{
				u16 pc = M6502_ProgWord(cpu);
				cpu->pc = pc;
			}
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// EOR absolute
		case 0x4D:
			M6502_EOR(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LSR absolute
		case 0x4E:
			M6502_LSR_M(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SRE absolute ... undocumented
		case 0x4F:
			M6502_SRE(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// BVC relative
		case 0x50:
			M6502_BRA(cpu, (cpu->f & M6502_V) == 0);
			break;

		// EOR (indirect),Y
		case 0x51:
			M6502_EOR(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SRE (indirect),Y ... undocumented
		case 0x53:
			M6502_SRE(cpu, M6502_IndYW(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// EOR zeropage,X
		case 0x55:
			M6502_EOR_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LSR zeropage,X
		case 0x56:
			M6502_LSR_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SRE zeropage,X ... undocumented
		case 0x57:
			M6502_SRE(cpu, M6502_ZeropageX(cpu));	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// CLI implied
		case 0x58:
			cpu->f &= ~M6502_I;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// EOR absolute,Y
		case 0x59:
			M6502_EOR(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// SRE absolute,Y ... undocumented
		case 0x5B:
			M6502_SRE(cpu, M6502_AbsoluteYW(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// EOR absolute,X
		case 0x5D:
			M6502_EOR(cpu, M6502_AbsoluteXR(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LSR absolute,X
		case 0x5E:
			M6502_LSR_M(cpu, M6502_AbsoluteXW(cpu)); // immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// SRE absolute,X ... undocumented
		case 0x5F:
			M6502_SRE(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// RTS implied
		case 0x60:
			{
				u8 pcl = M6502_Pop(cpu);
				u8 pch = M6502_Pop(cpu);
				u16 pc = pcl | ((u16)pch << 8);
				cpu->pc = pc + 1;
			}
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// ADC (indirect,X)
		case 0x61:
			M6502_ADC(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// RRA (indirect,X) ... undocumented
		case 0x63:
			M6502_RRA(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// ADC zeropage
		case 0x65:
			M6502_ADC_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// ROR zeropage
		case 0x66:
			M6502_ROR_Zp(cpu, M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// RRA zeropage ... undocumented
		case 0x67:
			M6502_RRA(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// PLA implied
		case 0x68:
			{
				u8 n = M6502_Pop(cpu);
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ADC #immediate
		case 0x69:
			M6502_ADC(cpu, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ROR A
		case 0x6A:
			cpu->a = M6502_ROR(cpu, cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ARR #immediate ... undocumented
		case 0x6B:
			{
				u8 n = M6502_ProgByte(cpu); // load immediate
				n &= cpu->a; // A & #immediate
				n >>= 1; // shift result right
				u8 f = cpu->f;
				n |= f << 7; // add carry flag to bit 7
				cpu->a = n; // store result

				f &= ~M6502_FLAGALL; // clear flags
				if (n == 0) f |= M6502_Z; // zero flag
				if ((s8)n < 0) f |= M6502_N; // negative flag
				f |= (n ^ (n << 1)) & M6502_V; // V flag
				f |= (n >> 6) & M6502_C; // carry exchanged with original sign
				cpu->f = f; // save flags
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// JMP indirect
		case 0x6C:
			{
				u16 a = M6502_ProgWord(cpu); // get indirect address
				u8 pcl = cpu->readmem(a); // load address LOW
				a = (a & 0xff00) | ((u8)(a+1)); // next address in the same page
				cpu->pch = cpu->readmem(a); // load address HIGH
				cpu->pcl = pcl;
			}
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// ADC absolute
		case 0x6D:
			M6502_ADC(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ROR absolute
		case 0x6E:
			M6502_ROR_M(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// RRA absolute ... undocumented
		case 0x6F:
			M6502_RRA(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// BVS relative
		case 0x70:
			M6502_BRA(cpu, (cpu->f & M6502_V) != 0);
			break;

		// ADC (indirect),Y
		case 0x71:
			M6502_ADC(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// RRA (indirect),Y ... undocumented
		case 0x73:
			M6502_RRA(cpu, M6502_IndYW(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// ADC zeropage,X
		case 0x75:
			M6502_ADC_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ROR zeropage,X
		case 0x76:
			M6502_ROR_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// RRA zeropage,X ... undocumented
		case 0x77:
			M6502_RRA(cpu, M6502_ZeropageX(cpu));	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SEI implied
		case 0x78:
			cpu->f |= M6502_I;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ADC absolute,Y
		case 0x79:
			M6502_ADC(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// RRA absolute,Y ... undocumented
		case 0x7B:
			M6502_RRA(cpu, M6502_AbsoluteYW(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// ADC absolute,X
		case 0x7D:
			M6502_ADC(cpu, M6502_AbsoluteXR(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ROR absolute,X
		case 0x7E:
			M6502_ROR_M(cpu, M6502_AbsoluteXW(cpu)); // immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// RRA absolute,X ... undocumented
		case 0x7F:
			M6502_RRA(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// STA (indirect,X)
		case 0x81:
			cpu->writemem(M6502_IndX(cpu), cpu->a);	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SAX (indirect,X) ... undocumented
		case 0x83:
			cpu->writemem(M6502_IndX(cpu), cpu->a & cpu->x); // 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// STY zeropage
		case 0x84:
			cpu->writemem(M6502_ProgByte(cpu), cpu->y); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// STA zeropage
		case 0x85:
			cpu->writemem(M6502_ProgByte(cpu), cpu->a); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// STX zeropage
		case 0x86:
			cpu->writemem(M6502_ProgByte(cpu), cpu->x); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// SAX zeropage ... undocumented
		case 0x87:
			cpu->writemem(M6502_ProgByte(cpu), cpu->a & cpu->x);
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// DEY implied
		case 0x88:
			{
				u8 n = cpu->y - 1;
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// TXA implied
		case 0x8A:
			{
				u8 n = cpu->x;
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// ANE #immediate ... XAA #immediate ... undocumented
		case 0x8B:
			{
				u8 n = M6502_ProgByte(cpu);
				n = n & cpu->a & cpu->x;
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// STY absolute
		case 0x8C:
			cpu->writemem(M6502_ProgWord(cpu), cpu->y);
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// STA absolute
		case 0x8D:
			cpu->writemem(M6502_ProgWord(cpu), cpu->a);
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// STX absolute
		case 0x8E:
			cpu->writemem(M6502_ProgWord(cpu), cpu->x);
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// SAX absolute ... undocumented
		case 0x8F:
			cpu->writemem(M6502_ProgWord(cpu), cpu->a & cpu->x);
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// BCC relative
		case 0x90:
			M6502_BRA(cpu, (cpu->f & M6502_C) == 0);
			break;

		// STA (indirect),Y
		case 0x91:
			cpu->writemem(M6502_IndYW(cpu), cpu->a); // 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SHA (indirect),Y  ... undocumented
		case 0x93:
			{
				u8 n = M6502_ProgByte(cpu); // get short address from 2nd byte of the instruction
				u8 low = cpu->readmem(n); // load indirect address LOW from zero page
				u8 high = cpu->readmem((u8)(n+1)); // load indirect address HIGH from zero page
				n = cpu->a & cpu->x & (high + 1); // do operation
				u16 a = (u16)(low | ((u16)high << 8)) + cpu->y; // compose effective address
				cpu->writemem(a, n); // write result to memory
			}
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// STY zeropage,X
		case 0x94:
			cpu->writemem(M6502_ZeropageX(cpu), cpu->y);	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// STA zeropage,X
		case 0x95:
			cpu->writemem(M6502_ZeropageX(cpu), cpu->a);	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// STX zeropage,Y
		case 0x96:
			cpu->writemem(M6502_ZeropageY(cpu), cpu->x);	// 8-bit zero page address 8(imm + Y)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// SAX zeropage,Y ... undocumented
		case 0x97:
			cpu->writemem(M6502_ZeropageY(cpu), cpu->a & cpu->x); // 8-bit zero page address 8(imm + Y)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// TYA implied
		case 0x98:
			{
				u8 n = cpu->y;
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// STA absolute,Y
		case 0x99:
			cpu->writemem(M6502_AbsoluteYW(cpu), cpu->a); // immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// TXS implied
		case 0x9A:
			{
				u8 n = cpu->x;
				cpu->sp = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// TAS absolute,Y ... SHS absolute,Y ... undocumented
		case 0x9B:
			{
				u16 a = M6502_ProgWord(cpu); // get absolute address
				u8 n = cpu->a & cpu->x;
				cpu->sp = n;
				n = (u8)(n & ((a >> 8) + 1)); // do operation
				cpu->writemem(a + cpu->y, n); // write result to memory
			}
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SHY absolute,X ... undocumented
		case 0x9C:
			{
				u16 a = M6502_ProgWord(cpu); // get absolute address
				u8 n = (u8)(cpu->y & ((a >> 8) + 1)); // do operation
				cpu->writemem(a + cpu->x, n); // write result to memory
			}
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// STA absolute,X
		case 0x9D:
			cpu->writemem(M6502_AbsoluteXW(cpu), cpu->a); // immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SHX absolute,Y ... undocumented
		case 0x9E:
			{
				u16 a = M6502_ProgWord(cpu); // get absolute address
				u8 n = (u8)(cpu->x & ((a >> 8) + 1)); // do operation
				cpu->writemem(a + cpu->y, n); // write result to memory
			}
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// SHA absolute,Y ... undocumented
		case 0x9F:
			{
				u16 a = M6502_ProgWord(cpu); // get absolute address
				u8 n = (u8)(cpu->a & cpu->x & ((a >> 8) + 1)); // do operation
				cpu->writemem(a + cpu->y, n); // write result to memory
			}
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// LDY #immediate
		case 0xA0:
			{
				u8 n = M6502_ProgByte(cpu);
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LDA (indirect,X)
		case 0xA1:
			{
				u8 n = cpu->readmem(M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// LDX #immediate
		case 0xA2:
			{
				u8 n = M6502_ProgByte(cpu);
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LAX (indirect,X) ... undocumented
		case 0xA3:
			M6502_LAX(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// LDY zeropage
		case 0xA4:
			{
				u8 n = cpu->readmem(M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// LDA zeropage
		case 0xA5:
			{
				u8 n = cpu->readmem(M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// LDX zeropage
		case 0xA6:
			{
				u8 n = cpu->readmem(M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// LAX zeropage ... undocumented
		case 0xA7:
			M6502_LAX_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// TAY implied
		case 0xA8:
			{
				u8 n = cpu->a;
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LDA #immediate
		case 0xA9:
			{
				u8 n = M6502_ProgByte(cpu);
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// TAX implied
		case 0xAA:
			{
				u8 n = cpu->a;
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LXA #immediate ... undocumented
		case 0xAB:
			M6502_LAX(cpu, cpu->pc);		// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LDY absolute
		case 0xAC:
			{
				u8 n = cpu->readmem(M6502_ProgWord(cpu));
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDA absolute
		case 0xAD:
			{
				u8 n = cpu->readmem(M6502_ProgWord(cpu));
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDX absolute
		case 0xAE:
			{
				u8 n = cpu->readmem(M6502_ProgWord(cpu));
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LAX absolute ... undocumented
		case 0xAF:
			M6502_LAX(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// BCS relative
		case 0xB0:
			M6502_BRA(cpu, (cpu->f & M6502_C) != 0);
			break;

		// LDA (indirect),Y
		case 0xB1:
			{
				u8 n = cpu->readmem(M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// LAX (indirect),Y ... undocumented
		case 0xB3:
			M6502_LAX(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// LDY zeropage,X
		case 0xB4:
			{
				u8 n = cpu->readmem(M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDA zeropage,X
		case 0xB5:
			{
				u8 n = cpu->readmem(M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDX zeropage,Y
		case 0xB6:
			{
				u8 n = cpu->readmem(M6502_ZeropageY(cpu)); // 8-bit zero page address 8(imm + Y)
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LAX zeropage,Y ... undocumented
		case 0xB7:
			M6502_LAX_Zp(cpu, M6502_ZeropageY(cpu));	// 8-bit zero page address 8(imm + Y)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// CLV implied
		case 0xB8:
			cpu->f &= ~M6502_V;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LDA absolute,Y
		case 0xB9:
			{
				u8 n = cpu->readmem(M6502_AbsoluteYR(cpu)); // immediate 16-bit absolute address 16(imm) + Y
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// TSX implied
		case 0xBA:
			{
				u8 n = cpu->sp;
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// LAS absolute,Y ... undocumented
		case 0xBB:
			{
				u8 n = cpu->readmem(M6502_AbsoluteYR(cpu)); // immediate 16-bit absolute address 16(imm) + Y
				n &= cpu->sp;
				cpu->a = n;
				cpu->x = n;
				cpu->sp = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDY absolute,X
		case 0xBC:
			{
				u8 n = cpu->readmem(M6502_AbsoluteXR(cpu)); // immediate 16-bit absolute address 16(imm) + X
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDA absolute,X
		case 0xBD:
			{
				u8 n = cpu->readmem(M6502_AbsoluteXR(cpu)); // immediate 16-bit absolute address 16(imm) + X
				cpu->a = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LDX absolute,Y
		case 0xBE:
			{
				u8 n = cpu->readmem(M6502_AbsoluteYR(cpu)); // immediate 16-bit absolute address 16(imm) + Y
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// LAX absolute,Y  ... undocumented
		case 0xBF:
			M6502_LAX(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// CPY #immediate
		case 0xC0:
			M6502_CMP(cpu, cpu->y, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// CMP (indirect,X)
		case 0xC1:
			M6502_CMP(cpu, cpu->a, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// DCP (indirect,X) ... undocumented
		case 0xC3:
			M6502_DCP(cpu, M6502_IndX(cpu));		// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// CPY zeropage
		case 0xC4:
			M6502_CMP_Zp(cpu, cpu->y, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// CMP zeropage
		case 0xC5:
			M6502_CMP_Zp(cpu, cpu->a, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// DEC zeropage
		case 0xC6:
			M6502_DEC_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// DCP zeropage ... undocumented
		case 0xC7:
			M6502_DCP(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// INY implied
		case 0xC8:
			{
				u8 n = cpu->y + 1;
				cpu->y = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// CMP #immediate
		case 0xC9:
			M6502_CMP(cpu, cpu->a, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// DEX implied
		case 0xCA:
			{
				u8 n = cpu->x - 1;
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// SBX #immediate ... undocumented
		case 0xCB:
			{
				u8 n = M6502_ProgByte(cpu);
				u8 res = cpu->x & cpu->a;
				cpu->f |= M6502_C;
				if (res < n) cpu->f ^= M6502_C;
				res -= n;
				cpu->x = res;
				M6502_SetNZ(cpu, res);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// CPY absolute
		case 0xCC:
			M6502_CMP(cpu, cpu->y, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// CMP absolute
		case 0xCD:
			M6502_CMP(cpu, cpu->a, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// DEC absolute
		case 0xCE:
			M6502_DEC(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// DCP absolute ... undocumented
		case 0xCF:
			M6502_DCP(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// BNE relative
		case 0xD0:
			M6502_BRA(cpu, (cpu->f & M6502_Z) == 0);
			break;

		// CMP (indirect),Y
		case 0xD1:
			M6502_CMP(cpu, cpu->a, M6502_IndYR(cpu)); // 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// DCP (indirect),Y ... undocumented
		case 0xD3:
			M6502_DCP(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// CMP zeropage,X
		case 0xD5:
			M6502_CMP_Zp(cpu, cpu->a, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// DEC zeropage,X
		case 0xD6:
			M6502_DEC_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// DCP zeropage,X ... undocumented
		case 0xD7:
			M6502_DCP(cpu, M6502_ZeropageX(cpu));	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// CLD implied
		case 0xD8:
			cpu->f &= ~M6502_D;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// CMP absolute,Y
		case 0xD9:
			M6502_CMP(cpu, cpu->a, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// DCP absolute,Y ... undocumented
		case 0xDB:
			M6502_DCP(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// CMP absolute,X
		case 0xDD:
			M6502_CMP(cpu, cpu->a, M6502_AbsoluteXR(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// DEC absolute,X
		case 0xDE:
			M6502_DEC(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// DCP absolute,X  ... undocumented
		case 0xDF:
			M6502_DCP(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// CPX #immediate
		case 0xE0:
			M6502_CMP(cpu, cpu->x, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// SBC (indirect,X)
		case 0xE1:
			M6502_SBC(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// ISC (indirect,X) ... undocumented
		case 0xE3:
			M6502_ISC(cpu, M6502_IndX(cpu));	// 16-bit address from zero page 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// CPX zeropage
		case 0xE4:
			M6502_CMP_Zp(cpu, cpu->x, M6502_ProgByte(cpu)); // 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// SBC zeropage
		case 0xE5:
			M6502_SBC_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*3;
			break;

		// INC zeropage
		case 0xE6:
			M6502_INC_Zp(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// ISC zeropage ... undocumented
		case 0xE7:
			M6502_ISC(cpu, M6502_ProgByte(cpu));	// 8-bit zero page address 8(imm)
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// INX implied
		case 0xE8:
			{
				u8 n = cpu->x + 1;
				cpu->x = n;
				M6502_SetNZ(cpu, n);
			}
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// SBC #immediate
		case 0xE9:
		case 0xEB: // USBC #immediate ... undocumented
			M6502_SBC(cpu, cpu->pc);	// 16-bit address
			cpu->pc++;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// CPX absolute
		case 0xEC:
			M6502_CMP(cpu, cpu->x, M6502_ProgWord(cpu)); // 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// SBC absolute
		case 0xED:
			M6502_SBC(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// INC absolute
		case 0xEE:
			M6502_INC(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// ISC absolute ... undocumented
		case 0xEF:
			M6502_ISC(cpu, M6502_ProgWord(cpu));	// 16-bit address
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// BEQ relative
		case 0xF0:
			M6502_BRA(cpu, (cpu->f & M6502_Z) != 0);
			break;

		// SBC (indirect),Y
		case 0xF1:
			M6502_SBC(cpu, M6502_IndYR(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*5;
			break;

		// ISC (indirect),Y ... undocumented
		case 0xF3:
			M6502_ISC(cpu, M6502_IndYW(cpu));	// 16-bit address from zero page 8(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*8;
			break;

		// SBC zeropage,X
		case 0xF5:
			M6502_SBC_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// INC zeropage,X
		case 0xF6:
			M6502_INC_Zp(cpu, M6502_ZeropageX(cpu)); // 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// ISC zeropage,X  ... undocumented
		case 0xF7:
			M6502_ISC(cpu, M6502_ZeropageX(cpu));	// 8-bit zero page address 8(imm + X)
			cpu->sync.clock += M6502_CLOCKMUL*6;
			break;

		// SED implied
		case 0xF8:
			cpu->f |= M6502_D;
			cpu->sync.clock += M6502_CLOCKMUL*2;
			break;

		// SBC absolute,Y
		case 0xF9:
			M6502_SBC(cpu, M6502_AbsoluteYR(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// ISC absolute,Y ... undocumented
		case 0xFB:
			M6502_ISC(cpu, M6502_AbsoluteYW(cpu));	// immediate 16-bit absolute address 16(imm) + Y
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// SBC absolute,X
		case 0xFD:
			M6502_SBC(cpu, M6502_AbsoluteXR(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*4;
			break;

		// INC absolute,X
		case 0xFE:
			M6502_INC(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;

		// ISC absolute,X ... undocumented
		case 0xFF:
			M6502_ISC(cpu, M6502_AbsoluteXW(cpu));	// immediate 16-bit absolute address 16(imm) + X
			cpu->sync.clock += M6502_CLOCKMUL*7;
			break;
		}

		// time synchronization
		EmuSync(&cpu->sync);
	}
}

// execute M6502 emulator on CPU 1
static void M6502_ExecCore1()
{
	// start time synchronization
	M6502_SyncStart((sM6502*)M6502_Cpu);

	// execute
	M6502_Exec((sM6502*)M6502_Cpu);

	// clear descriptor
	M6502_Cpu = NULL;
	dsb();
}

// start emulation on core 1 (initialize synchronization, reset processor, start synchronization, execute program in core 1)
//  cpu ... pointer to CPU structure (fill-up entries before start: test, readrom, writerom, writeramport, writeromport, readromport)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (for 125 MHz system clock supported range: 62 kHz to 12 MHz)
// Returns real emulated frequency in Hz.
// To achieve accurate timing, an integer ratio between system clock and clock frequency*M6502_CLOCKMUL is recommended to maintain.
// Emulation can be stopped by M6502_Stop() function or by STOP 0 instruction.
u32 M6502_Start(sM6502* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (M6502_Cpu != NULL) M6502_Stop(pwm);

	// initialize time synchronization
	freq = M6502_SyncInit(cpu, pwm, freq);

	// reset processor
	M6502_Reset(cpu);

	// current CPU descriptor
	M6502_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(M6502_ExecCore1);
#else
	Core1Exec(M6502_ExecCore1);
#endif

	return freq;
}

// continue emulation without reset processor
u32 M6502_Cont(sM6502* cpu, int pwm, u32 freq)
{
	// stop emulation
	if (M6502_Cpu != NULL) M6502_Stop(pwm);

	// initialize time synchronization
	freq = M6502_SyncInit(cpu, pwm, freq);

	// current CPU descriptor
	M6502_Cpu = cpu;
	dsb();

	// execute program
#if USE_MINIVGA				// use mini-VGA display
	VgaCore1Exec(M6502_ExecCore1);
#else
	Core1Exec(M6502_ExecCore1);
#endif

	return freq;
}

// stop emulation (can be also activated by STOP 0 instruction)
//  pwm ... index of used PWM slice (0..7)
void M6502_Stop(int pwm)
{
	// get CPU descriptor
	sM6502* cpu = (sM6502*)M6502_Cpu;

	// check if emulation is running
	if (cpu == NULL) return;

	// request to stop emulation
	cpu->stop = 1;

	// wait to complete
	while (M6502_Cpu != NULL) { dsb(); }

	// terminate time synchronization (stop PWM counter)
	M6502_SyncTerm(pwm);

#if !USE_MINIVGA				// use mini-VGA display
	// core 1 reset
	Core1Reset();
#endif
}

#endif // USE_EMU_M6502
