
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define EMU_PWMTOP	4095	// PWM sound top (period = EMU_PWMTOP + 1 = 4096)
#define EMU_PWMCLOCK	(AUDIO_SAMPLE_RATE*(EMU_PWMTOP+1)) // PWM clock (= 32768*4096 = 134 217 728)

#define DISP_MINFPS	8	// minimal reuired display FPS (limit rendering to speed-up program emulation)
#define DISP_MAXFPS	22	// maximal reuired display FPS (limit rendering to speed-up program emulation)
#define DISP_MINDELAYUS	(1000000/(DISP_MAXFPS*HEIGHT)) // minimal delay per scanline in [us]
#define DISP_MAXDELAYUS	(1000000/(DISP_MINFPS*HEIGHT)) // maximal delay per scanline in [us]
#define DISP_INCDELAY	3	// speed of delay adaptation

// Time synchronization
#define EMU_SYNCTIME	((unsigned)(1000000/VERTICAL_SYNC)) // sync delta time in [us]
u32 OldSyncTime;		// old sync time
u32 LastLineUs;			// time of last line display
volatile u32 DelayLineUs = ((DISP_MINDELAYUS + DISP_MAXDELAYUS)/2); // current delay per line display

// CRAM
int CRamSize;		// size of CRAM to save (0 = no CRAM or no backup battery)
Bool CRamModi;		// CRAM modified
u32 CRamLastWrite;	// last time of write CRAM
u32 CRamBatch;		// batch counter of writes

u8 GPState = 0;		// game pad state

#if DEB_WRITECRAM	// 1=debug display write bytes into CRAM (in game menu)
u32 CRamWriteNum = 0;	// number of writes to CRAM
#endif

// home path
char HomePath[APPPATH_PATHMAX+15+1];
int HomePathLen;

// GameBoy context
struct gb_s gbContext;

// game title CRC
u16 TitleCrc;		// CRC16A of game title, game code, support code and maker code, address 0x0134..0x0145
u8 TitleCrc2;		// game title small CRC (value from address 0x14d)

#if DEB_FPS			// debug display FPS
u32 DebFpsTime;			// last time FPS
int DebFpsCount;		// FPS counter
int DebFps;			// last FPS
u16 DebFpsCol;			// FPS color
#endif

// sound
s16 StreamBuf[AUDIO_SAMPBUF*2];
u16 SndBuf[AUDIO_SAMPBUF];
int SndBufInx = AUDIO_SAMPBUF;

// ----------------------------------------------------------------------------
//                               ROM and cache
// ----------------------------------------------------------------------------

// Cache
ALIGNED FRAMETYPE FrameBuf[GB_CACHERAM_NUM*GB_CACHE_SIZE/sizeof(FRAMETYPE)]; // RAM cache pages
#define CacheBuf ((u8*)FrameBuf)	// RAM cache buffer
sGB_Cache GB_CacheDesc[GB_CACHE_NUM];	// cache page descriptors
u8 GB_CacheROM[GB_ROMCACHEMAX];		// ROM cache pages

sFile GB_ROMFile; // open ROM file
Bool GB_ReqExit; // request to exit program

// read ROM error (returns True to repeat)
Bool ReadRomErr(const char* msg)
{
	int i;
	u8 key;

 	GB_TextClear();
	GB_TextSetCol(COL_RED);
	GB_TextY = 1;
	GB_TextPrintCenter("Cannot read ROM");

	GB_TextSetCol(COL_GREEN);
	GB_TextY = 2;
	GB_TextX = 0;
	GB_PrepSaveFile(-2);
	if (SaveNameLen > GB_MSG_WIDTH)
		GB_TextPrint(&HomePath[SaveNameLen - GB_MSG_WIDTH]);
	else
		GB_TextPrintCenter(HomePath);
	GB_UnprepSaveFile();

	GB_TextY = 3;
	GB_TextSetCol(COL_WHITE);
	GB_TextPrintCenter("A=repeat, Y=exit");

	GB_TextSetCol(COL_GRAY);
	GB_TextY = 7;
	GB_TextPrintCenter(msg);

	// wait a key
	WaitMs(300);
	KeyFlush();
#if USE_USB_HOST_HID
	UsbFlushKey();
#endif
	while (True)
	{
		key = KeyGet();
		if (key == KEY_Y) return False;
		if (key == KEY_A) return True;

#if USE_USB_HOST_HID
		key = UsbGetKey() & 0xff;
		if ((key == HID_KEY_ESCAPE) ||
			(key == HID_KEY_BACKSPACE) ||
			(key == HID_KEY_Y)) return False;
		if (key == HID_KEY_A) return True;
#endif
	}
}

