
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                                Video modes
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico/Pico2 and RP2040/RP2350
// Copyright (c) 2023-2025 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// Note: The following 2 switches are not yet defined in the PicoLibSDK at this point, so the global.h file is included.
#if USE_DISPHSTX && DISPHSTX_PICOSDK	// 0=use PicoLibSDK library, 1=use PicoSDK original Raspberry SDK library
#include "disphstx_picolibsk.h"
#else
#include "../../global.h"
#endif

#if USE_DISPHSTX		// 1=use HSTX Display driver

#include "disphstx.h"

#ifndef COLOR16
#define COLOR16(r,g,b)	((u16)( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) ))
#endif

#ifndef COL16_BLACK
#define COL16_BLACK	COLOR16(  0,  0,  0)
#endif

#ifndef COL16_BLUE
#define COL16_BLUE	COLOR16(  0,  0,255)
#endif

#ifndef COL16_RED
#define COL16_RED	COLOR16(255,  0,  0)
#endif

#ifndef COL16_YELLOW
#define COL16_YELLOW	COLOR16(255,255,  0)
#endif

#ifndef COL16_WHITE
#define COL16_WHITE	COLOR16(255,255,255)
#endif

#if DISPHSTX_CHECK_LOAD	// 1 = check CPU load during IRQ interrupt (get DispHstxTimeIn, DispHstxTimeOut)
u32 DispHstxTimeOld;
volatile u64 DispHstxTimeIn, DispHstxTimeOut;
volatile u32 DispHstxTimeInMax, DispHstxTimeOutMin, DispHstxTimeNum;
#endif

// current selected display mode DISPHSTX_DISPMODE_*
int DispHstxDispMode = DISPHSTX_DISPMODE_NONE;
Bool DispHstxDispSelInitOK = False;

// default palettes of 1-bit format DISPHSTX_FORMAT_1 (black & white)
u16 DispHstxPal1b[2] ALIGNED = { COL16_BLACK, COL16_WHITE };

// default color 2-bit palettes
u16 DispHstxPal2bcol[4] ALIGNED = { COL16_BLACK, COL16_BLUE, COL16_RED, COL16_YELLOW, };

// default palettes of 2-bit format DISPHSTX_FORMAT_2 (grascale 0..3)
u16 DispHstxPal2b[4] ALIGNED = { COL16_BLACK, COLOR16(85,85,85), COLOR16(170,170,170), COL16_WHITE };

// default palettes of 3-bit format DISPHSTX_FORMAT_3 (RGB111)
u16 DispHstxPal3b[8] ALIGNED = {
	0x0000, 0x001F, 0x07E0, 0x07FF, 0xF800, 0xF81F, 0xFFE0, 0xFFFF,
};

// default palettes of 4-bit format DISPHSTX_FORMAT_4 (grayscale 0..15)
u16 DispHstxPal4b[16] ALIGNED = {
	0x0000, 0x1082, 0x2104, 0x3186, 0x4228, 0x52AA, 0x632C, 0x73AE,
	0x8C51, 0x9CD3, 0xAD55, 0xBDD7, 0xCE79, 0xDEFB, 0xEF7D, 0xFFFF
};

// default color 4-bit palettes YRGB1111
u16 DispHstxPal4bcol[16] ALIGNED = { 
	0x0000, 0x0015, 0x0540, 0x0555, 0xA800, 0xA815, 0xAAA0, 0xAD55,
	0x52AA, 0x52BF, 0x57EA, 0x57FF, 0xFAAA, 0xFABF, 0xFFEA, 0xFFFF, 

	/*COL16_BLACK, COL16_BLUE, COL16_GREEN, COL16_CYAN, COL16_RED,
	COL16_MAGENTA, COL16_YELLOW, COL16_LTGRAY, COL16_GRAY, COL16_LTBLUE, COL16_LTGREEN,
	COL16_LTCYAN, COL16_LTRED, COL16_LTMAGENTA, COL16_LTYELLOW, COL16_WHITE*/
 };

// default palettes of 6-bit format DISPHSTX_FORMAT_6 (RGB222)
u16 DispHstxPal6b[64] ALIGNED = {
	0x0000, 0x000A, 0x0015, 0x001F, 0x02A0, 0x02AA, 0x02B5, 0x02BF, 0x0540, 0x054A, 0x0555, 0x055F, 0x07E0, 0x07EA, 0x07F5, 0x07FF, 
	0x5000, 0x500A, 0x5015, 0x501F, 0x52A0, 0x52AA, 0x52B5, 0x52BF, 0x5540, 0x554A, 0x5555, 0x555F, 0x57E0, 0x57EA, 0x57F5, 0x57FF, 
	0xA800, 0xA80A, 0xA815, 0xA81F, 0xAAA0, 0xAAAA, 0xAAB5, 0xAABF, 0xAD40, 0xAD4A, 0xAD55, 0xAD5F, 0xAFE0, 0xAFEA, 0xAFF5, 0xAFFF, 
	0xF800, 0xF80A, 0xF815, 0xF81F, 0xFAA0, 0xFAAA, 0xFAB5, 0xFABF, 0xFD40, 0xFD4A, 0xFD55, 0xFD5F, 0xFFE0, 0xFFEA, 0xFFF5, 0xFFFF, 
};

// default palettes of 8-bit format DISPHSTX_FORMAT_8 (RGB332)
u16 DispHstxPal8b[256] ALIGNED = {
	0x0000, 0x000A, 0x0018, 0x001F, 0x0140, 0x014A, 0x0158, 0x015F, 0x02A0, 0x02AA, 0x02B8, 0x02BF, 0x03A0, 0x03AA, 0x03B8, 0x03BF, 
	0x0520, 0x052A, 0x0538, 0x053F, 0x0600, 0x060A, 0x0618, 0x061F, 0x0720, 0x072A, 0x0738, 0x073F, 0x07E0, 0x07EA, 0x07F8, 0x07FF, 
	0x2800, 0x280A, 0x2818, 0x281F, 0x2940, 0x294A, 0x2958, 0x295F, 0x2AA0, 0x2AAA, 0x2AB8, 0x2ABF, 0x2BA0, 0x2BAA, 0x2BB8, 0x2BBF, 
	0x2D20, 0x2D2A, 0x2D38, 0x2D3F, 0x2E00, 0x2E0A, 0x2E18, 0x2E1F, 0x2F20, 0x2F2A, 0x2F38, 0x2F3F, 0x2FE0, 0x2FEA, 0x2FF8, 0x2FFF, 
	0x5000, 0x500A, 0x5018, 0x501F, 0x5140, 0x514A, 0x5158, 0x515F, 0x52A0, 0x52AA, 0x52B8, 0x52BF, 0x53A0, 0x53AA, 0x53B8, 0x53BF, 
	0x5520, 0x552A, 0x5538, 0x553F, 0x5600, 0x560A, 0x5618, 0x561F, 0x5720, 0x572A, 0x5738, 0x573F, 0x57E0, 0x57EA, 0x57F8, 0x57FF, 
	0x7000, 0x700A, 0x7018, 0x701F, 0x7140, 0x714A, 0x7158, 0x715F, 0x72A0, 0x72AA, 0x72B8, 0x72BF, 0x73A0, 0x73AA, 0x73B8, 0x73BF, 
	0x7520, 0x752A, 0x7538, 0x753F, 0x7600, 0x760A, 0x7618, 0x761F, 0x7720, 0x772A, 0x7738, 0x773F, 0x77E0, 0x77EA, 0x77F8, 0x77FF, 
	0xA000, 0xA00A, 0xA018, 0xA01F, 0xA140, 0xA14A, 0xA158, 0xA15F, 0xA2A0, 0xA2AA, 0xA2B8, 0xA2BF, 0xA3A0, 0xA3AA, 0xA3B8, 0xA3BF, 
	0xA520, 0xA52A, 0xA538, 0xA53F, 0xA600, 0xA60A, 0xA618, 0xA61F, 0xA720, 0xA72A, 0xA738, 0xA73F, 0xA7E0, 0xA7EA, 0xA7F8, 0xA7FF, 
	0xC000, 0xC00A, 0xC018, 0xC01F, 0xC140, 0xC14A, 0xC158, 0xC15F, 0xC2A0, 0xC2AA, 0xC2B8, 0xC2BF, 0xC3A0, 0xC3AA, 0xC3B8, 0xC3BF, 
	0xC520, 0xC52A, 0xC538, 0xC53F, 0xC600, 0xC60A, 0xC618, 0xC61F, 0xC720, 0xC72A, 0xC738, 0xC73F, 0xC7E0, 0xC7EA, 0xC7F8, 0xC7FF, 
	0xE000, 0xE00A, 0xE018, 0xE01F, 0xE140, 0xE14A, 0xE158, 0xE15F, 0xE2A0, 0xE2AA, 0xE2B8, 0xE2BF, 0xE3A0, 0xE3AA, 0xE3B8, 0xE3BF, 
	0xE520, 0xE52A, 0xE538, 0xE53F, 0xE600, 0xE60A, 0xE618, 0xE61F, 0xE720, 0xE72A, 0xE738, 0xE73F, 0xE7E0, 0xE7EA, 0xE7F8, 0xE7FF, 
	0xF800, 0xF80A, 0xF818, 0xF81F, 0xF940, 0xF94A, 0xF958, 0xF95F, 0xFAA0, 0xFAAA, 0xFAB8, 0xFABF, 0xFBA0, 0xFBAA, 0xFBB8, 0xFBBF, 
	0xFD20, 0xFD2A, 0xFD38, 0xFD3F, 0xFE00, 0xFE0A, 0xFE18, 0xFE1F, 0xFF20, 0xFF2A, 0xFF38, 0xFF3F, 0xFFE0, 0xFFEA, 0xFFF8, 0xFFFF, 
};

