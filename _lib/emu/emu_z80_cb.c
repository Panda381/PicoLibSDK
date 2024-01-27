
// ****************************************************************************
//
//                        Z80 CPU Emulator, CB instructions
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

	// switch 0xCB operation code

	// source is (HL)
	if (low == 6)
	{
		switch (op)
		{
		// RLC (HL)
		case 0:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_RLC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// RRC (HL)
		case 1:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_RRC(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// RL (HL)
		case 2:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_RL(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// RR (HL)
		case 3:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_RR(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// SLA (HL)
		case 4:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_SLA(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// SRA (HL)
		case 5:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_SRA(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// SLL (HL)
		case 6:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_SLL(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// SRL (HL)
		case 7:
			{
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_SRL(n);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// BIT bit,(HL)
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			{
				op -= 8;
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				Z80_BIT(op, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*12;
			break;

		// RES bit,(HL)
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			{
				op -= 16;
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				n &= ~BIT(op);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;

		// SET bit,(HL)
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			{
				op -= 24;
				u16 nn = cpu->hl;
				u8 n = cpu->readmem(nn);
				n |= BIT(op);
				cpu->writemem(nn, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*15;
			break;
		}
	}

	// source is register
	else
	{
		u8* rs = &cpu->reg[7 - low]; // source register

		switch (op)
		{
		// RLC r
		case 0:
			Z80_RLC(*rs);
			break;

		// RRC r
		case 1:
			Z80_RRC(*rs);
			break;

		// RL r
		case 2:
			Z80_RL(*rs);
			break;

		// RR r
		case 3:
			Z80_RR(*rs);
			break;

		// SLA r
		case 4:
			Z80_SLA(*rs);
			break;

		// SRA r
		case 5:
			Z80_SRA(*rs);
			break;

		// SLL r
		case 6:
			Z80_SLL(*rs);
			break;

		// SRL r
		case 7:
			Z80_SRL(*rs);
			break;

		// BIT bit,r
		case 8:
		case 9:
		case 10:
		case 11:
		case 12:
		case 13:
		case 14:
		case 15:
			op -= 8;
			Z80_BIT(op, *rs);
			break;

		// RES bit,r
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			op -= 16;
			*rs &= ~BIT(op);
			break;

		// SET bit,r
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			op -= 24;
			*rs |= BIT(op);
			break;
		}
		cpu->sync.clock += Z80_CLOCKMUL*8;
	}
