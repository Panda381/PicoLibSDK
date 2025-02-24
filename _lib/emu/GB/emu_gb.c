
// ****************************************************************************
//
//                             Game Boy Emulator
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

#include "../../../_sdk/inc/sdk_timer.h"
#include "../../../_font/_include.h"
#include "../../../_display/_include.h"
#include "../../inc/lib_text.h"
#include "../../inc/lib_filesel.h"
#include "../../inc/lib_alarm.h"
#include "../../../_sdk/inc/sdk_systick.h"
#include "../../inc/lib_decnum.h"
#include "../../../_sdk/usb_inc/sdk_usb_hid.h"
#include "../../../_sdk/usb_inc/sdk_usb_host_hid.h"
#include "../../inc/lib_pwmsnd.h"
#include "../../../_sdk/inc/sdk_clocks.h"

/*
#include "../../../_sdk/inc/sdk_flash.h"
#include "../../inc/lib_draw.h"
#include "../../inc/lib_rand.h"
*/

// current CPU
sX80* GB_Cpu = NULL; // used CPU
int GB_Pwm; // used PWM controller
sFile* GB_ROMFile; // open ROM file
int GB_ROMSize; // size of ROM file
char* GB_ROMName; // pointer to buffer with ROM file name (can modify extension)
int GB_ROMNameLen; // ROM file name length without extension (= index of '.')
char GB_ROMNameExt[5]; // saved original ROM name extension

volatile u32 GB_Freq; // used frequency
volatile u32 GB_RealFreq; // real frequency

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
u32 GB_FreqH, GB_FreqL; // used frequency high and low
u32 GB_RealFreqH, GB_RealFreqL; // real frequency high and low
u16 GB_ClkDivH, GB_ClkDivL; // clock divider high and low
#endif

// keypad handler
Bool GB_KeyIsReg = False;		// key handler is registered
sAlarm GB_AlarmKey;			// key alarm structure
volatile u8 GB_Key14;			// pressed keys on port P14 (0=key is pressed)
volatile u8 GB_Key15;			// pressed keys on port P15 (0=key is pressed)
volatile Bool GB_KeyX;			// key X
volatile Bool GB_KeyY;			// key Y
volatile Bool GB_KeyA;			// key A
volatile Bool GB_KeyB;			// key B

Bool GB_KeyMenuReq;			// requirement for game menu (keys Y + UP)

#include "emu_gb_disp.c"	// display
#include "emu_gb_mem.c"		// memory service
#include "emu_gb_menu.c"	// game menu
#include "emu_gb_msg.c"		// message

// flush keys
void GB_KeyFlush()
{
	GB_KeyX = False;
	GB_KeyY = False;
	GB_KeyA = False;
	GB_KeyB = False;
	GB_KeyMenuReq = False;
}

// get key KEY_X, KEY_Y, KEY_A or KEY_B (return NOKEY if no key)
u8 GB_KeyGet()
{
	if (GB_KeyX)
	{
		GB_KeyX = False;
		return KEY_X;
	}
	
	if (GB_KeyY)
	{
		GB_KeyY = False;
		return KEY_Y;
	}

	if (GB_KeyA)
	{
		GB_KeyA = False;
		return KEY_A;
	}

	if (GB_KeyB)
	{
		GB_KeyB = False;
		return KEY_B;
	}
}

// key handler (called from systick alarm every 50 ms)
void GB_KeyHandler(sAlarm* alarm)
{
	// get key
	u8 key = KeyGet();
	if (key != NOKEY)
	{
		if (key == KEY_X) GB_KeyX = True;

		if (key == KEY_Y)
		{
			GB_KeyY = True;
			if (KeyPressed(KEY_UP)) GB_KeyMenuReq = True; // requirement for game menu (keys Y + UP)
		}

		if (key == KEY_A) GB_KeyA = True;

		if (key == KEY_B) GB_KeyB = True;

		if (key == KEY_UP)
		{
			if (KeyPressed(KEY_Y)) GB_KeyMenuReq = True; // requirement for game menu (keys Y + UP)
		}
	}

	// pressed keys on port 14
	key = 0xff;
	if (KeyPressed(KEY_RIGHT))	key &= ~GB_KEY14_RIGHT;
	if (KeyPressed(KEY_LEFT))	key &= ~GB_KEY14_LEFT;
	if (KeyPressed(KEY_UP))		key &= ~GB_KEY14_UP;
	if (KeyPressed(KEY_DOWN))	key &= ~GB_KEY14_DOWN;
	GB_Key14 = key;

	// pressed keys on port 15
	key = 0xff;
	if (KeyPressed(KEY_A))		key &= ~GB_KEY15_A;
	if (KeyPressed(KEY_B))		key &= ~GB_KEY15_B;
	if (KeyPressed(KEY_Y))		key &= ~GB_KEY15_SELECT;
	if (KeyPressed(KEY_X))		key &= ~GB_KEY15_START;
	GB_Key15 = key;
}

