
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// frame buffer, with emulator RAM
#if USE_PICOPADHSTX		// use PicoPadHSTX device configuration
ALIGNED FRAMETYPE FrameBuf2[(sizeof(sNES)+1)/sizeof(FRAMETYPE)];
#else
ALIGNED FRAMETYPE FrameBuf[(sizeof(sNES)+1)/sizeof(FRAMETYPE)];
#endif

int NES_XRamSize;		// size of extra RAM

// home path
char HomePath[APPPATH_PATHMAX+15+1];
int HomePathLen;

// draw on screen
volatile u8 NES_DispMode = NES_DISPMODE_EMU;	// display mode NES_DISPMODE_*
volatile u8 NES_DispMap[NES_DISP_HEIGHT];	// map of rendered scanlines

u8 MyPadState = 0;		// PAD state (1=pressed)

#ifdef DEB_REMAP			// Debug - enable mapper number incrementing using B key in menu
int ReMapperNo = -1;
#endif

// ROM CRC
u32 NES_RomCrc;
int NES_VROMSize; // VROM size (bytes)

// zoom
Bool UseZoom = NES_CLIP_ZOOM;	// use zoom

// sound
u16 SndBuf = 0;			// next sound sample
#if PWMSND_GPIO_R >= 0
u16 SndBufR = 0;		// next sound sample
#endif

// X pixel offset map in source scanline
u8 PixOffMap[WIDTH];

// source width
#define SOURCE_WIDTH	(NES_DISP_WIDTH - NES_CLIP_LEFT - NES_CLIP_RIGHT)

// prepare pixel offset map in source scanline
void InitPixOffMap()
{
	int x, xs;
	for (x = 0; x < WIDTH; x++)
	{
		xs = x*SOURCE_WIDTH/WIDTH;
		if (xs >= SOURCE_WIDTH) xs = SOURCE_WIDTH - 1;
		xs += NES_CLIP_LEFT;
		PixOffMap[x] = (u8)xs;
	}
}

// PWM audio interrupt handler
void FASTCODE NOFLASH(PWMSndIrq)()
{
	// clear interrupt request
	PWM_IntClear(PWMSND_SLICE);

	// output sample
	PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, SndBuf);
#if PWMSND_GPIO_R >= 0
	PWM_Comp(PWMSND_SLICE_R, PWMSND_CHAN_R, SndBufR);
#endif // PWMSND_GPIO_R

	// pause
	if (NES_DispMode != NES_DISPMODE_MSG)
	{
		// clock sound chip by CPU clock
		NES_ApuSndClock(APU, NES_CPUCLOCK_PER_SAMPLE);

		// prepare next sound sample (k = 0..1308)

// get output sample
// Mono output level: 0..1308
// Stereo output level: 12 LOW bits 0..1308, 12 HIGH bits 0..1308

		int k = NES_ApuSndOut(APU);

#if PWMSND_GPIO_R >= 0

		int kR = k >> 12;
		k &= 0xfff;

		// prepare sound volume
		int vol = Config.volume; // volume 0..255, 100=normal
		if ((vol == 0) || GlobalSoundOff)
		{
			k = 0;
			kR = 0;
		}
		else
		{
			// k = max 1308, vol = normal 100, k * vol = 130800, output should be max. 1023
			k = (k*vol + 64) >> 7; // (1308*100 + 64) >> 7 = 1022
			if (k > EMU_PWMTOP) k = EMU_PWMTOP;

			kR = (kR*vol + 64) >> 7; // (1308*100 + 64) >> 7 = 1022
			if (kR > EMU_PWMTOP) kR = EMU_PWMTOP;
		}
		SndBuf = k;
		SndBufR = kR;

#else // PWMSND_GPIO_R
		// prepare sound volume
		int vol = Config.volume; // volume 0..255, 100=normal
		if ((vol == 0) || GlobalSoundOff)
			k = 0;
		else
		{
			// k = max 1308, vol = normal 100, k * vol = 130800, output should be max. 1023
			k = (k*vol + 64) >> 7; // (1308*100 + 64) >> 7 = 1022
			if (k > EMU_PWMTOP) k = EMU_PWMTOP;
		}
		SndBuf = k;
#endif // PWMSND_GPIO_R
	}
}

