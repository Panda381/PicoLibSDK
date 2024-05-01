
// ****************************************************************************
//
//                       IBM PC Emulator - Interrupts
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

// processing trap
Bool PC_IntTrapEnter = False;

// interrupt trap: returns instruction code I8086_INS_IRET if continue, or I8086_INS_NOP if waiting in loop
u8 PC_IntTrap(int num)
{
	// return instruction IRET
	u8 res = I8086_INS_IRET;

	// start processing trap
	if (PC_IntTrapEnter) return res;
	PC_IntTrapEnter = True;

	// pointer to CPU
	sI8086* cpu = PC_Cpu;

	switch (num)
	{
	// INT 05h - Print Screen
	case 0x05:
#if USE_EMUSCREENSHOT		// use emulator screen shots
		DoEmuScreenShot = True;	// request to do emulator screenshot
#endif
		break;

	// INT 08h - system clock
	//  The INT 08h timer of the emulator cannot be reprogrammed to a different interrupt rate.
	//  crystal frequency: 14.31816 MHz (this is IBM calculation; more precise should be 14.31818 MHz)
	//  clock divisor: 12
	//  clock: 14.31816/12 = 1.19318 MHz
	//  timer divisor: 65536
	//  INT 08h frequency: 1193180/65536 = 18.2065 Hz
	//  INT 08h interval: 1/18.2065 = 0.0549255 sec
	//  clocks per day: 24*60*60 / 0.0549255 = 1573040 = 0x1800B0
	//     (more precise period should be 1573042 = 0x1800B2)
	case 0x08:
		{
			// increase system timer
			u32 n = *(u32*)&PC_RAM_BASE[0x046c] + 1;
			if (n >= 0x1800B0)
			{
				n = 0;
				PC_RAM_BASE[0x0470] = 1; // midnight flag
			}
			*(u32*)&PC_RAM_BASE[0x046c] = n;

			// raise INT 1Ch interrupt
			I8086_RaiseIRQ(cpu, I8086_IRQ_INT1C);
		}
		break;

	// INT 09h - keyboard
	case 0x09:
		// INT 09h service - function
		PC_Int09fnc();
		break;

	// INT 10h - display service
	case 0x10:
		// INT 10h service - function
		PC_Int10fnc();
		break;

	// INT 11h - get equipment list
	case 0x11:
		cpu->ax = *(u16*)&PC_RAM_BASE[0x0410];
		break;

	// INT 12h - get memory size in KB
	case 0x12:
		cpu->ax = *(u16*)&PC_RAM_BASE[0x0413];
		break;

	// INT 13h - disk service (emulator: not supported, return CY)
	case 0x13:
	// INT 25h - DOS disk read (emulator: not supported, return CY)
	case 0x25:
	// INT 26h - DOS disk write (emulator: not supported, return CY)
	case 0x26:
	// INT 2Fh - DOS interrupts (emulator: not supported, return CY)
	case 0x2F:
		{
			// set carry flag
			u16 f = I8086_GetWord(cpu, cpu->ss, cpu->sp+4);
			f |= I8086_CF;
			I8086_SetWord(cpu, cpu->ss, cpu->sp+4, f);
			cpu->ah = 1; // invalid function
		}
		break;

	// INT 14h - COM port (emulator: not supported)
	case 0x14:
		cpu->ax = (u16)-1; // error flag
		break;

	// INT 15h - extension
	case 0x15:
		// INT 15h service - function
		res = PC_Int15fnc();
		break;

	// INT 16h - keyboard
	case 0x16:
		// INT 16h service - function
		res = PC_Int16fnc();
		break;

	// INT 17h - LPT port (emulator: not supported)
	case 0x17:
		cpu->ax = (u16)-1; // error flag
		break;

	// INT 1Ah - system time
	case 0x1A:
		// INT 1Ah service - function
		PC_Int1Afnc();
		break;

	// INT 21h - DOS function
	case 0x21:
		// INT 21h service - function
		res = PC_Int21fnc();
		break;

	// INT 1Bh - Ctrl+Break
	case 0x1B:
	// INT 1Ch - timer tick
	case 0x1C:
	// INT 28h - DOS waiting
	case 0x28:
	// INT 2Ah - Network
	case 0x2A:
	// INT 33h - Mouse @TODO !!!!!!!!
	case 0x33:
		break;

	// INT 20h - terminate program
	case 0x20:
	// INT 22h - old program, terminate program
	case 0x22:
	// INT 23h - Ctrl+Break, terminate program
	case 0x23:
	// INT 24h - DOS error handler, terminate program
	case 0x24:
	// INT 27h - stay resident, terminate program
	case 0x27:
		cpu->stop = 1;
		break;

	// unhandled interrupt
	// INT 00h - divide by zero
	// INT 01h - program step
	// INT 02h - nonmaskable interrupt NMI
	// INT 03h - break point
	// INT 04h - overflow
	// INT 0Ah - EGA back trace
	// INT 0Bh - COM1
	// INT 0Ch - COM2
	// INT 0Dh - fixed disk XT, LPT2
	// INT 0Eh - floppy disk
	// INT 0Fh - LPT1
	// INT 18h - BASIC loader
	// INT 19h - reset
	// INT 1Dh - video table
	// INT 1Eh - floppy disk table
	// INT 1Fh - graphics font
	// INT 2Eh - DOS execute
	default:
		{
			// get caller return address
			u16 ip = I8086_GetWord(cpu, cpu->ss, cpu->sp);
			u16 cs = I8086_GetWord(cpu, cpu->ss, cpu->sp+2);

			// error handler
			PC_Fatal(num, cs, ip);

#if I8086_CPU_80186	// 1=use 80186 or later
			// interrupt 0 - skip this instruction
			if (num == 0)
			{
				// instruction 0xF6 or 0xF7
				u8 op = I8086_GetByte(cpu, cs, ip);
				ip++;
				if ((op == 0xf6) || (op == 0xf7))
				{
					u8 modrm = I8086_GetByte(cpu, cs, ip);
					ip++;

					op = (modrm >> 3) & 7; // required operation
					if (op >= 6) // divide
					{
						if ((modrm >= 0x40) && (modrm < 0xc0))
						{
							if (modrm >= 0x80)
								ip += 2;
							else
								ip += 1;
						}

						// write new return address
						I8086_SetWord(cpu, cpu->ss, cpu->sp, ip);
					}
				}
			}
#endif
		}
		break;
	}

	// waiting loop (return instruction NOP)
	if (res == I8086_INS_NOP)
	{
		// decrease instruction pointer back to this instruction
		cpu->ip--;

		// enable interrupts
		cpu->flags |= I8086_IF;
	}

	// stop processing trap
	PC_IntTrapEnter = False;

	// restart time synchronization
	I8086_SyncStart(cpu);

	return res;
}