// GameBoy ROM read function
u8 FASTCODE NOFLASH(gbRomRead)(struct gb_s *gb, const uint_fast32_t addr)
{
	const char* msg;
	int i, n, n2;
	u8* d;
	Bool res;

	// address is out of range
	if (addr >= gameRomOrig) return 0xff;

	// get ROM page and offset
	int page = addr >> 14; // 16 KB per ROM page
	int off = addr & 0x3fff;

	// page is located at Flash memory
	if (page < gameFlashPages)
	{
		// get last byte of the page
		if (off == 0x3fff) return gameRomLastList[page];

		// stuff bytes, behind valid data of the page
		if (off >= gameRomSizeList[page]) return gameRomStuffList[page];

		// valid data of the page
		return gameRom[gameRomOffList[page] + off];
	}

	// cache page
	page = addr >> GB_CACHE_SHIFT; // ROM cache page
	if (page >= GB_ROMCACHEMAX) return 0xff; // invalid ROM page
	off = addr & GB_CACHE_MASK; // address offset

	// cache index is not valid
	u8 inx = GB_CacheROM[page];
	sGB_Cache* c;
	if (inx == GB_CACHEINX_INV)
	{
		// already exiting
		if (GB_ReqExit) return 0xff; // request to exit program

		// search free page
		c = GB_CacheDesc;
		for (inx = 0; inx < GB_CACHE_NUM; inx++)
		{
			// free page?
			if (c->type == GB_CACHETYPE_FREE) break;
			c++;
		}

		// no free page, must free oldest page
		//  Note: some used page will always be found
		if (inx == GB_CACHE_NUM)
		{
			u32 t = Time();
			u32 delta;
			u32 deltabest = 0;
			i = GB_CACHE_NUM-1;
			c = &GB_CacheDesc[i];
			for (; i >= 0; i--)
			{
				// used page?
				if (c->type == GB_CACHETYPE_VALID)
				{
					// check delta interval
					delta = t - c->last;
					if (delta > deltabest)
					{
						inx = (u8)i;
						deltabest = delta;
					}
				}
				c--;
			}

			// unuse oldest cache
			c = &GB_CacheDesc[inx];
			GB_CacheROM[c->rominx] = GB_CACHEINX_INV;
		}

		// mark this page as used
		c->type = GB_CACHETYPE_VALID;
		c->rominx = page;
		GB_CacheROM[page] = inx;

		// prepare page offset in the ROM file
		u32 romoff = page << GB_CACHE_SHIFT;

		// prepare cache page address
		if (inx < GB_CACHEINX_CRAM) // page is in RAM
		{
			d = &CacheBuf[((int)inx << GB_CACHE_SHIFT)];
		}
		else
		{
			d = &gbContext.cram[((int)(inx - GB_CACHEINX_CRAM) << GB_CACHE_SHIFT)];
		}

// repeat ROM open
ROMAGAIN:
		msg = "";
		n = 0;
		n2 = 0;

		// open file
		if (!FileIsOpen(&GB_ROMFile))
		{
			// mount disk
			if (!DiskAutoMount())
			{
				msg = "No SD disk";
				goto ROMERR;
			}

			// set directory
			if (!SetDir(HomePath)) // set directory
			{
				msg = "Invalid directory";
				goto ROMERR;
			}

			// open file
			GB_PrepSaveFile(-2); // prepare save filename
			res = FileOpen(&GB_ROMFile, HomePath); // open file
			GB_UnprepSaveFile(); // unprepare save filename
			if (!res)			
			{
				msg = "Cannot open file";
				goto ROMERR;
			}
		}

		// seek file
		if (!FileSeek(&GB_ROMFile, romoff))
		{
			msg = "Seek error";
			goto ROMERR;
		}

		// read page
		n = gameRomOrig - romoff;
		if (n > GB_CACHE_SIZE) n = GB_CACHE_SIZE;
		n2 = FileRead(&GB_ROMFile, d, n);
		if (n != n2)
		{
			msg = "Read error";
// ROM read error
ROMERR:
			// set message display mode
			GB_DispMode = GB_DISPMODE_MSG;

			// display message (returns True to repeat)
			res = ReadRomErr(msg);

			// set emlator display mode
			GB_DispMode = GB_DISPMODE_EMU;

			// request to exit
			if (!res)
			{
				GB_ReqExit = True;
				return 0xff;
			}

			// repeat
			FileClose(&GB_ROMFile);
			DiskUnmount(); // unmount disk
			DiskMount(); // mount disk
			goto ROMAGAIN;
		}
	}

	// cache is now valid, read byte from the cache
	c = &GB_CacheDesc[inx]; // get cache descriptor
	c->last = Time(); // time mark
	if (inx < GB_CACHEINX_CRAM) // page is at RAM
	{
		// read byte from RAM
		return CacheBuf[((int)inx << GB_CACHE_SHIFT) + off];
	}
	else
	{
		// read byte from CRAM
		return gbContext.cram[((int)(inx - GB_CACHEINX_CRAM) << GB_CACHE_SHIFT) + off];
	}
}