// draw one line from frame buffer
void FASTCODE NOFLASH(core1DrawFrame)()
{
	int x, y;
	u16 c;

	// do screenshot
	dmb();
	if (DoEmuScreenShotReq)
	{
		DoEmuScreenShot = True;	// request to do emulator screenshot
		dmb();
		DoEmuScreenShotReq = False;
		dmb();
	}

	// start sending image data
	DispStartImg(0, WIDTH, 0, HEIGHT);

	// source address
	u8* s;
	u16* pal = NesPalette;

	// using zoom
	if (UseZoom)
	{
		// source Y
		int ys2 = NES_CLIP_UP;
		int ys;

// source height
#define SOURCE_HEIGHT	(NES_DISP_HEIGHT - NES_CLIP_UP - NES_CLIP_DOWN)

		// destination Y
		for (y = 0; y < HEIGHT;)
		{
			// source address
			ys = ys2;
			s = &NES->framebuf[ys*NES_DISP_WIDTH];

			// send scanline
			for (x = 0; x < WIDTH; x++)
			{
				c = pal[s[PixOffMap[x]] & 0x3f];
				DispSendImg2(c);
			}

			// next source Y
			y++;
			ys2 = y*SOURCE_HEIGHT/HEIGHT;
			if (ys2 >= SOURCE_HEIGHT) ys2 = SOURCE_HEIGHT - 1;
			ys2 += NES_CLIP_UP;
		
			// clear scanline map - only if we will not need this scan line again
			if ((ys != ys2) || (y == HEIGHT)) NES_DispMap[ys] = 0;
		}

	}

	// not using zoom
	else
	{
		// source image
		s = NES->framebuf;

		// destination Y
		for (y = 0; y < HEIGHT; y++)
		{
			// send scanline
			for (x = NES_DISP_WIDTH/4; x > 0; x--)
			{
				c = pal[*s++ & 0x3f];
				DispSendImg2(c);
				DispSendImg2(c);
				c = pal[*s++ & 0x3f];
				DispSendImg2(c);
				c = pal[*s++ & 0x3f];
				DispSendImg2(c);
				c = pal[*s++ & 0x3f];
				DispSendImg2(c);
			}

			// clear scanline map
			NES_DispMap[y] = 0;
		}
	}

	// stop sending data
	DispStopImg();
}

volatile Bool RunEmul = False;

// Main function for Core 1
void core1_entry()
{
	RunEmul = True;

	while (RunEmul)
	{
		// message text mode
		if (NES_DispMode == NES_DISPMODE_MSG)
		{
			NES_TextUpdate();
		}
		else
		{
			core1DrawFrame();
		}
	}
}

// parse ROM (returns False on incorrect NES header)
Bool ParseROM(const u8 *rom)
{
	// get NES header
	memcpy(&NesHeader, rom, sizeof(NesHeader));
	rom += sizeof(NesHeader);

	// check NES header format
	if (	(NesHeader.byID[0] != 'N') ||
		(NesHeader.byID[1] != 'E') ||
		(NesHeader.byID[2] != 'S') ||
		(NesHeader.byID[3] != 26))
		return False;

	// check NES2 format
	if ((NesHeader.byInfo2 & 0x0c) == 0x08)
		NesFormat = 2;

	// check NES1 format
	else if ((NesHeader.byReserve[4] == 0) && (NesHeader.byReserve[5] == 0) &&
			(NesHeader.byReserve[6] == 0) && (NesHeader.byReserve[7] == 0))
		NesFormat = 1;

	// NES0 format otherwise
	else
		NesFormat = 0;

	// get trainer SRAM
	if (NesHeader.byInfo1 & 4)
	{
		memcpy(&SRAM[0x1000], rom, 512);
		rom += 512;
	}

	// get ROM size in bytes
	u32 romSize = NesHeader.byRomSize * 0x4000;
	NES_ROMSize = romSize; // ROM size (bytes)

	// start of ROM
	ROM = (BYTE*)rom;
	rom += romSize;

	// get video ROM
	NES_VROMSize = 0;
	if (NesHeader.byVRomSize > 0)
	{
		u32 vromSize = NesHeader.byVRomSize * 0x2000;
		NES_VROMSize = vromSize;
		VROM = (BYTE*)rom;
		rom += vromSize;
	}

	return True;
}

