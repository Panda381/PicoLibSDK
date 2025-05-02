
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

#if USE_DISPHSTX		// 1=use HSTX Display driver

#if USE_DRAWCAN		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)
#include "../../_lib/inc/lib_drawcan.h"
#endif

#ifndef _DISPHSTX_VMODE_H
#define _DISPHSTX_VMODE_H

#ifdef __cplusplus
extern "C" {
#endif

// selected display mode
#define DISPHSTX_DISPMODE_NONE	0	// display mode - none (use auto-detection)

#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)
#define DISPHSTX_DISPMODE_DVI	1	// display mode - DVI (HDMI)
#endif

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
#define DISPHSTX_DISPMODE_VGA	2	// display mode - VGA
#endif

#if DISPHSTX_CHECK_LOAD	// 1 = check CPU load during IRQ interrupt (get DispHstxTimeIn, DispHstxTimeOut)
extern u32 DispHstxTimeOld;
extern volatile u64 DispHstxTimeIn, DispHstxTimeOut;
extern volatile u32 DispHstxTimeInMax, DispHstxTimeOutMin, DispHstxTimeNum;
#endif

// current selected display mode DISPHSTX_DISPMODE_*
extern int DispHstxDispMode;

// width align type
enum {
	DISPHSTX_WALIGN_32,		// 32-bit width align, single-pixel
	DISPHSTX_WALIGN_16,		// 16-bit width align, single-pixel
	DISPHSTX_WALIGN_8,		// 8-bit width align, single-pixel
	DISPHSTX_WALIGN_REP,		// double-pixel
};

// videomode slot descriptor
// - Structure definition must match definition in files disphstx_*.S
typedef struct sDispHstxVSlot_ {

	// graphics format
	u8		format;		// 0x00: pixel format DISPHSTX_FORMAT_*
	u8		walign;		// 0x01: width align type DISPHSTX_WALIGN_* (use 16-bit HDMI colors)

	// text cursor
	u8		curpos;		// 0x02: text cursor position (default -1 = disabled)
	u8		currow;		// 0x03: text cursor row (default -1 = disabled)
	u8		curbeg;		// 0x04: text cursor begin line (default fonth - 2)
	u8		curend;		// 0x05: text cursor end line (default fonth - 1)
	u8		curspeed;	// 0x06: text cursor speed - number of shifts of [us] timer (default 12)
	u8		res, res2, res3; // 0x07: ... reserved (align)

	s16		fonth;		// 0x0A: height of font, or color modulo, or pitch of tile row (or 0=column), or pitch of attributes
// aligned
	u16		data_inx;	// 0x0C: index of data address in command buffer strip->vactive_cmd

	// width of the slot
	u16		width;		// 0x0E: real width of the slot in video pixels, without color separator
					//	- width must be equal w*hdbl
					//	- pitch must be aligned to u32, width need not to be aligned
					//	- sum of widths + separators of all slots must be equal vtime.hactive
					//	- width can be 0, to use only color separator
// aligned
	u16		hdbl;		// 0x10: number of video pixels per graphics pixel 1..16 (fast) or 1..32 (normal) (1=full resolution, 2=double pixels, ...)
					//	- if hdbl > 1 with HDMI, pixel formats 1-6 bits are not supported
					//	- width must be divisible by hdbl
	u16		w;		// 0x12: width of slot in graphics pixels
					//	- w*hdbl must be equal width
					//	- pitch must be aligned to u32, width need not to be aligned
					//	- w can be 0, to use only color separator
// aligned
	u16		pitch;		// 0x14: length of line or row in bytes
					//	- pitch should be aligned to u32 (width can be less than pitch)

	// height of the slot
	//  also: int strip->height ... real height of the strip in video scanlines
	u16		vdbl;		// 0x16: number of scanlines per graphics line 1.. (1=single line, 2=double lines, ...)
					//	- height of the strip does not have to be a multiple of vdbl
// aligned
	u16		lines;		// 0x18: height of the slot in graphics lines
					//	- lines*vdbl must be equal or greater than height
	u16		h;		// 0x1A: height of the slot in graphics lines, or number of rows in text mode
					//	- h*vdbl (or h*fonth*vdbl if font) must be equal or greater height of the strip
// aligned

	// color separator (after this slot)
	u16		gap_col;	// 0x1C: color separator in RGB565 format (not precise color, near DC balanced TMDS colors will be used)
	s16		gap_len;	// 0x1E: number of video pixels of color separator between slots (should be even numer in range 10..20)
					//	- used to enable switching slot to another format (when needed to change registers)
					//	- if too small (less than 10 or 17), signal can drop-out
					//	- if too large (greater than 20 or 25), colors can be broken
// aligned

	// frame buffer
	u8*		buf;		// 0x20: frame buffer base address (size in bytes: pitch * h)

	// palettes
	//  also: int vcolor.palnum ... number of required palette entries (0 = palette not used)
	const u16*	pal;		// 0x24: pointer to palettes in RGB565 format (used in some formats)
					//	- palette buffer should be in RAM for faster access
	const u32*	palvga;		// 0x28: pointer to palettes for VGA, double-size format with time dithering;
					//	- it must be large enough to accept palette entries in 2*u32 format (size in bytes = palnum*8)

	// font
	const u8*	font;		// 0x2C: pointer to font (width 8 pixels), or to column/row of tiles, or offsets of lines of HSTX, or attributes
					//	- should be in RAM for faster access

	// HSTX command - active scanline, prefix (before image data)
	// - HSTX register expand_shift setup
	// - HSTX register expand_tmds setup ... only DVI, not VGA
	// - HSTX data command
	u32		vactive_1[3];	// 0x30 (size 0x0C):

	// HSTX command - active scanline, color separator after slot
	//  This command must be 9 words long (not containing TMDS codes) to fill up FIFO queue
	//  and shifter to be able to redefine expander (so that its shifter is not used and is empty).
	u32		vactive_2[9];	// 0x3C (size 0x24):

	// format descriptor
	//  also: int vcolor.colbits ... number of bits per pixel (including unused bits)
	sDispHstxVColor	vcolor;		// 0x60 (size 0x20 = DISPHSTXVCOLOR_SIZE): copy of pixel color format descriptor

} sDispHstxVSlot;

#define DISPHSTXVSLOT_SIZE (0x60+DISPHSTXVCOLOR_SIZE)	// size of the sDispHstxVSlot structure
STATIC_ASSERT(sizeof(sDispHstxVSlot) == DISPHSTXVSLOT_SIZE, "Incorrect sDispHstxVSlot!");

#define DISPHSTX_DVI_CMDLEN	4	// length of one command (number of u32 words)
	// each command contains 4x u32 words as alias 3:
	// - control word
	// - write address
	// - transfer count
	// - read address + trigger

#define DISPHSTX_DVI_CMDNUM	4	// max. number of commands per slot
	// - expand_shift + expand_tmds, from vactive_1[0-1]
	// - prefix (HSTX data command), from vactive_1[2] ... not in hstxmode of DVI
	// - data, from render buffer or from frame buffer
	// - suffix (color separator), from vactive_2[0-7]
	// After all slots:
	// - HSYNC suffix, from vactive_3[0-7]
	// - NULL stop mark

// length of command buffer of active scanline, in u32 (+ HSYNC suffix + NULL)
#define DISPHSTX_DVI_ACMDLEN	(DISPHSTX_DVI_CMDLEN*(DISPHSTX_DVI_CMDNUM*DISPHSTX_SLOT_MAX+2)) // = 4*(4*DISPHSTX_SLOT_MAX+2) = DISPHSTX_SLOT_MAX*16+8

// videomode strip descriptor
// - Structure definition must match definition in files disphstx_*.S
typedef struct {
	// real height of the strip in video scanlines
	u16		height;		// 0x00: height of strip in number of video scanlines
					//	- sum of heights of all strips must be equal or greater vtime.vactive
	u8		slotnum;	// 0x02: number of videomode slots
	u8		res;		// 0x03: ... reserved (align)
// aligned

	// HSTX command - active scanline, suffix after all slots (8 words)
	//  This command must be 9 words long (not containing TMDS codes) to fill up FIFO queue
	//  and shifter to be able to redefine expander (so that its shifter is not used and is empty).
	// - (2) horizontal front porch
	// - (1) NOP
	// - (2) HSYNC
	// - (2) 2x NOP
	// - (2) horizontal back porch
	u32		vactive_3[9];	// 0x04 (size 0x24)

	// 2 command buffers of active scanline (4 commands per slot (expand, prefix, image, suffix) + HSYNC + NULL)
	u32		vactive_cmd[DISPHSTX_DVI_ACMDLEN * 2]; // 0x28 (size DISPHSTX_SLOT_MAX*0x80 + 0x40):

	// slots
	// - sum of widths of all slots must be equal vtime.hactive
	sDispHstxVSlot	slot[DISPHSTX_SLOT_MAX]; // 0x68+DISPHSTX_SLOT_MAX*0x80 (size DISPHSTX_SLOT_MAX*DISPHSTXVSLOT_SIZE): list of videomode slots

} sDispHstxVStrip;

#define DISPHSTXVSTRIP_SIZE (0x68+DISPHSTX_SLOT_MAX*(0x80+DISPHSTXVSLOT_SIZE)) // size of the sDispHstxVStrip structure
STATIC_ASSERT(sizeof(sDispHstxVStrip) == DISPHSTXVSTRIP_SIZE, "Incorrect sDispHstxVStrip!");

// videomode state descriptor
// - Structure definition must match definition in files disphstx_*.S
typedef struct {
	u8		stripnum;	// 0x00: number of videomode strips
	volatile u8	bufinx;		// 0x01: next buffer index (0 or 1)
	volatile u16	line;		// 0x02: current scanline (0 = first line of the image)
// aligned
	volatile u32*	bufnext;	// 0x04: pointer to next buffer

	// HSTX command - scanline with VSYNC OFF, blank scanline (6 words)
	// - (2) active pixels, horizontal front porch
	// - (2) HSYNC
	// - (2) horizontal back porch
	u32		vsync_off[6];	// 0x08 (size 0x18)
	// command buffer: 1 command + NULL
	u32		vsync_off_cmd[DISPHSTX_DVI_CMDLEN*2]; // 0x20 (size 0x20)

	// HSTX command - scanline with VSYNC ON (6 words)
	// - (2) VSYNC + active pixels, horizontal front porch
	// - (2) VSYNC + HSYNC
	// - (2) VSYNC + horizontal back porch
	u32		vsync_on[6];	// 0x40 (size 0x18)
	// command buffer: 1 command + NULL
	u32		vsync_on_cmd[DISPHSTX_DVI_CMDLEN*2]; // 0x58 (size 0x20)

	// HSTX command - blank scanline (8 words) ... only DVI; VGA uses vsync_off* as blank scanline
	// - (2) black active pixels
	// - (2) horizontal front porch
	// - (2) HSYNC
	// - (2) horizontal back porch
	u32		vsync_blank[8];	// 0x78 (size 0x20)
	// command buffer: 1 command + NULL
	u32		vsync_blank_cmd[DISPHSTX_DVI_CMDLEN*2]; // 0x98 (size 0x20)

	// copy of videomode timings descriptor
	//   int vtime.hactive ... horizontal resolution in video pixels
	//   int vtime.vactive ... vertical resolution in scanlines
	sDispHstxVModeTime	vtime;		// 0xB8 (size DISPHSTXVMODETIME_SIZE = 0x1C)

	// videomode strips
	// - sum of heights of all strips must be equal or greater vtime.vactive
	// - sum of widths + separators of all slots in every strip must be equal vtime.hactive
	sDispHstxVStrip	strip[DISPHSTX_STRIP_MAX]; // 0xB8 + DISPHSTXVMODETIME_SIZE (size DISPHSTXVSTRIP_SIZE*DISPHSTX_STRIP_MAX): list of videomode strips

} sDispHstxVModeState;

// Example of size:
//	DISPHSTX_STRIP_MAX = 4
//	DISPHSTX_SLOT_MAX = 4
//	DISPHSTXVMODETIME_SIZE = 0x1C
//	DISPHSTXVCOLOR_SIZE = 0x20
//	DISPHSTXVSLOT_SIZE = 0x60+DISPHSTXVCOLOR_SIZE = 0x80
//	DISPHSTXVSTRIP_SIZE = 0x68+DISPHSTX_SLOT_MAX*(0x80+DISPHSTXVSLOT_SIZE) = 0x68+4*(0x80+0x80) = 0x468
//	DISPHSTXVMODESTATE_SIZE = 0xB8+DISPHSTXVMODETIME_SIZE+DISPHSTXVSTRIP_SIZE*DISPHSTX_STRIP_MAX = 0xB8+0x1C+0x468*4 = 0x1274 = 4724 bytes
//STATIC_ASSERT(sizeof(sDispHstxVModeState) == 4724, "Incorrect sDispHstxVModeState!"); // only to check our calculations

#define DISPHSTXVMODESTATE_SIZE (0xB8+DISPHSTXVMODETIME_SIZE+DISPHSTXVSTRIP_SIZE*DISPHSTX_STRIP_MAX) // size of the sDispHstxVModeState structure
STATIC_ASSERT(sizeof(sDispHstxVModeState) == DISPHSTXVMODESTATE_SIZE, "Incorrect sDispHstxVModeState!");

// default palettes of 1-bit format DISPHSTX_FORMAT_1 (black & white)
extern u16 DispHstxPal1b[2] ALIGNED;

// default palettes of 2-bit format DISPHSTX_FORMAT_2 (grascale 0..3)
extern u16 DispHstxPal2b[4] ALIGNED;

// default color 2-bit palettes
extern u16 DispHstxPal2bcol[4] ALIGNED;

// default palettes of 3-bit format DISPHSTX_FORMAT_3 (RGB111)
extern u16 DispHstxPal3b[8] ALIGNED;

// default palettes of 4-bit format DISPHSTX_FORMAT_4 (grayscale 0..15)
extern u16 DispHstxPal4b[16] ALIGNED;

// default color 4-bit palettes YRGB1111
extern u16 DispHstxPal4bcol[16] ALIGNED;

// default palettes of 6-bit format DISPHSTX_FORMAT_6 (RGB222)
extern u16 DispHstxPal6b[64] ALIGNED;

// default palettes of 8-bit format DISPHSTX_FORMAT_8 (RGB332)
extern u16 DispHstxPal8b[256] ALIGNED;

// render buffer (VGA: 1 pixel = 1 u32 in PWM, DVI: 1 pixel = 1 u16 in RGB565)
#define DISPHSTX_RENDERLINE_SIZE	(DISPHSTX_WIDTHMAX + 2*DISPHSTX_SLOT_MAX + 4) // start of every slot must be aligned to u32
#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)
#define DISPHSTX_RENDERLINE_TYPE u32
#else
#define DISPHSTX_RENDERLINE_TYPE u16
#endif
extern DISPHSTX_RENDERLINE_TYPE DispHstxRenderLine[DISPHSTX_RENDERLINE_SIZE*2];

extern volatile u32 DispHstxFrame;		// current frame

extern sDispHstxVModeState DispHstxVMode;	// default current videomode state descriptor
extern sDispHstxVModeState* pDispHstxVMode;	// pointer to current videomode state descriptor

// check VSYNC
Bool DispHstxIsVSync();

// wait for VSync scanline
void DispHstxWaitVSync();
INLINE void WaitVSync() { DispHstxWaitVSync(); }
INLINE void VgaWaitVSync() { WaitVSync(); }

// Initialize videomode state descriptor (set timings and clear list of strips)
//  vmode ... videomode state descriptor (must not currently be used)
//  vtime ... videomode timings descriptor
void DispHstxVModeInitTime(sDispHstxVModeState* vmode, const sDispHstxVModeTime* vtime);

// Return codes of function DispHstxVModeAdd()
#define DISPHSTX_ERR_OK			0	// all OK
#define DISPHSTX_ERR_BROKEN		1	// internal structure is broken (not initialized?)
#define DISPHSTX_ERR_STRIPNUM		2	// exceeded number of strips
#define DISPHSTX_ERR_NOSTRIP		3	// trying to add slot to empty videomode descriptor (no strip was added)
#define DISPHSTX_ERR_SLOTNUM		4	// exceeded number of slots
#define DISPHSTX_ERR_FONTH		5	// incorrect font height in text mode
#define DISPHSTX_ERR_FONT		6	// incorrect font or tile pointer (font = NULL in text or tile mode)
#define DISPHSTX_ERR_BUF		7	// memory error on frame buffer allocation
#define DISPHSTX_ERR_PALVGA		8	// memory error on VGA palettes buffer allocation
#define DISPHSTX_ERR_ALIGN		9	// in direct mode (no palettes) width must be at least multiply of u8 (incorrect align of modes 1-6 bits per pixel)
#define DISPHSTX_ERR_SUPPORT		10	// 1-6 bit modes are not supported in direct mode with double pixels (error if hdbl > 1; palnum = 0)
#define DISPHSTX_ERR_COLMOD		11	// fonth in DISPHSTX_FORMAT_COL mode must be > 0 (color index modulo)
#define DISPHSTX_ERR_HSTXBUF		12	// buffer must be valid in HSTX RLE format, with width equal to width of the slot
#define DISPHSTX_ERR_HSTXFONT		13	// font must point to array of line offsets in HSTX RLE format
#define DISPHSTX_ERR_PATDIM		14	// pattern format requires dimension in pitch and fonth parameters
#define DISPHSTX_ERR_ATTRBUF		15	// memory error creating attribute buffer
#define DISPHSTX_ERR_PITCHALIGN		16	// pitch must be aligned to 32-bit word
#define DISPHSTX_ERR_FORMAT		17	// format is not supported

// add empty videomode strip to videomode state descriptor
//  vmode ... videomode state descriptor (must not currently be used)
//  height ... height of the strip in number of video scanlines, can be 0, or -1=use all remaining scanlines
//		- final sum of heights of all strips must be equal or greater than vtime.vactive
// Returns error code DISPHSTX_ERR_* (0=all OK)
int DispHstxVModeAddStrip(sDispHstxVModeState* vmode, int height);

// add videomode slot to last added videomode strip
//  vmode ... videomode state descriptor (must not currently be used)
//  hdbl ... number of video pixels per graphics pixel 1..16 (fast) or 1..32 (normal) (1=full resolution, 2=double pixels, ...)
//		- if hdbl > 1, pixel formats 1-6 bits are not supported
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
//  buf ... pointer to frame buffer (aligned to 32-bit), or NULL to create new one with malloc() (not used in single-color modes)
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
		const u16* pal, u32* palvga, const u8* font, int fonth, u16 gap_col, int gap_len);