// ----------------------------------------------------------------------------
//                             Cartridge RAM
// ----------------------------------------------------------------------------

// GameBoy cartridge RAM read function
u8 FASTCODE NOFLASH(gbCartRamRead)(struct gb_s *gb, const uint_fast32_t addr)
{
	// prepare to invalidate ROM page
	int page = addr >> GB_CACHE_SHIFT; // cache page
	int pagemask = BIT(page); // page flag mask

	// page is not assigned to CRAM yet
	if ((gbContext.cram_valid & pagemask) == 0) return 0;

	// return data from CRAM
	return gbContext.cram[addr];
}

// GameBoy cartridge RAM write function
void FASTCODE NOFLASH(gbCartRamWrite)(struct gb_s *gb, const uint_fast32_t addr, const u8 val)
{
	// prepare to invalidate ROM page
	int page = addr >> GB_CACHE_SHIFT; // cache page
	int pagemask = BIT(page); // page flag mask

	// page is not assigned to CRAM yet
	if ((gbContext.cram_valid & pagemask) == 0)
	{
		// assigning page to CRAM memory
		gbContext.cram_valid |= pagemask;

		// clear the page
		memset(&gbContext.cram[page << GB_CACHE_SHIFT], 0, GB_CACHE_SIZE);

		// get cache descriptor
		page += GB_CACHEINX_CRAM; // index of the cache page
		sGB_Cache* c = &GB_CacheDesc[page]; // cache descriptor

		// page was used
		if (c->type == GB_CACHETYPE_VALID)
		{
			// unuse cache
			GB_CacheROM[c->rominx] = GB_CACHEINX_INV;
		}

		// forbidden this page
		c->type = GB_CACHETYPE_FORB;
	}

#if DEB_WRITECRAM	// 1=debug display write bytes into CRAM (in game menu)
	CRamWriteNum++;	// number of writes to CRAM
#endif

	// write counter
	if (addr < CRamSize)		// only if need backup
	{
		CRamBatch++;		// batch counter of writes
		CRamLastWrite = Time();		// time of last modification
	}

	// store value
	if (gbContext.cram[addr] != val)	// value changed?
	{
		gbContext.cram[addr] = val;	// write new value
		if (addr < CRamSize)		// only if need backup
		{
			CRamModi = True;		// CRAM was modified
		}
	}
}

// save CRAM if modified
void gbCartRamSave()
{
	sFile f;
	int i, i2, n, n2, mask, off;
	u8 buf[512];
	Bool res;
	Bool repeat = True;

	// check CRAM size
	int num = CRamSize;
	if (num == 0) return;

	// mount disk
	res = DiskAutoMount();
	if (!res)
	{
CartSaveRepeat:
		repeat = False;
		DiskUnmount(); // unmount disk
		res = DiskAutoMount();
	}

	if (res) 
	{
		// set directory
		res = SetDir(HomePath);
		if (!res && repeat) goto CartSaveRepeat;

		if (res)
		{
			// prepare save filename
			GB_PrepSaveFile(-1);

			// delete old file
			FileDelete(HomePath);

			// create file
			res = FileCreate(&f, HomePath);

			// unprepare save filename
			GB_UnprepSaveFile();

			if (!res && repeat) goto CartSaveRepeat;

			if (res)
			{
				// clear temporary buffer
				memset(buf, 0, 512);

				// save pages
				mask = B0;	// mask of ROM cache
				off = 0;	// offset in CRAM buffer
				while (num > 0)	// while some data to save
				{
					// size of this page
					n = num;
					if (n > GB_CACHE_SIZE) n = GB_CACHE_SIZE;

					// page is assigned to CRAM
					if ((gbContext.cram_valid & mask) != 0)
					{
						// write this page of valid CRAM data
						n2 = FileWrite(&f, &gbContext.cram[off], n);
						if ((n2 != n) && repeat)
						{
							FileClose(&f);
							goto CartSaveRepeat;
						}
					}

					// page is used as ROM cache - write zero bytes
					else
					{
						// write zero sectors
						for (i = n; i > 0; i -= 512)
						{
							// write invalid CRAM data
							i2 = (i > 512) ? 512 : i;
							n2 = FileWrite(&f, buf, i2);
							if ((n2 != i2) && repeat)
							{
								FileClose(&f);
								goto CartSaveRepeat;
							}
						}
					}

					// shift to next CRAM page
					off += GB_CACHE_SIZE;	// shift offset in CRAM buffer
					num -= n;		// decrease number of reamining data
					mask <<= 1;		// shift mask of ROM cache
				}

				// close file
				FileClose(&f);
			}
		}
	}
}

