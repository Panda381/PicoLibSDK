
// ****************************************************************************
//
//                        IBM PC Emulator - Ports
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

// >>> Keep tables and functions in RAM (without "const") for faster access.

// mask of EGA/VGA setup registers 0x3C4/0x3C5
const u8 PC_EgaSetMask[PC_EGA_SET_MAX+1] = {
	B0+B1,			// 0: PC_EGA_SET_RESET sequencer reset
	B0+B1+B2+B3+B4+B5,	// 1: PC_EGA_SET_CLOCK sequencer clocking mode
	B0+B1+B2+B3,		// 2: PC_EGA_SET_MASK sequencer write mask
	B0+B1+B2+B3+B4+B5+B6,	// 3: PC_EGA_SET_CHAR sequencer character map select
	B0+B1+B2+B3,		// 4: PC_EGA_SET_MEM sequencer memory mode
};

// mask of EGA/VGA graphics registers 0x3CE/0x3CF
const u8 PC_EgaGrMask[PC_EGA_GR_MAX+1] = {
	B0+B1+B2+B3,		// 0: PC_EGA_GR_COLOR graphics color to set
	B0+B1+B2+B3,		// 1: PC_EGA_GR_MASK graphics mask of color planes to set
	B0+B1+B2+B3,		// 2: PC_EGA_GR_COMP graphics compare color
	B0+B1+B2+B3+B4+B5,	// 3: PC_EGA_GR_SHIFT graphics data rotate
	B0+B1,			// 4: PC_EGA_GR_READ graphics read plane
	B0+B1+B2+B3+B4+B5+B6,	// 5: PC_EGA_GR_MODE graphics mode
	B0+B1+B2+B3,		// 6: PC_EGA_GR_MEM graphics memory mapping
	B0+B1+B2+B3,		// 7: PC_EGA_GR_CARE graphics color don't care
	0xff,			// 8: PC_EGA_GR_BIT graphics bit mask
};

// read port byte
u8 FASTCODE NOFLASH(PC_GetPort)(u16 addr)
{
	// get timer counter
	if (addr == 0x40)
	{
		if (PC_TimerHigh)
		{
			// high byte
			PC_TimerHigh = False; // shift flip/flop
			return (u8)(PC_TimerLatch >> 8);
		}
		else
		{
			// low byte
			PC_TimerLatch = (u32)Time(); // get counter
			PC_TimerHigh = True; // shift flip/flop
			return (u8)PC_TimerLatch;
		}
	}

	// get sound setup
	if (addr == 0x42)
	{
		// access mode
		int mode = (PC_SoundMode >> 4) & 3;

		// low/high byte
		if (mode == 3)
		{
			if (PC_SoundHigh)
			{
				// high byte
				PC_SoundHigh = False; // shift flip/flop
				return (u8)(PC_SoundDiv >> 8); // get high byte
			}
			else
			{
				// low byte
				PC_SoundHigh = True; // shift flip/flop
				return (u8)PC_SoundDiv; // get low byte
			}
		}

		// low byte
		else if (mode == 1)
		{
			return (u8)PC_SoundDiv; // get low byte
		}

		// high byte
		else if (mode == 2)
		{
			return (u8)(PC_SoundDiv >> 8); // get high byte
		}

		// counter low/high byte
		else
		{
			if (PC_SoundHigh)
			{
				// high byte
				PC_SoundHigh = False; // shift flip/flop
				return (u8)(PC_SoundLatch >> 8);
			}
			else
			{
				// low byte
				PC_SoundLatch = PWM_GetCount(PWMSND_SLICE); // get counter
				PC_SoundHigh = True; // shift flip/flop
				return (u8)PC_SoundLatch;
			}
		}
	}

	// get sound mode
	if (addr == 0x43)
	{
		// sound counter 2
		return PC_SoundMode;
	}

	// keyboard - get key
	if (addr == 0x60)
	{
		u8 n = PC_Int9KeyNum;
		u8 key = PC_Int9KeyScan[0];

		// data not ready - return last scan code
		if (n == 0) return key;

		// send more keys
		if (n > 1)
		{
			// time of last key
			PC_Int9KeyTime = Time();

			// shift buffer
			PC_Int9KeyScan[0] = PC_Int9KeyScan[1];
			PC_Int9KeyScan[1] = PC_Int9KeyScan[2];
			PC_Int9KeyScan[2] = PC_Int9KeyScan[3];

			// raise INT 09h
			I8086_RaiseIRQ(PC_Cpu, I8086_IRQ_INT09);
		}

		// decrease number of keys
		dmb(); // data memory barrier
		PC_Int9KeyNum = n-1;
		return key;
	}

	// get sound output
	if (addr == 0x61)
	{
		return PC_SoundGate;
	}

	// keyboard - get status
	if (addr == 0x64)
	{
		return (PC_Int9KeyNum > 0) ? B0 : 0; // key scan code ready
	}

	// EGA registers
	if ((addr >= 0x3C0) && (addr <= 0x3CF))
	{
		// pointer to current videomode descriptor
		sPC_Vmode* m = &PC_Vmode;

		// EGA select setup register
		if (addr == 0x3C4)
			return m->egasetsel; // EGA select setup register 0x3C4

		// EGA setup register
		else if (addr == 0x3C5)
		{
			u8 sel = m->egasetsel;
			if (sel <= PC_EGA_SET_MAX)
				return m->egaset[sel];
			else
				return 0;
		}

		// EGA select control register
		else if (addr == 0x3CE)
			return m->egagrsel; // EGA select graphics register 0x3CE

		// EGA graphics register
		else if (addr == 0x3CF)
		{
			u8 sel = m->egagrsel;
			if (sel <= PC_EGA_GR_MAX)
				return m->egagr[sel];
			else
				return 0;
		}
	}

	// CGA color select register
	if (addr == 0x3d9)
	{
		return PC_RAM_BASE[0x0466];
	}

	// CGA VSYNC
	if (addr == 0x3da)
	{
		return PC_VSync ? B3 : 0;
	}

	return 0xff;
}

