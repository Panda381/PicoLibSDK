
// ****************************************************************************
//
//                       IBM PC Emulator - Memory service
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

// BIOS ID header (16 bytes at address 0xFFFF0, page 255)
const u8 PC_BiosID[16] = {
	0xea, 0xf0, 0xff, 0x00, 0xf0,		// F000:FFF0 (5): jump to POST (reset), here we use instruction JMP F000:FFF0 (loop)
	'0','1','/','0','1','/','8','6',	// F000:FFF5 (8): BIOS date (= "01/01/86')
	0,					// F000:FFFD (1): ASCIIZ terminator
	0xfc,					// F000:FFFE (1): PC model (FF=PC, FE=XT, FD=PCjr, FC=AT, FB=XT2, F9=convertible PC)
	0x00,					// F000:FFFF (1): PC sub-model
};

// 4-bit mask of EGA color planes
const u32 PC_EgaPlaneMask[16] = {
	0x00000000,
	0x000000ff,
	0x0000ff00,
	0x0000ffff,
	0x00ff0000,
	0x00ff00ff,
	0x00ffff00,
	0x00ffffff,
	0xff000000,
	0xff0000ff,
	0xff00ff00,
	0xff00ffff,
	0xffff0000,
	0xffff00ff,
	0xffffff00,
	0xffffffff,
};

// base address of program in ROM (must be ALIGNED), program size
int PC_RomSize;

// map of memory pages (PC_INV_PAGE = invalid page)
u8 PC_MemMap[PC_PAGE_NUM];

// index of next free RAM page
u8 PC_NextPageRAM;

// segment of end of DOS allocated memory
u16 PC_AllocEnd;

// bottom video-RAM allocated page (PC_RAM_PAGE_NUM = no video-RAM allocated)
u8 PC_VmemPageRAM;

