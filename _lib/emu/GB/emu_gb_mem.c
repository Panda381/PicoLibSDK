
// ****************************************************************************
//
//                     Game Boy Emulator - Memory service
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

// search ROM cache page by offset (returns GB_ROM_PAGEINV = not found)
u8 GB_GetROMbyOff(u32 off)
{
	int i = GB_CACHE_PAGENUM-1;
	sGB_Cache* c = &GBC->cache_list[i];
	for (; i >= 0; i--)
	{
		if (c->off == off) return (u8)i;
		c--;
	}
	return GB_ROM_PAGEINV;
}

// read memory
u8 FASTCODE NOFLASH(GB_GetMem)(u16 addr)
{
	// ROM
	if (addr < 0x8000)
	{
		// ROM page
		u8 pg = (u8)(addr >> GB_PAGE_SHIFT);

		// ROM cache page
		u8 rom = GBC->rom[pg];

		// page is not assigned
		if (rom == GB_ROM_PAGEINV)
		{
			// prepare ROM base offset
			u32 off = (u32)pg << GB_PAGE_SHIFT;
			if (addr >= 0x4000) off = off - 0x4000 + GBC->rom_base;

			// search ROM by offset
			rom = GB_GetROMbyOff(off);

			// page not found, need to read from the ROM file
			if (rom == GB_ROM_PAGEINV)
			{
				// prepare free page
				rom = GB_GetROMbyOff(GB_CACHE_INV);

				// no free page - free the oldest one
				if (rom == GB_ROM_PAGEINV)
				{
					// all pages are used, find the oldest one
					u32 t = Time();
					u32 delta;
					u32 deltabest = 0;
					int i = GB_CACHE_PAGENUM-1;
					rom = (u8)i;
					sGB_Cache* c = &GBC->cache_list[i];
					for (; i >= 0; i--)
					{
						delta = t - c->last;
						if (delta > deltabest)
						{
							rom = (u8)i;
							deltabest = delta;
						}
						c--;
					}

					// mark this page as not used
					GBC->cache_list[rom].off = GB_CACHE_INV;
					for (i = GB_ROM_PAGENUM-1; i >= 0; i--)
					{
						if (GBC->rom[i] == rom) GBC->rom[i] = GB_ROM_PAGEINV;
					}
				}

				// bytes to read
				int len = GB_ROMSize - off;
				if (len < 0) len = 0;
				if (len > GB_PAGE_SIZE) len = GB_PAGE_SIZE;

				// read data from the file
				if (len > 0)
				{
					// read page content
					while (True)
					{
						// seek file
						FileSeek(GB_ROMFile, off);

						// read data
						int n = FileRead(GB_ROMFile, &GBC->cache[rom*GB_PAGE_SIZE], len);
						if (n == len) break; // data read OK

						// error message (False = ignore or exit)
						if (!GB_ErrorMsg("Cannot read", "ROM file", True, False)) break;

						// reopen file on error
						DiskUnmount(); // unmount disk
						DiskMount(); // mount disk
#if USE_FILESEL		// use file selection (lib_filesel.c, lib_filesel.h)
						SetDir(FileSelPath); // set directory
						memcpy(&GB_ROMName[GB_ROMNameLen], GB_ROMNameExt, 5); // add extension
						FileOpen(GB_ROMFile, GB_ROMName); // re-open file
#endif
					}
				}

				// clear rest of data
				if (len < GB_PAGE_SIZE) memset(&GBC->cache[rom*GB_PAGE_SIZE + len], 0xff, GB_PAGE_SIZE - len);

				// mark this offset
				GBC->cache_list[rom].off = off;
			}

			// access to this page
			GBC->rom[pg] = rom;
		}

		// mark last time
		GBC->cache_list[rom].last = Time();

		// get ROM byte
		u8* s = &GBC->cache[rom*GB_PAGE_SIZE]; // page address
		return s[addr & GB_PAGE_MASK];
	}

	// video VRAM
	if (addr < 0xa000)
	{
		return GBC->vram_map[addr - 0x8000];
	}

	// external XRAM
	if (addr < 0xc000)
	{
		// if has extern XRAM and if XRAM is enable
		if ((GBC->mbc_xram != 0) && (GBC->mbc_xram_en != 0))
		{
			// MBC2: only 9-bit address and only 4 bits
			if (GBC->mbc == 2)
				return GBC->xram_map[addr & 0x1ff] & 0x0f;
			else
				return GBC->xram_map[addr - 0xa000];
		}
		else
			return 0xff;
	}

	// working WRAM area
	if (addr < 0xfe00)
	{
		// area 0xc000-0xdfff, 0xe000-0xffff -> mask to 8 KB area
		addr &= 0x1fff;

		// bank 0
		if (addr < 0x1000)
		{
			return GBC->wram[addr];
		}

		// bank 1-7
		return GBC->wram_map[addr - 0x1000];
	}

	// OAM-RAM
	if (addr < 0xff00)
	{
		return GBC->oam[addr - 0xfe00];
	}

	// IO ports
	addr -= 0xff00;
	u8 d = GBC->hram[addr];

	// special cases
	switch (addr)
	{
	// 0x00: get controller data
	case GB_IO_P1:
		d |= 0x0f;
		if ((d & B4) == 0) d &= GB_Key14;
		if ((d & B5) == 0) d &= GB_Key15;
		break;

	// 0x01: serial transfer data (Emulator: Not supported)
//	case GB_IO_SB:
//		break;

	// 0x02: serial transfer control (Emulator: Not supported)
//	case GB_IO_SC:
//		break;

	// 0x04: divider
	case GB_IO_DIV:
		d = (u8)(GBC->cpu.div >> 8);
		break;

	// 0x05: timer counter
	case GB_IO_TIMA:
		d = GBC->cpu.tima;
		break;

	// 0x06: timer modulo
	case GB_IO_TMA:
		d = GBC->cpu.tma;
		break;

	// 0x07: timer control
	//    B0,B1: input clock select 0=f/2^10 (4096 Hz), 1=f/2^4 (262144 Hz),
	//           2=f/f^6 (65536 Hz), 3=f/2^8 (16384 Hz)
	//    B2: 1=start timer, 0=stop timer
	case GB_IO_TAC:
		d = GBC->hram[GB_IO_TAC] | 0xf8;
		break;

	// 0x0F: interrupt request
	//    B0: vertical blanking (jump to 0x0040)
	//    B1: LCDC status (jump to 0x0048)
	//    B2: timer overflow (jump to 0x0050)
	//    B3: serial I/O transfer completion (jump to 0x0058)
	//    B4: P10-P13 terminal negative edge (jump to 0x0060)
	case GB_IO_IF:
		d = (u8)(EmuInterGet(&GBC->cpu.sync) | 0xe0);
		break;

	// 0x41: LCD status
	//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
	//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
	//    B3: HSYNC status interrupt
	//    B4: VSYNC status interrupt
	//    B5: searching OAM interrupt
	//    B6: LYC==LY status interrupt
	case GB_IO_STAT:
		// prepare match status
		d |= B2 | B7; // LYC == LCDC LY
		if (GBC->hram[GB_IO_LYC] != GBC->hram[GB_IO_LY]) d ^= B2; // LYC != LCDC LY
		break;

	// 0x4D: set CPU speed
	case GB_IO_KEY1:
		d &= ~B7;
		if (GBC->freqdbl) d |= B7;
		break;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	// 0x4F: VRAM bank
	case GB_IO_VBK:
		d = GBC->vram_sel | 0xfe;
		break;

	// 0x51: DMA source HIGH
	case GB_IO_HDMA1:
		d = (u8)(GBC->dmasrc >> 8);
		break;

	// 0x52: DMA source LOW
	case GB_IO_HDMA2:
		d = (u8)(GBC->dmasrc & 0xf0);
		break;

	// 0x53: DMA destination HIGH
	case GB_IO_HDMA3:
		d = (u8)(GBC->dmadst >> 8);
		break;

	// 0x54: DMA destination LOW
	case GB_IO_HDMA4:
		d = (u8)(GBC->dmadst & 0xf0);
		break;

	// 0x55: DMA control
	//    B0-B6: number of lines = n+1 (number of bytes 16*(n+1))
	//    B7: 1=horizontal blanking DMA transfer, 0=general purpose DMA transfer
	case GB_IO_HDMA5:
		d = (u8)((GBC->dmanotrun << 7) | (GBC->dmasize - 1));
		break;

	// 0x56: infrared port (Emulator: Not supported)
//	case GB_IO_RP:
//		break;

	// 0x68: BG palette index
	case GB_IO_BCPS:
		d = (GBC->bgpal_id & 0x3f) | (GBC->bgpal_inc << 7);
		break;

	// 0x69: BG palette
	case GB_IO_BCPD:
		d = GBC->bg_pal2[GBC->bgpal_id & 0x3f];
		break;

	// 0x6A: Sprite palette index
	case GB_IO_OCPS:
		d = (GBC->sppal_id & 0x3f) | (GBC->sppal_inc << 7);
		break;

	// 0x6B: Sprite palette
	case GB_IO_OCPD:
		d = GBC->sp_pal2[GBC->sppal_id & 0x3f];
		break;

	// 0x70: select working WRAM bank 0xD000-0xDFFF
	case GB_IO_SVBK:
		d = GBC->wram_sel;
		break;
#endif

	// 0xFF: interrupt enable
	case GB_IO_IE:
		d = GBC->cpu.ie;
		break;
	}

	return d;
}