// render buffer (VGA: 1 pixel = 1 u32 in PWM, HDMI: 1 pixel = 1 u16 in RGB565)
DISPHSTX_RENDERLINE_TYPE DispHstxRenderLine[DISPHSTX_RENDERLINE_SIZE*2];

volatile u32 DispHstxFrame;		// current frame

sDispHstxVModeState DispHstxVMode;	// default current videomode state descriptor
sDispHstxVModeState* pDispHstxVMode = &DispHstxVMode; // pointer to current videomode state descriptor

// check VSYNC (do not use ">=" - it would be too early)
Bool DispHstxIsVSync() { return (pDispHstxVMode->line > pDispHstxVMode->vtime.vactive); }

// wait for VSync scanline
void DispHstxWaitVSync()
{
	// wait for end of VSync
	while (DispHstxIsVSync()) { dmb(); }

	// wait for start of VSync
	while (!DispHstxIsVSync()) { dmb(); }
}

// Initialize videomode state descriptor (set timings and clear list of strips)
//  vmode ... videomode state descriptor (must not currently be used)
//  vtime ... videomode timings descriptor
void DispHstxVModeInitTime(sDispHstxVModeState* vmode, const sDispHstxVModeTime* vtime)
{
	memcpy(&vmode->vtime, vtime, sizeof(sDispHstxVModeTime));
	vmode->stripnum = 0;
}

// add empty videomode strip to videomode state descriptor
//  vmode ... videomode state descriptor (must not currently be used)
//  height ... height of the strip in number of video scanlines, can be 0, or -1=use all remaining scanlines
//		- final sum of heights of all strips must be equal or greater than vtime.vactive
// Returns error code DISPHSTX_ERR_* (DISPHSTX_ERR_OK = 0 = all OK)
int DispHstxVModeAddStrip(sDispHstxVModeState* vmode, int height)
{
	// check number of strips
	int stripnum = vmode->stripnum;
	if (stripnum < 0) return DISPHSTX_ERR_BROKEN;
	if (stripnum >= DISPHSTX_STRIP_MAX) return DISPHSTX_ERR_STRIPNUM; // exceeded number of strips

	// calculate remaining lines
	int maxheight = vmode->vtime.vactive; // vertical resolution in scanlines
	int i;
	for (i = 0; i < stripnum; i++) maxheight -= vmode->strip[i].height;
	if (maxheight < 0) return DISPHSTX_ERR_BROKEN;

	// prepare height of the strip
	if ((height < 0) || (height > maxheight)) height = maxheight;

	// create new strip
	sDispHstxVStrip* strip = &vmode->strip[stripnum];
	vmode->stripnum = stripnum + 1;
	strip->slotnum = 0;
	strip->height = height;

	// all OK
	return DISPHSTX_ERR_OK;
}