// load CRAM on startup
void gbCartRamLoad()
{
	sFile f;
	int i, n, mask, off;
	sGB_Cache* c;
	Bool res;
	Bool repeat = True;

	// check CRAM size
	int num = CRamSize;
	if (num == 0) return;

	// mount disk
	res = DiskAutoMount();
	if (!res)
	{
CartLoadRepeat:
		repeat = False;
		DiskUnmount(); // unmount disk
		res = DiskAutoMount();
	}

	if (res) 
	{
		// set directory
		res = SetDir(HomePath);
		if (!res && repeat) goto CartLoadRepeat;

		if (res)
		{
			// prepare save filename
			GB_PrepSaveFile(-1);

			// open file
			res = FileOpen(&f, HomePath);

			// unprepare save filename
			GB_UnprepSaveFile();

			if (!res && repeat) goto CartLoadRepeat;

			if (res)
			{
				// load file
				n = FileRead(&f, gbContext.cram, num);

				// close file
				FileClose(&f);

				if ((n != num) && repeat) goto CartLoadRepeat;

				// mark pages
				mask = B0;	// mask of ROM cache
				off = 0;	// offset in CRAM buffer
				c = &GB_CacheDesc[GB_CACHEINX_CRAM]; // first cache descriptor

				while (num > 0)	// while some data to save
				{
					// page was used by ROM cache - invalidate cache
					if (c->type == GB_CACHETYPE_VALID)
					{
						// unuse cache
						GB_CacheROM[c->rominx] = GB_CACHEINX_INV;
					}

					// size of this page
					n = num;
					if (n > GB_CACHE_SIZE) n = GB_CACHE_SIZE;

					// check if this page has some data
					c->type = GB_CACHETYPE_FREE;
					gbContext.cram_valid &= ~mask; // clear mask (= not used by CRAM)
					for (i = 0; i < n; i++)
					{
						// check zero data
						if (gbContext.cram[off + i] != 0)
						{
							// has valid data
							gbContext.cram_valid |= mask; // mask as used by CRAM

							// forbidden this page
							c->type = GB_CACHETYPE_FORB;
							break;
						}
					}

					// shift to next CRAM page
					off += GB_CACHE_SIZE;	// shift offset in CRAM buffer
					num -= n;		// decrease number of reamining data
					mask <<= 1;		// shift mask of ROM cache
					c++;			// next cache descriptor
				}
			}
		}
	}
}

// ----------------------------------------------------------------------------
//                              Palettes
// ----------------------------------------------------------------------------

// convert BGR55 to RGB565
u16 convertBGR555toRGB565(u16 color)
{
	int r = (color & 0x001F) << 11; // red
	int g = ((color & 0x03E0) << 1) | ((color & 0x0200) >> 4);  // green
	int b = (color & 0x7C00) >> 10; // blue
	return (u16)(r | g | b);
}

// set palette
void gbSetPal(u8 info)
{
	// store current index
	int i, j;
	for (i = 0; i < PAL_NUM; i++)
	{
		if (info == gbColorPalSel[i])
		{
			gbContext.display.palinx = (u8)i;
			break;
		}
	}

	// get colorization palette info
	u8 palette = info & 0x1F; // palette
	u8 flags = (info & 0xE0) >> 5; // flags

	if (flags == 7)
	{
		palette += 0x20;
		flags = 5;
	}

	// Normally the first palette is used as OBP0.
	// If bit 0 is zero, the third palette is used instead.
	const u16* obp0 = 0;
	if (flags & 1)
		obp0 = gbColorizationPaletteData[palette][0];
	else
		obp0 = gbColorizationPaletteData[palette][2];

	// Normally the second palette is used as OBP1.
	// If bit 1 is set, the first palette is used instead.
	// If bit 2 is zero, the third palette is used instead.
	const u16* obp1 = 0;
	if (!(flags & 4))
		obp1 = gbColorizationPaletteData[palette][2];
	else if (flags & 2)
		obp1 = gbColorizationPaletteData[palette][0];
	else
		obp1 = gbColorizationPaletteData[palette][1];

	// Third palette is always used for BGP.
	const u16* bgp = gbColorizationPaletteData[palette][2];

	// convert palettes
	u16 c;
	for (i = 0; i < 4; i++)
	{
		// OBJ0
		c = convertBGR555toRGB565(*obp0++);
		for (j = 0; j < 16; j += 4) gbPalette[i+j] = c;

		// OBJ1
		c = convertBGR555toRGB565(*obp1++);
		for (; j < 32; j += 4) gbPalette[i+j] = c;

		// BGP
		c = convertBGR555toRGB565(*bgp++);
		for (; j < 64; j += 4) gbPalette[i+j] = c;
	}
}

