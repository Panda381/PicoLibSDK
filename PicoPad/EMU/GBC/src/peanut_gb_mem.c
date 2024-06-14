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

// Internal function used to read bytes.
u8 FASTCODE NOFLASH(__gb_read)(struct gb_s *gb, u16 addr)
{
	switch(PEANUT_GB_GET_MSN16(addr))
	{
	case 0x0:
		// IO_BANK is only set to 1 if gb->gb_bootrom_read was not NULL on reset.
		if(gb->hram_io[IO_BANK] == 0 && addr < 0x0100)
		{
			return gb->gb_bootrom_read(gb, addr);
		}

		// Fallthrough
	case 0x1:
	case 0x2:
	case 0x3:
		return gb->gb_rom_read(gb, addr);

	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		if(gb->mbc == MBC1 && gb->cart_mode_select)
			return gb->gb_rom_read(gb, addr + ((gb->selected_rom_bank & 0x1F) - 1) * ROM_BANK_SIZE);
		else
			return gb->gb_rom_read(gb, addr + (gb->selected_rom_bank - 1) * ROM_BANK_SIZE);

	case 0x8:
	case 0x9:
		return gb->vram[addr - gb->cgb.vramBankOffset];

	case 0xA:
	case 0xB:
		if(gb->mbc == MBC3 && gb->cart_ram_bank >= 0x08)
		{
			return gb->cart_rtc[gb->cart_ram_bank - 0x08];
		}
		else if(gb->cart_ram && gb->enable_cart_ram)
		{
			if(gb->mbc == MBC2)
			{
				// Only 9 bits are available in address.
				addr &= 0x1FF;
				return gb->gb_cart_ram_read(gb, addr);
			}
			else if((gb->cart_mode_select || gb->mbc != MBC1) &&
					gb->cart_ram_bank < gb->num_ram_banks)
			{
				return gb->gb_cart_ram_read(gb, addr - CART_RAM_ADDR +
							    (gb->cart_ram_bank * CRAM_BANK_SIZE));
			}
			else
				return gb->gb_cart_ram_read(gb, addr - CART_RAM_ADDR);
		}

		return 0xFF;

	case 0xC:
	case 0xD:
	if(gb->cgb.cgbMode && addr >= WRAM_1_ADDR)
		return gb->wram[addr - gb->cgb.wramBankOffset];
	else
		return gb->wram[addr - WRAM_0_ADDR];

	case 0xE:
		return gb->wram[addr - ECHO_ADDR];

	case 0xF:
		if(addr < OAM_ADDR)
			return gb->wram[(addr - 0x2000) - gb->cgb.wramBankOffset];

		if(addr < UNUSED_ADDR)
			return gb->oam[addr - OAM_ADDR];

		// Unusable memory area. Reading from this area returns 0xFF.
		if(addr < IO_ADDR)
			return 0xFF;

		// APU registers.
		if((addr >= 0xFF10) && (addr <= 0xFF3F))
		{
			return audio_read(addr);
		}

		// IO and Interrupts.
		switch (addr & 0xFF)
		{
		// Speed Switch
		case 0x4D:
			return (gb->cgb.doubleSpeed << 7) + gb->cgb.doubleSpeedPrep;
		// CGB VRAM Bank
		case 0x4F:
			return gb->cgb.vramBank | 0xFE;
		/* CGB DMA*/
		case 0x51:
			return (gb->cgb.dmaSource >> 8);
		case 0x52:
			return (gb->cgb.dmaSource & 0xF0);
		case 0x53:
			return (gb->cgb.dmaDest >> 8);
		case 0x54:
			return (gb->cgb.dmaDest & 0xF0);
		case 0x55:
			return (gb->cgb.dmaSize - 1) | (gb->cgb.dmaRunning ? 0 : 0x80);
		// IR Register
		case 0x56:
			return gb->hram_io[0x56];
		// CGB BG Palette Index
		case 0x68:
			return (gb->cgb.BGPaletteID & 0x3F) + (gb->cgb.BGPaletteInc << 7);
		// CGB BG Palette
		case 0x69:
			return gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3F)];
		// CGB OAM Palette Index
		case 0x6A:
			return (gb->cgb.OAMPaletteID & 0x3F) + (gb->cgb.OAMPaletteInc << 7);
		// CGB OAM Palette
		case 0x6B:
			return gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3F)];
		// CGB WRAM Bank
		case 0x70:
			return gb->cgb.wramBank;
		default:
			// HRAM 
			if(addr >= IO_ADDR)
				return gb->hram_io[addr - IO_ADDR];
		}
	}

	// invalid address
	return 0xff;
}