// read port word
u16 FASTCODE NOFLASH(PC_GetPort16)(u16 addr)
{
	u8 data = PC_GetPort(addr);
	return data | ((u16)PC_GetPort(addr+1) << 8);
}

// write port byte
void FASTCODE NOFLASH(PC_SetPort)(u16 addr, u8 data)
{
	// set INT 08h timer interval
	if (addr == 0x40)
	{
		// access mode
		int mode = (PC_TimerMode >> 4) & 3;

		// low/high byte
		if (mode == 3)
		{
			if (PC_TimerHigh)
			{
				// high byte
				PC_TimerDiv = (PC_TimerDiv & 0xff) | ((u16)data << 8); // get high byte
				PC_TimerHigh = False; // shift flip/flop
				PC_Int8SetDelta(PC_TimerDiv); // update timer
			}
			else
			{
				// low byte
				PC_TimerDiv = (PC_TimerDiv & 0xff00) | data; // get low byte
				PC_TimerHigh = True; // shift flip/flop
			}
		}

		// low byte
		else if (mode == 1)
		{
			PC_TimerDiv = (PC_TimerDiv & 0xff00) | data; // get low byte
			PC_Int8SetDelta(PC_TimerDiv); // update timer
		}

		// high byte
		else if (mode == 2)
		{
			PC_TimerDiv = (PC_TimerDiv & 0xff) | ((u16)data << 8); // get high byte
			PC_Int8SetDelta(PC_TimerDiv); // update timer
		}
	}

	// set sound divider
	if (addr == 0x42)
	{
		// inicialize PC speaker output
		if (PC_SoundOut != PC_SND_SPK) PC_SoundInit(PC_SND_SPK);

		// access mode
		int mode = (PC_SoundMode >> 4) & 3;

		// low/high byte
		if (mode == 3)
		{
			if (PC_SoundHigh)
			{
				// high byte
				PC_SoundDiv = (PC_SoundDiv & 0xff) | ((u16)data << 8); // get high byte
				PC_SoundHigh = False; // shift flip/flop
				PC_SoundFreqUpdate(); // update frequency
			}
			else
			{
				// low byte
				PC_SoundDiv = (PC_SoundDiv & 0xff00) | data; // get low byte
				PC_SoundHigh = True; // shift flip/flop
			}
		}

		// low byte
		else if (mode == 1)
		{
			PC_SoundDiv = (PC_SoundDiv & 0xff00) | data; // get low byte
			PC_SoundFreqUpdate(); // update frequency
		}

		// high byte
		else if (mode == 2)
		{
			PC_SoundDiv = (PC_SoundDiv & 0xff) | ((u16)data << 8); // get high byte
			PC_SoundFreqUpdate(); // update frequency
		}

		// counter low/high byte
		else
		{
			if (PC_SoundHigh)
			{
				// high byte
				PC_SoundLatch = (PC_SoundLatch & 0xff) | ((u16)data << 8); // get high byte
				PC_SoundHigh = False; // shift flip/flop
				PWM_Count(PWMSND_SLICE, PC_SoundLatch); // set counter
			}
			else
			{
				// low byte
				PC_SoundLatch = (PC_SoundLatch & 0xff00) | data; // get low byte
				PC_SoundHigh = True; // shift flip/flop
			}
		}
	}

	// set sound mode
	if (addr == 0x43)
	{
		// timer counter 0
		u8 d = data & (B6|B7);
		if (d == (0 << 6))
		{
			PC_TimerMode = data; // setup
			PC_TimerHigh = False; // flip/flop - read/write high byte
		}

		// sound counter 2
		if (d == (2 << 6))
		{
			// inicialize PC speaker output
			if (PC_SoundOut != PC_SND_SPK) PC_SoundInit(PC_SND_SPK);

			PC_SoundHigh = False; // flip/flop - read/write high byte
			if (data != PC_SoundMode)
			{
				PC_SoundMode = data; // setup
				PC_SoundFreqUpdate(); // update frequency
			}
		}
	}

	// set sound output
	if (addr == 0x61)
	{
		PC_SoundGate = data;
		PC_SoundOnUpdate();
	}

	// EGA registers
	if ((addr >= 0x3C0) && (addr <= 0x3CF))
	{
		// pointer to current videomode descriptor
		sPC_Vmode* m = &PC_Vmode;

		// EGA select setup register
		if (addr == 0x3C4)
			m->egasetsel = data & 7; // EGA select setup register 0x3C4

		// EGA setup register
		else if (addr == 0x3C5)
		{
			u8 sel = m->egasetsel;
			if (sel <= PC_EGA_SET_MAX) m->egaset[sel] = data & PC_EgaSetMask[sel];
		}

		// EGA select control register
		else if (addr == 0x3CE)
			m->egagrsel = data & 0x0f; // EGA select graphics register 0x3CE

		// EGA graphics register
		else if (addr == 0x3CF)
		{
			u8 sel = m->egagrsel;
			if (sel <= PC_EGA_GR_MAX) m->egagr[sel] = data & PC_EgaGrMask[sel];
		}
	}

	// CGA color select register
	if (addr == 0x3d9)
	{
		// set default CGA 4-color palette
		memcpy(PC_CurPal4, PC_Pal4_CgaCrt[(data >> 4) & 3], 4*sizeof(u16));

		// blinking in text mode
		PC_Vmode.blink = ((data & B4) == 0);

		// graphics color
		u16 cf = PC_Pal16[data & 0x0f]; // color
		PC_CurPal4[0] = cf; // background color of 4-color palette
		PC_CurPal2[1] = cf; // foreground color of 2-color palette

		// 0040:0066 (1) current CGA palette register (last value on 03D9h)
		//	B5: palette 0 (red-green-brown) or 1 (cyan-magenta-white)
		//	B4: intense colors in graphics, intense background in text mode
		//	B3: intense border color in 40x25, intense background in 320x200, intense foreground in 640x200
		//	B2: red border in 40x25, red background in 320x200, red foreground in 640x200
		//	B1: green border in 40x25, red background in 320x200, red foreground in 640x200
		//	B0: blue border in 40x25, red background in 320x200, red foreground in 640x200
		PC_RAM_BASE[0x0466] = data;
	}

	// CGA/MDA control register
	if ((addr == 0x3b4) || (addr == 0x3d4)) PC_CgaSelReg = data;

	if ((addr == 0x3b5) || (addr == 0x3d5))
	{
		if (PC_CgaSelReg == 0x0c) // offset of video memory HIGH
		{
			if (PC_Vmode.vclass == PC_VCLASS_TEXT)
				PC_Vmode.off = (PC_Vmode.off & 0x1ff) | ((int)data << 9);
			else
				PC_Vmode.off = (PC_Vmode.off & 0xff) | ((int)data << 8);
		}

		if (PC_CgaSelReg == 0x0d) // offset of video memory LOW
		{
			if (PC_Vmode.vclass == PC_VCLASS_TEXT)
				PC_Vmode.off = (PC_Vmode.off & 0xfe00) | ((int)data << 1);
			else
				PC_Vmode.off = (PC_Vmode.off & 0xff00) | data;
		}
	}

	// unhandled port
	//GPIO_Flip(LED1_PIN);
}

// write port word
void FASTCODE NOFLASH(PC_SetPort16)(u16 addr, u16 data)
{
	PC_SetPort(addr, (u8)data);
	PC_SetPort(addr+1, (u8)(data >> 8));
}