// initialize memory and add program ROM to memory map
//  size ... size of program (will be aligned up to memory page; must not exceed end of virtual space)
void PC_MemMapSetupRom(int size)
{
	// clear memory map
	int i;
	for (i = 1; i < PC_PAGE_NUM; i++) PC_MemMap[i] = PC_INV_PAGE;

	// first page is vector table
	PC_MemMap[0] = PC_RAM_PAGE_FIRST;

	// initialize vector table
	for (i = 0; i < 256; i++) 
	{
		*(u16*)&PC_RAM_BASE[i*4] = (u16)i;	// jump offset
		*(u16*)&PC_RAM_BASE[i*4+2] = (PC_TRAP_ADDR>>4);	// jump segment
	}

	// initialize interrupt traps
	for (i = PC_TRAP_ADDR; i < PC_TRAP_ADDR+256; i++) PC_RAM_BASE[i] = I8086_INS_IRET; // instruction IRET

	// initialize BIOS data
	for (i = 0x0400; i < 0x0600; i++) PC_RAM_BASE[i] = 0;
	// 0040:0000 (8) base COM1..COM4 address (0=none)
	// 0040:0008 (8) base LPT1..LPT4 address (0=none)
	// 0040:0010 (2) installed hardware
	//	B14-B15: number of LPTs 0..3
	//	B13: internal modem
	//	B12: game port
	//	B9-B12: number of COMs 0..7
	//	B6-B7: number of floppy - 1
	//	B4-B5: initial video mode 0=EGA/VGA, 1=40x25, 2=80x25, 3=80x25 mono
	//	B2-B3: RAM size 0=64K, 1=128K, 2=192K, 3=256K
	//	B1: 1=math coprocessor
	//	B0: floppy disk drives
	PC_RAM_BASE[0x0410] = B4 + (3<<2) + B0; // 1 floppy A:, videomode 40x25, 256 KB RAM
	// 0040:0012 (1) POST status
	// 0040:0013 (2) base memory size in KB (INT 12h)
	*(u16*)&PC_RAM_BASE[0x0413] = 640; // 640 KB base memory
	// 0040:0015 (2) adapter memory size
	// 0040:0017 (1) keyboard status flag 1
	//	B7: Insert active
	//	B6: Caps Lock active
	//	B5: Num Lock active
	//	B4: Scroll Lock active
	//	B3: L-Alt or R-Alt pressed
	//	B2: L-Ctrl or R-Ctrl pressed
	//	B1: L-Shift pressed
	//	B0: R-Shift pressed
	// 0040:0018 (1) keyboard status flag 2
	//	B7: Insert pressed
	//	B6: Caps Lock pressed
	//	B5: Num Lock pressed
	//	B4: Scroll Lock pressed
	//	B3: Pause active
	//	B2: SysReq pressed
	//	B1: L-Alt pressed
	//	B0: L-Ctrl pressed
	// 0040:0019 (1) ALT-nnn keypad value
	// 0040:001A (2) read pointer from keyboard buffer
	PC_RAM_BASE[0x041A] = 0x1E; // read pointer
	// 0040:001C (2) write pointer to keyboard buffer
	PC_RAM_BASE[0x041C] = 0x1E; // write pointer
	// 0040:001E (32) keyboard ring buffer (16 word entries)
	// 0040:003E (1) diskette recalibrate seek status (bit 0..3: recalibrate diskette 0..3)
	// 0040:003F (1) diskette motor status (bit 0..3: diskette 0..3 motos is on, bit 4-5: selected drive, bit 7: write)
	// 0040:0040 (1) diskette motor turn-off timeout counter (in number of ticks of INT 08)
	// 0040:0041 (1) diskette last operation status
	// 0040:0042 (7) floppy/hard drive status/command bytes
	// 0040:0049 (1) current video mode (0 = text 40x25, 16 colors)
	// 0040:004A (2) number of columns on screen
	PC_RAM_BASE[0x044A] = 40; // number of columns
	// 0040:004C (2) page size in bytes, aligned
	*(u16*)&PC_RAM_BASE[0x044C] = 2048; // page size
	// 0040:004E (2) page start address
	// 0040:0050 (16) cursor positions in 8 pages (column and row)
	// 0040:0060 (2) cursor mode setting (end scan line / start scan line)
	// 0040:0062 (1) current page number
	// 0040:0063 (2) CRT controller base I/O port address (03B4h MDA, 03D4h color)
	*(u16*)&PC_RAM_BASE[0x0463] = 0x03D4; // CRT controller port address
	// 0040:0065 (1) current CRT mode selector register (last value on 03D8h)
	//	B5: bit 7 blinks
	//	B4: mode 6 monochrome
	//	B3: video signal enabled
	//	B2: monochrome
	//	B1: graphics
	//	B0: 80x25 text
	// 0040:0066 (1) current CGA palette register (last value on 03D9h)
	//	B5: palette 0 (red-green-brown) or 1 (cyan-magenta-white)
	//	B4: intense colors in graphics, intense background in text mode
	//	B3: intense border color in 40x25, intense background in 320x200, intense foreground in 640x200
	//	B2: red border in 40x25, red background in 320x200, red foreground in 640x200
	//	B1: green border in 40x25, red background in 320x200, red foreground in 640x200
	//	B0: blue border in 40x25, red background in 320x200, red foreground in 640x200
	// 0040:0067 (4) reset restart address (after CPU reset or jump to F000:FFF0)
	// 0040:006B (1) mask of last IRQs (bit 0..7 = IRQ0..IRQ7)
	// 0040:006C (4) timer ticks (55 ms) since midnight (INT 08h, 18.2065 per second, 54.9255 ms)
	//	... crystal 14.31816/12 = 1.19318 MHz, /65536 = 18.2065 Hz, counter value 0..0x1800AF (1573039)
	*(u32*)&PC_RAM_BASE[0x046c] = RandU32() & 0xFFFFF; // randomize some time - games use it as seed of random number generator
	// 0040:0070 (1) timer overflow (midnight) ... overflow on value 1800B0H = 1573040
	// 0040:0071 (1) bit 7 = Ctrl+Break pressed flag
	// 0040:0072 (2) post reset flag (1234h = bypass memory test, warm boot)
	// 0040:0074 (1) fixed disk last operation status
	// 0040:0075 (1) count of fixed drives
	// 0040:0076 (1) fixed disk head control byte
	// 0040:0077 (1) fixed disk I/O port offset
	// 0040:0078 (4) LPT1..LPT4 time-out counters
	// 0040:007C (4) COM1..COM4 time-out counters
	// 0040:0080 (2) start address of keyboard ring buffer
	PC_RAM_BASE[0x0480] = 0x1E; // start address
	// 0040:0082 (2) end address+1 of keyboard ring buffer
	PC_RAM_BASE[0x0482] = 0x3E; // end address
	// 0040:0084 (1) last row on screen (= rows - 1)
	PC_RAM_BASE[0x0484] = 24; // last row
	// 0040:0085 (2) bytes per character (character height in scan-lines)
	PC_RAM_BASE[0x0485] = 8; // character height
	// 0040:0087 (1) video control
	//	B7: do not clear RAM on mode set
	//	B5-B6: size of videoRAM 0=64K, 1=128K, 2=192K, 3=256K
	//	B3: video system inactive
	//	B2: wait for display enable
	//	B1: mono monitor
	//	B0: alphanumeric cursor emulation disabled
	PC_RAM_BASE[0x0487] = (PC_CardMem[PC_Vmode.card] > PC_VMEM_64K) ? B5 : 0; // 64KB or 128KB videoRAM
	// 0040:0088 (1) video switches
	// 0040:0089 (1) VGA mode-set option
	// 0040:008A (1) VGA index combination
	// 0040:008B (1) diskette media control
	// 0040:008C (1) fixed disk controller status
	// 0040:008D (1) fixed disk error status
	// 0040:008E (1) fixed disk interrupt
	// 0040:008F (1) diskette controller information
	// 0040:0090 (2) diskette drive 0-1 media state
	// 0040:0092 (2) diskette drive 0-1 start operation state
	// 0040:0094 (2) diskette drive 0-1 current track number
	// 0040:0096 (1) keyboard status byte 1
	//	B7: 1=read-ID in progress
	//	B6: 1=last code read was first of two IDs
	//	B5: 1=force Num Lock if read-ID on enhanced
	//	B4: 1=enhanced keyboard installed
	//	B3: 1=R-Alt pressed
	//	B2: 1=R-Ctrl pressed
	//	B1: 1=last code read was E0h
	//	B0: 1=last code read was E1h
	PC_RAM_BASE[0x0496] = B4; // enhanced keyboard installed
	// 0040:0097 (1) keyboard status byte 2
	//	B7: 1=keyboard transmit error flag
	//	B6: 1=LED update in progress
	//	B5: 1=RESEND received
	//	B4: 1=ACK received
	//	B2: 1=Caps Lock LED
	//	B1: 1=Num Lock LED
	//	B0: 1=Scroll Lock LED
	// 0040:0098 (4) pointer to user wait-complete flag
	// 0040:009C (4) user wait count in microseconds
	// 0040:00A0 (1) wait active flag (1=busy, 80h=posted, 00h=acknowledged)
	// 0040:00A1 (7) network
	// 0040:00A8 (4) pointer to EGA parameter control block
	// ...XT/AT reserved
	// 0050:0000 (1) print-screen status

	// save ROM base and size
	PC_RomSize = size;

	// index of next free RAM page
	PC_NextPageRAM = 1;

	// bottom video-RAM allocated page (PC_RAM_PAGE_NUM = no video-RAM allocated)
	PC_VmemPageRAM = PC_RAM_PAGE_NUM;

	// number of pages
	int n = PC_PAGE_ALIGNUP(size) >> PC_PAGE_SHIFT;

	// setup ROM pages
	for (i = 1; i <= n; i++) PC_MemMap[i] = (u8)(PC_ROM_PAGE_FIRST + i - 1);

	// setup BIOS pages
	PC_MemMap[255] = PC_BIOS_PAGE_ID;	// BIOS ROM page with ID
	PC_MemMap[254] = PC_BIOS_PAGE_FONT16;	// BIOS ROM page with font16
	PC_MemMap[253] = PC_BIOS_PAGE_FONT14;	// BIOS ROM page with font14
	PC_MemMap[252] = PC_BIOS_PAGE_FONT8;	// BIOS ROM page with font8

	// no videomode set yet
	PC_Vmode.vmode = PC_VMODE_NUM;
}