// default VGA palette buffer (used by the DispHstxVModeInitSimple() function)
extern u32 DispHstxDefVgaPal[2*256];

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
//  pal ... pointer to palettes in RGB565 format (should be in RAM for faster access; can be NULL to use default palettes)
//  font ... pointer to font (width 8 pixels), or to column or row of tiles, or offsets of lines of HSTX, or attributes (should be in RAM for faster access; can be NULL if not text/tile format)
//  fonth ... height of font, or color modulo, or pitch of tile row (or 0=column), or pitch of attributes (-1=auto)
// Returns error code DISPHSTX_ERR_* (0=all OK)
int DispHstxVModeInitSimple(sDispHstxVModeState* vmode, const sDispHstxVModeTime* vtime, int hdbl, int vdbl,
	int format, void* buf, const u16* pal, const u8* font, int fonth);

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
// Returns error code DISPHSTX_ERR_* (DISPHSTX_ERR_OK = 0 = all OK)
int DispHstxVModeStartSimple(int dispmode, void* buf, int vmodeinx, int hdbl, int vdbl, int format);

// pre-prepare videomode state descriptor before start video mode
// - called internaly from DispHstxDviPrepare and DispHstxVgaPrepare
// - adds missing slots
void DispHstxPrePrepare(sDispHstxVModeState* v);

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
void DispHstxLinkCan(const sDispHstxVSlot* slot, sDrawCan* can);
#endif