// Internal function used to write bytes.
void FASTCODE NOFLASH(__gb_write)(struct gb_s *gb, uint_fast16_t addr, u8 val)
{
	switch(PEANUT_GB_GET_MSN16(addr))
	{
	case 0x0:
	case 0x1:
		// Set RAM enable bit. MBC2 is handled in fall-through.
		if(gb->mbc > MBC0 && gb->mbc != MBC2 && gb->cart_ram)
		{
			gb->enable_cart_ram = ((val & 0x0F) == 0x0A);
			return;
		}

	// Intentional fall through.
	case 0x2:
		if(gb->mbc == MBC5)
		{
			gb->selected_rom_bank = (gb->selected_rom_bank & 0x100) | val;
			gb->selected_rom_bank =
				gb->selected_rom_bank & gb->num_rom_banks_mask;
			return;
		}

	// Intentional fall through.
	case 0x3:
		if(gb->mbc == MBC1)
		{
			//selected_rom_bank = val & 0x7;
			gb->selected_rom_bank = (val & 0x1F) | (gb->selected_rom_bank & 0x60);

			if((gb->selected_rom_bank & 0x1F) == 0x00)
				gb->selected_rom_bank++;
		}
		else if(gb->mbc == MBC2)
		{
			// If bit 8 is 1, then set ROM bank number.
			if(addr & 0x100)
			{
				gb->selected_rom_bank = val & 0x0F;
				// Setting ROM bank to 0, sets it to 1.
				if(!gb->selected_rom_bank)
					gb->selected_rom_bank++;
			}
			// Otherwise set whether RAM is enabled or not.
			else
			{
				gb->enable_cart_ram = ((val & 0x0F) == 0x0A);
				return;
			}
		}
		else if(gb->mbc == MBC3)
		{
			gb->selected_rom_bank = val & 0x7F;

			if(!gb->selected_rom_bank)
				gb->selected_rom_bank++;
		}
		else if(gb->mbc == MBC5)
			gb->selected_rom_bank = (val & 0x01) << 8 | (gb->selected_rom_bank & 0xFF);

		gb->selected_rom_bank = gb->selected_rom_bank & gb->num_rom_banks_mask;
		return;

	case 0x4:
	case 0x5:
		if(gb->mbc == MBC1)
		{
			gb->cart_ram_bank = (val & 3);
			gb->selected_rom_bank = ((val & 3) << 5) | (gb->selected_rom_bank & 0x1F);
			gb->selected_rom_bank = gb->selected_rom_bank & gb->num_rom_banks_mask;
		}
		else if(gb->mbc == MBC3)
			gb->cart_ram_bank = val;
		else if(gb->mbc == MBC5)
			gb->cart_ram_bank = (val & 0x0F);

		return;

	case 0x6:
	case 0x7:
		gb->cart_mode_select = (val & 1);
		return;

	case 0x8:
	case 0x9:
		gb->vram[addr - gb->cgb.vramBankOffset] = val;
		return;

	case 0xA:
	case 0xB:
		if(gb->mbc == MBC3 && gb->cart_ram_bank >= 0x08)
		{
			gb->cart_rtc[gb->cart_ram_bank - 0x08] = val;
		}
		// Do not write to RAM if unavailable or disabled.
		else if(gb->cart_ram && gb->enable_cart_ram)
		{
			if(gb->mbc == MBC2)
			{
				// Only 9 bits are available in address.
				addr &= 0x1FF;
				// Data is only 4 bits wide in MBC2 RAM.
				val &= 0x0F;
				gb->gb_cart_ram_write(gb, addr, val);
			}
			else if(gb->cart_mode_select &&
					gb->cart_ram_bank < gb->num_ram_banks)
			{
				gb->gb_cart_ram_write(gb,
						      addr - CART_RAM_ADDR + (gb->cart_ram_bank * CRAM_BANK_SIZE), val);
			}
			else if(gb->num_ram_banks)
				gb->gb_cart_ram_write(gb, addr - CART_RAM_ADDR, val);
		}

		return;

	case 0xC:
		gb->wram[addr - WRAM_0_ADDR] = val;
		return;

	case 0xD:
		gb->wram[addr - gb->cgb.wramBankOffset] = val;
		return;

	case 0xE:
		gb->wram[addr - ECHO_ADDR] = val;
		return;

	case 0xF:
		if(addr < OAM_ADDR)
		{
			gb->wram[(addr - 0x2000) - gb->cgb.wramBankOffset] = val;
			return;
		}

		if(addr < UNUSED_ADDR)
		{
			gb->oam[addr - OAM_ADDR] = val;
			return;
		}

		// Unusable memory area.
		if(addr < IO_ADDR)
			return;

		if(HRAM_ADDR <= addr && addr < INTR_EN_ADDR)
		{
			gb->hram_io[addr - IO_ADDR] = val;
			return;
		}

		if((addr >= 0xFF10) && (addr <= 0xFF3F))
		{
			audio_write(addr, val);
			return;
		}

		u16 fixPaletteTemp;

		// IO and Interrupts.
		switch(PEANUT_GB_GET_LSB16(addr))
		{
		// Joypad
		case 0x00:
			// Only bits 5 and 4 are R/W.
			// The lower bits are overwritten later, and the two most
			// significant bits are unused. 
			gb->hram_io[IO_JOYP] = val;

			// Direction keys selected
			if((gb->hram_io[IO_JOYP] & 0x10) == 0)
				gb->hram_io[IO_JOYP] |= (gb->direct.joypad >> 4);
			// Button keys selected
			else
				gb->hram_io[IO_JOYP] |= (gb->direct.joypad & 0x0F);

			return;

		// Serial
		case 0x01:
			gb->hram_io[IO_SB] = val;
			return;

		case 0x02:
			gb->hram_io[IO_SC] = val;
			return;

		// Timer Registers
		case 0x04:
			gb->hram_io[IO_DIV] = 0x00;
			return;

		case 0x05:
			gb->hram_io[IO_TIMA] = val;
			return;

		case 0x06:
			gb->hram_io[IO_TMA] = val;
			return;

		case 0x07:
			gb->hram_io[IO_TAC] = val;
			return;

		// Interrupt Flag Register
		case 0x0F:
			gb->hram_io[IO_IF] = (val | 0xE0);
			return;

		// LCD Registers
		case 0x40:
		{
			u8 lcd_enabled;

			// Check if LCD is already enabled.
			lcd_enabled = (gb->hram_io[IO_LCDC] & LCDC_ENABLE);

			gb->hram_io[IO_LCDC] = val;

			// Check if LCD is going to be switched on.
			if (!lcd_enabled && (val & LCDC_ENABLE))
			{
				gb->lcd_blank = 1;
			}
			// Check if LCD is being switched off.
			else if (lcd_enabled && !(val & LCDC_ENABLE))
			{
				// Peanut-GB will happily turn off LCD outside
				// of VBLANK even though this damages real
				// hardware.

				// Set LCD to Mode 0.
				gb->hram_io[IO_STAT] =
					(gb->hram_io[IO_STAT] & ~STAT_MODE) |
					IO_STAT_MODE_HBLANK;
				// LY fixed to 0 when LCD turned off.
				gb->hram_io[IO_LY] = 0;
				// Reset LCD timer.
				gb->counter.lcd_count = 0;
			}
			return;
		}

		case 0x41:
			gb->hram_io[IO_STAT] = (val & STAT_USER_BITS) | (gb->hram_io[IO_STAT] & STAT_MODE);
			return;

		case 0x42:
			gb->hram_io[IO_SCY] = val;
			return;

		case 0x43:
			gb->hram_io[IO_SCX] = val;
			return;

		// LY (0xFF44) is read only.
		case 0x45:
			gb->hram_io[IO_LYC] = val;
			return;

		// DMA Register
		case 0x46:
		{
			u16 dma_addr;
			u16 i;
			dma_addr = (uint_fast16_t)val << 8;
			gb->hram_io[IO_DMA] = val;

			for(i = 0; i < OAM_SIZE; i++)
			{
				gb->oam[i] = __gb_read(gb, dma_addr + i);
			}

			return;
		}

		// DMG Palette Registers
		case 0x47:
			gb->hram_io[IO_BGP] = val;
			gb->display.bg_palette[0] = (gb->hram_io[IO_BGP] & 0x03);
			gb->display.bg_palette[1] = (gb->hram_io[IO_BGP] >> 2) & 0x03;
			gb->display.bg_palette[2] = (gb->hram_io[IO_BGP] >> 4) & 0x03;
			gb->display.bg_palette[3] = (gb->hram_io[IO_BGP] >> 6) & 0x03;
			return;

		case 0x48:
			gb->hram_io[IO_OBP0] = val;
			gb->display.sp_palette[0] = (gb->hram_io[IO_OBP0] & 0x03);
			gb->display.sp_palette[1] = (gb->hram_io[IO_OBP0] >> 2) & 0x03;
			gb->display.sp_palette[2] = (gb->hram_io[IO_OBP0] >> 4) & 0x03;
			gb->display.sp_palette[3] = (gb->hram_io[IO_OBP0] >> 6) & 0x03;
			return;

		case 0x49:
			gb->hram_io[IO_OBP1] = val;
			gb->display.sp_palette[4] = (gb->hram_io[IO_OBP1] & 0x03);
			gb->display.sp_palette[5] = (gb->hram_io[IO_OBP1] >> 2) & 0x03;
			gb->display.sp_palette[6] = (gb->hram_io[IO_OBP1] >> 4) & 0x03;
			gb->display.sp_palette[7] = (gb->hram_io[IO_OBP1] >> 6) & 0x03;
			return;

		// Window Position Registers
		case 0x4A:
			gb->hram_io[IO_WY] = val;
			return;

		case 0x4B:
			gb->hram_io[IO_WX] = val;
			return;

		// Prepare Speed Switch
		case 0x4D:
			gb->cgb.doubleSpeedPrep = val & 1;
			return;

		// CGB VRAM Bank
		case 0x4F:
			gb->cgb.vramBank = val & 0x01;
			if(gb->cgb.cgbMode) gb->cgb.vramBankOffset = VRAM_ADDR - (gb->cgb.vramBank << 13);
			return;

		// Turn off boot ROM
		case 0x50:
			gb->hram_io[IO_BANK] = val;
			return;

		// DMA Register
		case 0x51:
			gb->cgb.dmaSource = (gb->cgb.dmaSource & 0xFF) + (val << 8);
			return;

		case 0x52:
			gb->cgb.dmaSource = (gb->cgb.dmaSource & 0xFF00) + val;
			return;

		case 0x53:
			gb->cgb.dmaDest = (gb->cgb.dmaDest & 0xFF) + (val << 8);
			return;

		case 0x54:
			gb->cgb.dmaDest = (gb->cgb.dmaDest & 0xFF00) + val;
			return;

		// DMA Register
		case 0x55:
			gb->cgb.dmaSize = (val & 0x7F) + 1;
			gb->cgb.dmaMode = val >> 7;
			// DMA GBC
			if(!gb->cgb.dmaRunning)
			{  // Only transfer if dma is not active (=1) otherwise treat it as a termination
				if(gb->cgb.cgbMode && (!gb->cgb.dmaMode))
				{
					for (int i = 0; i < (gb->cgb.dmaSize << 4); i++)
					{
						__gb_write(gb, ((gb->cgb.dmaDest & 0x1FF0) | 0x8000) + i,
							 __gb_read(gb, (gb->cgb.dmaSource & 0xFFF0) + i));
					}
					gb->cgb.dmaSource += (gb->cgb.dmaSize << 4);
					gb->cgb.dmaDest += (gb->cgb.dmaSize << 4);
					gb->cgb.dmaSize = 0;
				}
			}
			gb->cgb.dmaRunning = gb->cgb.dmaMode;  // set active if it's an HBlank DMA
			return;

		// IR Register
		case 0x56:
			gb->hram_io[0x56] = val;
			return;

		// CGB BG Palette Index
		case 0x68:
			gb->cgb.BGPaletteID = val & 0x3F;
			gb->cgb.BGPaletteInc = val >> 7;
			return;

		// CGB BG Palette
		case 0x69:
			gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3F)] = val;
			fixPaletteTemp = (gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3E) + 1] << 8) 
					+ (gb->cgb.BGPalette[(gb->cgb.BGPaletteID & 0x3E)]);

			// convert to RGB565
			gb->cgb.fixPalette[((gb->cgb.BGPaletteID & 0x3E) >> 1)] =
					((fixPaletteTemp & 0x7C00) >> 10) |	// blue
					((fixPaletteTemp & 0x03E0) << 1) | ((fixPaletteTemp & 0x0200) >> 4) | // green
					((fixPaletteTemp & 0x001F) << 11);	// red

			if(gb->cgb.BGPaletteInc) gb->cgb.BGPaletteID = (++gb->cgb.BGPaletteID) & 0x3F;
			return;

		// CGB OAM Palette Index
		case 0x6A:
			gb->cgb.OAMPaletteID = val & 0x3F;
			gb->cgb.OAMPaletteInc = val >> 7;
			return;

		// CGB OAM Palette
		case 0x6B:
			gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3F)] = val;
			fixPaletteTemp = (gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3E) + 1] << 8) 
					+ (gb->cgb.OAMPalette[(gb->cgb.OAMPaletteID & 0x3E)]);

			// convert to RGB565
			gb->cgb.fixPalette[0x20 + ((gb->cgb.OAMPaletteID & 0x3E) >> 1)] =
					((fixPaletteTemp & 0x7C00) >> 10) |	// blue
					((fixPaletteTemp & 0x03E0) << 1) | ((fixPaletteTemp & 0x0200) >> 4) | // green
					((fixPaletteTemp & 0x001F) << 11); 	// red

			if(gb->cgb.OAMPaletteInc) gb->cgb.OAMPaletteID = (++gb->cgb.OAMPaletteID) & 0x3F;
			return;

		// CGB WRAM Bank
		case 0x70:
			gb->cgb.wramBank = val;
			gb->cgb.wramBankOffset = WRAM_1_ADDR - (1 << 12);
			if(gb->cgb.cgbMode && (gb->cgb.wramBank & 7) > 0)
				gb->cgb.wramBankOffset = WRAM_1_ADDR - ((gb->cgb.wramBank & 7) << 12);
			return;

		// Interrupt Enable Register
		case 0xFF:
			gb->hram_io[IO_IE] = val;
			return;
		}
	}

	// Invalid writes are ignored.
	return;
}