// add videomode slot to last added videomode strip
//  vmode ... videomode state descriptor (must not currently be used)
//  hdbl ... number of video pixels per graphics pixel 1..16 (fast) or 1..32 (normal) (1=full resolution, 2=double pixels, ...)
//		- if hdbl > 1 with HDMI, pixel formats 1-6 bits are not supported
//		- width must be divisible by hdbl
//  vdbl ... number of scanlines per graphics line 1.. (1=single line, 2=double lines, ...)
//		- height of the slot does not have to be a multiple of vdbl
//  w ... width of the slot in graphics pixels (not video pixels), without color separator, or -1=use all remaining width of the strip
//		- can be 0, to use only color separator (which can be combined with any color format)
//		- width in video pixels = w * hdbl
//		- pitch must be aligned to u32, width need not to be aligned
//		- sum of widths + separators of all slots must be equal vtime.hactive
//		- in direct mode (no palettes) width must be at least multiply of u8, or better multiply of u32 (uses faster DMA)
//		- in HSTX RLE compression mode the slot width must be the same as the image width
//  format ... index of pixel format DISPHSTX_FORMAT_*
//  buf ... pointer to frame buffer, or NULL to create new one (not used in single-color modes)
//		- the driver does not automatically delete the buffer it created, it must be manually deleted when the driver terminates
//  pitch ... length of line or row in bytes, or -1 = use default
//  pal ... pointer to palettes in RGB565 format (should be in RAM for faster access; can be NULL to use default palettes)
//  palvga ... pointer to palettes for VGA, double-size format with time dithering (or NULL to create new one with malloc(), or NULL if VGA is disabled)
//		- it must be large enough to accept palette entries in 2*u32 format (size in bytes = palnum*8)
//  font ... pointer to font (width 8 pixels), or to column or row of tiles, or offsets of lines of HSTX, or attributes (should be in RAM for faster access; can be NULL if not text/tile format)
//  fonth ... height of font, or color modulo, or pitch of tile row (or 0=column), or pitch of attributes (-1=auto)
//  gap_col ... color separator in RGB565 format, after this slot (not precise color, near DC balanced TMDS colors will be used)
//  gap_len ... number of video pixels of color separator between slots (recommended 20, 0 = do not use, or -1 = auto detection insert when needed)
//		- used to enable switching slot to another format (when needed to change registers)
//		- can be used as simple color on end of the strip, when w = 0
//		- if too small (less than 10 or 17), signal can drop-out
// Returns error code DISPHSTX_ERR_* (DISPHSTX_ERR_OK = 0 = all OK)
int DispHstxVModeAddSlot(sDispHstxVModeState* vmode, int hdbl, int vdbl, int w, int format, void* buf, int pitch,
		const u16* pal, u32* palvga, const u8* font, int fonth, u16 gap_col, int gap_len)
{
	// get last strip
	int stripnum = vmode->stripnum - 1;
	if (stripnum < 0) return DISPHSTX_ERR_NOSTRIP;
	if (stripnum >= DISPHSTX_STRIP_MAX) return DISPHSTX_ERR_BROKEN;
	sDispHstxVStrip* strip = &vmode->strip[stripnum];

	// check number of slots
	int slotnum = strip->slotnum;
	if (slotnum < 0) return DISPHSTX_ERR_BROKEN;
	if (slotnum >= DISPHSTX_SLOT_MAX) return DISPHSTX_ERR_SLOTNUM; // exceeded number of slots

	// create new slot
	sDispHstxVSlot* slot = &strip->slot[slotnum];
	strip->slotnum = slotnum + 1;

	// check zooming
	int hdbl_max = 32/vmode->vtime.clkdiv;
	if (hdbl < 1) hdbl = 1;
	if (hdbl > hdbl_max) hdbl = hdbl_max;
	if (vdbl > strip->height) vdbl = strip->height; // strip->height can be 0
	if (vdbl < 1) vdbl = 1;

	// check color modulo
#if DISPHSTX_USE_FORMAT_COL
	if ((format == DISPHSTX_FORMAT_COL) && (fonth < 1)) return DISPHSTX_ERR_COLMOD;
#endif

	// check pattern dimension
#if DISPHSTX_USE_FORMAT_PAT_8 || DISPHSTX_USE_FORMAT_PAT_8_PAL
	if (((pitch <= 0) || (fonth <= 0)) &&
#if DISPHSTX_USE_FORMAT_PAT_8 && DISPHSTX_USE_FORMAT_PAT_8_PAL
		((format == DISPHSTX_FORMAT_PAT_8) || (format == DISPHSTX_FORMAT_PAT_8_PAL))
#elif DISPHSTX_USE_FORMAT_PAT_8
		(format == DISPHSTX_FORMAT_PAT_8)
#else
		(format == DISPHSTX_FORMAT_PAT_8_PAL)
#endif
		) return DISPHSTX_ERR_PATDIM;
#endif

	// add pixel format
	slot->format = format;
	if (format == DISPHSTX_FORMAT_CUSTOM)
		memcpy(&slot->vcolor, DispHstxVColorCustom, sizeof(sDispHstxVColor));
	else
		memcpy(&slot->vcolor, &DispHstxVColorList[format], sizeof(sDispHstxVColor));

	// check HSTX mode
	if (slot->vcolor.hstxmode)
	{
		if (buf == NULL) return DISPHSTX_ERR_HSTXBUF;
		if (font == NULL) return DISPHSTX_ERR_HSTXFONT;
	}

	// number of palettes
	int palnum = slot->vcolor.palnum;
	if (slot->vcolor.colmode) palnum = fonth;

	// calculate remaining width
	int oldw = w;
	int oldgap_len = gap_len;

slot_regap:

	int maxwidth = vmode->vtime.hactive; // horizontal resolution in video pixels
	int i;
	for (i = 0; i < slotnum; i++)
	{
		maxwidth -= strip->slot[i].width;
		if (strip->slot[i].gap_len > 0) maxwidth -= strip->slot[i].gap_len;
	}

	if (maxwidth < 0)
	{
		if ((slotnum > 0) && (strip->slot[slotnum-1].gap_len > 0))
		{
			// limit previous color separator
			maxwidth += strip->slot[slotnum-1].gap_len;
			if (maxwidth >= 0)
			{
				strip->slot[i].gap_len = maxwidth;
				maxwidth = 0;
			}
			else
				return DISPHSTX_ERR_BROKEN;
		}
		else	
			return DISPHSTX_ERR_BROKEN;
	}

	// width of the slot
	int width = w * hdbl;
	if ((width < 0) && (gap_len > 0))
	{
		if (gap_len > maxwidth) gap_len = maxwidth;
		maxwidth -= gap_len;
	}
	if ((width < 0) || (width > maxwidth)) width = maxwidth;
	w = width / hdbl;
	int width2 = w * hdbl;
	if (width2 != width) // remaining space is not aligned
	{
		if (gap_len <= 0) gap_col = COL16_BLACK;
		gap_len = width - width2;
		width = width2;
	}
	slot->width = width;
	slot->hdbl = hdbl;
	slot->w = w;

	// limit gap_len, or fill-up remaining space
	maxwidth -= width;
	if ((gap_len > maxwidth) || (oldw < 0)) gap_len = maxwidth;

	// prepare width align flag (use 16-bit HDMI colors)
	int colbits = slot->vcolor.colbits; 		// number of color bits
	if (palnum != 0) colbits = 16;			// in palette mode, pixel is always u16
	int pixword = 32 / colbits;			// number of pixels per u32 word
	int pixhalf = 16 / colbits;			// number of pixels per u16 half-word
	int pixbyte = 1;
	if (colbits < 8) pixbyte = 8 / colbits;
	int walign = DISPHSTX_WALIGN_REP;		// pre-set double-pixel mode
	if ((hdbl == 1) && (colbits != 12))		// use single-pixel mode
	{
		walign = DISPHSTX_WALIGN_32;		// 32-bit width align

#if DISPHSTX_USE_FORMAT_3
		if (format == DISPHSTX_FORMAT_3) // 3 bits per pixel must be aligned to 10 pixels
		{
			if ((w % 10) != 0) return DISPHSTX_ERR_ALIGN; // align error
		}
		else
#endif

#if DISPHSTX_USE_FORMAT_6
		if (format == DISPHSTX_FORMAT_6) // 6 bits per pixel must be aligned to 5 pixels
		{
			if ((w % 5) != 0) return DISPHSTX_ERR_ALIGN; // align error
		}
		else
#endif

		if ((w & (pixword-1)) != 0)	// if not 32-bit width align
		{
			walign = DISPHSTX_WALIGN_16;	// 16-bit width align
			if ((w & (pixhalf-1)) != 0)	// if not 16-bit width align
			{
				walign = DISPHSTX_WALIGN_8; // 8-bit width align
				if ((w & (pixbyte-1)) != 0) return DISPHSTX_ERR_ALIGN; // align error, not aligned to byte
			}
		}
	}
	else
	{
		if (colbits < 8) return DISPHSTX_ERR_SUPPORT; // 1..6 bits not supported with double pixels
	}
	if (slot->vcolor.hstxmode || (colbits == 12)) walign = DISPHSTX_WALIGN_16; // send single pixels in HSTX and 12-bit mode
	slot->walign = walign; // width align mode

	// auto gap of previous slot
	if (slotnum > 0)
	{
		sDispHstxVSlot* s = &strip->slot[slotnum-1];
		if (s->gap_len < 0)
		{
			// slot with zero length does not require to setup registers
			if (width == 0)
			{
				s->gap_len = 0;
			}
			else
			{
				// find previous valid slot
				int i;
				sDispHstxVSlot* sprev = s;
				for (i = slotnum-1; i >= 0; i--)
				{
					sprev = &strip->slot[i];
					if ((sprev->width > 0) && (sprev->format != DISPHSTX_FORMAT_NONE)) break;
				}

				// need separator, if format group changed, or if hdbl changed, or if width align changed
				if ((slot->vcolor.grp != sprev->vcolor.grp) ||
					(slot->hdbl != sprev->hdbl) ||
					(walign != sprev->walign))
				{
					s->gap_len = 20;
					s->gap_col = COL16_BLACK; // use near-black color with DC balanced
					w = oldw;
					gap_len = oldgap_len;
					goto slot_regap;
				}
			}
		}
	}

	// height of the slot
	int lines = (strip->height + vdbl-1)/vdbl; // height of the slot in graphics lines
	int h = lines;
	if (slot->vcolor.textmode) // get number of rows in text mode
	{
		if (fonth < 1) return DISPHSTX_ERR_FONTH;
		if (font == NULL) return DISPHSTX_ERR_FONT;
		h = (h + fonth-1)/fonth; // number of rows in text mode
	}

#if DISPHSTX_USE_FORMAT_MTEXT
	if (format == DISPHSTX_FORMAT_MTEXT) palnum = h*2;
#endif

	if (slot->vcolor.tilemode) h = (h + slot->vcolor.tilew-1)/slot->vcolor.tilew; // number of rows in tile mode

	slot->vcolor.palnum = (u16)palnum;

	slot->vdbl = vdbl;
	slot->lines = lines;
	slot->h = h;

	// pitch (length of line or row in bytes)
	if (pitch < 0)
	{
		pitch = 0;

		// graphics mode
		if (slot->vcolor.colbits == 1)
			pitch = ((w + 7)/8 + 3) & ~3;
		else if (slot->vcolor.colbits == 2)
			pitch = ((w + 3)/4 + 3) & ~3;
		else if (slot->vcolor.colbits == 3)
			pitch = (w + 9)/10*4; // 3-bit mode: 10 pixels per one 31-bit word
		else if (slot->vcolor.colbits == 4)
			pitch = ((w + 1)/2 + 3) & ~3;
		else if (slot->vcolor.colbits == 6)
			pitch = (w + 4)/5*4; // 6-bit mode: 5 pixels per one 31-bit word
		else if (slot->vcolor.colbits == 8)
			pitch = (w + 3) & ~3;
		else if (slot->vcolor.colbits == 12)
			pitch = (w+7)/8*12; // 12-bit mode: 8 pixels per three 32-bit words
		else
			pitch = w*2;

#if DISPHSTX_USE_FORMAT_MTEXT
		if (format == DISPHSTX_FORMAT_MTEXT) pitch = (w + 7) / 8;
#endif

#if DISPHSTX_USE_FORMAT_ATEXT
		if (format == DISPHSTX_FORMAT_ATEXT) pitch = (w + 7) / 8 * 2;
#endif

		if (slot->vcolor.tilemode) pitch = (w + slot->vcolor.tilew - 1)/slot->vcolor.tilew;

		// attribute mode
		if (slot->vcolor.attrmode)
		{
#if DISPHSTX_USE_FORMAT_ATTR1_PAL || DISPHSTX_USE_FORMAT_ATTR5_PAL
			if (
#if DISPHSTX_USE_FORMAT_ATTR1_PAL && DISPHSTX_USE_FORMAT_ATTR5_PAL
				(format == DISPHSTX_FORMAT_ATTR1_PAL) || (format == DISPHSTX_FORMAT_ATTR5_PAL)
#elif DISPHSTX_USE_FORMAT_ATTR1_PAL
				(format == DISPHSTX_FORMAT_ATTR1_PAL)
#else
				(format == DISPHSTX_FORMAT_ATTR5_PAL)
#endif
			)
				pitch = (w+7)/8; // 1 bit per pixel, except modes 2 and 6
			else
#endif
				pitch = (w+3)/4; // 2 bits per pixel, or 2x 1 bit per pixel with modes 2 and 6

#if DISPHSTX_USE_FORMAT_ATTR2_PAL || DISPHSTX_USE_FORMAT_ATTR6_PAL
			if (
#if DISPHSTX_USE_FORMAT_ATTR2_PAL && DISPHSTX_USE_FORMAT_ATTR6_PAL
				(format == DISPHSTX_FORMAT_ATTR2_PAL) || (format == DISPHSTX_FORMAT_ATTR6_PAL)
#elif DISPHSTX_USE_FORMAT_ATTR2_PAL
				(format == DISPHSTX_FORMAT_ATTR2_PAL)
#else
				(format == DISPHSTX_FORMAT_ATTR6_PAL)
#endif
				)
				h = (h+1)/2; // 1 row is for 2 lines
#endif
		}

		pitch = (pitch + 3) & ~3; // align pitch to u32
	}
	if ((pitch & 3) != 0) return DISPHSTX_ERR_PITCHALIGN; // pitch must be aligned to 32-bit word
	slot->pitch = pitch;

	// create frame buffer (not in color mode)
	if (!slot->vcolor.colmode && !slot->vcolor.hstxmode && (pitch > 0) && (h > 0))
	{
		if (buf == NULL) buf = malloc(pitch * h);
		if (buf == NULL) return DISPHSTX_ERR_BUF;
	}
	slot->buf = (u8*)buf;	// frame buffer

	// pitch of attributes
#if DISPHSTX_USE_FORMAT_ATTR1_PAL || DISPHSTX_USE_FORMAT_ATTR2_PAL || \
	DISPHSTX_USE_FORMAT_ATTR3_PAL || DISPHSTX_USE_FORMAT_ATTR4_PAL || \
	DISPHSTX_USE_FORMAT_ATTR5_PAL || DISPHSTX_USE_FORMAT_ATTR6_PAL || \
	DISPHSTX_USE_FORMAT_ATTR7_PAL || DISPHSTX_USE_FORMAT_ATTR8_PAL
	if (slot->vcolor.attrmode && (fonth < 0))
	{
		// cell width
		switch (format)
		{
#if DISPHSTX_USE_FORMAT_ATTR1_PAL
		case DISPHSTX_FORMAT_ATTR1_PAL: fonth = (w+7)/8*2/2; break; // 4 bits, 8x8, 2 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR2_PAL
		case DISPHSTX_FORMAT_ATTR2_PAL:	fonth = (w+3)/4*2/2; break; // 4 bits, 4x4, 2 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR3_PAL
		case DISPHSTX_FORMAT_ATTR3_PAL:	fonth = (w+7)/8*4/2; break; // 4 bits, 8x8, 4 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR4_PAL
		case DISPHSTX_FORMAT_ATTR4_PAL:	fonth = (w+3)/4*4/2; break; // 4 bits, 4x4, 4 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR5_PAL
		case DISPHSTX_FORMAT_ATTR5_PAL:	fonth = (w+7)/8*2; break; // 8 bits, 8x8, 2 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR6_PAL
		case DISPHSTX_FORMAT_ATTR6_PAL:	fonth = (w+3)/4*2; break; // 8 bits, 4x4, 2 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR7_PAL
		case DISPHSTX_FORMAT_ATTR7_PAL:	fonth = (w+7)/8*4; break; // 8 bits, 8x8, 4 colors
#endif
#if DISPHSTX_USE_FORMAT_ATTR8_PAL
		case DISPHSTX_FORMAT_ATTR8_PAL:	fonth = (w+3)/4*4; break; // 8 bits, 4x4, 4 colors
#endif
		};
	}
#endif

	// create attribute buffer
	if (slot->vcolor.attrmode && (font == NULL))
	{
		int h2;
#if DISPHSTX_USE_FORMAT_ATTR1_PAL || DISPHSTX_USE_FORMAT_ATTR3_PAL || DISPHSTX_USE_FORMAT_ATTR5_PAL || DISPHSTX_USE_FORMAT_ATTR7_PAL
		if (
#if DISPHSTX_USE_FORMAT_ATTR1_PAL
			(format == DISPHSTX_FORMAT_ATTR1_PAL) || 
#endif
#if DISPHSTX_USE_FORMAT_ATTR3_PAL
			(format == DISPHSTX_FORMAT_ATTR3_PAL) ||
#endif
#if DISPHSTX_USE_FORMAT_ATTR5_PAL
			(format == DISPHSTX_FORMAT_ATTR5_PAL) ||
#endif
#if DISPHSTX_USE_FORMAT_ATTR7_PAL
			(format == DISPHSTX_FORMAT_ATTR7_PAL) ||
#endif
			False)
			h2 = (h + 7)/8;
		else
#endif
			h2 = (h + 4)/4;
		font = (u8*)malloc(fonth * h2);
		if (font == NULL) return DISPHSTX_ERR_ATTRBUF;
	}

	// default palettes
	Bool use_defpalvga = False;
	if ((pal == NULL) && (palnum > 0))
	{
		pal = slot->vcolor.defpal;
		if (palvga == NULL)
		{
			palvga = slot->vcolor.defpalvga;
			use_defpalvga = True;
		}
	}

	// build-in VGA palettes
	if (palnum == 0) palvga = slot->vcolor.defpalvga;

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	// palettes
	if ((palvga == NULL) && (palnum > 0))
	{
		palvga = (u32*)malloc(palnum*sizeof(u32)*2);
		if (palvga == NULL) return DISPHSTX_ERR_PALVGA;
	}
#endif
	slot->pal = pal;
	slot->palvga = palvga;

	// convert VGA palettes
#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if ((palnum > 0) && !use_defpalvga) DispHstxVgaPal2(palvga, pal, palnum);
#endif

	// font or tile or attributes
	slot->font = font;
	slot->fonth = fonth;

	// color separator
	slot->gap_col = gap_col;
	slot->gap_len = gap_len;

	// remove text cursor (it is OFF by default)
	slot->curpos = -1;
	slot->currow = -1;
	slot->curbeg = fonth - 2;
	slot->curend = fonth - 1;
	slot->curspeed = 12;

	// all OK
	return DISPHSTX_ERR_OK;
}

