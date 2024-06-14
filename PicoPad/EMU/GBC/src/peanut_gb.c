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

void FASTCODE NOFLASH(gb_run_frame)(struct gb_s *gb)
{
	gb->gb_frame = 0;

	while(!gb->gb_frame)
		__gb_step_cpu(gb);
}

// Gets the size of the save file required for the ROM. (0 = no RAM or no backup battery)
uint_fast32_t gb_get_save_size(struct gb_s *gb)
{
	if (gb->battery == 0) return 0;

	const uint_fast16_t ram_size_location = 0x0149;
	const uint_fast32_t ram_sizes[] =
	{
		0x00, 0x800, 0x2000, 0x8000, 0x20000
	};
	u8 ram_size = gb->gb_rom_read(gb, ram_size_location);

	// MBC2 always has 512 half-bytes of cart RAM.
	if(gb->mbc == MBC2)
		return 0x200;

	return ram_sizes[ram_size];
}

void gb_init_serial(struct gb_s *gb,
		    void (*gb_serial_tx)(struct gb_s*, const u8),
		    enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s*,
			    u8*))
{
	gb->gb_serial_tx = gb_serial_tx;
	gb->gb_serial_rx = gb_serial_rx;
}

u8 gb_colour_hash(struct gb_s *gb)
{
#define ROM_TITLE_START_ADDR	0x0134
#define ROM_TITLE_END_ADDR	0x0143

	u8 x = 0;
	u16 i;

	for(i = ROM_TITLE_START_ADDR; i <= ROM_TITLE_END_ADDR; i++)
		x += gb->gb_rom_read(gb, i);

	return x;
}

// Resets the context, and initialises startup values for a DMG console.
void gb_reset(struct gb_s *gb)
{
	gb->gb_halt = 0;
	gb->gb_ime = 1;
	gb->gb_pause = 0;

	// Initialise MBC values.
	gb->selected_rom_bank = 1;
	gb->cart_ram_bank = 0;
	gb->enable_cart_ram = 0;
	gb->cart_mode_select = 0;

	// Use values as though the boot ROM was already executed.
	if(gb->gb_bootrom_read == NULL)
	{
		u8 hdr_chk;
		hdr_chk = gb->gb_rom_read(gb, ROM_HEADER_CHECKSUM_LOC) != 0;

		gb->cpu_reg.a = 0x01;
		gb->cpu_reg.f_bits.z = 1;
		gb->cpu_reg.f_bits.n = 0;
		gb->cpu_reg.f_bits.h = hdr_chk;
		gb->cpu_reg.f_bits.c = hdr_chk;
		gb->cpu_reg.bc.reg = 0x0013;
		gb->cpu_reg.de.reg = 0x00D8;
		gb->cpu_reg.hl.reg = 0x014D;
		gb->cpu_reg.sp.reg = 0xFFFE;
		gb->cpu_reg.pc.reg = 0x0100;

		gb->hram_io[IO_DIV ] = 0xAB;
		gb->hram_io[IO_LCDC] = 0x91;
		gb->hram_io[IO_STAT] = 0x85;
		gb->hram_io[IO_BANK] = 0x01;

		if(gb->cgb.cgbMode)
		{
			gb->cpu_reg.a = 0x11;
			gb->cpu_reg.f_bits.z = 1;
			gb->cpu_reg.f_bits.n = 0;
			gb->cpu_reg.f_bits.h = hdr_chk;
			gb->cpu_reg.f_bits.c = hdr_chk;
			gb->cpu_reg.bc.reg = 0x0000;
			gb->cpu_reg.de.reg = 0x0008;
			gb->cpu_reg.hl.reg = 0x007C;
			gb->hram_io[IO_DIV] = 0xFF;
		}

		memset(gb->vram, 0x00, VRAM_SIZE);
	}
	else
	{
		// Set value as though the console was just switched on.
		// CPU registers are uninitialised.
		gb->cpu_reg.pc.reg = 0x0000;
		gb->hram_io[IO_DIV ] = 0x00;
		gb->hram_io[IO_LCDC] = 0x00;
		gb->hram_io[IO_STAT] = 0x84;
		gb->hram_io[IO_BANK] = 0x00;
	}

	gb->counter.lcd_count = 0;
	gb->counter.div_count = 0;
	gb->counter.tima_count = 0;
	gb->counter.serial_count = 0;

	gb->direct.joypad = 0xFF;
	gb->hram_io[IO_JOYP] = 0xCF;
	gb->hram_io[IO_SB  ] = 0x00;
	gb->hram_io[IO_SC  ] = 0x7E;
	if(gb->cgb.cgbMode) gb->hram_io[IO_SC] = 0x7F;

	// DIV
	gb->hram_io[IO_TIMA] = 0x00;
	gb->hram_io[IO_TMA ] = 0x00;
	gb->hram_io[IO_TAC ] = 0xF8;
	gb->hram_io[IO_IF  ] = 0xE1;

	// LCDC
	// STAT
	gb->hram_io[IO_SCY ] = 0x00;
	gb->hram_io[IO_SCX ] = 0x00;
	gb->hram_io[IO_LY  ] = 0x00;
	gb->hram_io[IO_LYC ] = 0x00;
	__gb_write(gb, 0xFF47, 0xFC); // BGP
	__gb_write(gb, 0xFF48, 0xFF); // OBJP0
	__gb_write(gb, 0xFF49, 0xFF); // OBJP1
	gb->hram_io[IO_WY] = 0x00;
	gb->hram_io[IO_WX] = 0x00;
	gb->hram_io[IO_IE] = 0x00;
	gb->hram_io[IO_IF] = 0xE1;

	// Initialize some CGB registers
	gb->cgb.doubleSpeed = 0;
	gb->cgb.doubleSpeedPrep = 0;
	gb->cgb.wramBank = 1;
	gb->cgb.wramBankOffset = WRAM_0_ADDR;
	gb->cgb.vramBank = 0;
	gb->cgb.vramBankOffset = VRAM_ADDR;
	for (int i = 0; i < 0x20; i++)
	{
		gb->cgb.OAMPalette[(i << 1)] = gb->cgb.BGPalette[(i << 1)] = 0x7F;
		gb->cgb.OAMPalette[(i << 1) + 1] = gb->cgb.BGPalette[(i << 1) + 1] = 0xFF;
	}
	gb->cgb.OAMPaletteID = 0;
	gb->cgb.BGPaletteID = 0;
	gb->cgb.OAMPaletteInc = 0;
	gb->cgb.BGPaletteInc = 0;
	gb->cgb.dmaRunning = 0;
	gb->cgb.dmaMode = 0;
	gb->cgb.dmaSize = 0;
	gb->cgb.dmaSource = 0;
	gb->cgb.dmaDest = 0;
}