// execute STOP instruction (return True = continue, False = repeat STOP instruction)
Bool FASTCODE NOFLASH(GB_StopIns)()
{
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	// check condition of change speed
	if ((GBC->hram[GB_IO_KEY1] & B0) != 0) // enable speed switching?
	{
		// set clock divider
		if (GBC->freqdbl)
		{
			// set normal frequency
			PWM_ClkDiv(GB_Pwm, GB_ClkDivL);
			GB_Freq = GB_FreqL;
			GB_RealFreq = GB_RealFreqL;
			GBC->freqdbl = False;
			GBC->cpu.lcd_cycles = GB_LCD_CYCLES; // LCD cycles per scanline
			GBC->cpu.lcd_cycles2 = GB_LCD_CYCLES2; // LCD cycles per start mode 2 (OAM access)
			GBC->cpu.lcd_cycles3 = GB_LCD_CYCLES3; // LCD cycles per start mode 3 (LCD transfer)
			GBC->hram[GB_IO_KEY1] &= ~B7;
		}
		else
		{
			// set double frequency
			PWM_ClkDiv(GB_Pwm, GB_ClkDivH);
			GB_Freq = GB_FreqH;
			GB_RealFreq = GB_RealFreqH;
			GBC->freqdbl = True;
			GBC->cpu.lcd_cycles = GB_LCD_CYCLES*2; // LCD cycles per scanline
			GBC->cpu.lcd_cycles2 = GB_LCD_CYCLES2*2; // LCD cycles per start mode 2 (OAM access)
			GBC->cpu.lcd_cycles3 = GB_LCD_CYCLES3*2; // LCD cycles per start mode 3 (LCD transfer)
			GBC->hram[GB_IO_KEY1] |= B7;
		}

		GBC->hram[GB_IO_KEY1] &= ~B0; // reset flag
		return True; // continue
	}
#endif

	return True;
}

// process scanline
void FASTCODE NOFLASH(GB_LcdLine)()
{
	// check if LCD is enabled
	if ((GBC->hram[GB_IO_LCDC] & B7) == 0) return;

	// increase line
	int line = GBC->hram[GB_IO_LY] + 1;
	if (line >= GB_LINES)
	{
		line -= GB_LINES;
		GBC->frame++; // display frame counter
	}
	GBC->hram[GB_IO_LY] = line;

	// LYC update
	if (line == GBC->hram[GB_IO_LYC]) // LY == LYC
	{
		//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
		//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
		//    B3: HSYNC status interrupt
		//    B4: VSYNC status interrupt
		//    B5: searching OAM interrupt
		//    B6: LYC==LY status interrupt
		GBC->hram[GB_IO_STAT] |= B2; // set status

		// raise LCDC status interrupt (if required)
		if ((GBC->hram[GB_IO_STAT] & B6) != 0) X80_RaiseIRQ(&GBC->cpu, X80_IRQ_LCDC);
	}
	else
		GBC->hram[GB_IO_STAT] &= ~B2; // clear status

	// VBLANK start
	if (line == GB_HEIGHT)
	{
		// set VBLANK state (start mode 1)
		//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
		//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
		//    B3: HSYNC status interrupt
		//    B4: VSYNC status interrupt
		//    B5: searching OAM interrupt
		//    B6: LYC==LY status interrupt
		GBC->hram[GB_IO_STAT] = (GBC->hram[GB_IO_STAT] & ~3) | 1;

		// raise LCDC interrupt
		X80_RaiseIRQ(&GBC->cpu, X80_IRQ_VBLANK);

		// raise LCDC status interrupt (if required)
		if ((GBC->hram[GB_IO_STAT] & B4) != 0) X80_RaiseIRQ(&GBC->cpu, X80_IRQ_LCDC);
	}

	// normal visible line - start HSYNC
	else if (line < GB_HEIGHT)
	{
		// start HSYNC state (start mode 0)
		//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
		//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
		//    B3: HSYNC status interrupt
		//    B4: VSYNC status interrupt
		//    B5: searching OAM interrupt
		//    B6: LYC==LY status interrupt
		GBC->hram[GB_IO_STAT] &= ~3;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
		// DMA transfer during HSYNC
		if ((GBC->gbmode != GB_MODE_GB) && (GBC->dmanotrun == 0) && (GBC->dmamode == 1))
		{
			int n = 16;
			u16 dst = (GBC->dmadst & 0x1ff0) | 0x8000;
			u16 src = GBC->dmasrc & 0xfff0;
			GBC->dmadst += n;
			GBC->dmasrc += n;
			for (; n > 0; n--) GBC->cpu.writemem(dst++, GBC->cpu.readmem(src++));
			GBC->dmasize--;
			if (GBC->dmasize == 0) GBC->dmanotrun = 1;
		}
#endif

		// raise LCDC status interrupt (if required)
		if ((GBC->hram[GB_IO_STAT] & B3) != 0) X80_RaiseIRQ(&GBC->cpu, X80_IRQ_LCDC);
	}
}