// select colorization palette
// - This part of code was taken from the Visual Boy Advance emulator
void gbSelectColorizationPalette()
{
	int infoIdx;
	int i, j;
	u16 crc = TitleCrc;
	u8 crc2 = TitleCrc2;
	u8 b;

	// custom setup palette
	const u16* c = gbPalExt;
	while ((c[0] != 0) || (c[1] != 0))
	{
		if (c[0] == crc)
		{
			b = (u8)(c[1] >> 8);
			if ((b == 0) || (b == crc2))
			{
				gbSetPal(gbColorPalSel[(c[1] & 0xff) - 1]);
				return;
			}
		}
		c += 2;
	}

	// default palette 0
	infoIdx = 0;

	// Check if licensee is Nintendo. If not, use default palette.
	if (gameRom[0x014B] == 0x01 ||
		(gameRom[0x014B] == 0x33 && gameRom[0x0144] == 0x30 && gameRom[0x0145] == 0x31))
	{
		// Calculate the checksum over 16 title bytes.
		u8 checksum = 0;
		for (i = 0; i < 16; i++) checksum += gameRom[0x0134 + i];

		// Check if the checksum is in the list.
		int idx;
		for (idx = 0; idx < sizeof(gbColorizationChecksums); idx++)
		{
			if (gbColorizationChecksums[idx] == checksum) break;
		}

		// Was the checksum found in the list?
		if (idx < sizeof(gbColorizationChecksums))
		{
			// Indexes above 0x40 (64) have to be disambiguated.
			if (idx > 0x40)
			{
				// No idea how that works. But it works.
				i = idx - 0x41;
				j = 0;
				for (; i < sizeof(gbColorizationDisambigChars); i += 14, j += 14)
				{
					if (gameRom[0x0137] == gbColorizationDisambigChars[i])
					{
						infoIdx = idx + j;
						break;
					}
				}
			}
			else
				// Lower indexes just use the index from the checksum list.
				infoIdx = idx;
		}
	}

	// set palette
	gbSetPal(gbColorizationPaletteInfo[infoIdx]);
}

// ----------------------------------------------------------------------------
//                                Run game
// ----------------------------------------------------------------------------

// Error handler function
void gbErrorHandler(struct gb_s *gb, const enum gb_error_e gb_err, const u16 addr) { reset_usb_boot(0, 0); }

// draw one line from frame buffer
void FASTCODE NOFLASH(core1DrawFrame)()
{
	int x, y, ys, ys2, rinx;
	u16* s;
	u16 c;

#if DEB_FPS			// debug display FPS
	u16 buf[16*16];
	u8 d1 = DebFps/10 + '0';
	u8 d2 = (DebFps % 10) + '0';
	u16* d = buf;
	for (y = 0; y < 16; y++)
	{
		u8 ch = FontBold8x16[d1 + y*256];
		for (x = 8; x > 0; x--)
		{
			*d++ = (ch & 0x80) ? DebFpsCol : COL_BLACK;
			ch <<= 1;
		}

		ch = FontBold8x16[d2 + y*256];
		for (x = 8; x > 0; x--)
		{
			*d++ = (ch & 0x80) ? DebFpsCol : COL_BLACK;
			ch <<= 1;
		}
	}
#endif

	// wait for first scanline (without opening output to LCD)
	rinx = gbContext.frame_read;
	while (rinx == gbContext.frame_write)
	{
		if (GB_DispMode == GB_DISPMODE_MSG) return;
	}

	// do screenshot
	y = gbContext.frame_rline;
	dmb();
	if (DoEmuScreenShotReq && (y == 0))
	{
		DoEmuScreenShot = True;	// request to do emulator screenshot
		dmb();
		DoEmuScreenShotReq = False;
		dmb();
	}

	// start sending image data
	DispStartImg(0, WIDTH, y, HEIGHT);

	ys2 = y*LCD_HEIGHT/HEIGHT;

	for (; y < HEIGHT;)
	{
		// wait for scan line to be ready
		rinx = gbContext.frame_read;
		while (rinx == gbContext.frame_write)
		{
			if (GB_DispMode == GB_DISPMODE_MSG)
			{
				DispStopImg();
				return;
			}
		}

		// source address
		ys = ys2;
		s = &gbContext.framebuf[rinx*LCD_WIDTH];

#if DEB_FPS			// debug display FPS
		if (y < 16)
		{
			u16* s2 = &buf[16*y];
			for (x = LCD_WIDTH; x > 0; x--)
			{
				if (x <= 8)
				{
					DispSendImg2(*s2++);
					DispSendImg2(*s2++);
				}
				else
				{
					c = *s++;
					DispSendImg2(c);
					DispSendImg2(c);
				}
			}
		}
		else
#endif
		for (x = LCD_WIDTH; x > 0; x--)
		{
			c = *s++;
			DispSendImg2(c);
			DispSendImg2(c);
		}

		// next source Y
		// Check case: y = 239, old ys = 239*144/240 = 143, new ys2 = 240*144/240 = 144, it is OK
		y++;
		ys2 = y*LCD_HEIGHT/HEIGHT;

		// update read index - only if we will not need this scan line again
		if (ys != ys2)
		{
			dmb();

			rinx++;	// increase read index
			if (rinx >= LCD_FRAMEHEIGHT) rinx = 0; // wrap read index
			gbContext.frame_read = rinx;

			dmb();
		}

		// increase Y
		rinx = gbContext.frame_rline + 1; // increase absolute line
		if (rinx >= HEIGHT) rinx = 0; // wrap current line
		gbContext.frame_rline = rinx;

		// slow down FPS to speed up emulation
		u32 del = DelayLineUs;
		u32 line = LastLineUs;
		while ((u32)(Time() - line) < del) {}
		LastLineUs = Time();
	}

	// stop sending data
	DispStopImg();
}

