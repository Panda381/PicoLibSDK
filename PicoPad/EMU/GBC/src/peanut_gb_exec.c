/**
 * MIT License
 *
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Please note that at least three parts of source code within this project was
 * taken from the SameBoy project at https://github.com/LIJI32/SameBoy/ which at
 * the time of this writing is released under the MIT License. Occurrences of
 * this code is marked as being taken from SameBoy with a comment.
 * SameBoy, and code marked as being taken from SameBoy,
 * is Copyright (c) 2015-2019 Lior Halphon.
 */

// Picopad modification: (2024) Miroslav Nemecek

#include "../include.h"

// execute 0xCB prefix instruction
u8 FASTCODE NOFLASH(__gb_execute_cb)(struct gb_s *gb)
{
	u8 inst_cycles;
	u8 cbop = __gb_read(gb, gb->cpu_reg.pc.reg++);
	u8 r = (cbop & 0x7);
	u8 b = (cbop >> 3) & 0x7;
	u8 d = (cbop >> 3) & 0x1;
	u8 val;
	u8 writeback = 1;

	inst_cycles = 8;

	// Add an additional 8 cycles to these sets of instructions.
	switch(cbop & 0xC7)
	{
	case 0x06:
	case 0x86:
    	case 0xC6:
		inst_cycles += 8;
	    	break;
    	case 0x46:
		inst_cycles += 4;
    		break;
	}

	// load input value
	switch(r)
	{
	case 0:
		val = gb->cpu_reg.bc.bytes.b;
		break;

	case 1:
		val = gb->cpu_reg.bc.bytes.c;
		break;

	case 2:
		val = gb->cpu_reg.de.bytes.d;
		break;

	case 3:
		val = gb->cpu_reg.de.bytes.e;
		break;

	case 4:
		val = gb->cpu_reg.hl.bytes.h;
		break;

	case 5:
		val = gb->cpu_reg.hl.bytes.l;
		break;

	case 6:
		val = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	// Only values 0-7 are possible here, so we make the final case
	// default to satisfy -Wmaybe-uninitialized warning.
	default:
		val = gb->cpu_reg.a;
		break;
	}

	// do operation
	switch(cbop >> 6)
	{
	case 0x0:
		cbop = (cbop >> 4) & 0x3;

		switch(cbop)
		{
		case 0x0: // RdC R
		case 0x1: // Rd R
			if(d) // RRC R / RR R
			{
				u8 temp = val;
				val = (val >> 1);
				val |= cbop ? (gb->cpu_reg.f_bits.c << 7) : (temp << 7);
				gb->cpu_reg.f_bits.z = (val == 0x00);
				gb->cpu_reg.f_bits.n = 0;
				gb->cpu_reg.f_bits.h = 0;
				gb->cpu_reg.f_bits.c = (temp & 0x01);
			}
			else // RLC R / RL R
			{
				u8 temp = val;
				val = (val << 1);
				val |= cbop ? gb->cpu_reg.f_bits.c : (temp >> 7);
				gb->cpu_reg.f_bits.z = (val == 0x00);
				gb->cpu_reg.f_bits.n = 0;
				gb->cpu_reg.f_bits.h = 0;
				gb->cpu_reg.f_bits.c = (temp >> 7);
			}
			break;

		case 0x2:
			if(d) // SRA R
			{
				gb->cpu_reg.f_bits.c = val & 0x01;
				val = (val >> 1) | (val & 0x80);
				gb->cpu_reg.f_bits.z = (val == 0x00);
				gb->cpu_reg.f_bits.n = 0;
				gb->cpu_reg.f_bits.h = 0;
			}
			else // SLA R
			{
				gb->cpu_reg.f_bits.c = (val >> 7);
				val = val << 1;
				gb->cpu_reg.f_bits.z = (val == 0x00);
				gb->cpu_reg.f_bits.n = 0;
				gb->cpu_reg.f_bits.h = 0;
			}
			break;

		case 0x3:
			if(d) // SRL R
			{
				gb->cpu_reg.f_bits.c = val & 0x01;
				val = val >> 1;
				gb->cpu_reg.f_bits.z = (val == 0x00);
				gb->cpu_reg.f_bits.n = 0;
				gb->cpu_reg.f_bits.h = 0;
			}
			else // SWAP R
			{
				u8 temp = (val >> 4) & 0x0F;
				temp |= (val << 4) & 0xF0;
				val = temp;
				gb->cpu_reg.f_bits.z = (val == 0x00);
				gb->cpu_reg.f_bits.n = 0;
				gb->cpu_reg.f_bits.h = 0;
				gb->cpu_reg.f_bits.c = 0;
			}
			break;
		}

		break;

	case 0x1: // BIT B, R
		gb->cpu_reg.f_bits.z = !((val >> b) & 0x1);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 1;
		writeback = 0;
		break;

	case 0x2: // RES B, R
		val &= (0xFE << b) | (0xFF >> (8 - b));
		break;

	case 0x3: // SET B, R
		val |= (0x1 << b);
		break;
	}

	// write result back
	if(writeback)
	{
		switch(r)
		{
		case 0:
			gb->cpu_reg.bc.bytes.b = val;
			break;

		case 1:
			gb->cpu_reg.bc.bytes.c = val;
			break;

		case 2:
			gb->cpu_reg.de.bytes.d = val;
			break;

		case 3:
			gb->cpu_reg.de.bytes.e = val;
			break;

		case 4:
			gb->cpu_reg.hl.bytes.h = val;
			break;

		case 5:
			gb->cpu_reg.hl.bytes.l = val;
			break;

		case 6:
			__gb_write(gb, gb->cpu_reg.hl.reg, val);
			break;

		case 7:
			gb->cpu_reg.a = val;
			break;
		}
	}
	return inst_cycles;
}