// start OAM-RAM access
void FASTCODE NOFLASH(GB_LcdOam)()
{
	// check if LCD is enabled
	if ((GBC->hram[GB_IO_LCDC] & B7) == 0) return;

	// set OAM state (start mode 2)
	//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
	//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
	//    B3: HSYNC status interrupt
	//    B4: VSYNC status interrupt
	//    B5: searching OAM interrupt
	//    B6: LYC==LY status interrupt
	GBC->hram[GB_IO_STAT] = (GBC->hram[GB_IO_STAT] & ~3) | 2;

	// raise LCDC status interrupt (if required)
	if ((GBC->hram[GB_IO_STAT] & B5) != 0) X80_RaiseIRQ(&GBC->cpu, X80_IRQ_LCDC);
}

// start LCD transfer
void FASTCODE NOFLASH(GB_LcdTrans)()
{
	// check if LCD is enabled
	if ((GBC->hram[GB_IO_LCDC] & B7) == 0) return;

	// set transfer state (start mode 2)
	//    B0,B1: mode 0=HSYNC, 1=VSYNC, 2=searching OAM RAM, 3=transfering data to LCD
	//    B2: match 1= LYC == LCDC LY, 0= LYC != LCDC LY
	//    B3: HSYNC status interrupt
	//    B4: VSYNC status interrupt
	//    B5: searching OAM interrupt
	//    B6: LYC==LY status interrupt
	GBC->hram[GB_IO_STAT] = (GBC->hram[GB_IO_STAT] & ~3) | 3;

	// render current line
	GB_RenderLine();
}

// MBC (memory controller) type
const s8 GB_MBCType[32] = {
//	0x?0 0x?1 0x?2 0x?3 0x?4 0x?5 0x?6 0x?7 0x?8 0x?9 0x?A 0x?B 0x?C 0x?D 0x?E 0x?F
	   0,   1,   1,   1,   2,   2,   2,   0,   0,   0,  -1,   0,   0,   0,  -1,   3, // 0x0?
	   3,   3,   3,   3,  -1,  -1,  -1,  -1,  -1,   5,   5,   5,   5,   5,   5,  -1, // 0x1?
};

// flag if cartridge has external XRAM
// - MBC2 has SRAM even if not indicated
const u8 GB_MBCXram[32] = {
//	0x?0 0x?1 0x?2 0x?3 0x?4 0x?5 0x?6 0x?7 0x?8 0x?9 0x?A 0x?B 0x?C 0x?D 0x?E 0x?F
	   0,   0,   1,   1,   0,   1,   1,   0,   1,   1,   0,   0,   0,   0,   0,   0, // 0x0?
	   1,   0,   1,   1,   0,   0,   0,   0,   0,   0,   1,   1,   0,   1,   1,   0, // 0x1?
};