enum gb_init_error_e gb_init(struct gb_s *gb,
	     u8 (*gb_rom_read)(struct gb_s*, const uint_fast32_t),
	     u8 (*gb_cart_ram_read)(struct gb_s*, const uint_fast32_t),
	     void (*gb_cart_ram_write)(struct gb_s*, const uint_fast32_t, const u8),
	     void (*gb_error)(struct gb_s*, const enum gb_error_e, const u16),
	     void *priv)
{
	const u16 cgb_flag = 0x0143;
	const u16 mbc_location = 0x0147;
	const u16 bank_count_location = 0x0148;
	const u16 ram_size_location = 0x0149;

	// Table for cartridge type (MBC). -1 if invalid.
	//	MMM01 is untested.
	//	MBC6 is untested.
	//	MBC7 is unsupported.
	//	POCKET CAMERA is unsupported.
	//	BANDAI TAMA5 is unsupported.
	//	HuC3 is unsupported.
	//	HuC1 is unsupported.
	const s8 cart_mbc[] =
	{
		//0x00	0x01	0x02	0x03	0x04	0x05	0x06	0x07
		0,	1,	1,	1,	2,	2,	2,	0,

		//0x08	0x09	0x0A	0x0B	0x0C	0x0D	0x0E	0x0F
		0,	0,	-1,	0,	0,	0,	-1,	3,

		//0x10	0x11	0x12	0x13	0x14	0x15	0x16	0x17
		3,	3,	3,	3,	-1,	-1,	-1,	-1,

		//0x18	0x19	0x1A	0x1B	0x1C	0x1D	0x1E	0x1F
		-1,	5,	5,	5,	5,	5,	5,	-1
	};

	// If cartridge has SRAM
	const u8 cart_ram[] =
	{
		//0x00	0x01	0x02	0x03	0x04	0x05	0x06	0x07
		0,	0,	1,	1,	0,	1,	1,	0,

		//0x08	0x09	0x0A	0x0B	0x0C	0x0D	0x0E	0x0F
		1,	1,	0,	0,	0,	0,	0,	0,

		//0x10	0x11	0x12	0x13	0x14	0x15	0x16	0x17
		1,	0,	1,	1,	0,	0,	0,	0,

		//0x18	0x19	0x1A	0x1B	0x1C	0x1D	0x1E	0x1F
		0,	0,	1,	1,	0,	1,	1,	0
	};

	// number of ROM banks
	const u16 num_rom_banks_mask[] =
	{
		2, 4, 8, 16, 32, 64, 128, 256, 512
	};

	// number of CRAM 8KB banks
	const u8 num_ram_banks[] =
	{
		//0KB	8KB	8KB	32KB	128KB	64KB
		0,	1,	1,	4,	16,	8
	};

	// If cartridge has backup batter
	const u8 cart_battery[] = 
	{
		//0x00	0x01	0x02	0x03	0x04	0x05	0x06	0x07
		0,	0,	0,	1,	0,	0,	1,	0,

		//0x08	0x09	0x0A	0x0B	0x0C	0x0D	0x0E	0x0F
		0,	1,	0,	0,	0,	0,	0,	1,

		//0x10	0x11	0x12	0x13	0x14	0x15	0x16	0x17
		1,	0,	0,	1,	0,	0,	0,	0,

		//0x18	0x19	0x1A	0x1B	0x1C	0x1D	0x1E	0x1F
		0,	0,	0,	1,	0,	0,	1,	0
	};

	gb->gb_rom_read = gb_rom_read;
	gb->gb_cart_ram_read = gb_cart_ram_read;
	gb->gb_cart_ram_write = gb_cart_ram_write;
	gb->gb_error = gb_error;
	gb->direct.priv = priv;

	// Initialise serial transfer function to NULL. If the front-end does
	// not provide serial support, Peanut-GB will emulate no cable connected
	// automatically.
	gb->gb_serial_tx = NULL;
	gb->gb_serial_rx = NULL;

	gb->gb_bootrom_read = NULL;

	// Check valid ROM using checksum value.
	{
		u8 x = 0;
		u16 i;

		for(i = 0x0134; i <= 0x014C; i++)
			x = x - gb->gb_rom_read(gb, i) - 1;

		if(x != gb->gb_rom_read(gb, ROM_HEADER_CHECKSUM_LOC))
			return GB_INIT_INVALID_CHECKSUM;
	}

	// Check if cartridge type is supported, and set MBC type.
	{
		gb->cgb.cgbMode = (gb->gb_rom_read(gb, cgb_flag) & 0x80) >> 7;
		const u8 mbc_value = gb->gb_rom_read(gb, mbc_location);

		if(mbc_value > sizeof(cart_mbc) - 1 ||
				(gb->mbc = cart_mbc[mbc_value]) == -1)
			return GB_INIT_CARTRIDGE_UNSUPPORTED;
	}

	gb->cart_ram = cart_ram[gb->gb_rom_read(gb, mbc_location)];
	gb->num_rom_banks_mask = num_rom_banks_mask[gb->gb_rom_read(gb, bank_count_location)] - 1;
	gb->num_ram_banks = num_ram_banks[gb->gb_rom_read(gb, ram_size_location)];
	gb->battery = cart_battery[gb->gb_rom_read(gb, mbc_location)]; // check if cartridge has battery

	// Note that MBC2 will appear to have no RAM banks, but it actually
	// always has 512 half-bytes of RAM. Hence, gb->num_ram_banks must be
	// ignored for MBC2.

	gb->lcd_blank = 0;

	gb_reset(gb);

	return GB_INIT_NO_ERROR;
}