// PWM audio interrupt handler
void FASTCODE NOFLASH(PWMSndIrq)()
{
	// clear interrupt request
	PWM_IntClear(PWMSND_SLICE);

	// end of buffer
	int i = SndBufInx;
	if (i >= AUDIO_SAMPBUF)
	{
		// check mute
		int vol = Config.volume; // volume 0..255, 100=normal (we will use 128)
		if ((vol == 0) || GlobalSoundOff)
		{
			// mute sound
			for (i = 0; i < AUDIO_SAMPBUF; i++) SndBuf[i] = EMU_PWMTOP/2;
		}

		// prepare next sound buffer
		else
		{
			// render buffer
			audio_callback(StreamBuf, AUDIO_SAMPBUF);

			// convert sound to mono
			s16* s = StreamBuf; // input stream
			u16* d = SndBuf; // output stream

			int leftChannel, rightChannel;
			int i;
			for (i = AUDIO_SAMPBUF; i > 0; i--)
			{
				leftChannel = *s++;
				rightChannel = *s++;

				// (left + right)*vol ... max. 24 bit number, normal 23 bit number, we will need 12 bit number
				int mono = ((leftChannel + rightChannel)*vol) >> 11;
				mono += EMU_PWMTOP/2;

				// Clip the value to the range of a 16-bit signed integer
				if (mono < 0) mono = 0;
				if (mono > EMU_PWMTOP) mono = EMU_PWMTOP;
				*d++ = (u16)mono;
			}
		}

		i = 0;
	}

	// get next sample
	u16 samp = SndBuf[i];
	i++;
	SndBufInx = i;

	// output sample
	PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, samp);
}

// Main function for Core 1
void core1_entry()
{
	LastLineUs = Time();

#if DEB_FPS			// debug display FPS
	DebFpsTime = Time();	// last time FPS
	DebFpsCount = 0;	// FPS counter
	DebFps = 0;		// last FPS
#endif

	while (True)
	{
		// message text mode
		if (GB_DispMode == GB_DISPMODE_MSG)
		{
			GB_TextUpdate();
			LastLineUs = Time();
		}
		else
		{
			core1DrawFrame();
		}

#if DEB_FPS			// debug display FPS
		DebFpsCount++;
		if ((Time() - DebFpsTime) >= 1000000)
		{
			DebFps = DebFpsCount;
			DebFpsTime = Time();
			DebFpsCount = 0;
		}
#endif
	}
}

// ----------------------------------------------------------------------------
//                               Initialize
// ----------------------------------------------------------------------------

// initialize system clock
void GB_InitSysClk()
{
	// setup voltage
	u32 clk = gbContext.cgb.cgbMode ? EMU_CLKSYS_GBC : EMU_CLKSYS_GB;
	if (clk >= 285000)
	{
		SSI_InitFlash(6);
		VregSetVoltage(VREG_VOLTAGE_1_30);
	}
	else if (clk >= 245000)
		VregSetVoltage(VREG_VOLTAGE_1_30);
	else if (clk >= 220000)
		VregSetVoltage(VREG_VOLTAGE_1_25);
	else if (clk >= 190000)
		VregSetVoltage(VREG_VOLTAGE_1_20);
	else if (clk >= 150000)
		VregSetVoltage(VREG_VOLTAGE_1_15);
	else
		VregSetVoltage(VREG_VOLTAGE_1_10);

	// setup system clock
	ClockPllSysFreq(clk);
}