// default VGA palette buffer (used by the DispHstxVModeInitSimple() function)
u32 DispHstxDefVgaPal[2*256];

// initialize videomode state desciptor to simple videomode (uses 1 strip and 1 slot), links video slot to DrawCan
//  vmode ... videomode state descriptor (must not currently be used)
//  vtime ... videomode timings descriptor
//  hdbl ... number of video pixels per graphics pixel 1..16 (fast) or 1..32 (normal) (1=full resolution, 2=double pixels, ...)
//		- if hdbl > 1, pixel formats 1-6 bits are not supported
//		- width must be divisible by hdbl
//  vdbl ... number of scanlines per graphics line 1.. (1=single line, 2=double lines, ...)
//		- height of the slot does not have to be a multiple of vdbl
//  format ... index of pixel format DISPHSTX_FORMAT_*
//  buf ... pointer to frame buffer (aligned to 32-bit), or NULL to create new one (not used in single-color modes)
//		- in HSTX RLE compression mode the videomode width must be the same as the image width
//		- the driver does not automatically delete the buffer it created, it must be manually deleted when the driver terminates
//  pal ... pointer to palettes in RGB565 format (should be in RAM for faster access; can be NULL if palettes will not be used)
//  font ... pointer to font (width 8 pixels), or to column or row of tiles, or offsets of lines of HSTX, or attributes (should be in RAM for faster access; can be NULL if not text/tile format)
//  fonth ... height of font, or color modulo, or pitch of tile row (or 0=column), or pitch of attributes (-1=auto)
// Returns error code DISPHSTX_ERR_* (0=all OK)
int DispHstxVModeInitSimple(sDispHstxVModeState* vmode, const sDispHstxVModeTime* vtime, int hdbl, int vdbl,
	int format, void* buf, const u16* pal, const u8* font, int fonth)
{
	// Initialize videomode state descriptor (set timings and clear list of strips)
	DispHstxVModeInitTime(vmode, vtime);

	// add empty videomode strip to videomode state descriptor
	int res = DispHstxVModeAddStrip(vmode, -1);
	if (res != DISPHSTX_ERR_OK) return res;

	// add videomode slot to last added videomode strip
	res = DispHstxVModeAddSlot(vmode, hdbl, vdbl, -1, format, buf, -1, pal, DispHstxDefVgaPal, font, fonth, 0, 0);
	if (res != DISPHSTX_ERR_OK) return res;

#if USE_DRAWCAN 	// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)
	sDispHstxVSlot* slot = &vmode->strip[0].slot[0];
	if (slot->vcolor.graphmode)
	{
#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
		DispHstxLinkCan(slot, &DrawCan);
		pDrawCan = &DrawCan;
#endif

		switch (slot->format)
		{
#if USE_DRAWCAN1 && (DISPHSTX_USE_FORMAT_1 || DISPHSTX_USE_FORMAT_1_PAL) // 1=use DrawCan1 1-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_1
		case DISPHSTX_FORMAT_1:
#endif
#if DISPHSTX_USE_FORMAT_1_PAL
		case DISPHSTX_FORMAT_1_PAL:
#endif
			DispHstxLinkCan(slot, &DrawCan1);
			pDrawCan1 = &DrawCan1;
			break;
#endif

#if USE_DRAWCAN2 && (DISPHSTX_USE_FORMAT_2 || DISPHSTX_USE_FORMAT_2_PAL) // 1=use DrawCan2 2-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_2
		case DISPHSTX_FORMAT_2:
#endif
#if DISPHSTX_USE_FORMAT_2_PAL
		case DISPHSTX_FORMAT_2_PAL:
#endif
			DispHstxLinkCan(slot, &DrawCan2);
			pDrawCan2 = &DrawCan2;
			break;
#endif

#if USE_DRAWCAN3 && (DISPHSTX_USE_FORMAT_3 || DISPHSTX_USE_FORMAT_3_PAL) // 1=use DrawCan3 3-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_3
		case DISPHSTX_FORMAT_3:
#endif
#if DISPHSTX_USE_FORMAT_3_PAL
		case DISPHSTX_FORMAT_3_PAL:
#endif
			DispHstxLinkCan(slot, &DrawCan3);
			pDrawCan3 = &DrawCan3;
			break;
#endif

#if USE_DRAWCAN4 && (DISPHSTX_USE_FORMAT_4 || DISPHSTX_USE_FORMAT_4_PAL) // 1=use DrawCan4 4-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_4
		case DISPHSTX_FORMAT_4:
#endif
#if DISPHSTX_USE_FORMAT_4_PAL
		case DISPHSTX_FORMAT_4_PAL:
#endif
			DispHstxLinkCan(slot, &DrawCan4);
			pDrawCan4 = &DrawCan4;
			break;
#endif

#if USE_DRAWCAN6 && (DISPHSTX_USE_FORMAT_6 || DISPHSTX_USE_FORMAT_6_PAL) // 1=use DrawCan6 6-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_6
		case DISPHSTX_FORMAT_6:
#endif
#if DISPHSTX_USE_FORMAT_6_PAL
		case DISPHSTX_FORMAT_6_PAL:
#endif
			DispHstxLinkCan(slot, &DrawCan6);
			pDrawCan6 = &DrawCan6;
			break;
#endif

#if USE_DRAWCAN8 && (DISPHSTX_USE_FORMAT_8 || DISPHSTX_USE_FORMAT_8_PAL) // 1=use DrawCan8 8-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_8
		case DISPHSTX_FORMAT_8:
#endif
#if DISPHSTX_USE_FORMAT_8_PAL
		case DISPHSTX_FORMAT_8_PAL:
#endif
			DispHstxLinkCan(slot, &DrawCan8);
			pDrawCan8 = &DrawCan8;
			break;
#endif

#if USE_DRAWCAN12 && DISPHSTX_USE_FORMAT_12 // 1=use DrawCan12 12-bit functions, if use drawing canvas
		case DISPHSTX_FORMAT_12:
			DispHstxLinkCan(slot, &DrawCan12);
			pDrawCan12 = &DrawCan12;
			break;
#endif

#if USE_DRAWCAN16 && DISPHSTX_USE_FORMAT_15 // 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
		case DISPHSTX_FORMAT_15:
			DispHstxLinkCan(slot, &DrawCan15);
			pDrawCan15 = &DrawCan15;
			break;
#endif

#if USE_DRAWCAN16 && DISPHSTX_USE_FORMAT_16 // 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
		case DISPHSTX_FORMAT_16:
			DispHstxLinkCan(slot, &DrawCan16);
			pDrawCan16 = &DrawCan16;
			break;
#endif

		default:
			break;
		}
	}
#endif // USE_DRAWCAN
	return res;
}