/*
// Cartridge software type (0x0147)
0 ... ROM (no memory control)
1 ... ROM, MBC1
2 ... ROM, MBC1, SRAM
3 ... ROM, MBC1, SRAM, battery
4 ... (not supported)
5 ... ROM, MBC2
6 ... ROM, MBC2, battery
7 ... (not supported)
8 ... ROM, SRAM (no memory control)
9 ... ROM, SRAM, battery (no memory control)
10 (0x0A) ... (not supported)
11 (0x0B) ... ROM, MMM01
12 (0x0C) ... ROM, MMM01, SRAM
13 (0x0D) ... ROM, MMM01, SRAM, battery
14 (0x0E) ... (not supported)
15 (0x0F) ... ROM, MBC3, RTC, battery
16 (0x10) ... ROM, MBC3, RTC, SRAM, battery
17 (0x11) ... ROM, MBC3 (no RTC)
18 (0x12) ... ROM, MBC3 (no RTC), SRAM
19 (0x13) ... ROM, MBC3 (no RTC), SRAM, battery
20-24 (0x14..0x18) ... (not supported)
25 (0x19) ... ROM, MBC5 (no rumble)
26 (0x1A) ... ROM, MBC5 (no rumble), SRAM
27 (0x1B) ... ROM, MBC5 (no rumble), SRAM, battery
28 (0x1C) ... ROM, MBC5, rumble
29 (0x1D) ... ROM, MBC5, rumble, SRAM
30 (0x1E) ... ROM, MBC5, rumble, SRAM, battery
31 (0x1F) ... pocket acamera (not supported)
0xFD ... Bandai TAMA5 (not supported)
0xFE ... Hudson HuC-3 (not supported)
0xFF ... Hudson HuC-1 (not supported)
*/

// number of XRAM 8KB banks
const u8 GB_MBCXramNum[6] = { 0, 1, 1, 4, 16, 8 };