// restore system clock
void GB_TermSysClk()
{
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
}

// initialize core 1
void GB_InitCore1()
{
	// Start Core1, which processes requests to the LCD
	Core1Exec(core1_entry);
}

// terminate core 1
void GB_TermCore1()
{
	Core1Reset();
}

void GB_Setup()
{
	// get title CRC (CRC16A of game title, game code, support code and maker code, address 0x0134..0x0145)
	TitleCrc = Crc16AFast(&gameRom[0x134], 18);

	// game title small CRC (value from address 0x14d)
	TitleCrc2 = gameRom[0x14d];

	// clear caches
	memset(GB_CacheDesc, GB_CACHETYPE_FREE, sizeof(GB_CacheDesc));
	memset(GB_CacheROM, GB_CACHEINX_INV, sizeof(GB_CacheROM));
	GB_ReqExit = False; // request to exit program

	// clear context
	memset(&gbContext, 0, sizeof(gbContext));

	// select colorization palette
	gbSelectColorizationPalette();

	// Initialize GB context
	enum gb_init_error_e initStatus = gb_init(&gbContext, &gbRomRead,
			&gbCartRamRead, &gbCartRamWrite, &gbErrorHandler, nullptr);
	if (initStatus != GB_INIT_NO_ERROR) reset_usb_boot(0, 0);

	// setup system clock
	GB_InitSysClk();

	// get size of CRAM to save (0 = no CRAM or no backup battery)
	CRamSize = gb_get_save_size(&gbContext);

	// load CRAM on startup
	gbCartRamLoad();
	CRamBatch = 0;		// batch counter of writes

	// initialize LCD interface
	gb_init_lcd(&gbContext);

	// Init audio
	audio_init(&gbContext);

	// initialize PWM sound output
	PWM_Reset(PWMSND_SLICE);		// reset PWM to default state
	PWM_GpioInit(PWMSND_GPIO);		// set GPIO function to PWM
	SetHandler(IRQ_PWM_WRAP, PWMSndIrq);	// set IRQ handler
	NVIC_IRQEnable(IRQ_PWM_WRAP);		// enable interrupt on NVIC controller
	PWM_Clock(PWMSND_SLICE, EMU_PWMCLOCK);	// set clock divider
	PWM_Top(PWMSND_SLICE, EMU_PWMTOP);	// set period to 256 cycles
	PWM_Comp(PWMSND_SLICE, PWMSND_CHAN, EMU_PWMTOP/2); // write default PWM sample
	PWM_Enable(PWMSND_SLICE);		// enable PWM
	PWM_IntEnable(PWMSND_SLICE);		// PWM interrupt enable

	// Start Core1, which processes requests to the LCD
	multicore_launch_core1(core1_entry);

	OldSyncTime = Time();		// old sync time
}

// ----------------------------------------------------------------------------
//                              Main function
// ----------------------------------------------------------------------------