// Start simple display graphics videomode
//  dispmode ... display mode DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA. Use DISPHSTX_DISPMODE_NONE or 0 to auto-detect display mode selection.
//  buf ... pointer to frame buffer (aligned to 32-bit), or NULL to create new one
//		- the driver does not automatically delete the buffer it created, it must be manually deleted when the driver terminates
//  vmodeinx ... videomode timings index vmodetime_*
//  hdbl ... number of video pixels per graphics pixel 1..16 (fast) or 1..32 (normal) (1=full resolution, 2=double pixels, ...)
//		- if hdbl > 1, pixel formats 1-6 bits are not supported
//		- width must be divisible by hdbl
//  vdbl ... number of scanlines per graphics line 1.. (1=single line, 2=double lines, ...)
//		- height of the slot does not have to be a multiple of vdbl
//  format ... index of pixel format DISPHSTX_FORMAT_*
// Returns error code DISPHSTX_ERR_* (0=all OK)
int DispHstxVModeStartSimple(int dispmode, void* buf, int vmodeinx, int hdbl, int vdbl, int format)
{
	// forbidden format
	if (format == DISPHSTX_FORMAT_NONE) return DISPHSTX_ERR_FORMAT;

	// auto-detect display mode
	if (dispmode == DISPHSTX_DISPMODE_NONE)
	{
		dispmode = DispHstxAutoDispSel();
	}

	// terminate all current devices
	DispHstxAllTerm();

	// initialize videomode descriptor
	int res = DispHstxVModeInitSimple(&DispHstxVMode, &DispHstxVModeTimeList[vmodeinx],
		hdbl, vdbl, format, buf, NULL, NULL, 0);
	if (res != DISPHSTX_ERR_OK) return res;

	// start videomode
	DispHstxSelDispMode(dispmode, &DispHstxVMode);
	return res;
}