void InfoNES_PadState(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem)
{
	*pdwPad1 = MyPadState;
}

// NES palettes in RGB565 format
u16 NesPalette[64] = {
#if NES_PAL == 1		// GBAR4444-main
	// Source: NesPalette[] from PicoSystem_InfoNes: main.cpp
	0x73ae, 0x018e, 0x2111, 0x4111, 0x608e, 0x8888, 0x8882, 0x6100, 
	0x4180, 0x1220, 0x0220, 0x0222, 0x0228, 0x0000, 0x0000, 0x0000, 
	0xbdd7, 0x13b9, 0x433b, 0x8abb, 0xba37, 0xca2e, 0xca26, 0xaaa0, 
	0x7320, 0x43a0, 0x1442, 0x0448, 0x0451, 0x0000, 0x0000, 0x0000, 
	0xffff, 0x667f, 0x9ddf, 0xdcdf, 0xfcdf, 0xfcd9, 0xfcd1, 0xfd4a, 
	0xcdc6, 0x9e66, 0x66ec, 0x46f3, 0x46fb, 0x52aa, 0x0000, 0x0000, 
	0xffff, 0xcf7f, 0xef7f, 0xff7f, 0xff7f, 0xfeff, 0xfefb, 0xff79, 
	0xff77, 0xdff7, 0xcff9, 0xbffb, 0xbfff, 0xce79, 0x0000, 0x0000,
#elif NES_PAL == 2		// RGB555-Win
	// Source: NesPalette[] from PicoSystem_InfoNes: InfoNES_System_Win.cpp
	0x738e, 0x20d1, 0x0015, 0x4013, 0x880e, 0xa802, 0xa000, 0x7840, 
	0x4140, 0x0200, 0x0280, 0x01c2, 0x19cb, 0x0000, 0x0000, 0x0000, 
	0xbdf7, 0x039d, 0x21dd, 0x801e, 0xb817, 0xe00b, 0xd940, 0xca41, 
	0x8b80, 0x04a0, 0x0560, 0x04a7, 0x0431, 0x0000, 0x0000, 0x0000, 
	0xffff, 0x3dff, 0x5cbf, 0x447f, 0xf3df, 0xfb96, 0xfb8c, 0xfce7, 
	0xf5e7, 0x86a2, 0x4ee9, 0x5ff3, 0x077b, 0x0000, 0x0000, 0x0000, 
	0xffff, 0xaf3f, 0xc6bf, 0xd67f, 0xfe3f, 0xfe3b, 0xfdf6, 0xfef5, 
	0xff34, 0xe7f4, 0xafb7, 0xb7f9, 0x9ffe, 0x0000, 0x0000, 0x0000, 
#elif NES_PAL == 3		// RGB888-Matthew
	// Source: NesPalette[] from PicoSystem_InfoNes: InfoNES_System_ppc2003.cpp (Matthew Conte's Palette)
	//  https://www.nesdev.org/nespal.txt
	0x8410, 0x0017, 0x3017, 0x8014, 0xb80d, 0xb003, 0xb000, 0x9120, 
	0x7940, 0x01e0, 0x0241, 0x01e4, 0x016c, 0x0000, 0x0020, 0x0020, 
	0xce59, 0x02df, 0x41ff, 0xb199, 0xf995, 0xf9ab, 0xf9a3, 0xd240, 
	0xc300, 0x3bc0, 0x1c22, 0x04ac, 0x0438, 0x1082, 0x0841, 0x0841, 
	0xffff, 0x04bf, 0x6c3f, 0xd37f, 0xfbb9, 0xfb73, 0xfbcb, 0xfc8b, 
	0xfd06, 0xa5e0, 0x56cd, 0x4eb5, 0x06df, 0x632c, 0x0861, 0x0861, 
	0xffff, 0x85ff, 0xbddf, 0xd5df, 0xfdfd, 0xfdf9, 0xfe36, 0xfe75, 
	0xfed4, 0xcf13, 0xaf76, 0xafbd, 0xb77f, 0xdefb, 0x1082, 0x1082,
#elif NES_PAL == 4		// RGB888-Piotr
	// Source: Piotr Grochowski, https://lospec.com/palette-list/nintendo-entertainment-system
	0x5acb, 0x010f, 0x0893, 0x3012, 0x480d, 0x6006, 0x5820, 0x40c0, 
	0x2160, 0x09e0, 0x0220, 0x0200, 0x01a8, 0x0000, 0x0000, 0x0000, 
	0xa514, 0x0a9a, 0x31bf, 0x611e, 0x90b7, 0xa8ad, 0xa124, 0x8200, 
	0x5b00, 0x2bc0, 0x0c20, 0x03e5, 0x0370, 0x0000, 0x0000, 0x0000, 
	0xffff, 0x553f, 0x843f, 0xbb5f, 0xf2df, 0xfaf8, 0xfb8d, 0xe484, 
	0xada0, 0x7e80, 0x56e4, 0x3ece, 0x3e1b, 0x4208, 0x0000, 0x0000, 
	0xffff, 0xb6df, 0xce5f, 0xe5ff, 0xfddf, 0xfddc, 0xfe17, 0xf693, 
	0xdf10, 0xc770, 0xb793, 0xa798, 0xaf3e, 0xad75, 0x0000, 0x0000,
#elif NES_PAL == 5		// RGB888-NesDev
	// Source: NesDev 2C02 https://www.nesdev.org/wiki/PPU_palettes
	0x630c, 0x00f6, 0x2039, 0x5016, 0x700e, 0x8004, 0x7040, 0x5140, 
	0x2220, 0x02a0, 0x02e0, 0x0284, 0x01ee, 0x0000, 0x0000, 0x0000, 
	0xad55, 0x0abf, 0x499f, 0x889f, 0xb85a, 0xd08d, 0xc160, 0x9aa0, 
	0x63c0, 0x24c0, 0x0500, 0x04c8, 0x03f6, 0x0000, 0x0000, 0x0000, 
	0xffff, 0x557f, 0x943f, 0xd33f, 0xfabf, 0xfaf9, 0xfbaa, 0xfce0, 
	0xbe20, 0x7f20, 0x47a2, 0x276f, 0x2ebe, 0x4a69, 0x0000, 0x0000, 
	0xffff, 0xb71f, 0xce9f, 0xee1f, 0xfdff, 0xfdfe, 0xfe38, 0xfeb3, 
	0xef30, 0xcfb0, 0xb7d3, 0xafd8, 0xaf9e, 0xbdd7, 0x0000, 0x0000, 
#elif NES_PAL == 6		// RGB888-Wiki
	// Source: Wikipedia https://en.wikipedia.org/wiki/List_of_video_game_console_palettes
	0x5acb, 0x0011, 0x1811, 0x380e, 0x500a, 0x5002, 0x5000, 0x4100, 
	0x3180, 0x1180, 0x0182, 0x0228, 0x022c, 0x0000, 0x0841, 0x0841, 
	0xad55, 0x023b, 0x509d, 0x701d, 0x9817, 0xa80a, 0x9980, 0x8a20, 
	0x6320, 0x3320, 0x0320, 0x032a, 0x02b1, 0x0841, 0x0841, 0x0841, 
	0xef7d, 0x445f, 0x73bf, 0x9a3f, 0xba3d, 0xcab3, 0xdb28, 0xcc40, 
	0xbd40, 0x75c0, 0x25c4, 0x25ce, 0x25d9, 0x4228, 0x0841, 0x0841, 
	0xef7d, 0x9e7f, 0xad5f, 0xbcdf, 0xdcdf, 0xecdb, 0xed55, 0xedd3, 
	0xeef1, 0xbef1, 0x9ef3, 0x9ef7, 0x9efd, 0xad55, 0x0841, 0x0841, 
#elif NES_PAL == 7		// RGB888-Smooth
	// Source: Smooth https://www.firebrandx.com/nespalette.html
	0x6b6d, 0x0090, 0x1811, 0x380f, 0x500a, 0x5803, 0x4880, 0x38e0, 
	0x2180, 0x01e0, 0x0200, 0x01c4, 0x016a, 0x0000, 0x0000, 0x0000, 
	0xbdf7, 0x1a98, 0x499c, 0x711a, 0x90f5, 0x994b, 0x99a0, 0x7a60, 
	0x5b20, 0x23a0, 0x03e0, 0x03a8, 0x0371, 0x0000, 0x0000, 0x0000, 
	0xffff, 0x6d3f, 0x8cff, 0xb43f, 0xdbbf, 0xe3b7, 0xe46d, 0xd4e5, 
	0xb561, 0x7e02, 0x5648, 0x4650, 0x4618, 0x4a69, 0x0000, 0x0000, 
	0xffff, 0xcf5f, 0xdeff, 0xeedf, 0xfebf, 0xfebe, 0xfed9, 0xff36, 
	0xf795, 0xdfd5, 0xcff7, 0xc7da, 0xc7be, 0xbe17, 0x0000, 0x0000, 
#endif
};