// select ROM bank
void FASTCODE NOFLASH(GB_RomSelect)(u16 sel)
{
	sel = sel & (GBC->mbc_rom_num - 1); // mask with MAX value
	GBC->rom_sel = sel;
	GBC->rom_base = (sel <= 1) ? 0x4000 : (sel * 0x4000); // set ROM base
	memset(&GBC->rom[GB_ROM_PAGENUM/2], GB_ROM_PAGEINV, GB_ROM_PAGENUM/2); // invalidate ROM addresses
}

// select XRAM bank
void FASTCODE NOFLASH(GB_XRamSelect)(u8 sel)
{
	if (GBC->mbc_xram_num == 0)
		sel = 0;
	else
		sel &= (GBC->mbc_xram_num - 1); // mask with MAX value

	GBC->xram_sel = sel;
	GBC->xram_map = &GBC->xram[sel * 0x2000];
}

// timer control trigger shift
const u8 GB_TrigShift[4] = { 10, 4, 6, 8 };

// write memory
void FASTCODE NOFLASH(GB_SetMem)(u16 addr, u8 data)
{
	// ROM MBC control registers
	if (addr < 0x8000)
	{
		// index of ROM MBC control register
		u8 reg = (u8)(addr >> 12);
		switch (reg)
		{
		// 0x0000-0x1FFF: enable extended XRAM (except MBC2)
		case 0:
		case 1:
			if ((GBC->mbc > 0) && (GBC->mbc != 2) && (GBC->mbc_xram != 0))
			{
				if (GBC->mbc == 5)
					GBC->mbc_xram_en = (data == 0x0A);	// 1=extended XRAM is enabled
				else
					GBC->mbc_xram_en = ((data & 0x0f) == 0x0A);	// 1=extended XRAM is enabled
				return;
			}

/*
			if (GBC->mbc != 2)
			{
				if ((GBC->mbc > 0) && (GBC->mbc_xram != 0))
				{
					GBC->mbc_xram_en = ((data & 0x0f) == 0x0A);	// 1=extended XRAM is enabled
				}
				return;
			}
*/
			// MBC2: fall-through

		// 0x2000-0x2FFF: MBC5 select ROM bank, lower 8 bits
		case 2:
			if (GBC->mbc == 5)
			{
				GBC->rom_sel = (GBC->rom_sel & 0x100) | data; // mask with MAX value
				GBC->rom_base = GBC->rom_sel * 0x4000; // set ROM base
				memset(&GBC->rom[GB_ROM_PAGENUM/2], GB_ROM_PAGEINV, GB_ROM_PAGENUM/2); // invalidate ROM addresses
				//GB_RomSelect((GBC->rom_sel & 0x100) | data);
				return;
			}
			// not MBC5: fall-through

		// 0x3000-0x3FFF: select ROM bank register
		case 3:
			// MBC1: select ROM bank, lower 5 bits
			if (GBC->mbc == 1)
			{
				GBC->rom_sel = (data & 0x1f) | (GBC->rom_sel & 0x60);
				if ((GBC->rom_sel & 0x1f) == 0) GBC->rom_sel++; // simulate MBC1 error
				GBC->rom_base = GBC->rom_sel * 0x4000; // set ROM base
				memset(&GBC->rom[GB_ROM_PAGENUM/2], GB_ROM_PAGEINV, GB_ROM_PAGENUM/2); // invalidate ROM addresses
			}

			// MBC2: if bit 8 is set - select ROM bank, if bit 8 is cleared - enable XRAM
			else if (GBC->mbc == 2)
			{
				// bit 8 is set - select ROM bank
				if ((addr & 0x100) != 0)
				{
					GB_RomSelect(data & 0x0f);
				}

				// bit 8 is cleared - enable XRAM
				else
				{
					GBC->mbc_xram_en = ((data & 0x0f) == 0x0A);	// 1=extended XRAM is enabled
				}
			}

			// MBC3: 
			else if (GBC->mbc == 3)
			{
				GB_RomSelect(data & 0x7f);
			}

			// NBC5: select ROM bank, upper 1 bit
			else if (GBC->mbc == 5)
			{
				GBC->rom_sel = (GBC->rom_sel & 0xff) | ((u16)(data & 1) << 8); // mask with MAX value
				GBC->rom_base = GBC->rom_sel * 0x4000; // set ROM base
				memset(&GBC->rom[GB_ROM_PAGENUM/2], GB_ROM_PAGEINV, GB_ROM_PAGENUM/2); // invalidate ROM addresses
				//GB_RomSelect((GBC->rom_sel & 0xff) | ((u16)(data & 1) << 8));
			}
			return;

		// 0x4000-0x5FFF: select RAM bank
		case 4:
		case 5:
			// MBC1: select ROM bank, upper 2 bits, or select RAM bank
			if (GBC->mbc == 1)
			{
				data &= 3;
				GB_RomSelect((GBC->rom_sel & 0x1f) | (data << 5)); // select ROM

				// select ROM and RAM
				if (GBC->bank_sel_mode != 0) GB_XRamSelect(data);
			}

			// MBC3: select RAM bank
			else if (GBC->mbc == 3)
			{
				GB_XRamSelect(data & 3);
			}

			// MBC5: select RAM bank
			else if (GBC->mbc == 5)
			{
				GB_XRamSelect(data & 0x0f);
			}
			return;

		// 0x6000-0x7FFF: extra
		case 6:
		case 7:
			// MBC1: bank selection mode
			if (GBC->mbc == 1)
			{
				GBC->bank_sel_mode = data & 1;	// MBC1: bank selection mode 0=ROM, 1=ROM+RAM
			}
			return;
		}
	}

	// video VRAM
	if (addr < 0xa000)
	{
		GBC->vram_map[addr - 0x8000] = data;
		return;;
	}

	// external XRAM
	if (addr < 0xc000)
	{
		// if has extern XRAM and if XRAM is enable
		if ((GBC->mbc_xram != 0) && (GBC->mbc_xram_en != 0))
		{
			// MBC2: only 9-bit address and only 4 bits
			if (GBC->mbc == 2)
				GBC->xram_map[addr & 0x1ff] = data & 0x0f;
			else
				GBC->xram_map[addr - 0xa000] = data;
		}
		return;
	}

	// working WRAM area
	if (addr < 0xfe00)
	{
		// area 0xc000-0xdfff, 0xe000-0xffff -> mask to 8 KB area
		addr &= 0x1fff;

		// bank 0
		if (addr < 0x1000)
		{
			GBC->wram[addr] = data;
			return;
		}

		// bank 1-7
		GBC->wram_map[addr - 0x1000] = data;
		return;
	}

	// OAM-RAM
	if (addr < 0xff00)
	{
		GBC->oam[addr - 0xfe00] = data;
		return;
	}

	// IO ports
	addr -= 0xff00;

	// special cases
	switch (addr)
	{
	// 0x00: controller data
	case GB_IO_P1:
		data = (data & (B4+B5)) | (GBC->hram[GB_IO_P1] & ~(B4+B5));
		break;

	// 0x01: serial transfer data (Emulator: Not supported)
//	case GB_IO_SB:
//		break;

	// 0x02: serial transfer control (Emulator: Not supported)
//	case GB_IO_SC:
//		break;

	// 0x04: reset divider
	case GB_IO_DIV:
		data = 0;
		GBC->cpu.div = 0;
		break;

	// 0x05: timer counter
	case GB_IO_TIMA:
		GBC->cpu.tima = data;
		break;

	// 0x06: timer modulo
	case GB_IO_TMA:
		GBC->cpu.tma = data;
		break;

	// 0x07: timer control
	//    B0,B1: input clock select 0=f/2^10 (4096 Hz), 1=f/2^4 (262144 Hz),
	//           2=f/f^6 (65536 Hz), 3=f/2^8 (16384 Hz)
	//    B2: 1=start timer, 0=stop timer
	case GB_IO_TAC:
		// setup timer
		if ((data & B2) == 0) // disable timer
			GBC->cpu.divshift = 255;
		else
			GBC->cpu.divshift = GB_TrigShift[data & 3];
		break;

	// 0x0F: interrupt request
	//    B0: vertical blanking (jump to 0x0040)
	//    B1: LCDC status (jump to 0x0048)
	//    B2: timer overflow (jump to 0x0050)
	//    B3: serial I/O transfer completion (jump to 0x0058)
	//    B4: P10-P13 terminal negative edge (jump to 0x0060)
	case GB_IO_IF:
		EmuInterSet(&GBC->cpu.sync, data & X80_IF_ALL);
		break;

	// 0x40: LCD control register
	// (default 0x83 = BG/OBJ ON, LCDC operation)
	//    B0: 1=BG display on (only GB; GBC is always on)
	//    B1: 1=OBJ is on
	//    B2: 1=8x16 dots, 0=8x8 dots
	//    B3: code area 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B4: character data 1=0x8000-0x8FFF, 0=0x8800-0x97FF
	//    B5: 1=windowing on
	//    B6: window code are selection 1=0x9C00-0x9FFF, 0=0x9800-0x9BFF
	//    B7: LCDC controller operation stop 1=LCDC on, 0=LCDC off
	case GB_IO_LCDC:
		{
			// check change of display enabled state
			if (((GBC->hram[GB_IO_LCDC] ^ data) & B7) != 0)
			{
				// GB_IO_STAT
				//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
				//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
				//    B3: HSYNC status interrupt
				//    B4: VSYNC status interrupt
				//    B5: searching OAM interrupt
				//    B6: LYC==LY status interrupt

				// set LCD to mode 0 (start HSYNC)
				GBC->hram[GB_IO_STAT] &= ~3;

				// reset LY
				GBC->hram[GB_IO_LY] = 0;

				// reset LCD timer
				GBC->cpu.lcd_count = 0;

				// LCD is on
				if ((data & B7) != 0)
				{
					// LYC update
					if (GBC->hram[GB_IO_LY] == GBC->hram[GB_IO_LYC]) // LY == LYC
					{
						// set status
						GBC->hram[GB_IO_STAT] |= B2; // set status

						// raise LCDC status interrupt (if required)
						if ((GBC->hram[GB_IO_STAT] & B6) != 0) X80_RaiseIRQ(&GBC->cpu, X80_IRQ_LCDC);
					}
					else
						GBC->hram[GB_IO_STAT] &= ~B2; // clear status

					// raise LCDC status interrupt (if required)
					if ((GBC->hram[GB_IO_STAT] & B3) != 0) X80_RaiseIRQ(&GBC->cpu, X80_IRQ_LCDC);
				}
			}
		}
		break;

	// 0x41: LCD status
	//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
	//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
	//    B3: HSYNC status interrupt
	//    B4: VSYNC status interrupt
	//    B5: searching OAM interrupt
	//    B6: LYC==LY status interrupt
	case GB_IO_STAT:
		// change only user bits B3..B6
		data = (data & 0xf8) | (GBC->hram[GB_IO_STAT] & 7);
		break;

	// 0x44: current Y line
	case GB_IO_LY:
		return; // register is read-only (do not write)

	// 0x46: DMA register
	case GB_IO_DMA:
		{
			// transfer data from WRAM to OAM
//#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
//			data = data % 0xf1;
//#endif
			u16 addr = (u16)data << 8; // source address 0x0000-0xFF00, increments 0x100
			int i;
			for (i = 0; i < GB_OAM_SIZE; i++)
			{
				GBC->oam[i] = GBC->cpu.readmem(addr);
				addr++;
			}
		}
		break;

	// 0x47: BG palette
	case GB_IO_BGP:
		GBC->bg_pal[0] = data & 3;
		GBC->bg_pal[1] = (data >> 2) & 3;
		GBC->bg_pal[2] = (data >> 4) & 3;
		GBC->bg_pal[3] = (data >> 6) & 3;
		break;

	// 0x48: sprite palette 0
	case GB_IO_OBP0:
		GBC->sp_pal[0] = data & 3;
		GBC->sp_pal[1] = (data >> 2) & 3;
		GBC->sp_pal[2] = (data >> 4) & 3;
		GBC->sp_pal[3] = (data >> 6) & 3;
		break;

	// 0x49: sprite palette 1
	case GB_IO_OBP1:
		GBC->sp_pal[4] = data & 3;
		GBC->sp_pal[5] = (data >> 2) & 3;
		GBC->sp_pal[6] = (data >> 4) & 3;
		GBC->sp_pal[7] = (data >> 6) & 3;
		break;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	// 0x4F: VRAM bank
	case GB_IO_VBK:
		GBC->vram_sel = data & 1;
		if (GBC->gbmode != GB_MODE_GB) GBC->vram_map = &GBC->vram[GBC->vram_sel * 0x2000];
		break;

	// 0x51: DMA source HIGH
	case GB_IO_HDMA1:
		GBC->dmasrc = (GBC->dmasrc & 0xff) + ((u16)data << 8);
		break;

	// 0x52: DMA source LOW
	case GB_IO_HDMA2:
		GBC->dmasrc = (GBC->dmasrc & 0xff00) + (data & 0xf0);
		break;

	// 0x53: DMA destination HIGH
	case GB_IO_HDMA3:
		GBC->dmadst = (GBC->dmadst & 0xff) + ((u16)data << 8);
		break;

	// 0x54: DMA destination LOW
	case GB_IO_HDMA4:
		GBC->dmadst = (GBC->dmadst & 0xff00) + (data & 0xf0);
		break;

	// 0x55: DMA control
	//    B0-B6: number of lines = n+1 (number of bytes 16*(n+1))
	//    B7: 1=horizontal blanking DMA transfer, 0=general purpose DMA transfer
	case GB_IO_HDMA5:
		GBC->dmasize = (data & 0x7f) + 1; // number of lines to transfer
		GBC->dmamode = data >> 7; // DMA mode; 1=horizontal blanking DMA transfer, 0=general purpose DMA transfer

		// process general transfer if DMA not active
		if (GBC->dmanotrun && (GBC->gbmode != GB_MODE_GB) && (GBC->dmamode == 0))
		{
			int n = (int)GBC->dmasize << 4; // number of bytes to transfer
			u16 dst = (GBC->dmadst & 0x1ff0) | 0x8000;
			u16 src = GBC->dmasrc & 0xfff0;
			GBC->dmadst += n;
			GBC->dmasrc += n;
			GBC->dmasize = 0;
			for (; n > 0; n--) GBC->cpu.writemem(dst++, GBC->cpu.readmem(src++));
		}

		// start horizontal blanking DMA transfer
		GBC->dmanotrun = GBC->dmamode ^ 1;
		break;

	// 0x56: infrared port (Emulator: Not supported)
//	case GB_IO_RP:
//		break;

	// 0x68: BG palette index
	case GB_IO_BCPS:
		GBC->bgpal_id = data & 0x3f;
		GBC->bgpal_inc = data >> 7;
		break;

	// 0x69: BG palette
	case GB_IO_BCPD:
		{
			int id = GBC->bgpal_id & 0x3f; // get palette index
			GBC->bg_pal2[id] = data; // save palette
			if (GBC->bgpal_inc != 0) GBC->bgpal_id = (id + 1) & 0x3f; // auto increment index

			id &= 0x3e; // base index
			u16 pal = ((u16)GBC->bg_pal2[id + 1] << 8) + GBC->bg_pal2[id]; // get this palette
			pal = ((pal & 0x7c00) >> 10) | ((pal & 0x03e0) << 1) | ((pal & 0x0200) >> 4) | ((pal & 0x001f) << 11); // convert to RGB565
			GBC->fixpal[id >> 1] = pal;
		}
		break;

	// 0x6A: Sprite palette index
	case GB_IO_OCPS:
		GBC->sppal_id = data & 0x3f;
		GBC->sppal_inc = data >> 7;
		break;

	// 0x6B: Sprite palette
	case GB_IO_OCPD:
		{
			int id = GBC->sppal_id & 0x3f; // get palette index
			GBC->sp_pal2[id] = data; // save palette
			if (GBC->sppal_inc != 0) GBC->sppal_id = (id + 1) & 0x3f; // auto increment index

			id &= 0x3e; // base index
			u16 pal = ((u16)GBC->sp_pal2[id + 1] << 8) + GBC->sp_pal2[id]; // get this palette
			pal = ((pal & 0x7c00) >> 10) | ((pal & 0x03e0) << 1) | ((pal & 0x0200) >> 4) | ((pal & 0x001f) << 11); // convert to RGB565
			GBC->fixpal[(id >> 1) + 0x20] = pal;
		}
		break;

	// 0x70: select working WRAM bank 0xD000-0xDFFF
	case GB_IO_SVBK:
		{
			u8 bank = data & 7;
			GBC->wram_sel = data;
			if (bank == 0) bank = 1;
			GBC->wram_map = &GBC->wram[bank * GB_WRAM_BANK_SIZE];
		}
		break;
#endif

	// 0xFF: interrupt enable
	case GB_IO_IE:
		GBC->cpu.ie = data;
		break;
	}

	// write new value
	GBC->hram[addr] = data;
}