// pre-prepare videomode state descriptor before start video mode
// - called internaly from DispHstxDviPrepare and DispHstxVgaPrepare
// - adds missing slots
void DispHstxPrePrepare(sDispHstxVModeState* v)
{
	// loop all strips
	int stripinx, slotinx, w, w0;
	sDispHstxVStrip* strip;
	sDispHstxVSlot* slot;
	for (stripinx = 0; stripinx < v->stripnum; stripinx++)
	{
		strip = &v->strip[stripinx];

		// calculate total width
		w = 0;
		for (slotinx = 0; slotinx < strip->slotnum; slotinx++)
		{
			slot = &strip->slot[slotinx];
			w += slot->width;
			if (slot->gap_len > 0) w += slot->gap_len;
		}

		// strip width is not full
		w0 = v->vtime.hactive;
		if (w < w0)
		{
			// check last slot, auto update gap
			slotinx = strip->slotnum;
			if (slotinx > 0)
			{
				slot = &strip->slot[slotinx-1];
				if ((slot->gap_len < 0) || (slotinx >= DISPHSTX_SLOT_MAX))
				{
					// auto gap
					if (slot->gap_len > 0)
						slot->gap_len += w0 - w;
					else
						slot->gap_len = w0 - w;
					continue; // continue to next strip
				}
			}

			// add new slot
			slot = &strip->slot[slotinx];
			strip->slotnum = slotinx + 1;

			// add pixel format
			int format = DISPHSTX_FORMAT_NONE;
			slot->format = format;
			memcpy(&slot->vcolor, &DispHstxVColorList[format], sizeof(sDispHstxVColor));

			// set gap
			slot->gap_len = w0 - w;
			slot->gap_col = COL16_BLACK;

			// setup size
			slot->width = 0;
			slot->hdbl = 1;
			slot->w = 0;
			slot->walign = DISPHSTX_WALIGN_8;
			slot->vdbl = 1;
			slot->lines = strip->height;
			slot->pitch = 0;
			slot->buf = 0;
			slot->pal = NULL;
			slot->palvga = NULL;
		}
	}
}

#if USE_DRAWCAN		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)
// link slot to drawing canvas (fill-up canvas entries)
// Supported graphics formats and their mapping to canvas format:
//	DISPHSTX_FORMAT_1	-> DRAWCAN_FORMAT_1
//	DISPHSTX_FORMAT_1_PAL	-> DRAWCAN_FORMAT_1
//	DISPHSTX_FORMAT_2	-> DRAWCAN_FORMAT_2
//	DISPHSTX_FORMAT_2_PAL	-> DRAWCAN_FORMAT_2
//	DISPHSTX_FORMAT_3	-> DRAWCAN_FORMAT_3
//	DISPHSTX_FORMAT_3_PAL	-> DRAWCAN_FORMAT_3
//	DISPHSTX_FORMAT_4	-> DRAWCAN_FORMAT_4
//	DISPHSTX_FORMAT_4_PAL	-> DRAWCAN_FORMAT_4
//	DISPHSTX_FORMAT_6	-> DRAWCAN_FORMAT_6
//	DISPHSTX_FORMAT_6_PAL	-> DRAWCAN_FORMAT_6
//	DISPHSTX_FORMAT_8	-> DRAWCAN_FORMAT_8
//	DISPHSTX_FORMAT_8_PAL	-> DRAWCAN_FORMAT_8
//	DISPHSTX_FORMAT_12	-> DRAWCAN_FORMAT_12
//	DISPHSTX_FORMAT_15	-> DRAWCAN_FORMAT_15
//	DISPHSTX_FORMAT_16	-> DRAWCAN_FORMAT_16
void DispHstxLinkCan(const sDispHstxVSlot* slot, sDrawCan* can)
{
	switch (slot->format)
	{
#if USE_DRAWCAN1 && (DISPHSTX_USE_FORMAT_1 || DISPHSTX_USE_FORMAT_1_PAL) // 1=use DrawCan1 1-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_1
	case DISPHSTX_FORMAT_1:
#endif
#if DISPHSTX_USE_FORMAT_1_PAL
	case DISPHSTX_FORMAT_1_PAL:
#endif
		can->format = DRAWCAN_FORMAT_1;
		can->colbit = 1;
		break;
#endif

#if USE_DRAWCAN2 && (DISPHSTX_USE_FORMAT_2 || DISPHSTX_USE_FORMAT_2_PAL) // 1=use DrawCan2 2-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_2
	case DISPHSTX_FORMAT_2:
#endif
#if DISPHSTX_USE_FORMAT_2_PAL
	case DISPHSTX_FORMAT_2_PAL:
#endif
		can->format = DRAWCAN_FORMAT_2;
		can->colbit = 2;
		break;
#endif

#if USE_DRAWCAN3 && (DISPHSTX_USE_FORMAT_3 || DISPHSTX_USE_FORMAT_3_PAL) // 1=use DrawCan3 3-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_3
	case DISPHSTX_FORMAT_3:
#endif
#if DISPHSTX_USE_FORMAT_3_PAL
	case DISPHSTX_FORMAT_3_PAL:
#endif
		can->format = DRAWCAN_FORMAT_3;
		can->colbit = 3;
		break;
#endif

#if USE_DRAWCAN4 && (DISPHSTX_USE_FORMAT_4 || DISPHSTX_USE_FORMAT_4_PAL) // 1=use DrawCan4 4-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_4
	case DISPHSTX_FORMAT_4:
#endif
#if DISPHSTX_USE_FORMAT_4_PAL
	case DISPHSTX_FORMAT_4_PAL:
#endif
		can->format = DRAWCAN_FORMAT_4;
		can->colbit = 4;
		break;
#endif

#if USE_DRAWCAN6 && (DISPHSTX_USE_FORMAT_6 || DISPHSTX_USE_FORMAT_6_PAL) // 1=use DrawCan6 6-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_6
	case DISPHSTX_FORMAT_6:
#endif
#if DISPHSTX_USE_FORMAT_6_PAL
	case DISPHSTX_FORMAT_6_PAL:
#endif
		can->format = DRAWCAN_FORMAT_6;
		can->colbit = 6;
		break;
#endif

#if USE_DRAWCAN8 && (DISPHSTX_USE_FORMAT_8 || DISPHSTX_USE_FORMAT_8_PAL) // 1=use DrawCan8 8-bit functions, if use drawing canvas
#if DISPHSTX_USE_FORMAT_8
	case DISPHSTX_FORMAT_8:
#endif
#if DISPHSTX_USE_FORMAT_8_PAL
	case DISPHSTX_FORMAT_8_PAL:
#endif
		can->format = DRAWCAN_FORMAT_8;
		can->colbit = 8;
		break;
#endif

#if USE_DRAWCAN12 && DISPHSTX_USE_FORMAT_12 // 1=use DrawCan12 12-bit functions, if use drawing canvas
	case DISPHSTX_FORMAT_12:
		can->format = DRAWCAN_FORMAT_12;
		can->colbit = 12;
		break;
#endif

#if USE_DRAWCAN16 && DISPHSTX_USE_FORMAT_15 // 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
	case DISPHSTX_FORMAT_15:
		can->format = DRAWCAN_FORMAT_15;
		can->colbit = 15;
		break;
#endif

#if USE_DRAWCAN16 && DISPHSTX_USE_FORMAT_16 // 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
	case DISPHSTX_FORMAT_16:
		can->format = DRAWCAN_FORMAT_16;
		can->colbit = 16;
		break;
#endif

	default:
		return;
	}

	can->strip = 0;
	can->stripnum = 1;

	can->fontw = FONTW;
	can->fonth = FONTH;
	can->printinv = 0;
	can->printsize = 0;

	int w = slot->w;
	int h = slot->h;

	can->w = w;
	can->h = h;
	can->wb = slot->pitch;
	can->striph = h;

	can->printposnum = w/FONTW;
	can->printrownum = h/FONTH;
	can->printpos = 0;
	can->printrow = 0;
#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
	can->printcol = DrawCanFncList[can->format]->col_white;
#else
	can->printcol = COL16_WHITE;
#endif

	can->basey = 0;
	can->clipx1 = 0;
	can->clipx2 = w;
	can->clipy1 = 0;
	can->clipy2 = h;
	can->clipy1_orig = 0;
	can->clipy2_orig = h;

	can->dirtyx1 = 0;
	can->dirtyx2 = w;
	can->dirtyy1 = 0;
	can->dirtyy2 = h;
	
	can->buf = (u8*)slot->buf;
	can->frontbuf = NULL;
	can->autoupdatelast = Time();
	can->font = FONT;
#if USE_DRAWCAN0		// 1=use DrawCan common functions, if use drawing canvas
	can->drawfnc = DrawCanFncList[can->format];
#else
	can->drawfnc = NULL;
#endif
}
#endif