// start emulation (emulator context GBC will be used; returns 0 on error)
//  file ... open ROM file
//  size ... size of ROM file
//  name ... pointer to buffer with ROM file name (can modify extension)
//  pwm ... index of (unused) PWM slice (0..7)
//  freq ... required emulated frequency in Hz (GB: use 4 MHz, GBC: use 8 MHz)
u32 GB_Start(sFile* file, int size, char* name, int pwm, u32 freq)
{
	GB_ROMFile = file; // open ROM file
	GB_ROMSize = size; // size of ROM file
	GB_ROMName = name; // buffer with filename

	int len = StrLen(name); // ROM file name length without extension
	if (name[len-1] == '.')
		len = len-1;
	else if (name[len-2] == '.')
		len = len-2;
	else if (name[len-3] == '.')
		len = len-3;
	else if (name[len-4] == '.')
		len = len-4;
	GB_ROMNameLen = len;
	memcpy(GB_ROMNameExt, &name[len], 5); // save original ROM name extension

	sX80* cpu = &GBC->cpu;

	// save current CPU
	GB_Cpu = cpu;
	GB_Pwm = pwm;

	// setup callbacks
	cpu->readmem = GB_GetMem;	// read byte from memory
	cpu->writemem = GB_SetMem;	// write byte to memory
	cpu->stopins = GB_StopIns;	// STOP instruction
	cpu->lcdline = GB_LcdLine;	// process LCD scanline
	cpu->lcdoam = GB_LcdOam;	// start LCD OAM-RAM access
	cpu->lcdtrans = GB_LcdTrans;	// start LCD transfer

#if USE_EMUSCREENSHOT		// use emulator screen shots
	DoEmuScreenShot = False;	// request to do emulator screenshot
#endif

	GBC->res2 = 0;
	GBC->res3 = 0;
	GBC->res4 = 0;
	GBC->res5 = 0;

	// memory mapping
	GBC->vram_map = GBC->vram;
	GBC->xram_map = GBC->xram;
	GBC->wram_map = &GBC->wram[1*GB_WRAM_BANK_SIZE];
	GBC->rom_base = 0x4000;
	GBC->rom_sel = 1; 	// selected ROM bank
	GBC->mbc_xram_en = 0;	// 1=extended XRAM is enabled
	GBC->xram_sel = 0;	// selected XRAM bank 8 KB (0..15)
	GBC->bank_sel_mode = 0;	// MBC1: bank selection mode 0=ROM, 1=RAM
	GBC->vram_sel = 0;	// selected video VRAM bank 8 KB (0 or 1)
	GBC->wram_sel = 1;	// selected working WRAM bank 4 KB (1..7)
	GBC->frame = 0;		// display frame counter
	GBC->dmanotrun = 1;	// DMA not running
	GBC->dmamode = 0;	// DMA mode
	GBC->dmasize = 0;	// DMA size
	GBC->dmasrc = 0;	// DMA source
	GBC->dmadst = 0;	// DMA destination

	// clear memory
	memset(GBC->vram, 0, sizeof(GBC->vram));
	memset(GBC->xram, 0, sizeof(GBC->xram));
	memset(GBC->wram, 0, sizeof(GBC->wram));
	memset(GBC->hram, 0, sizeof(GBC->hram));
	memset(GBC->rom, GB_ROM_PAGEINV, sizeof(GBC->rom));
	memset(GBC->cache, 0xff, sizeof(GBC->cache));
	memset(GBC->cache_list, 0xff, sizeof(GBC->cache_list));

	// GBC palettes
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	GBC->bgpal_id = 0;		// GBC background palette index
	GBC->sppal_id = 0;		// GBC sprite palette index
	GBC->bgpal_inc = 0;		// GBC background palette increment
	GBC->sppal_inc = 0;		// GBC sprite palette increment

	int ii;
	for (ii = 0; ii < GBC_PAL_NUM; ii++)
	{
		GBC->bg_pal2[ii] = 0xff;
		GBC->sp_pal2[ii] = 0xff;
		ii++;
		GBC->bg_pal2[ii] = 0x7f;
		GBC->sp_pal2[ii] = 0x7f;
	}
#endif

	// reset CPU
	X80_Reset(cpu);

	// setup ports
	GBC->hram[GB_IO_LCDC] = 0x91;
	GBC->hram[GB_IO_STAT] = 0x85;
	GBC->hram[GB_IO_BANK] = 1;
	GBC->hram[GB_IO_P1] = 0xcf;
	GBC->hram[GB_IO_TAC] = 0xf8;
	GBC->cpu.writemem(0xff47, 0xfc); // BGP
	GBC->cpu.writemem(0xff48, 0xff); // OBJP0
	GBC->cpu.writemem(0xff49, 0xff); // OBJP1

	// setup registers - use values as though the boot ROM was already executed
	u8 crc = cpu->readmem(ROM_CRC_ADDR);
	Bool hdr_check = crc != 0;
	cpu->f = X80_Z;
	if (hdr_check) cpu->f |= X80_H | X80_C;

#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	// Game Boy Color
	cpu->a = 0x11;
	cpu->bc = 0x0000;
	cpu->de = 0xff56; //0x0008;
	cpu->hl = 0x000D; //0x007C;

#else
	// Game Boy
	cpu->a = 1;
	cpu->bc = 0x0013;
	cpu->de = 0x00D8;
	cpu->hl = 0x014D;

#endif
	cpu->sp = 0xFFFE;
	cpu->pc = 0x0100;

	cpu->ime = True; 	// interrupt enabled

	// check ROM checksum
	u8 x = 0;
	u16 i;
	for (i = 0x134; i <= 0x14c; i++) x = x - cpu->readmem(i) - 1;
	if (x != crc) if (!GB_ErrorMsg("Invalid ROM checksum", "", False, True)) return 0;

	// get GB mode
	u8 gbmode = cpu->readmem(0x143);	// read GB mode GB_MODE_*
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	GBC->gbmode = gbmode;
#else
	GBC->gbmode = GB_MODE_GB;	// GB mode = GB exclusive
	if (gbmode == 0xc0) // exclusive GBC
	{
		if (!GB_ErrorMsg("Cannot run", "Game Boy Color", False, True)) return 0;
	}
#endif

	// check if cartridge type is supported, set MBC type
	u8 carttype = cpu->readmem(0x147);
	u8 mbc = (carttype < count_of(GB_MBCType)) ? GB_MBCType[carttype] : (u8)-1;
	if (mbc == (u8)-1)
	{
		if (!GB_ErrorMsg("Unsupported type", "of cartridge", False, True)) return 0;
		mbc = (size > 256*1024) ? 5 : 1;
	}
	GBC->mbc = mbc;

	// check if cartridge has extended XRAM
	GBC->mbc_xram = (carttype < count_of(GB_MBCType)) ? GB_MBCXram[carttype] : ((mbc == 5) ? 1 : 0);

	// get number of 16KB ROM banks
	u8 romnum = cpu->readmem(0x148); // ROM size (0=2 banks, 1=4 banks, 2=8 banks ... 8=512 banks)
	GBC->mbc_rom_num = (u16)BIT(romnum + 1); // number of ROM banks 16 KB (2..512)

	// get number of 8KB banks of extended XRAM
	u8 ramnum = cpu->readmem(0x149); // XRAM size (0=0 banks, 1=1 bank, 2=1 bank, 3=4 banks, 4=16 banks, 5=8 banks)
	GBC->mbc_xram_num = (ramnum < count_of(GB_MBCXramNum)) ? GB_MBCXramNum[ramnum] : 0; // number of XRAM banks 8 KB (0..16)

	// flush keyboard buffer
	GB_KeyFlush();

	// prepare key handler
	GB_AlarmKey.time = SysTime + GB_KEY_DELTA; // next time to alarm
	GB_AlarmKey.delta = GB_KEY_DELTA;  // repeat time in [ms]
	GB_AlarmKey.callback = GB_KeyHandler; // callback

	// clear text screen
	GB_TextClear();

	// display mode
	GB_DispMode = GB_DISPMODE_EMU;

	// prepare clock divider
#if USE_EMU_GB == 2			// 1=use Game Boy emulator, 2=use Game Boy Color emulator
	GB_FreqH = freq;		// required high frequency
	GB_RealFreqH = X80_SyncInit(cpu, pwm, freq); // setup high frequency
	GB_ClkDivH = PWM_GetClkDiv(pwm); // divider of high frequency

	freq /= 2;			// low frequency
	GB_FreqL = freq;		// required low frequency
	GB_RealFreqL = X80_SyncInit(cpu, pwm, freq); // setup low frequency
	GB_RealFreq = GB_RealFreqL;	// real frequency
	GB_ClkDivL = PWM_GetClkDiv(pwm); // divider of low frequency
#else
	// do not set GB_RealFreq after X80_Cont, program could change it in meantime
	GB_RealFreq = X80_SyncInit(cpu, pwm, freq); // setup low frequency
#endif

	// start emulation
	GBC->freqdbl = False;		// not using double frequency
	GBC->cpu.lcd_cycles = GB_LCD_CYCLES; // LCD cycles per scanline
	GB_Freq = freq;			// used frequency
	freq = X80_Cont(cpu, pwm, freq);

	// register key handler
	if (!GB_KeyIsReg)
	{
		GB_KeyIsReg = True;	// handler is registered
		AlarmReg(&GB_AlarmKey); // register alarm
	}

	return freq;
}