const char* gb_get_rom_name(struct gb_s* gb, char *title_str)
{
	uint_fast16_t title_loc = 0x134;
	// End of title may be 0x13E for newer games.
	const uint_fast16_t title_end = 0x143;
	const char* title_start = title_str;

	for(; title_loc <= title_end; title_loc++)
	{
		const char title_char = gb->gb_rom_read(gb, title_loc);

		if(title_char >= ' ' && title_char <= '_')
		{
			*title_str = title_char;
			title_str++;
		}
		else
			break;
	}

	*title_str = '\0';
	return title_start;
}

void gb_init_lcd(struct gb_s *gb)
{
	memset(gb->framebuf, 0, sizeof(gb->framebuf));
	gb->frame_write = 0;
	gb->frame_wline = 0;
	gb->frame_read = 0;
	gb->frame_rline = 0;

	gb->direct.interlace = 0;
	gb->display.interlace_count = 0;
	gb->direct.frame_skip = 0;
	gb->display.frame_skip_count = 0;

	gb->display.window_clear = 0;
	gb->display.WY = 0;

	return;
}

void gb_set_bootrom(struct gb_s *gb,
		 u8 (*gb_bootrom_read)(struct gb_s*, const uint_fast16_t))
{
	gb->gb_bootrom_read = gb_bootrom_read;
}

// This was taken from SameBoy, which is released under MIT Licence.
void gb_tick_rtc(struct gb_s *gb)
{
	// is timer running?
	if((gb->cart_rtc[4] & 0x40) == 0)
	{
		if(++gb->rtc_bits.sec == 60)
		{
			gb->rtc_bits.sec = 0;

			if(++gb->rtc_bits.min == 60)
			{
				gb->rtc_bits.min = 0;

				if(++gb->rtc_bits.hour == 24)
				{
					gb->rtc_bits.hour = 0;

					if(++gb->rtc_bits.yday == 0)
					{
						if(gb->rtc_bits.high & 1)  // Bit 8 of days
						{
							gb->rtc_bits.high |= 0x80; // Overflow bit
						}

						gb->rtc_bits.high ^= 1;
					}
				}
			}
		}
	}
}

void gb_set_rtc(struct gb_s *gb, const struct tm * const time)
{
	gb->cart_rtc[0] = time->tm_sec;
	gb->cart_rtc[1] = time->tm_min;
	gb->cart_rtc[2] = time->tm_hour;
	gb->cart_rtc[3] = time->tm_yday & 0xFF; // Low 8 bits of day counter.
	gb->cart_rtc[4] = time->tm_yday >> 8; // High 1 bit of day counter.
}