// terminate all current devices
void DispHstxAllTerm()
{
	// terminate all current devices
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	DispHstxDviStop();
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	DispHstxVgaStop();
#endif

#if !DISPHSTX_USEPLL	// 1 = use separate PLL_SYS for HSTX generator (sys_clk will use PLL_USB)
	// restore default system clock
	ClockPllSysFreqVolt(PLL_KHZ);
#endif

#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	// terminate display selection switch
	DispHstxDispSelTerm();
#endif
}

// select and start display mode and videomode
//  dispmode ... display mode DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA, set to DISPHSTX_DISPMODE_NONE or 0 to auto-detect display mode selection
//  vmode ... initialized videomode state descriptor
void DispHstxSelDispMode(int dispmode, sDispHstxVModeState* vmode)
{
	// terminate all current devices
	DispHstxAllTerm();

	// auto-detect display mode
	if (dispmode == DISPHSTX_DISPMODE_NONE)
	{
		dispmode = DispHstxAutoDispSel();
	}

	// set new display mode
	DispHstxDispMode = dispmode;

	// set pointer to new current state descriptor
	pDispHstxVMode = vmode;
	dmb();

	// prepare clock divider
#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	int clkdiv = (dispmode == DISPHSTX_DISPMODE_VGA) ? 1 : vmode->vtime.clkdiv;
#else
	int clkdiv = vmode->vtime.clkdiv;
#endif

#if DISPHSTX_USEPLL	// 1 = use separate PLL_SYS for HSTX generator (sys_clk will use PLL_USB)
	// setup HSTX PLL
	PllSetFreq(PLL_SYS, vmode->vtime.sysclk/clkdiv);

	// setup CLK_HSTX to PLL_SYS
#if DISPHSTX_PICOSDK
	clock_configure(clk_hstx, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, vmode->vtime.sysclk*1000/clkdiv, vmode->vtime.sysclk*1000/clkdiv);
#else
	ClockSetup(CLK_HSTX, CLK_PLL_SYS, 0, 0);
#endif

#else // DISPHSTX_USEPLL

	// update system clock
	ClockPllSysFreqVolt(vmode->vtime.sysclk); // set system clock

	// setup CLK_HSTX
#if DISPHSTX_PICOSDK
	clock_configure(clk_hstx, 0, CLOCKS_CLK_PERI_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS, vmode->vtime.sysclk*1000, vmode->vtime.sysclk*1000/clkdiv);
#else
	ClockSetup(CLK_HSTX, CLK_PLL_SYS, vmode->vtime.sysclk*1000/clkdiv, vmode->vtime.sysclk*1000);
#endif

#endif // DISPHSTX_USEPLL

	// initialize new devices (require initialized pDispHstxVMode descriptor)
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	if (dispmode == DISPHSTX_DISPMODE_DVI) DispHstxDviStart();
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if (dispmode == DISPHSTX_DISPMODE_VGA) DispHstxVgaStart();
#endif

#if USE_PWMSND		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
	// update all sound speeds after changing system clock
	SpeedSoundUpdate();
#endif
}

// exchange video mode, without terminating current mode
// - HSTX DVI/VGA mode must be started using DispHstxDviStart()/DispHstxVgaStart()
// - VGA palettes must be re-generated (use DispHstxVgaPal2())
void DispHstxExchange(sDispHstxVModeState* vmode)
{
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_DVI)
	{
		DispHstxDviPrepare(vmode);
//		DispHstxWaitVSync();
		DispHstxDviExchange(vmode);
	}
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_VGA)
	{
		DispHstxVgaPrepare(vmode);
//		DispHstxWaitVSync();
		DispHstxVgaExchange(vmode);
	}
#endif
}

// re-select display mode with old videomode
void DispHstxReSelDispMode(int dispmode)
{
	DispHstxSelDispMode(dispmode, pDispHstxVMode);
}

// auto re-select current display mode by selection switch
// - should be called periodically from the main program loop
void DispHstxAutoDispMode()
{
#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	int sel = DispHstxCheckDispSel();
	if ((sel != DISPHSTX_DISPMODE_NONE) && (sel != DispHstxDispMode)) DispHstxReSelDispMode(sel);
#endif
}

// init display selection switch
void DispHstxDispSelInit()
{
#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	// initialize display selection switch
	GPIO_Init(DISPHSTX_DISP_SEL);
	GPIO_InDisable(DISPHSTX_DISP_SEL); // disable input (reason: RP2350-A2 errata RP2350-E9)
	DispHstxDispSelInitOK = True;
#endif
}

// terminate display selection switch
void DispHstxDispSelTerm()
{
#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch
	GPIO_Reset(DISPHSTX_DISP_SEL);
	DispHstxDispSelInitOK = False;
#endif
}

