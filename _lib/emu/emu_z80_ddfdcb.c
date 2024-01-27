
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

	// switch 0xDDFDCB operation code

	// source is (HL)
	if (low == 6)
	{
		switch (op)
		{
		// RLC (IXY+d)
		case 0:
			Z80_RLC(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RRC (IXY+d)
		case 1:
			Z80_RRC(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RL (IXY+d)
		case 2:
			Z80_RL(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RR (IXY+d)
		case 3:
			Z80_RR(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SLA (IXY+d)
		case 4:
			Z80_SLA(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SRA (IXY+d)
		case 5:
			Z80_SRA(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SLL (IXY+d)
		case 6:
			Z80_SLL(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SRL (IXY+d)
		case 7:
			Z80_SRL(n);
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// BIT bit,(IXY+d)
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
				Z80_BIT(op, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*20;
			break;

		// RES bit,(IXY+d)
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
				n &= ~BIT(op);
				cpu->writemem(tempaddr, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SET bit,(IXY+d)
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
				n |= BIT(op);
				cpu->writemem(tempaddr, n);
			}
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;
		}
	}

	// source is register
	else
	{
		u8* rs = &cpu->reg[7 - low]; // source register

		switch (op)
		{
		// RLC (IXY+d),r
		case 0:
			Z80_RLC(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RRC (IXY+d),r
		case 1:
			Z80_RRC(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RL (IXY+d),r
		case 2:
			Z80_RL(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RR (IXY+d),r
		case 3:
			Z80_RR(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SLA (IXY+d),r
		case 4:
			Z80_SLA(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SRA (IXY+d),r
		case 5:
			Z80_SRA(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SLL (IXY+d),r
		case 6:
			Z80_SLL(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SRL (IXY+d),r
		case 7:
			Z80_SRL(n);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// RES bit,(IXY+d),r
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
			op -= 16;
			n &= ~BIT(op);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;

		// SET bit,(IXY+d),r
		case 24:
		case 25:
		case 26:
		case 27:
		case 28:
		case 29:
		case 30:
		case 31:
			op -= 24;
			n |= BIT(op);
			*rs = n;
			cpu->writemem(tempaddr, n);
			cpu->sync.clock += Z80_CLOCKMUL*23;
			break;
		}
	}