// main function
int main()
{
	Bool oldoff;
	u32 dif, del;

// Debug internal - check duplicate CRC in palette table
#if 0
	{
		int i, j;
		u16 crc, crc2;
		u8 crcB, crc2B;
		for (i = 0; ; i++)
		{
			crc = gbPalExt[i*2];
			if ((crc == 0) && (gbPalExt[i*2+1] == 0)) break;
			crcB = (u8)(gbPalExt[i*2+1] >> 8);

			for (j = i + 1; ; j++)
			{
				crc2 = gbPalExt[j*2];
				if ((crc2 == 0) && (gbPalExt[j*2+1] == 0)) break;
				crc2B = (u8)(gbPalExt[j*2+1] >> 8);

				if ((crc == crc2) && (crcB == crc2B))
				{
					GB_MenuEnter();
					GB_TextPrintCenter("IntError:Dupl.CRC!");
					GB_TextPrintNL(1);
					GB_TextPrintHexByte((u8)(crc >> 8));
					GB_TextPrintHexByte((u8)crc);
					GB_TextUpdate();
					while (True) {}
				}
			}
		}
	}
#endif			

	// initialize USB
#if USE_USB_HOST_HID
	UsbHostInit();
#endif
	Bool press_up, press_down, press_left, press_right, press_a, press_b, press_x, press_y;

	// get home path
	HomePathLen = GetHomePath(HomePath, "/GAMEBOY");

	// display mode
	GB_DispMode = GB_DISPMODE_EMU;
	DoEmuScreenShot = False;	// request to do emulator screenshot
	FileInit(&GB_ROMFile);

	// setup
	GB_Setup();

	// check access to ROM file - read last byte from the ROM
	u8 b = gbRomRead(&gbContext, gameRomOrig-1);

	while (True)
	{
		// run one frame
		gb_run_frame(&gbContext);

		// auto speed FPS
		dif = Time() - OldSyncTime;
		del = DelayLineUs;
		if (dif >= EMU_SYNCTIME) // lost synchronization?
		{
			// slow emulation, decrease FPS
#if DEB_FPS			// debug display FPS
			DebFpsCol = COL_RED;
#endif
			del += DISP_INCDELAY;
			if (del > DISP_MAXDELAYUS) del = DISP_MAXDELAYUS;
		}
		else
		{
			// fast emulation, increase FPS
#if DEB_FPS			// debug display FPS
			DebFpsCol = COL_GREEN;
#endif
			del -= DISP_INCDELAY;
			if (del < DISP_MINDELAYUS) del = DISP_MINDELAYUS;
		}
		DelayLineUs = del;

		// time synchronization
		for (;;)
		{
			dif = Time() - OldSyncTime;
			if (dif >= EMU_SYNCTIME) break;
		}

		if (dif >= 20*EMU_SYNCTIME)
			OldSyncTime = Time();
		else
			OldSyncTime += EMU_SYNCTIME;

		// exit
		if (GB_ReqExit)
		{
			// write CRAM if modified
			if (CRamModi) gbCartRamSave();
			ResetToBootLoader();
		}

		// Map Picopad buttons to Peanut GB emulator
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

		// Prevent passing simultaneous arrow presses with Y/X to enable game menu recall.
		if ((gbContext.direct.joypad_bits.up == 0) || !press_y)
			gbContext.direct.joypad_bits.up = !press_up;

		gbContext.direct.joypad_bits.down = !press_down;

		if ((gbContext.direct.joypad_bits.left == 0) || !press_y)
			gbContext.direct.joypad_bits.left = !press_left;

		if ((gbContext.direct.joypad_bits.right == 0) || !press_y)
			gbContext.direct.joypad_bits.right = !press_right;

		gbContext.direct.joypad_bits.a = !press_a;
		gbContext.direct.joypad_bits.b = !press_b;
		gbContext.direct.joypad_bits.start = !press_x;

		if ((gbContext.direct.joypad_bits.select == 0) || (!press_up && !press_left && !press_right))
			gbContext.direct.joypad_bits.select = !press_y;

		// key input
		switch (KeyGet())
		{
		// Y + up (or down or X): game menu
		case KEY_Y:
			if (!press_up && !press_left && !press_right) break;
			if (!press_left && !press_right) goto RunMenu;
			break;

		// game menu
		case KEY_UP:

			if (!press_y) break;
RunMenu:
			oldoff = GlobalSoundOff;
			GlobalSoundOff = True;

			if (!GB_Menu())
			{
				// write CRAM if modified
				if (CRamModi) gbCartRamSave();

				// reset to boot loader
				ResetToBootLoader();
			}

			GlobalSoundOff = oldoff;
			OldSyncTime = Time();
			break;

		// previous palette
		case KEY_LEFT:
			if (press_y)
			{
				u8 pal = gbContext.display.palinx;
				pal = (pal > 0) ? (pal - 1) : (PAL_NUM-1);
				gbSetPal(gbColorPalSel[pal]);
			}
			break;

		// next palette
		case KEY_RIGHT:
			if (press_y)
			{
				u8 pal = gbContext.display.palinx;
				pal = (pal < PAL_NUM-1) ? (pal + 1) : 0;
				gbSetPal(gbColorPalSel[pal]);
			}
			break;
		}

#if USE_USB_HOST_HID
		// USB key input - Esc raise game menu
		switch (UsbGetKey() & 0xff)
		{
		case HID_KEY_ESCAPE:
			oldoff = GlobalSoundOff;
			GlobalSoundOff = True;

			if (!GB_Menu())
			{
				// write CRAM if modified
				if (CRamModi) gbCartRamSave();

				// reset to boot loader
				ResetToBootLoader();
			}

			GlobalSoundOff = oldoff;
			OldSyncTime = Time();
			break;
		}
#endif

		// write CRAM if modified (delay 2 seconds from last modification)
		if (CRamModi && ((u32)(Time() - CRamLastWrite) > 2000000))
		{
			// save CRAM
			if (CRamBatch > GB_CRAMBATCH)
			{
				gbCartRamSave();
			}
			CRamModi = False; // not modified
			CRamBatch = 0;	// batch counter of writes
		}
	}
}