// ----------------------------------------------------------------------------
//                               Initialize
// ----------------------------------------------------------------------------

// initialize sound
void InfoNES_SoundInit()
{
	// initialize PWM sound output
	PWM_Reset(PWMSND_SLICE);		// reset PWM to default state
	PWM_GpioInit(PWMSND_GPIO);		// set GPIO function to PWM
#if PWMSND_GPIO_R >= 0
	PWM_GpioInit(PWMSND_GPIO_R);		// set GPIO function to PWM
#endif // PWMSND_GPIO_R
	SetHandler(IRQ_PWM_WRAP, PWMSndIrq);	// set IRQ handler
	NVIC_IRQEnable(IRQ_PWM_WRAP);		// enable interrupt on NVIC controller
	PWM_Clock(PWMSND_SLICE, EMU_PWMCLOCK);	// set clock divider
	PWM_Top(PWMSND_SLICE, EMU_PWMTOP);	// set period to 256 cycles
	PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, EMU_PWMTOP/2); // write default PWM sample
#if PWMSND_GPIO_R >= 0
	PWM_Comp(PWMSND_SLICE_R, PWMSND_CHAN_R, EMU_PWMTOP/2); // write default PWM sample
#endif // PWMSND_GPIO_R
	PWM_Enable(PWMSND_SLICE);		// enable PWM
	PWM_IntEnable(PWMSND_SLICE);		// PWM interrupt enable
}