// read memory
u8 FASTCODE NOFLASH(PC_GetMem)(u32 addr)
{
	// get memory page and offset
	u8 page = PC_MemMap[addr >> PC_PAGE_SHIFT];
	u32 off = addr & PC_PAGE_MASK;

	// get RAM
	if (page <= PC_RAM_PAGE_LAST)
	{
		// RAM address
		u32 a = ((int)(page - PC_RAM_PAGE_FIRST) << PC_PAGE_SHIFT) | off;

		// Interrupt traps
		if ((a < PC_TRAP_ADDR+256) && (a >= PC_TRAP_ADDR) && (PC_Cpu != NULL))
		{
			u32 pc = I8086_ADDR(PC_Cpu->cs, PC_Cpu->ip);
			if (pc == addr) return PC_IntTrap(a - PC_TRAP_ADDR);
		}

		u8* s = &PC_RAM_BASE[a];

		// get video-RAM
		if (page >= PC_VmemPageRAM)
		{
			// pointer to current videomode descriptor
			sPC_Vmode* m = &PC_Vmode;

			// EGA mode
			if (m->vclass == PC_VCLASS_EGA16)
			{
				// get plane size
				int planesize = m->planesize;

				// check address
				if ((addr >= (u32)0xa0000) && (addr < (u32)(0xa0000 + planesize)))
				{
					// get latch of all 4 planes
					u32 k = *s;
					s += planesize;
					k |= (u32)*s << 8;
					s += planesize;
					k |= (u32)*s << 16;
					s += planesize;
					k |= (u32)*s << 24;
					m->latch = k;

					// read mode 0 - read selected plane
					if ((m->egagr[PC_EGA_GR_MODE] & B3) == 0)
					{
						return (u8)(k >> (m->egagr[PC_EGA_GR_READ] * 8));
					}

					// read mode 1 - read compare color
					k ^= PC_EgaPlaneMask[m->egagr[PC_EGA_GR_COMP]]; // compare color bits - equal bits are set to 0, different are set to 1
					k &= PC_EgaPlaneMask[m->egagr[PC_EGA_GR_CARE]]; // mask colors - ignored bits are set to 0, valid bits are unchanged 0 or 1
					k |= k >> 16;
					k |= k >> 8; // degrade bits to one byte; bits with different color are set to 1, equal color or ignored are set to 0
					return (u8)(~k); // invert - equal color bits are set to 1
				}
			}
		}

		return *s;
	}

	// invalid memory
	if (page == PC_INV_PAGE) return 0xff;

	// get flash
	if (page <= PC_ROM_PAGE_LAST)
	{
		return PC_ROM_BASE[((int)(page - PC_ROM_PAGE_FIRST) << PC_PAGE_SHIFT) | off];
	}

	// get ROM BIOS
	switch (page)
	{
	case PC_BIOS_PAGE_ID:
		if (off >= 0xff0) return PC_BiosID[off - 0xff0];
		break;

	case PC_BIOS_PAGE_FONT16:
		if (off < PC_BIOS_FONT16_SIZE) return PC_Font16[off];
		break;

	case PC_BIOS_PAGE_FONT14:
		if (off < PC_BIOS_FONT14_SIZE) return PC_Font14[off];
		break;

	case PC_BIOS_PAGE_FONT8:
		if (off < PC_BIOS_FONT8_SIZE) return PC_Font8[off];
		break;
	}

	// invalid data
	return 0xff;
}