// stop/pause emulation
void GB_Stop()
{
	if (GB_KeyIsReg) // unregister key handler
	{
		AlarmUnreg(&GB_AlarmKey); // unregister alarm
		GB_KeyIsReg = False;	// handler is not registered
	}
	GB_KeyFlush();
	KeyFlush();
	sX80* cpu = GB_Cpu;
	if (cpu != NULL) X80_Stop(GB_Pwm);
}

// continue emulation
void GB_Cont()
{
	// reset keyboard handler (on start of emulation)
	KeyFlush();
	GB_KeyFlush();

#if USE_EMUSCREENSHOT		// use emulator screen shots
	DoEmuScreenShot = False;	// request to do emulator screenshot
#endif

	// prepare key handler
	GB_AlarmKey.time = SysTime + GB_KEY_DELTA; // next time to alarm
	GB_AlarmKey.delta = GB_KEY_DELTA;  // repeat time in [ms]
	GB_AlarmKey.callback = GB_KeyHandler; // callback

	sX80* cpu = GB_Cpu;
	if (cpu != NULL) X80_Cont(GB_Cpu, GB_Pwm, GB_Freq);

	// register key handler
	if (!GB_KeyIsReg)
	{
		GB_KeyIsReg = True;	// handler is registered
		AlarmReg(&GB_AlarmKey); // register alarm
	}
}