// initialize system clock
void NES_InitSysClk()
{
#if !USE_PICOPADHSTX || !USE_DISPHSTX		// use PicoPadHSTX device configuration
#if NES_CLKSYS >= 285000
	SSI_InitFlash(6);
	VregSetVoltage(VREG_VOLTAGE_1_30);
#elif NES_CLKSYS >= 245000
	VregSetVoltage(VREG_VOLTAGE_1_30);
#elif NES_CLKSYS >= 220000
	VregSetVoltage(VREG_VOLTAGE_1_25);
#elif NES_CLKSYS >= 190000
	VregSetVoltage(VREG_VOLTAGE_1_20);
#elif NES_CLKSYS >= 150000
	VregSetVoltage(VREG_VOLTAGE_1_15);
#else
	VregSetVoltage(VREG_VOLTAGE_1_10);
#endif
	ClockPllSysFreq(NES_CLKSYS);
#endif
}

// restore system clock
void NES_TermSysClk()
{
#if !USE_PICOPADHSTX || !USE_DISPHSTX	// use PicoPadHSTX device configuration
	// setup system clock
	ClockPllSysFreq(PLL_KHZ);

#ifdef PRESET_VREG_VOLTAGE	// preset VREG voltage
	VregSetVoltage(PRESET_VREG_VOLTAGE); // set VREG voltage
#else
	VregSetVoltage(VREG_VOLTAGE_1_10); // set VREG voltage
#endif

#ifdef PRESET_FLASH_CLKDIV	// preset flash CLK divider
	SSI_SetFlashClkDiv(PRESET_FLASH_CLKDIV); // preset flash divider
#else
	SSI_SetFlashClkDiv(FLASH_CLKDIV); // preset flash divider
#endif

#endif
}

// initialize core 1
void NES_InitCore1()
{
#if USE_PICOPADHSTX && USE_DISPHSTX		// use PicoPadHSTX device configuration
	// Start Core1, which processes requests to the LCD
	DispHstxCore1Exec(core1_entry);
#else
	// Start Core1, which processes requests to the LCD
	Core1Exec(core1_entry);
#endif
}