// terminate all current devices
void DispHstxAllTerm();

// select and start display mode and videomode
//  dispmode ... display mode DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA, set to DISPHSTX_DISPMODE_NONE or 0 to auto-detect display mode selection
//  vmode ... initialized videomode state descriptor
void DispHstxSelDispMode(int dispmode, sDispHstxVModeState* vmode);

// exchange video mode, without terminating current mode
// - HSTX DVI/VGA mode must be started using DispHstxDviStart()/DispHstxVgaStart()
// - VGA palettes must be re-generated (use DispHstxVgaPal2())
void DispHstxExchange(sDispHstxVModeState* vmode);

// re-select display mode with old videomode
void DispHstxReSelDispMode(int dispmode);

// auto re-select current display mode by selection switch
// - should be called periodically from the main program loop
void DispHstxAutoDispMode();

// init display selection switch
void DispHstxDispSelInit();

// terminate display selection switch
void DispHstxDispSelTerm();

// get current display selection switch (display selection switch is auto-initialized)
int DispHstxCheckDispSel();

// auto-detect display selection on program start (returns DISPHSTX_DISPMODE_DVI or DISPHSTX_DISPMODE_VGA)
// - Tests by pull-ups whether a VGA monitor is connected (that is, whether it pulls down
//   the input signal to 0). A connected HDMI monitor is not pulling down the signal to 0.
// - Can only be used at program start or before initializing the video mode because it resets the GPIO pins.
// - If the display selection switch is enabled, it is used in preference.
// - The RGB pins of the VGA display are supposed to be connected to GPIO 12 to 17.
int DispHstxAutoDispSel();

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
void DispHstxClockReinit(int khz);
#endif

// get buffer of first display slot
u8* DispHstxBuf();

// free buffer of first display slot
void DispHstxFreeBuf();

// execute core 1 remote function
void DispHstxCore1Exec(void (*fnc)());

// check if core 1 is busy (executing remote function)
Bool DispHstxCore1Busy();

// wait if core 1 is busy (executing remote function)
void DispHstxCore1Wait();

#ifdef __cplusplus
}
#endif

#endif // _DISPHSTX_VMODE_H

#endif // USE_DISPHSTX