static u8 op_cycles[0x100] =
{
	//0 1 2  3  4  5  6  7  8  9  A  B  C  D  E  F
	4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,	// 0x00
	4,12, 8, 8, 4, 4, 8, 4,12, 8, 8, 8, 4, 4, 8, 4,	// 0x10
	8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,	// 0x20
	8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4,	// 0x30
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0x40
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0x50
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0x60
	8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4, // 0x70
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0x80
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0x90
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0xA0
	4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	// 0xB0
	8,12,12,16,12,16, 8,16, 8,16,12, 8,12,24, 8,16,	// 0xC0
	8,12,12, 0,12,16, 8,16, 8,16,12, 0,12, 0, 8,16,	// 0xD0
	12,12,8, 0, 0,16, 8,16,16, 4,16, 0, 0, 0, 8,16,	// 0xE0
	12,12,8, 4, 0,16, 8,16,12, 8,16, 4, 0, 0, 8,16	// 0xF0
};
static uint_fast16_t TAC_CYCLES[4] = {1024, 16, 64, 256};

// Internal function used to step the CPU.
void FASTCODE NOFLASH(__gb_step_cpu)(struct gb_s *gb)
{
	u8 opcode;
	uint_fast16_t inst_cycles;

	// Handle interrupts
	// If gb_halt is positive, then an interrupt must have occured by the
	// time we reach here, becuase on HALT, we jump to the next interrupt
	// immediately.
	while(gb->gb_halt || (gb->gb_ime &&
			gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & ANY_INTR))
	{
		gb->gb_halt = 0;

		if(!gb->gb_ime)
			break;

		// Disable interrupts
		gb->gb_ime = 0;

		// Push Program Counter
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);

		// Call interrupt handler if required.
		if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & VBLANK_INTR)
		{
			gb->cpu_reg.pc.reg = VBLANK_INTR_ADDR;
			gb->hram_io[IO_IF] ^= VBLANK_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & LCDC_INTR)
		{
			gb->cpu_reg.pc.reg = LCDC_INTR_ADDR;
			gb->hram_io[IO_IF] ^= LCDC_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & TIME_INTR)
		{
			gb->cpu_reg.pc.reg = TIMER_INTR_ADDR;
			gb->hram_io[IO_IF] ^= TIME_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & SERIAL_INTR)
		{
			gb->cpu_reg.pc.reg = SERIAL_INTR_ADDR;
			gb->hram_io[IO_IF] ^= SERIAL_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & CONTROL_INTR)
		{
			gb->cpu_reg.pc.reg = CONTROL_INTR_ADDR;
			gb->hram_io[IO_IF] ^= CONTROL_INTR;
		}

		break;
	}

    // program is paused
    // This function is used during a screenshot to prevent
    // deformation of moving objects. It is enabled to run 
    // the program only in interrupt mode - to allow
    //  redefinition of palettes during image rendering.
    if (gb->gb_pause && gb->gb_ime)
    {
	opcode = 0; // NOP
	inst_cycles = 4;
    }
    else
    {
	// Obtain opcode
	opcode = __gb_read(gb, gb->cpu_reg.pc.reg++);
	inst_cycles = op_cycles[opcode];

	// Execute opcode
	switch(opcode)
	{
	case 0x00: // NOP
		break;

	case 0x01: // LD BC, imm
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x02: // LD (BC), A
		__gb_write(gb, gb->cpu_reg.bc.reg, gb->cpu_reg.a);
		break;

	case 0x03: // INC BC
		gb->cpu_reg.bc.reg++;
		break;

	case 0x04: // INC B
		gb->cpu_reg.bc.bytes.b++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.bc.bytes.b == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.bc.bytes.b & 0x0F) == 0x00);
		break;

	case 0x05: // DEC B
		PGB_INSTR_DEC_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0x06: // LD B, imm
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x07: // RLCA
		gb->cpu_reg.a = (gb->cpu_reg.a << 1) | (gb->cpu_reg.a >> 7);
		gb->cpu_reg.f_bits.z = 0;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = (gb->cpu_reg.a & 0x01);
		break;

	case 0x08: // LD (imm), SP
	{
		u8 h, l;
		u16 temp;
		l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		temp = PEANUT_GB_U8_TO_U16(h,l);
		__gb_write(gb, temp++, gb->cpu_reg.sp.bytes.p);
		__gb_write(gb, temp, gb->cpu_reg.sp.bytes.s);
		break;
	}

	case 0x09: // ADD HL, BC
	{
		uint_fast32_t temp = gb->cpu_reg.hl.reg + gb->cpu_reg.bc.reg;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h =
			(temp ^ gb->cpu_reg.hl.reg ^ gb->cpu_reg.bc.reg) & 0x1000 ? 1 : 0;
		gb->cpu_reg.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
		gb->cpu_reg.hl.reg = (temp & 0x0000FFFF);
		break;
	}

	case 0x0A: // LD A, (BC)
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.bc.reg);
		break;

	case 0x0B: // DEC BC
		gb->cpu_reg.bc.reg--;
		break;

	case 0x0C: // INC C
		gb->cpu_reg.bc.bytes.c++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.bc.bytes.c == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.bc.bytes.c & 0x0F) == 0x00);
		break;

	case 0x0D: // DEC C
		PGB_INSTR_DEC_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0x0E: // LD C, imm
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x0F: // RRCA
		gb->cpu_reg.f_bits.c = gb->cpu_reg.a & 0x01;
		gb->cpu_reg.a = (gb->cpu_reg.a >> 1) | (gb->cpu_reg.a << 7);
		gb->cpu_reg.f_bits.z = 0;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 0;
		break;

	case 0x10: // STOP
		if(gb->cgb.cgbMode & gb->cgb.doubleSpeedPrep)
		{
			gb->cgb.doubleSpeedPrep = 0;
			gb->cgb.doubleSpeed ^= 1;
		}
		break;

	case 0x11: // LD DE, imm
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x12: // LD (DE), A
		__gb_write(gb, gb->cpu_reg.de.reg, gb->cpu_reg.a);
		break;

	case 0x13: // INC DE
		gb->cpu_reg.de.reg++;
		break;

	case 0x14: // INC D
		gb->cpu_reg.de.bytes.d++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.de.bytes.d == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.de.bytes.d & 0x0F) == 0x00);
		break;

	case 0x15: // DEC D
		PGB_INSTR_DEC_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0x16: // LD D, imm
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x17: // RLA
	{
		u8 temp = gb->cpu_reg.a;
		gb->cpu_reg.a = (gb->cpu_reg.a << 1) | gb->cpu_reg.f_bits.c;
		gb->cpu_reg.f_bits.z = 0;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = (temp >> 7) & 0x01;
		break;
	}

	case 0x18: // JR imm
	{
		s8 temp = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.pc.reg += temp;
		break;
	}

	case 0x19: // ADD HL, DE
	{
		uint_fast32_t temp = gb->cpu_reg.hl.reg + gb->cpu_reg.de.reg;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h =
			(temp ^ gb->cpu_reg.hl.reg ^ gb->cpu_reg.de.reg) & 0x1000 ? 1 : 0;
		gb->cpu_reg.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
		gb->cpu_reg.hl.reg = (temp & 0x0000FFFF);
		break;
	}

	case 0x1A: // LD A, (DE)
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.de.reg);
		break;

	case 0x1B: // DEC DE
		gb->cpu_reg.de.reg--;
		break;

	case 0x1C: // INC E
		gb->cpu_reg.de.bytes.e++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.de.bytes.e == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.de.bytes.e & 0x0F) == 0x00);
		break;

	case 0x1D: // DEC E
		PGB_INSTR_DEC_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0x1E: // LD E, imm
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x1F: // RRA
	{
		u8 temp = gb->cpu_reg.a;
		gb->cpu_reg.a = gb->cpu_reg.a >> 1 | (gb->cpu_reg.f_bits.c << 7);
		gb->cpu_reg.f_bits.z = 0;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = temp & 0x1;
		break;
	}

	case 0x20: // JR NZ, imm
		if(!gb->cpu_reg.f_bits.z)
		{
			s8 temp = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x21: // LD HL, imm
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x22: // LDI (HL), A
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.a);
		gb->cpu_reg.hl.reg++;
		break;

	case 0x23: // INC HL
		gb->cpu_reg.hl.reg++;
		break;

	case 0x24: // INC H
		gb->cpu_reg.hl.bytes.h++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.hl.bytes.h == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.hl.bytes.h & 0x0F) == 0x00);
		break;

	case 0x25: // DEC H
		PGB_INSTR_DEC_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0x26: // LD H, imm
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x27: // DAA
	{
		// The following is from SameBoy. MIT License.
		s16 a = gb->cpu_reg.a;

		if(gb->cpu_reg.f_bits.n)
		{
			if(gb->cpu_reg.f_bits.h)
				a = (a - 0x06) & 0xFF;

			if(gb->cpu_reg.f_bits.c)
				a -= 0x60;
		}
		else
		{
			if(gb->cpu_reg.f_bits.h || (a & 0x0F) > 9)
				a += 0x06;

			if(gb->cpu_reg.f_bits.c || a > 0x9F)
				a += 0x60;
		}

		if((a & 0x100) == 0x100)
			gb->cpu_reg.f_bits.c = 1;

		gb->cpu_reg.a = a;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0);
		gb->cpu_reg.f_bits.h = 0;

		break;
	}

	case 0x28: // JR Z, imm
		if(gb->cpu_reg.f_bits.z)
		{
			s8 temp = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x29: // ADD HL, HL
	{
		gb->cpu_reg.f_bits.c = (gb->cpu_reg.hl.reg & 0x8000) > 0;
		gb->cpu_reg.hl.reg <<= 1;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = (gb->cpu_reg.hl.reg & 0x1000) > 0;
		break;
	}

	case 0x2A: // LD A, (HL+)
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl.reg++);
		break;

	case 0x2B: // DEC HL
		gb->cpu_reg.hl.reg--;
		break;

	case 0x2C: // INC L
		gb->cpu_reg.hl.bytes.l++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.hl.bytes.l == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.hl.bytes.l & 0x0F) == 0x00);
		break;

	case 0x2D: // DEC L
		PGB_INSTR_DEC_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0x2E: // LD L, imm
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x2F: // CPL
		gb->cpu_reg.a = ~gb->cpu_reg.a;
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h = 1;
		break;

	case 0x30: // JR NC, imm
		if(!gb->cpu_reg.f_bits.c)
		{
			s8 temp = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x31: // LD SP, imm
		gb->cpu_reg.sp.bytes.p = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.sp.bytes.s = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x32: // LD (HL), A
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.a);
		gb->cpu_reg.hl.reg--;
		break;

	case 0x33: // INC SP
		gb->cpu_reg.sp.reg++;
		break;

	case 0x34: // INC (HL)
	{
		u8 temp = __gb_read(gb, gb->cpu_reg.hl.reg) + 1;
		gb->cpu_reg.f_bits.z = (temp == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((temp & 0x0F) == 0x00);
		__gb_write(gb, gb->cpu_reg.hl.reg, temp);
		break;
	}

	case 0x35: // DEC (HL)
	{
		u8 temp = __gb_read(gb, gb->cpu_reg.hl.reg) - 1;
		gb->cpu_reg.f_bits.z = (temp == 0x00);
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h = ((temp & 0x0F) == 0x0F);
		__gb_write(gb, gb->cpu_reg.hl.reg, temp);
		break;
	}

	case 0x36: // LD (HL), imm 
		__gb_write(gb, gb->cpu_reg.hl.reg, __gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0x37: // SCF
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = 1;
		break;

	case 0x38: // JR C, imm
		if(gb->cpu_reg.f_bits.c)
		{
			s8 temp = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x39: // ADD HL, SP
	{
		uint_fast32_t temp = gb->cpu_reg.hl.reg + gb->cpu_reg.sp.reg;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h =
			((gb->cpu_reg.hl.reg & 0xFFF) + (gb->cpu_reg.sp.reg & 0xFFF)) & 0x1000 ? 1 : 0;
		gb->cpu_reg.f_bits.c = temp & 0x10000 ? 1 : 0;
		gb->cpu_reg.hl.reg = (u16)temp;
		break;
	}

	case 0x3A: // LD A, (HL)
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl.reg--);
		break;

	case 0x3B: // DEC SP
		gb->cpu_reg.sp.reg--;
		break;

	case 0x3C: // INC A
		gb->cpu_reg.a++;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a & 0x0F) == 0x00);
		break;

	case 0x3D: // DEC A
		gb->cpu_reg.a--;
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.a & 0x0F) == 0x0F);
		break;

	case 0x3E: // LD A, imm
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x3F: // CCF
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = ~gb->cpu_reg.f_bits.c;
		break;

	case 0x40: // LD B, B
		break;

	case 0x41: // LD B, C
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x42: // LD B, D
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.de.bytes.d;
		break;

	case 0x43: // LD B, E
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.de.bytes.e;
		break;

	case 0x44: // LD B, H
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x45: // LD B, L
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x46: // LD B, (HL)
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x47: // LD B, A
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.a;
		break;

	case 0x48: // LD C, B
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x49: // LD C, C
		break;

	case 0x4A: // LD C, D
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.de.bytes.d;
		break;

	case 0x4B: // LD C, E
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.de.bytes.e;
		break;

	case 0x4C: // LD C, H
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x4D: // LD C, L
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x4E: // LD C, (HL)
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x4F: // LD C, A
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.a;
		break;

	case 0x50: // LD D, B
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x51: // LD D, C
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x52: // LD D, D
		break;

	case 0x53: // LD D, E
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.de.bytes.e;
		break;

	case 0x54: // LD D, H
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x55: // LD D, L
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x56: // LD D, (HL)
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x57: // LD D, A
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.a;
		break;

	case 0x58: // LD E, B
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x59: // LD E, C
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x5A: // LD E, D
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.de.bytes.d;
		break;

	case 0x5B: // LD E, E
		break;

	case 0x5C: // LD E, H
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x5D: // LD E, L
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x5E: // LD E, (HL)
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x5F: // LD E, A
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.a;
		break;

	case 0x60: // LD H, B
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x61: // LD H, C
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x62: // LD H, D
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.de.bytes.d;
		break;

	case 0x63: // LD H, E
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.de.bytes.e;
		break;

	case 0x64: // LD H, H
		break;

	case 0x65: // LD H, L
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x66: // LD H, (HL)
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x67: // LD H, A
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.a;
		break;

	case 0x68: // LD L, B
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x69: // LD L, C
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x6A: // LD L, D
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.de.bytes.d;
		break;

	case 0x6B: // LD L, E
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.de.bytes.e;
		break;

	case 0x6C: // LD L, H
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x6D: // LD L, L
		break;

	case 0x6E: // LD L, (HL)
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x6F: // LD L, A
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.a;
		break;

	case 0x70: // LD (HL), B
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.bc.bytes.b);
		break;

	case 0x71: // LD (HL), C
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.bc.bytes.c);
		break;

	case 0x72: // LD (HL), D
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.de.bytes.d);
		break;

	case 0x73: // LD (HL), E
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.de.bytes.e);
		break;

	case 0x74: // LD (HL), H
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.hl.bytes.h);
		break;

	case 0x75: // LD (HL), L
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.hl.bytes.l);
		break;

	case 0x76: // HALT
	{
		int_fast16_t halt_cycles = INT_FAST16_MAX;

		// @TODO: Emulate HALT bug?
		gb->gb_halt = 1;

//		if (gb->hram_io[IO_IE] == 0)
//		{
			// Return program counter where this halt forever state started.
			// This may be intentional, but this is required to stop an infinite
			// loop.
			//(gb->gb_error)(gb, GB_HALT_FOREVER, gb->cpu_reg.pc.reg - 1);
			//PGB_UNREACHABLE();
//		}

		if(gb->hram_io[IO_SC] & SERIAL_SC_TX_START)
		{
			int serial_cycles = SERIAL_CYCLES -
				gb->counter.serial_count;

			if(serial_cycles < halt_cycles)
				halt_cycles = serial_cycles;
		}

		if(gb->hram_io[IO_TAC] & IO_TAC_ENABLE_MASK)
		{
			int tac_cycles = TAC_CYCLES[gb->hram_io[IO_TAC] & IO_TAC_RATE_MASK] -
				gb->counter.tima_count;

			if(tac_cycles < halt_cycles)
				halt_cycles = tac_cycles;
		}

		if((gb->hram_io[IO_LCDC] & LCDC_ENABLE))
		{
			int lcd_cycles;

			// If LCD is in HBlank, calculate the number of cycles
			// until the end of HBlank and the start of mode 2 or
			// mode 1.
			if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_HBLANK)
			{
				lcd_cycles = LCD_MODE_2_CYCLES -
					     gb->counter.lcd_count;
			}
			else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_SEARCH_OAM)
			{
				lcd_cycles = LCD_MODE_3_CYCLES -
					gb->counter.lcd_count;
			}
			else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_SEARCH_TRANSFER)
			{
				lcd_cycles = LCD_MODE_0_CYCLES -
					gb->counter.lcd_count;
			}
			else
			{
				// VBlank
				lcd_cycles =
					LCD_LINE_CYCLES - gb->counter.lcd_count;
			}

			if(lcd_cycles < halt_cycles)
				halt_cycles = lcd_cycles;
		}

		// Some halt cycles may already be very high, so make sure we
		// don't underflow here.
		if(halt_cycles <= 0)
			halt_cycles = 4;

		inst_cycles = (uint_fast16_t)halt_cycles;
		break;
	}

	case 0x77: // LD (HL), A
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.a);
		break;

	case 0x78: // LD A, B
		gb->cpu_reg.a = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x79: // LD A, C
		gb->cpu_reg.a = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x7A: // LD A, D
		gb->cpu_reg.a = gb->cpu_reg.de.bytes.d;
		break;

	case 0x7B: // LD A, E
		gb->cpu_reg.a = gb->cpu_reg.de.bytes.e;
		break;

	case 0x7C: // LD A, H
		gb->cpu_reg.a = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x7D: // LD A, L
		gb->cpu_reg.a = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x7E: // LD A, (HL)
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x7F: // LD A, A
		break;

	case 0x80: // ADD A, B
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.b, 0);
		break;

	case 0x81: // ADD A, C
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.c, 0);
		break;

	case 0x82: // ADD A, D
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.d, 0);
		break;

	case 0x83: // ADD A, E
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.e, 0);
		break;

	case 0x84: // ADD A, H
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.h, 0);
		break;

	case 0x85: // ADD A, L
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.l, 0);
		break;

	case 0x86: // ADD A, (HL)
		PGB_INSTR_ADC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), 0);
		break;

	case 0x87: // ADD A, A
		PGB_INSTR_ADC_R8(gb->cpu_reg.a, 0);
		break;

	case 0x88: // ADC A, B
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.b, gb->cpu_reg.f_bits.c);
		break;

	case 0x89: // ADC A, C
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.c, gb->cpu_reg.f_bits.c);
		break;

	case 0x8A: // ADC A, D
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.d, gb->cpu_reg.f_bits.c);
		break;

	case 0x8B: // ADC A, E
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.e, gb->cpu_reg.f_bits.c);
		break;

	case 0x8C: // ADC A, H
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.h, gb->cpu_reg.f_bits.c);
		break;

	case 0x8D: // ADC A, L
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.l, gb->cpu_reg.f_bits.c);
		break;

	case 0x8E: // ADC A, (HL)
		PGB_INSTR_ADC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), gb->cpu_reg.f_bits.c);
		break;

	case 0x8F: // ADC A, A
		PGB_INSTR_ADC_R8(gb->cpu_reg.a, gb->cpu_reg.f_bits.c);
		break;

	case 0x90: // SUB B
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.b, 0);
		break;

	case 0x91: // SUB C
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.c, 0);
		break;

	case 0x92: // SUB D
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.d, 0);
		break;

	case 0x93: // SUB E
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.e, 0);
		break;

	case 0x94: // SUB H
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.h, 0);
		break;

	case 0x95: // SUB L
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.l, 0);
		break;

	case 0x96: // SUB (HL)
		PGB_INSTR_SBC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), 0);
		break;

	case 0x97: // SUB A
		gb->cpu_reg.a = 0;
		gb->cpu_reg.f_bits.z = 1;
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = 0;
		break;

	case 0x98: // SBC A, B
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.b, gb->cpu_reg.f_bits.c);
		break;

	case 0x99: // SBC A, C
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.c, gb->cpu_reg.f_bits.c);
		break;

	case 0x9A: // SBC A, D
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.d, gb->cpu_reg.f_bits.c);
		break;

	case 0x9B: // SBC A, E
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.e, gb->cpu_reg.f_bits.c);
		break;

	case 0x9C: // SBC A, H
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.h, gb->cpu_reg.f_bits.c);
		break;

	case 0x9D: // SBC A, L
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.l, gb->cpu_reg.f_bits.c);
		break;

	case 0x9E: // SBC A, (HL)
		PGB_INSTR_SBC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), gb->cpu_reg.f_bits.c);
		break;

	case 0x9F: // SBC A, A
		gb->cpu_reg.a = gb->cpu_reg.f_bits.c ? 0xFF : 0x00;
		gb->cpu_reg.f_bits.z = !gb->cpu_reg.f_bits.c;
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h = gb->cpu_reg.f_bits.c;
		break;

	case 0xA0: // AND B
		PGB_INSTR_AND_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xA1: // AND C
		PGB_INSTR_AND_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xA2: // AND D
		PGB_INSTR_AND_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xA3: // AND E
		PGB_INSTR_AND_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xA4: // AND H
		PGB_INSTR_AND_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xA5: // AND L
		PGB_INSTR_AND_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xA6: // AND (HL)
		PGB_INSTR_AND_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xA7: // AND A
		PGB_INSTR_AND_R8(gb->cpu_reg.a);
		break;

	case 0xA8: // XOR B
		PGB_INSTR_XOR_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xA9: // XOR C
		PGB_INSTR_XOR_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xAA: // XOR D
		PGB_INSTR_XOR_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xAB: // XOR E
		PGB_INSTR_XOR_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xAC: // XOR H
		PGB_INSTR_XOR_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xAD: // XOR L
		PGB_INSTR_XOR_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xAE: // XOR (HL)
		PGB_INSTR_XOR_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xAF: // XOR A
		PGB_INSTR_XOR_R8(gb->cpu_reg.a);
		break;

	case 0xB0: // OR B
		PGB_INSTR_OR_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xB1: // OR C
		PGB_INSTR_OR_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xB2: // OR D
		PGB_INSTR_OR_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xB3: // OR E
		PGB_INSTR_OR_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xB4: // OR H
		PGB_INSTR_OR_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xB5: // OR L
		PGB_INSTR_OR_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xB6: // OR (HL)
		PGB_INSTR_OR_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xB7: // OR A
		PGB_INSTR_OR_R8(gb->cpu_reg.a);
		break;

	case 0xB8: // CP B
		PGB_INSTR_CP_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xB9: // CP C
		PGB_INSTR_CP_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xBA: // CP D
		PGB_INSTR_CP_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xBB: // CP E
		PGB_INSTR_CP_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xBC: // CP H
		PGB_INSTR_CP_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xBD: // CP L
		PGB_INSTR_CP_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xBE: // CP (HL)
		PGB_INSTR_CP_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xBF: // CP A
		gb->cpu_reg.f_bits.z = 1;
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h = 0;
		gb->cpu_reg.f_bits.c = 0;
		break;

	case 0xC0: // RET NZ
		if(!gb->cpu_reg.f_bits.z)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}

		break;

	case 0xC1: // POP BC
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;

	case 0xC2: // JP NZ, imm
		if(!gb->cpu_reg.f_bits.z)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xC3: // JP imm
	{
		u8 p, c;
		c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		p = __gb_read(gb, gb->cpu_reg.pc.reg);
		gb->cpu_reg.pc.bytes.c = c;
		gb->cpu_reg.pc.bytes.p = p;
		break;
	}

	case 0xC4: // CALL NZ imm
		if(!gb->cpu_reg.f_bits.z)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xC5: // PUSH BC
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.bc.bytes.b);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.bc.bytes.c);
		break;

	case 0xC6: // ADD A, imm
	{
		u8 val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_ADC_R8(val, 0);
		break;
	}

	case 0xC7: // RST 0x0000
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0000;
		break;

	case 0xC8: // RET Z
		if(gb->cpu_reg.f_bits.z)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}
		break;

	case 0xC9: // RET
	{
		gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;
	}

	case 0xCA: // JP Z, imm
		if(gb->cpu_reg.f_bits.z)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xCB: // CB INST
		inst_cycles = __gb_execute_cb(gb);
		break;

	case 0xCC: // CALL Z, imm
		if(gb->cpu_reg.f_bits.z)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xCD: // CALL imm
	{
		u8 p, c;
		c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		p = __gb_read(gb, gb->cpu_reg.pc.reg++);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.bytes.c = c;
		gb->cpu_reg.pc.bytes.p = p;
	}
	break;

	case 0xCE: // ADC A, imm
	{
		u8 val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_ADC_R8(val, gb->cpu_reg.f_bits.c);
		break;
	}

	case 0xCF: // RST 0x0008
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0008;
		break;

	case 0xD0: // RET NC
		if(!gb->cpu_reg.f_bits.c)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}

		break;

	case 0xD1: // POP DE
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;

	case 0xD2: // JP NC, imm
		if(!gb->cpu_reg.f_bits.c)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xD4: // CALL NC, imm
		if(!gb->cpu_reg.f_bits.c)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xD5: // PUSH DE
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.de.bytes.d);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.de.bytes.e);
		break;

	case 0xD6: // SUB imm
	{
		u8 val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		u16 temp = gb->cpu_reg.a - val;
		gb->cpu_reg.f_bits.z = ((temp & 0xFF) == 0x00);
		gb->cpu_reg.f_bits.n = 1;
		gb->cpu_reg.f_bits.h =
			(gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
		gb->cpu_reg.f_bits.c = (temp & 0xFF00) ? 1 : 0;
		gb->cpu_reg.a = (temp & 0xFF);
		break;
	}

	case 0xD7: // RST 0x0010
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0010;
		break;

	case 0xD8: // RET C
		if(gb->cpu_reg.f_bits.c)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}

		break;

	case 0xD9: // RETI
	{
		gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->gb_ime = 1;
	}
	break;

	case 0xDA: // JP C, imm
		if(gb->cpu_reg.f_bits.c)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xDC: // CALL C, imm
		if(gb->cpu_reg.f_bits.c)
		{
			u8 p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xDE: // SBC A, imm
	{
		u8 val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_SBC_R8(val, gb->cpu_reg.f_bits.c);
		break;
	}

	case 0xDF: // RST 0x0018
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0018;
		break;

	case 0xE0: // LD (0xFF00+imm), A
		__gb_write(gb, 0xFF00 | __gb_read(gb, gb->cpu_reg.pc.reg++),
			   gb->cpu_reg.a);
		break;

	case 0xE1: // POP HL
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;

	case 0xE2: // LD (C), A
		__gb_write(gb, 0xFF00 | gb->cpu_reg.bc.bytes.c, gb->cpu_reg.a);
		break;

	case 0xE5: // PUSH HL
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.hl.bytes.h);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.hl.bytes.l);
		break;

	case 0xE6: // AND imm
		gb->cpu_reg.a = gb->cpu_reg.a & __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.f_bits.z = (gb->cpu_reg.a == 0x00);
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = 1;
		gb->cpu_reg.f_bits.c = 0;
		break;

	case 0xE7: // RST 0x0020
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0020;
		break;

	case 0xE8: // ADD SP, imm
	{
		s8 offset = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.f_bits.z = 0;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.sp.reg & 0xF) + (offset & 0xF) > 0xF) ? 1 : 0;
		gb->cpu_reg.f_bits.c = ((gb->cpu_reg.sp.reg & 0xFF) + (offset & 0xFF) > 0xFF);
		gb->cpu_reg.sp.reg += offset;
		break;
	}

	case 0xE9: // JP (HL)
		gb->cpu_reg.pc.reg = gb->cpu_reg.hl.reg;
		break;

	case 0xEA: // LD (imm), A
	{
		u8 h, l;
		u16 addr;
		l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		addr = PEANUT_GB_U8_TO_U16(h, l);
		__gb_write(gb, addr, gb->cpu_reg.a);
		break;
	}

	case 0xEE: // XOR imm
		PGB_INSTR_XOR_R8(__gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0xEF: // RST 0x0028
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0028;
		break;

	case 0xF0: // LD A, (0xFF00+imm)
		gb->cpu_reg.a =
			__gb_read(gb, 0xFF00 | __gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0xF1: // POP AF
	{
		u8 temp_8 = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.f_bits.z = (temp_8 >> 7) & 1;
		gb->cpu_reg.f_bits.n = (temp_8 >> 6) & 1;
		gb->cpu_reg.f_bits.h = (temp_8 >> 5) & 1;
		gb->cpu_reg.f_bits.c = (temp_8 >> 4) & 1;
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;
	}

	case 0xF2: // LD A, (C)
		gb->cpu_reg.a = __gb_read(gb, 0xFF00 | gb->cpu_reg.bc.bytes.c);
		break;

	case 0xF3: // DI
		gb->gb_ime = 0;
		break;

	case 0xF5: // PUSH AF
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.a);
		__gb_write(gb, --gb->cpu_reg.sp.reg,
			   gb->cpu_reg.f_bits.z << 7 | gb->cpu_reg.f_bits.n << 6 |
			   gb->cpu_reg.f_bits.h << 5 | gb->cpu_reg.f_bits.c << 4);
		break;

	case 0xF6: // OR imm
		PGB_INSTR_OR_R8(__gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0xF7: // PUSH AF
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0030;
		break;

	case 0xF8: // LD HL, SP+/-imm
	{
		// Taken from SameBoy, which is released under MIT Licence.
		s8 offset = (s8) __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.hl.reg = gb->cpu_reg.sp.reg + offset;
		gb->cpu_reg.f_bits.z = 0;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = ((gb->cpu_reg.sp.reg & 0xF) + (offset & 0xF) > 0xF) ? 1 : 0;
		gb->cpu_reg.f_bits.c = ((gb->cpu_reg.sp.reg & 0xFF) + (offset & 0xFF) > 0xFF) ? 1 : 0;
		break;
	}

	case 0xF9: // LD SP, HL
		gb->cpu_reg.sp.reg = gb->cpu_reg.hl.reg;
		break;

	case 0xFA: // LD A, (imm)
	{
		u8 h, l;
		u16 addr;
		l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		addr = PEANUT_GB_U8_TO_U16(h, l);
		gb->cpu_reg.a = __gb_read(gb, addr);
		break;
	}

	case 0xFB: // EI
		gb->gb_ime = 1;
		break;

	case 0xFE: // CP imm
	{
		u8 val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_CP_R8(val);
		break;
	}

	case 0xFF: // RST 0x0038
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0038;
		break;

	default:
		// Return address where invlid opcode that was read.
		//(gb->gb_error)(gb, GB_INVALID_OPCODE, gb->cpu_reg.pc.reg - 1);
		//PGB_UNREACHABLE();
		break;
	}
    } // if (gb->gb_pause)

	do
	{
		// DIV register timing
		gb->counter.div_count += inst_cycles;
		while(gb->counter.div_count >= DIV_CYCLES)
		{
			gb->hram_io[IO_DIV]++;
			gb->counter.div_count -= DIV_CYCLES;
		}

		// Check serial transmission.
		if(gb->hram_io[IO_SC] & SERIAL_SC_TX_START)
		{
			unsigned int serial_cycles = SERIAL_CYCLES_1KB;

			// If new transfer, call TX function.
			if(gb->counter.serial_count == 0 &&
				gb->gb_serial_tx != NULL)
				(gb->gb_serial_tx)(gb, gb->hram_io[IO_SB]);

			if(gb->hram_io[IO_SC] & 0x3)
				serial_cycles = SERIAL_CYCLES_32KB;

			gb->counter.serial_count += inst_cycles;

			// If it's time to receive byte, call RX function.
			if(gb->counter.serial_count >= serial_cycles)
			{
				// If RX can be done, do it.
				// If RX failed, do not change SB if using external
				// clock, or set to 0xFF if using internal clock.
				u8 rx;

				if(gb->gb_serial_rx != NULL &&
					(gb->gb_serial_rx(gb, &rx) ==
						GB_SERIAL_RX_SUCCESS))
				{
					gb->hram_io[IO_SB] = rx;

					// Inform game of serial TX/RX completion.
					gb->hram_io[IO_SC] &= 0x01;
					gb->hram_io[IO_IF] |= SERIAL_INTR;
				}
				else if(gb->hram_io[IO_SC] & SERIAL_SC_CLOCK_SRC)
				{
					// If using internal clock, and console is not
					// attached to any external peripheral, shifted
					// bits are replaced with logic 1.
					gb->hram_io[IO_SB] = 0xFF;

					// Inform game of serial TX/RX completion.
					gb->hram_io[IO_SC] &= 0x01;
					gb->hram_io[IO_IF] |= SERIAL_INTR;
				}
				else
				{
					// If using external clock, and console is not
					// attached to any external peripheral, bits are
					// not shifted, so SB is not modified.
				}

				gb->counter.serial_count = 0;
			}
		}

		// TIMA register timing
		if(gb->hram_io[IO_TAC] & IO_TAC_ENABLE_MASK)
		{
			gb->counter.tima_count += inst_cycles;

			while(gb->counter.tima_count >=
				TAC_CYCLES[gb->hram_io[IO_TAC] & IO_TAC_RATE_MASK])
			{
				gb->counter.tima_count -=
					TAC_CYCLES[gb->hram_io[IO_TAC] & IO_TAC_RATE_MASK];

				if(++gb->hram_io[IO_TIMA] == 0)
				{
					gb->hram_io[IO_IF] |= TIME_INTR;
					// On overflow, set TMA to TIMA.
					gb->hram_io[IO_TIMA] = gb->hram_io[IO_TMA];
				}
			}
		}

		// If LCD is off, don't update LCD state or increase the LCD ticks.
		if(!(gb->hram_io[IO_LCDC] & LCDC_ENABLE))
			continue;

	// LCD Timing
        if (inst_cycles > 1)
            gb->counter.lcd_count += (inst_cycles >> gb->cgb.doubleSpeed);
        else
		gb->counter.lcd_count += inst_cycles;

		// New Scanline
		if(gb->counter.lcd_count >= LCD_LINE_CYCLES)
		{
			gb->counter.lcd_count -= LCD_LINE_CYCLES;

			// Next line
			gb->hram_io[IO_LY] = (gb->hram_io[IO_LY] + 1) % LCD_VERT_LINES;

			// LYC Update
			if(gb->hram_io[IO_LY] == gb->hram_io[IO_LYC])
			{
				gb->hram_io[IO_STAT] |= STAT_LYC_COINC;

				if(gb->hram_io[IO_STAT] & STAT_LYC_INTR)
					gb->hram_io[IO_IF] |= LCDC_INTR;
			}
			else
				gb->hram_io[IO_STAT] &= 0xFB;

			// VBLANK Start
			if(gb->hram_io[IO_LY] == LCD_HEIGHT)
			{
				gb->hram_io[IO_STAT] =
					(gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_VBLANK;
				gb->gb_frame = 1;
				gb->hram_io[IO_IF] |= VBLANK_INTR;
				gb->lcd_blank = 0;

				if(gb->hram_io[IO_STAT] & STAT_MODE_1_INTR)
					gb->hram_io[IO_IF] |= LCDC_INTR;

				// If frame skip is activated, check if we need to draw
				// the frame or skip it.
//				if(gb->direct.frame_skip)
//				{
//					gb->display.frame_skip_count =
//						!gb->display.frame_skip_count;
//				}

				// If interlaced is activated, change which lines get
				// updated. Also, only update lines on frames that are
				// actually drawn when frame skip is enabled.
//				if(gb->direct.interlace &&
//						(!gb->direct.frame_skip ||
//						 gb->display.frame_skip_count))
//				{
//					gb->display.interlace_count =
//						!gb->display.interlace_count;
//				}
			}
			// Normal Line
			else if(gb->hram_io[IO_LY] < LCD_HEIGHT)
			{
				if(gb->hram_io[IO_LY] == 0)
				{
					// Clear Screen
					gb->display.WY = gb->hram_io[IO_WY];
					gb->display.window_clear = 0;
				}

				gb->hram_io[IO_STAT] =
					(gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_HBLANK;

				// DMA GBC
				if(gb->cgb.cgbMode && gb->cgb.dmaRunning && gb->cgb.dmaMode)
				{
					for (u8 i = 0; i < 0x10; i++)
					{
						__gb_write(gb, ((gb->cgb.dmaDest & 0x1FF0) | 0x8000) + i,
								   __gb_read(gb, (gb->cgb.dmaSource & 0xFFF0) + i));
					}
					gb->cgb.dmaSource += 0x10;
					gb->cgb.dmaDest += 0x10;
					if(!(--gb->cgb.dmaSize)) gb->cgb.dmaRunning = 0;
				}

				if(gb->hram_io[IO_STAT] & STAT_MODE_0_INTR)
					gb->hram_io[IO_IF] |= LCDC_INTR;

				// If halted immediately jump to next LCD mode.
				if(gb->counter.lcd_count < LCD_MODE_2_CYCLES)
					inst_cycles = LCD_MODE_2_CYCLES - gb->counter.lcd_count;
			}
		}
		// OAM access
		else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_HBLANK &&
				gb->counter.lcd_count >= LCD_MODE_2_CYCLES)
		{
			gb->hram_io[IO_STAT] =
				(gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_SEARCH_OAM;

			if(gb->hram_io[IO_STAT] & STAT_MODE_2_INTR)
				gb->hram_io[IO_IF] |= LCDC_INTR;

			// If halted immediately jump to next LCD mode.
			if (gb->counter.lcd_count < LCD_MODE_3_CYCLES)
				inst_cycles = LCD_MODE_3_CYCLES - gb->counter.lcd_count;
		}
		// Update LCD
		else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_SEARCH_OAM &&
				gb->counter.lcd_count >= LCD_MODE_3_CYCLES)
		{
			gb->hram_io[IO_STAT] =
				(gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_SEARCH_TRANSFER;

			if(!gb->lcd_blank)
				__gb_draw_line(gb);

			// If halted immediately jump to next LCD mode.
			if (gb->counter.lcd_count < LCD_MODE_0_CYCLES)
				inst_cycles = LCD_MODE_0_CYCLES - gb->counter.lcd_count;
		}
	} while(gb->gb_halt && (gb->hram_io[IO_IF] & gb->hram_io[IO_IE]) == 0);
	// If halted, loop until an interrupt occurs.
}