// get current display selection switch (display selection switch is auto-initialized)
int DispHstxCheckDispSel()
{
	int res;

#if DISPHSTX_USE_DVI && DISPHSTX_USE_VGA

	res = DISPHSTX_DISPMODE_NONE; // do auto-detection

#if DISPHSTX_DISP_SEL >= 0	// >=0 GPIO pin with display selection switch, -1=do not use display selection switch

	u8 in;

	// init display selection switch
	if (!DispHstxDispSelInitOK) DispHstxDispSelInit();

	// select pull-up, check VGA position
	GPIO_PullUp(DISPHSTX_DISP_SEL);		// enable pull-up resistor
	WaitUs(50);				// short delay to stabilize signal
	GPIO_InEnable(DISPHSTX_DISP_SEL);	// input enable
	WaitUs(2);				// short delay to stabilize signal
	cb();
	in = GPIO_In(DISPHSTX_DISP_SEL);	// read input state
	cb();
	GPIO_InDisable(DISPHSTX_DISP_SEL);	// input disable (reason: RP2350-A2 errata RP2350-E9)
	GPIO_NoPull(DISPHSTX_DISP_SEL);		// disable pull-up resistor
	if (in == 0)
		res = DISPHSTX_DISPMODE_VGA; // VGA selected
	else
	{
		// select pull-down, check DVI position
		GPIO_PullDown(DISPHSTX_DISP_SEL);	// enable pull-down resistor
		WaitUs(50);				// short delay to stabilize signal
		GPIO_InEnable(DISPHSTX_DISP_SEL);	// input enable
		WaitUs(2);				// short delay to stabilize signal
		cb();
		in = GPIO_In(DISPHSTX_DISP_SEL);	// read input state
		cb();
		GPIO_InDisable(DISPHSTX_DISP_SEL);	// input disable (reason: RP2350-A2 errata RP2350-E9)
		GPIO_NoPull(DISPHSTX_DISP_SEL);		// disable pull-up resistor
		if (in == 1) res = DISPHSTX_DISPMODE_DVI; // DVI selected
	}

#endif // DISPHSTX_DISP_SEL >= 0

#elif DISPHSTX_USE_DVI // only DVI
	res = DISPHSTX_DISPMODE_DVI;
#else // only VGA
	res = DISPHSTX_DISPMODE_VGA;
#endif

	return res;
}

// auto-detect display selection on program start (returns DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA)
// - Tests by pull-ups whether a VGA monitor is connected (that is, whether it pulls down
//   the input signal to 0). A connected HDMI monitor is not pulling down the signal to 0.
// - Can only be used at program start or before initializing the video mode because it resets the GPIO pins.
// - If the display selection switch is enabled, it is used in preference.
// - The RGB pins of the VGA display are supposed to be connected to GPIO 12 to 17.
int DispHstxAutoDispSel()
{
	int res;

#if DISPHSTX_USE_DVI && DISPHSTX_USE_VGA

	// selected by the switch
	res = DispHstxCheckDispSel();

	// auto-detection
	if (res == DISPHSTX_DISPMODE_NONE)
	{
#define DISPHSTX_VGA_RGBMASK ( BIT(DISPHSTX_VGA_B0) | BIT(DISPHSTX_VGA_B1) | BIT(DISPHSTX_VGA_G0) \
		| BIT(DISPHSTX_VGA_G1) | BIT(DISPHSTX_VGA_R0) | BIT(DISPHSTX_VGA_R1) )

		// setup GPIO pins to input with pull-ups
		u32 mask = DISPHSTX_VGA_RGBMASK;
		GPIO_InitMask(mask);	// initialize pins (enable inputs)

		GPIO_PullUp(DISPHSTX_VGA_B0);
		GPIO_PullUp(DISPHSTX_VGA_B1);
		GPIO_PullUp(DISPHSTX_VGA_G0);
		GPIO_PullUp(DISPHSTX_VGA_G1);
		GPIO_PullUp(DISPHSTX_VGA_R0);
		GPIO_PullUp(DISPHSTX_VGA_R1);

		// Delay to stabilise capacities
		WaitUs(500);

		// check input - VGA returns 0, HDMI returns 0x3f
		mask >>= 12;
		res = (((GPIO_InAll() >> 12) & mask) == mask) ? DISPHSTX_DISPMODE_DVI : DISPHSTX_DISPMODE_VGA;

		// reset GPIP pins
		GPIO_ResetMask(DISPHSTX_VGA_RGBMASK);
	}

#elif DISPHSTX_USE_DVI // only DVI
	res = DISPHSTX_DISPMODE_DVI;
#else // only VGA
	res = DISPHSTX_DISPMODE_VGA;
#endif
	return res;
}

#if DISPHSTX_USEPLL	// 1 = use separate PLL_SYS for HSTX generator (sys_clk will use PLL_USB)
/*
Default clock usage:
- XOSC ... 12 MHz
	-> REF (Watchdog) .. 12 MHz
	-> PLLUSB ... 48 MHz
		-> USB ... 48 MHz
		-> ADC ... 48 MHz
		-> PERI (UART, SPI) ... 48 MHz
	-> PLLSYS ... 150 MHz
		-> SYS (CPU, PIO) ... 150 MHz
		-> HSTX ... 150 MHz

Reconfiguration:
- XOSC ... 12 MHz
	-> REF (Watchdog) .. 12 MHz
	-> PLLUSB ... (48, 96, 144, 192, 240,) 288, (336) MHz
		-> USB ... 48 MHz
		-> ADC ... 48 MHz
		-> PERI (UART, SPI) ... 144 MHz
		-> SYS (CPU, PIO) ... 288 MHz
	-> PLLSYS ... variable
		-> HSTX ... variable
*/
// Reconfigure system clock (khz = required system frequency in kHz, should be multiply of 48000)
// - To change HSTX speed, use PllSetFreq(PLL_SYS, khz) function.
void DispHstxClockReinit(int khz)
{
	// raise system voltage
	FlashInit(GetClkDivBySysClock(khz));
	VregSetVoltage(GetVoltageBySysClock(khz));

	// set PLL_USB frequency - should be multiply of USB clock
	PllSetFreq(PLL_USB, khz);

	// setup CLK_SYS to PLL_USB
	ClockSetup(CLK_SYS, CLK_PLL_USB, 0, 0);

	// setup CLK_USB to PLL_USB
	ClockSetup(CLK_USB, CLK_PLL_USB, 48*MHZ, 0);

	// setup CLK_ADC to PLL_USB
	ClockSetup(CLK_ADC, CLK_PLL_USB, 48*MHZ, 0);

	// setup CLK_PERI to PLL_USB/2
	ClockSetup(CLK_PERI, CLK_PLL_USB, CurrentFreq[CLK_PLL_USB]/2, 0);

	// setup CLK_HSTX to PLL_SYS
	ClockSetup(CLK_HSTX, CLK_PLL_SYS, 0, 0);
}
#endif // DISPHSTX_USEPLL

// get buffer of first display slot
u8* DispHstxBuf()
{
	sDispHstxVModeState* v = pDispHstxVMode;
	if (v->stripnum == 0) return NULL;
	sDispHstxVStrip* strip = &v->strip[0];
	if (strip->slotnum == 0) return NULL;
	sDispHstxVSlot* slot = &strip->slot[0];
	return slot->buf;
}

// free buffer of first display slot
void DispHstxFreeBuf()
{
	sDispHstxVModeState* v = pDispHstxVMode;
	if (v->stripnum == 0) return;
	sDispHstxVStrip* strip = &v->strip[0];
	if (strip->slotnum == 0) return;
	sDispHstxVSlot* slot = &strip->slot[0];
	free(slot->buf);
	slot->buf = NULL;
}

// execute core 1 remote function
void DispHstxCore1Exec(void (*fnc)())
{
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_DVI)
	{
		DispHstxDviCore1Exec(fnc);
		return;
	}
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_VGA)
		DispHstxVgaCore1Exec(fnc);
#endif
}

// check if core 1 is busy (executing remote function)
Bool DispHstxCore1Busy()
{
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_DVI)
		return DispHstxDviCore1Busy();

#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_VGA)
		return DispHstxVgaCore1Busy();
#endif

	return False;
}

// wait if core 1 is busy (executing remote function)
void DispHstxCore1Wait()
{
#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_DVI)
	{
		DispHstxDviCore1Wait();
		return;
	}
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
	if (DispHstxDispMode == DISPHSTX_DISPMODE_VGA)
		DispHstxVgaCore1Wait();
#endif
}

#endif // USE_DISPHSTX		// 1=use HSTX Display driver