// terminate core 1
void NES_TermCore1()
{
#if USE_PICOPADHSTX && USE_DISPHSTX	// use PicoPadHSTX device configuration
	RunEmul = False;
#else
	Core1Reset();
#endif
}

// NES initialize (returns False on error)
Bool NES_Init()
{
	// clear emulator context
	memset(NES, 0, sizeof(sNES) - sizeof(u32));

	// Start Core1, which processes requests to the LCD
	NES_InitCore1();

	// parse ROM (error - incorrect NES header)
	if (!ParseROM(builtinrom)) return False;

	// reset InfoNES (error - mapper is not supported)
	if (InfoNES_Reset() < 0) return False;

	// Initialize InfoNES
	InfoNES_Init();

	return True;
}

// NES terminate
void NES_Term()
{
	// terminate core 1
	NES_TermCore1();

	// terminate sound
	PWM_IntDisable(PWMSND_SLICE);		// PWM interrupt disable
	PWM_Disable(PWMSND_SLICE);		// disable PWM
	NVIC_IRQDisable(IRQ_PWM_WRAP);		// disable interrupt on NVIC controller
	PWM_IntClear(PWMSND_SLICE);		// clear interrupt request
}

// unlink pointer (0x00000000 absolute, 0xE0000000 ROM, 0xF0000000 NES structure)
void NES_pUnlink(void** p)
{
	u32 pp = (u32)*p;

	// NES
	if ((pp >= (u32)NES) && (pp < (u32)NES + sizeof(sNES)))
	{
		*p = (void*)(pp - (u32)NES + 0xF0000000);
	}

	// ROM
	else if ((pp >= (u32)builtinrom) && (pp < (u32)builtinrom + builtinrom_len))
	{
		*p = (void*)(pp - (u32)builtinrom + 0xE0000000);
	}

}

// link pointer back (0x00000000 absolute, 0xE0000000 ROM, 0xF0000000 NES structure)
void NES_pLink(void** p)
{
	u32 pp = (u32)*p;

	// NES
	if (pp >= 0xF0000000)
	{
		*p = (void*)(pp - 0xF0000000 + (u32)NES);
	}

	// ROM
	else if (pp >= 0xE0000000)
	{
		*p = (void*)(pp - 0xE0000000 + (u32)builtinrom);
	}
}

// unlink NES before save
void NES_Unlink()
{
	int i;
	NES_pUnlink((void**)&NES->srambank);
	NES_pUnlink((void**)&NES->vrom);
	for (i = 0; i < 4; i++) NES_pUnlink((void**)&NES->rombank[i]);
	NES_pUnlink((void**)&NES->ppu_bg_base);
	NES_pUnlink((void**)&NES->ppu_sp_base);
	NES_pUnlink((void**)&NES->apu.pulse1.sweep.target);
	NES_pUnlink((void**)&NES->apu.pulse2.sweep.target);
	for (i = 0; i < 16; i++) NES_pUnlink((void**)&NES->ppubank[i]);
}

// link NES back after save or load
void NES_Link()
{
	int i;
	NES_pLink((void**)&NES->srambank);
	NES_pLink((void**)&NES->vrom);
	for (i = 0; i < 4; i++) NES_pLink((void**)&NES->rombank[i]);
	NES_pLink((void**)&NES->ppu_bg_base);
	NES_pLink((void**)&NES->ppu_sp_base);
	NES_pLink((void**)&NES->apu.pulse1.sweep.target);
	NES_pLink((void**)&NES->apu.pulse2.sweep.target);
	for (i = 0; i < 16; i++) NES_pLink((void**)&NES->ppubank[i]);
}