// write memory
void FASTCODE NOFLASH(PC_SetMem)(u32 addr, u8 data)
{
	// get memory page and offset
	u8 page = PC_MemMap[addr >> PC_PAGE_SHIFT];
	u32 off = addr & PC_PAGE_MASK;

	// set RAM
	if (page <= PC_RAM_PAGE_LAST)
	{
		// RAM address
		u8* d = &PC_RAM_BASE[((int)(page - PC_RAM_PAGE_FIRST) << PC_PAGE_SHIFT) | off];

		// set video-RAM
		if (page >= PC_VmemPageRAM)
		{
			// pointer to current videomode descriptor
			sPC_Vmode* m = &PC_Vmode;

			// EGA mode
			if (m->vclass == PC_VCLASS_EGA16)
			{
				// get plane size
				int planesize = m->planesize;

				// check address
				if ((addr >= (u32)0xa0000) && (addr < (u32)(0xa0000 + planesize)))
				{
					// get write mode
					u8 wmode = m->egagr[PC_EGA_GR_MODE] & 3;
					u8 bitmask = m->egagr[PC_EGA_GR_BIT]; // bit mask
					u32 k; // value to write

					// write mode 0 - modify/write
					if (wmode == 0)
					{
						// number of bits to rotate data
						int rot = m->egagr[PC_EGA_GR_SHIFT] & 7;

						// rotate data value right
						data = (u8)((data >> rot) | (data << (8 - rot)));

						// expand data to 4 planes
						k = data | ((u32)data << 8);
						k |= k << 16;

						// enable set/reset mask
						u32 enmask = PC_EgaPlaneMask[m->egagr[PC_EGA_GR_MASK]];

						// set/reset color
						k &= ~enmask; // clear unused colors
						k |= PC_EgaPlaneMask[m->egagr[PC_EGA_GR_COLOR]] & enmask; // add preset color
					}

					// write mode 1 - fast transfers
					else if (wmode == 1)
					{
						k = m->latch;
					}

					// write mode 2 - write all pixels
					else if (wmode == 2)
					{
						k = PC_EgaPlaneMask[data & 0x0f];
					}

					// write mode 3 - VGA modify/write
					else
					{
						// number of bits to rotate data
						int rot = m->egagr[PC_EGA_GR_SHIFT] & 7;

						// rotate data value right
						data = (u8)((data >> rot) | (data << (8 - rot)));

						// mask color
						bitmask &= data;

						// write preset color
						k = PC_EgaPlaneMask[m->egagr[PC_EGA_GR_COLOR]];
					}

					// not for write mode 1
					if (wmode != 1)
					{
						// prepare logical operation
						int fnc = m->egagr[PC_EGA_GR_SHIFT] >> 3;

						// AND
						if (fnc == 1)
							k &= m->latch;
						// OR
						else if (fnc == 2)
							k |= m->latch;
						// XOR
						else if (fnc == 3)
							k ^= m->latch;

						// expand bit mask
						u32 bitmask2 = bitmask | ((u32)bitmask << 8);
						bitmask2 |= bitmask2 << 16;

						// apply bit mask
						k &= bitmask2;
						k |= m->latch & ~bitmask2;
					}

					// prepare write mask
					u8 wmask = m->egaset[PC_EGA_SET_WMASK];
					while (wmask != 0)
					{
						if ((wmask & 1) != 0) *d = (u8)k; // write data
						k >>= 8; // shift data
						wmask >>= 1; // shift mask
						d += planesize; // shift address to next plane
					}
				}
				return;
			}
		}

		*d = data;
		return;
	}

	// write to invalid memory - allocate empty RAM
	if (page == PC_INV_PAGE)
	{
		// must be valid RAM memory range
		if (addr < 0xa0000)
		{
			// next free RAM page
			u8 next = PC_NextPageRAM;

			// memory error, no free RAM page
			if (next >= PC_VmemPageRAM)
			{
				// flag - memory error (no free RAM)
				PC_Fatal(PC_ERROR_WRAM, addr >> 4, addr & 0x0f);
				return;
			}

			// allocate new RAM page
			PC_MemMap[addr >> PC_PAGE_SHIFT] = next + PC_RAM_PAGE_FIRST;
			PC_NextPageRAM = next + 1;

			// write data
			PC_RAM_BASE[((int)next << PC_PAGE_SHIFT) | off] = data;
		}
		return;
	}

	// write to BIOS
	if (page > PC_ROM_PAGE_LAST) return;

	// trying to write to flash
	u8 next = PC_NextPageRAM; // next free RAM page

	// memory error, no free RAM page
	if (next >= PC_VmemPageRAM)
	{
		// flag - memory error (no free RAM)
		PC_Fatal(PC_ERROR_WRAM, addr >> 4, addr & 0x0f);
		return;
	}

	// allocate new RAM page
	PC_MemMap[addr >> PC_PAGE_SHIFT] = next + PC_RAM_PAGE_FIRST;
	PC_NextPageRAM = next + 1;

	// copy data
	memcpy(&PC_RAM_BASE[((int)next << PC_PAGE_SHIFT)],
		&PC_ROM_BASE[((int)(page - PC_ROM_PAGE_FIRST) << PC_PAGE_SHIFT)],
		PC_PAGE_SIZE);

	// write data
	PC_RAM_BASE[((int)next << PC_PAGE_SHIFT) | off] = data;
}