int main()
{
	Bool oldoff;
	Bool press_up, press_down, press_left, press_right, press_a, press_b, press_x, press_y;

	// parse ROM (returns False on incorrect NES header)
	if (!ParseROM(builtinrom)) ResetToBootLoader();

	// calculate ROM CRC
	NES_RomCrc = Crc32BDMA(ROM, NES_ROMSize + NES_VROMSize);

	// setup system clock
	NES_InitSysClk();

	// initialize USB
#if USE_USB_HOST_HID
	UsbHostInit();
#endif

	// prepare pixel offset map in source scanline
	InitPixOffMap();

	// get home path
	HomePathLen = GetHomePath(HomePath, "/NES");
	NES_DispMode = NES_DISPMODE_EMU;
	DoEmuScreenShot = False;	// request to do emulator screenshot

	// NES initialize
	if (!NES_Init()) ResetToBootLoader();

	while (True)
	{
		// one frame emulation
		InfoNES_Cycle();

		// read buttons
#if USE_USB_HOST_HID
		press_up	= KeyPressed(KEY_UP)	|| UsbKeyIsPressed(HID_KEY_ARROW_UP);
		press_down	= KeyPressed(KEY_DOWN)	|| UsbKeyIsPressed(HID_KEY_ARROW_DOWN);
		press_left	= KeyPressed(KEY_LEFT)	|| UsbKeyIsPressed(HID_KEY_ARROW_LEFT);
		press_right	= KeyPressed(KEY_RIGHT)	|| UsbKeyIsPressed(HID_KEY_ARROW_RIGHT);
		press_a		= KeyPressed(KEY_A)	|| UsbKeyIsPressed(HID_KEY_CONTROL_LEFT) || UsbKeyIsPressed(HID_KEY_CONTROL_RIGHT);
		press_b		= KeyPressed(KEY_B)	|| UsbKeyIsPressed(HID_KEY_SHIFT_LEFT) || UsbKeyIsPressed(HID_KEY_SHIFT_RIGHT);
		press_x		= KeyPressed(KEY_X)	|| UsbKeyIsPressed(HID_KEY_ENTER) || UsbKeyIsPressed(HID_KEY_KEYPAD_ENTER);
		press_y		= KeyPressed(KEY_Y)	|| UsbKeyIsPressed(HID_KEY_BACKSPACE);
#else
		press_up	= KeyPressed(KEY_UP);
		press_down	= KeyPressed(KEY_DOWN);
		press_left	= KeyPressed(KEY_LEFT);
		press_right	= KeyPressed(KEY_RIGHT);
		press_a		= KeyPressed(KEY_A);
		press_b		= KeyPressed(KEY_B);
		press_x		= KeyPressed(KEY_X);
		press_y		= KeyPressed(KEY_Y);
#endif

		// load button state
		if (((MyPadState & GPUP) != 0) || !press_y)
		{
			MyPadState |= GPUP;
			if (!press_up) MyPadState ^= GPUP;
		}

		if (((MyPadState & GPDOWN) != 0) || !press_y)
		{
			MyPadState |= GPDOWN;
			if (!press_down) MyPadState ^= GPDOWN;
		}

		if (((MyPadState & GPY) != 0) || !(press_up || press_down))
		{
			MyPadState |= GPY;
			if (!press_y) MyPadState ^= GPY;
		}

		MyPadState |= GPLEFT | GPRIGHT | GPA | GPB | GPX; 
                if (!press_left) MyPadState ^= GPLEFT;
                if (!press_right) MyPadState ^= GPRIGHT;
                if (!press_a) MyPadState ^= GPA;
                if (!press_b) MyPadState ^= GPB;
                if (!press_x) MyPadState ^= GPX;

		// key input
		switch (KeyGet())
		{
		// Y + up: game menu
		case KEY_Y:
			if (!press_up && !press_down) break;
			if (press_up) goto RunMenu;
			break;

		// game menu
		case KEY_UP:

			if (!press_y) break;
RunMenu:
			oldoff = GlobalSoundOff;
			GlobalSoundOff = True;

			if (!NES_Menu())
			{
				// reset to boot loader
				ResetToBootLoader();
			}

			GlobalSoundOff = oldoff;
			NES_OldSyncTime = Time();
			break;

		// vertical zoom
		case KEY_DOWN:
			if (press_y) UseZoom = !UseZoom;
			break;
		}

#if USE_USB_HOST_HID
		// USB key input - Esc raise game menu
		switch (UsbGetKey() & 0xff)
		{
		case HID_KEY_ESCAPE:
			oldoff = GlobalSoundOff;
			GlobalSoundOff = True;

			if (!NES_Menu())
			{
				// reset to boot loader
				ResetToBootLoader();
			}

			GlobalSoundOff = oldoff;
			NES_OldSyncTime = Time();
			break;

		// Tab vertical zoom
		case HID_KEY_TAB:
			UseZoom = !UseZoom;
			break;
		}
#endif

	}
}
