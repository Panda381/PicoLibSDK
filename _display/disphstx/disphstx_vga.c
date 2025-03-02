
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                             HSTX VGA driver
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

#if DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

// convert 5-bit level 0..31 to VGA 32-bit code (with base on bit 0)
// - Do not use 'const', need to be in faster SRAM
// - Use index+0 with even frame, use index+32 with odd frame
u32 DispHstxVgaLev5[32*2] = {
	// even frame
	0x00000000, 0x00000000, 0x00010000, 0x00010000, 0x00000001, 0x00000001, 0x00010001, 0x00010001,
	0x00030001, 0x00010003, 0x00030003, 0x00030003, 0x00090003, 0x00030009, 0x00090009, 0x00090009,
	0x000C0009, 0x000C0009, 0x0009000C, 0x0009000C,	0x000C000C, 0x000C000C, 0x000D000C, 0x000D000C,
	0x000C000D, 0x000C000D, 0x000D000D, 0x000D000D,	0x000F000D, 0x000F000D, 0x000F000F, 0x000F000F,

	// odd frame
	0x00000000, 0x00010000, 0x00010000, 0x00000001, 0x00000001, 0x00010001, 0x00010001, 0x00030001,
	0x00010003, 0x00030003, 0x00030003, 0x00090003, 0x00030009, 0x00090009, 0x00090009, 0x000C0009,
	0x000C0009, 0x0009000C, 0x0009000C, 0x000C000C, 0x000C000C, 0x000D000C, 0x000D000C, 0x000C000D,
	0x000C000D, 0x000D000D, 0x000D000D, 0x000F000D, 0x000F000D, 0x000F000F, 0x000F000F, 0x000F000F,
};

#if DISPHSTX_USE_FORMAT_12
// convert 4-bit level 0..15 to VGA 32-bit code (with base on bit 0)
// - Do not use 'const', need to be in faster SRAM
// - Use index+0 with even frame, use index+16 with odd frame
u32 DispHstxVgaLev4[16*2] = {
	// even frame
	0x00000000, 0x00010000, 0x00000001, 0x00010001,
	0x00030001, 0x00030003, 0x00090003, 0x00090009,
	0x000C0009, 0x0009000C, 0x000C000C, 0x000D000C,
	0x000C000D, 0x000D000D, 0x000F000D, 0x000F000F,

	// odd frame
	0x00000000, 0x00010000, 0x00000001, 0x00010001,
	0x00010003, 0x00030003, 0x00030009, 0x00090009,
	0x000C0009, 0x0009000C, 0x000C000C, 0x000D000C,
	0x000C000D, 0x000D000D, 0x000F000D, 0x000F000F,
};
#endif

// VGA palettes (double format with time dithering)
u32 DispHstxPalVga1b[2*2];	// 1-bit, DISPHSTX_FORMAT_1 format, palettes DispHstxPal1b[2]
u32 DispHstxPalVga2b[4*2];	// 2-bit, DISPHSTX_FORMAT_2 format, palettes DispHstxPal2b[4]
u32 DispHstxPalVga2bcol[4*2];	// 2-bit palettes color, palettes DispHstxPal2bcol[4]
u32 DispHstxPalVga3b[8*2];	// 3-bit, DISPHSTX_FORMAT_3 format, palettes DispHstxPal3b[8]
u32 DispHstxPalVga4b[16*2];	// 4-bit, DISPHSTX_FORMAT_4 format, palettes DispHstxPal4b[16]
u32 DispHstxPalVga4bcol[16*2];	// 4-bit palettes YRGB1111, palettes DispHstxPal4bcol[16]
u32 DispHstxPalVga6b[64*2];	// 6-bit, DISPHSTX_FORMAT_6 format, palettes DispHstxPal6b[64]
u32 DispHstxPalVga8b[256*2];	// 8-bit, DISPHSTX_FORMAT_8 format, palettes DispHstxPal8b[256]

#if DISPHSTX_USE_FORMAT_15 || DISPHSTX_USE_FORMAT_16 || DISPHSTX_USE_FORMAT_HSTX_15 || DISPHSTX_USE_FORMAT_HSTX_16
u32 DispHstxPalVgaRG[1024*2];	// 2*10-bit, even and odd, convert RG components of RGB565/RGB555 to VGA 32-bit (convert B with DispHstxVgaLev5[])
#endif

#if DISPHSTX_USE_FORMAT_12
u32 DispHstxPalVgaRG12[256*2];	// 2*8-bit, even and odd, convert RG components of RGB444 to VGA 32-bit (convert B with DispHstxVgaLev4[])
#endif

Bool DispHstxPalVgaOk = False;

// convert 16-bit color in RGB565 format to VGA 32-bit palette (frame 0=even, 1=odd)
u32 DispHstxVgaPal1(u16 col, int frame)
{
	frame *= 32;
	int b = (col & 0x1f) + frame;
	int g = ((col >> 6) & 0x1f) + frame;
	int r = (col >> 11) + frame;
	return DispHstxVgaLev5[b] | (DispHstxVgaLev5[g] << 4) | (DispHstxVgaLev5[r] << 8);
}

// convert table of 16-bit colors in RGB565 format to VGA 32-bit palettes
void DispHstxVgaPal(u32* dst, const u16* src, int num)
{
	for (; num > 0; num--) *dst++ = DispHstxVgaPal1(*src++, 0);
}

// convert table of 16-bit colors in RGB565 format to VGA 32-bit palettes - double format with time dithering
void NOINLINE DispHstxVgaPal2(u32* dst, const u16* src, int num)
{
	int i;
	const u16* s = src;
	for (i = num; i > 0; i--) *dst++ = DispHstxVgaPal1(*s++, 0);
	for (; num > 0; num--) *dst++ = DispHstxVgaPal1(*src++, 1);
}

// prepare VGA tables
void DispHstxVgaTabSetup()
{
	if (DispHstxPalVgaOk) return;
	DispHstxPalVgaOk = True;

	DispHstxVgaPal2(DispHstxPalVga1b, DispHstxPal1b, 2);
	DispHstxVgaPal2(DispHstxPalVga2b, DispHstxPal2b, 4);
	DispHstxVgaPal2(DispHstxPalVga2bcol, DispHstxPal2bcol, 4);
	DispHstxVgaPal2(DispHstxPalVga3b, DispHstxPal3b, 8);
	DispHstxVgaPal2(DispHstxPalVga4b, DispHstxPal4b, 16);
	DispHstxVgaPal2(DispHstxPalVga4bcol, DispHstxPal4bcol, 16);
	DispHstxVgaPal2(DispHstxPalVga6b, DispHstxPal6b, 64);
	DispHstxVgaPal2(DispHstxPalVga8b, DispHstxPal8b, 256);

	int i;
#if DISPHSTX_USE_FORMAT_15 || DISPHSTX_USE_FORMAT_16 || DISPHSTX_USE_FORMAT_HSTX_15 || DISPHSTX_USE_FORMAT_HSTX_16
	for (i = 0; i < 1024; i++)
	{
		int g = i & 0x1f;
		int r = i >> 5;
		DispHstxPalVgaRG[i] = (DispHstxVgaLev5[g] << 4) | (DispHstxVgaLev5[r] << 8);
	}

	for (; i < 2*1024; i++)
	{
		int g = (i & 0x1f) + 32;
		int r = ((i >> 5) & 0x1f) + 32;
		DispHstxPalVgaRG[i] = (DispHstxVgaLev5[g] << 4) | (DispHstxVgaLev5[r] << 8);
	}
#endif

#if DISPHSTX_USE_FORMAT_12
	for (i = 0; i < 256; i++)
	{
		int g = i & 0x0f;
		int r = i >> 4;
		DispHstxPalVgaRG12[i] = (DispHstxVgaLev4[g] << 4) | (DispHstxVgaLev4[r] << 8);
	}

	for (; i < 2*256; i++)
	{
		int g = (i & 0x0f) + 16;
		int r = ((i >> 4) & 0x0f) + 16;
		DispHstxPalVgaRG12[i] = (DispHstxVgaLev4[g] << 4) | (DispHstxVgaLev4[r] << 8);
	}
#endif
}

// flag - HSTX VGA mode is active
Bool DispHstxVgaRunning = False;

// IRQ handler
void FASTCODE NOFLASH(DispHstxVgaIrqHandler)()
{
	// Clear the interrupt request for DMA control channel
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);

	// pointer to videomode state descriptor
	sDispHstxVModeState* v = pDispHstxVMode;

	// start next control buffer
	DMA_SetReadTrig(DISPHSTX_DMA_CMD, v->bufnext);

	// check CPU load time
#if DISPHSTX_CHECK_LOAD	// 1 = check CPU load during IRQ interrupt (get DispHstxTimeIn, DispHstxTimeOut)
	u32 t = TimeSysClk();
	u32 tt = t - DispHstxTimeOld;
	DispHstxTimeOut += tt;
	if (tt < DispHstxTimeOutMin) DispHstxTimeOutMin = tt;
	DispHstxTimeNum++;
#endif

#if DISPHSTX_CHECK_LEDIRQ >= 0	// LED_PIN, >=0 (use GPIO pin) = use debug LED to indicate that we are alive, interval about 1 sec
	static int kk = 0;
	kk++;
	if (kk >= 35000)
	{
		kk = 0;
		GPIO_Flip(DISPHSTX_CHECK_LEDIRQ);
	}
#endif

	// shift buffer index
	int bufinx = v->bufinx ^ 1;
	v->bufinx = bufinx;

	// increase scanline
	int line = v->line + 1;
	if (line >= v->vtime.vtotal)
	{
		line = 0;
		DispHstxFrame++;
	}
	v->line = line;

	// active image
	u32* bufnext;
	int vactive = v->vtime.vactive;	// vertical active scanlines
	if (line < vactive)
	{
		// pre-set default = blank scanline
		bufnext = v->vsync_off_cmd;

		// get strip
		int stripinx;
		for (stripinx = 0; stripinx < v->stripnum; stripinx++)
		{
			sDispHstxVStrip* strip = &v->strip[stripinx];
			if (line < strip->height) // does the scanline belong in this strip?
			{
				// pointer to command buffer
				bufnext = &strip->vactive_cmd[DISPHSTX_DVI_ACMDLEN*bufinx];

				// get slot
				int slotinx;
				for (slotinx = 0; slotinx < strip->slotnum; slotinx++)
				{
					sDispHstxVSlot* slot = &strip->slot[slotinx];

					// render line
					if (slot->w > 0) slot->vcolor.render_vga(slot, line/slot->vdbl, bufnext + slot->data_inx, (DispHstxFrame ^ line) & 1);
				}

				// all done for current strip, break down the loop
				break;
			}
       			else
				// shift line to next strip
				line -= strip->height;
		}
	}

	// VSYNC
	else if ((line >= vactive + v->vtime.vfront) && (line < v->vtime.vtotal - v->vtime.vback))
	{
		bufnext = v->vsync_on_cmd;
	}

	// front porch or back porch
	else
	{
		bufnext = v->vsync_off_cmd;
	}

	// setup next command buffer
	v->bufnext = bufnext;

#if DISPHSTX_CHECK_LOAD	// 1 = check CPU load during IRQ interrupt (get DispHstxTimeIn, DispHstxTimeOut)
	u32 t2 = TimeSysClk();
	DispHstxTimeOld = t2;
	t2 -= t;
	DispHstxTimeIn += t2;
	if (t2 > DispHstxTimeInMax) DispHstxTimeInMax = t2;
#endif
}

// DMA control word to write sync command to HSTX
#define DISPHSTX_VGA_CTRL_SYNC (			\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ(DREQ_HSTX) |		\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_INC_READ |			\
		DMA_CTRL_SIZE(DMA_SIZE_32) |		\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// DMA control word to stop transfer
#define DISPHSTX_VGA_CTRL_STOP (			\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ_FORCE |			\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// prepare videomode state descriptor before start HSTX VGA mode
void DispHstxVgaPrepare(sDispHstxVModeState* v)
{
	int n;

	// prepare VGA tables
	DispHstxVgaTabSetup();

	// pre-prepare videomode state descriptor before start video mode
	// - adds missing slots
	DispHstxPrePrepare(v);

	// pointer to videomode timings
	sDispHstxVModeTime* vtime = &v->vtime;
	int clkdiv = vtime->clkdiv;

	// prepare write address to HSTX FIFO
	u32 w = (u32)&hstx_fifo_hw->fifo;

	// prepare HSTX command "scanline with VSYNC OFF" and "blank scanline"
	u32* data = v->vsync_off;
	u32* cmd = v->vsync_off_cmd;

	cmd[0] = DISPHSTX_VGA_CTRL_SYNC;	// DMA control word
	cmd[1] = w;				// DMA write address
	cmd[2] = 6;				// DMA transfer count
	cmd[3] = (u32)data;			// DMA read address

	cmd[4] = DISPHSTX_VGA_CTRL_STOP;
	cmd[5] = 0;
	cmd[6] = 0;
	cmd[7] = 0;

	data[0] = HSTX_CMD_RAW_REPEAT | ((vtime->hactive + vtime->hfront)*clkdiv); // active pixels, horizontal front porch
	data[1] = VGA_SYNC_V0_H0;				// blank
	data[2] = HSTX_CMD_RAW_REPEAT | (vtime->hsync*clkdiv);	// HSYNC
	data[3] = VGA_SYNC_V0_H1;				// HSYNC
	data[4] = HSTX_CMD_RAW_REPEAT | (vtime->hback*clkdiv);	// horizontal back porch
	data[5] = VGA_SYNC_V0_H0;				// blank

	// prepare HSTX command "scanline with VSYNC ON"
	data = v->vsync_on;
	cmd = v->vsync_on_cmd;

	cmd[0] = DISPHSTX_VGA_CTRL_SYNC;	// DMA control word
	cmd[1] = w;				// DMA write address
	cmd[2] = 6;				// DMA transfer count
	cmd[3] = (u32)data;			// DMA read address

	cmd[4] = DISPHSTX_VGA_CTRL_STOP;
	cmd[5] = 0;
	cmd[6] = 0;
	cmd[7] = 0;

	data[0] = HSTX_CMD_RAW_REPEAT | ((vtime->hactive + vtime->hfront)*clkdiv); // active pixels, horizontal front porch
	data[1] = VGA_SYNC_V1_H0;				// blank
	data[2] = HSTX_CMD_RAW_REPEAT | (vtime->hsync*clkdiv);	// HSYNC
	data[3] = VGA_SYNC_V1_H1;				// HSYNC
	data[4] = HSTX_CMD_RAW_REPEAT | (vtime->hback*clkdiv);	// horizontal back porch
	data[5] = VGA_SYNC_V1_H0;				// blank

	// prepare stripes
	int stripinx;
	for (stripinx = 0; stripinx < v->stripnum; stripinx++)
	{
		// pointer to strip
		sDispHstxVStrip* strip = &v->strip[stripinx];

		// pointer to render buffer
		u32 render = (u32)DispHstxRenderLine;
		u32 render2 = render + DISPHSTX_RENDERLINE_SIZE*sizeof(DISPHSTX_RENDERLINE_TYPE);

		// pointer to command buffer of active scanline
		cmd = strip->vactive_cmd;
		u32* cmd2 = cmd + DISPHSTX_DVI_ACMDLEN;

		// prepare slots
		int slotinx;
		for (slotinx = 0; slotinx < strip->slotnum; slotinx++)
		{
			// pointer to slot
			sDispHstxVSlot* slot = &strip->slot[slotinx];

			// align render buffer to u32 (skip unaligned data from previous slot)
			render = (render + 3) & ~3;
			render2 = (render2 + 3) & ~3;

			// prepare expand_shift register
			u32 reg = slot->hdbl*clkdiv;
			if (reg > 32) reg = 32;
			reg &= 0x1f;
			reg = (reg << 24) |		// number of times to shift before refilling TMDS
				(0 << 16) |		// number of bits to rotate right on TMDS = color depth
				(reg << 8) |		// number of times to shift before refilling on raw data
				(0 << 0);		// number of bits to rotate right on raw data
			slot->vactive_1[0] = reg;	// setup expand_shift register

			// find previous valid slot
			sDispHstxVSlot* slotprev = NULL;
			int i = slotinx-1;
			for (; i >= 0; i--)
			{
				if ((strip->slot[i].w > 0) &&
					(strip->slot[i].format != DISPHSTX_FORMAT_NONE))
				{
					slotprev = &strip->slot[i];
					break;
				}
			}

			// setup expander register - only if this is first slot, or register has been changed
			if (	(slot->format != DISPHSTX_FORMAT_NONE) &&
				(slot->w > 0) &&
					((slotprev == NULL) ||
					(slot->vactive_1[0] != slotprev->vactive_1[0])))
			{
				// prepare command to setup expander registers
				reg =	// DMA_CTRL_SNIFF |		// not sniff
					// DMA_CTRL_BSWAP |		// use byte swap
					DMA_CTRL_QUIET |		// quiet (raise IRQ on NULL)
					DMA_CTRL_TREQ_FORCE |		// data request force
					DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) | // chain to DMA command channel
					// DMA_CTRL_RING_WRITE |	// not wrap ring on write
					// DMA_CTRL_RING_SIZE(0) |	// no wrap ring
					// DMA_CTRL_INC_WRITE_REV |	// write address is 1 = decremented, 0 = incremented
					DMA_CTRL_INC_WRITE |		// increment on write
					// DMA_CTRL_INC_READ_REV |	// read address is 1 = decremented, 0 = incremented
					DMA_CTRL_INC_READ |		// increment on read
					DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size
					DMA_CTRL_HIGH_PRIORITY |	// high priority
					DMA_CTRL_EN;			// enable DMA
				cmd[0] = reg;		// DMA control word
				cmd2[0] = reg;

				// write address - expand_shift register
				reg = (u32)&hstx_ctrl_hw->expand_shift;
				cmd[1] = reg;
				cmd2[1] = reg;

				// transfer count - 1 word u32
				cmd[2] = 1;
				cmd2[2] = 1;

				// read address
				reg = (u32)&slot->vactive_1[0];
				cmd[3] = reg;
				cmd2[3] = reg;

				// shift command register
				cmd += 4;
				cmd2 += 4;
			}

			// prepare DMA control word to write pixel data to HSTX
			u32 ctrl_data =
				// DMA_CTRL_SNIFF |		// not sniff
				// DMA_CTRL_BSWAP |		// use byte swap
				DMA_CTRL_QUIET |		// quiet (raise IRQ on NULL)
				DMA_CTRL_TREQ(DREQ_HSTX) |	// data request from HSTX
				DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) | // chain to DMA command channel
				// DMA_CTRL_RING_WRITE |	// not wrap ring on write
				// DMA_CTRL_RING_SIZE(0) |	// no wrap ring
				// DMA_CTRL_INC_WRITE_REV |	// write address is 1 = decremented, 0 = incremented
				// DMA_CTRL_INC_WRITE |		// no increment write
				// DMA_CTRL_INC_READ_REV |	// read address is 1 = decremented, 0 = incremented
				// DMA_CTRL_INC_READ |		// increment read
				DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size
				DMA_CTRL_HIGH_PRIORITY |	// high priority
				DMA_CTRL_EN;			// enable DMA

			// increment on read - only if not simple-color mode
			if (!slot->vcolor.colmode) ctrl_data |= DMA_CTRL_INC_READ; // increment on read

			// prepare DMA transfer count
			n = slot->w;

			// only if there are some active pixels (width > 0)
			if (n > 0)
			{
				// prepare HSTX command "active scanline, prefix"
				slot->vactive_1[2] = HSTX_CMD_RAW | (slot->width*clkdiv); // width in video pixels

				// prepare DMA command to send data prefix
				cmd[0] = DISPHSTX_VGA_CTRL_SYNC; // DMA control word
				cmd[1] = w;			// DMA write address
				cmd[2] = 1;			// DMA transfer count
				cmd[3] = (u32)&slot->vactive_1[2]; // DMA read address

				cmd2[0] = DISPHSTX_VGA_CTRL_SYNC; // DMA control word
				cmd2[1] = w;			// DMA write address
				cmd2[2] = 1;			// DMA transfer count
				cmd2[3] = (u32)&slot->vactive_1[2]; // DMA read address

				// prepare DMA command to send data
				cmd[4] = ctrl_data;		// DMA control word
				cmd[5] = w;			// DMA write address
				cmd[6] = n;			// DMA transfer count
				cmd[7] = render;		// DMA read address

				cmd2[4] = ctrl_data;		// DMA control word
				cmd2[5] = w;			// DMA write address
				cmd2[6] = n;			// DMA transfer count
				cmd2[7] = render2;		// DMA read address

				// save data index
				slot->data_inx = &cmd[7] - strip->vactive_cmd;

				// shift address of render buffer
				if ((ctrl_data & DMA_CTRL_INC_READ) == 0) n = 1; // render simple color
				n *= sizeof(u32);
				render += n;
				render2 += n;

				// shift command register
				cmd += 8;
				cmd2 += 8;
			}

			// prepare color separator
			data = slot->vactive_2;
			n = slot->gap_len;
			if (n > 0)
			{
				n *= clkdiv;
				u32 col = DispHstxVgaPal1(slot->gap_col, 0);
				if (n < 4)
				{
					data[0] = HSTX_CMD_RAW_REPEAT | n;
					data[1] = col;
					n = 2;
				}
				else if (n < 8)
				{
					data[0] = HSTX_CMD_RAW_REPEAT | ((n+0)/2);
					data[1] = col;
					data[2] = HSTX_CMD_RAW_REPEAT | ((n+1)/2);
					data[3] = col;
					n = 4;
				}
				else
				{
					data[0] = HSTX_CMD_RAW_REPEAT | ((n+0)/4);
					data[1] = col;
					data[2] = HSTX_CMD_NOP;
					data[3] = HSTX_CMD_RAW_REPEAT | ((n+1)/4);
					data[4] = col;
					data[5] = HSTX_CMD_RAW_REPEAT | ((n+2)/4);
					data[6] = col;
					data[7] = HSTX_CMD_RAW_REPEAT | ((n+3)/4);
					data[8] = col;
					n = 9;
				}

				// prepare control word
				ctrl_data =
					// DMA_CTRL_SNIFF |		// not sniff
					// DMA_CTRL_BSWAP |		// use byte swap
					DMA_CTRL_QUIET |		// quiet (raise IRQ on NULL)
					DMA_CTRL_TREQ(DREQ_HSTX) |	// data request from HSTX
					DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) | // chain to DMA command channel
					// DMA_CTRL_RING_WRITE |	// not wrap ring on write
					// DMA_CTRL_RING_SIZE(0) |	// no wrap ring
					// DMA_CTRL_INC_WRITE_REV |	// write address is 1 = decremented, 0 = incremented
					// DMA_CTRL_INC_WRITE |		// no increment write
					// DMA_CTRL_INC_READ_REV |	// read address is 1 = decremented, 0 = incremented
					DMA_CTRL_INC_READ |		// increment read
					DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size
					DMA_CTRL_HIGH_PRIORITY |	// high priority
					DMA_CTRL_EN;			// enable DMA

				// prepare DMA command to send suffix
				cmd[0] = ctrl_data;	// DMA control word
				cmd[1] = w;		// DMA write address
				cmd[2] = n;		// DMA transfer count
				cmd[3] = (u32)data;	// DMA read address

				cmd2[0] = ctrl_data;	// DMA control word
				cmd2[1] = w;		// DMA write address
				cmd2[2] = n;		// DMA transfer count
				cmd2[3] = (u32)data;	// DMA read address

				cmd += 4;
				cmd2 += 4;
			}
		}

		// prepare HSYNC suffix
		data = strip->vactive_3;
		data[0] = HSTX_CMD_RAW_REPEAT | (vtime->hfront*clkdiv); // active pixels, horizontal front porch
		data[1] = VGA_SYNC_V0_H0;				// blank
		data[2] = HSTX_CMD_NOP;					// filler
		data[3] = HSTX_CMD_RAW_REPEAT | (vtime->hsync*clkdiv);	// HSYNC
		data[4] = VGA_SYNC_V0_H1;				// HSYNC
		data[5] = HSTX_CMD_NOP;					// filler
		data[6] = HSTX_CMD_NOP;					// filler
		data[7] = HSTX_CMD_RAW_REPEAT | (vtime->hback*clkdiv);	// horizontal back porch
		data[8] = VGA_SYNC_V0_H0;				// blank

		cmd[0] = DISPHSTX_VGA_CTRL_SYNC;	// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = 9;				// DMA transfer count
		cmd[3] = (u32)data;			// DMA read address

		cmd2[0] = DISPHSTX_VGA_CTRL_SYNC;	// DMA control word
		cmd2[1] = w;				// DMA write address
		cmd2[2] = 9;				// DMA transfer count
		cmd2[3] = (u32)data;			// DMA read address

		// write DMA NULL stop mark
		cmd[4] = DISPHSTX_VGA_CTRL_STOP;
		cmd[5] = 0;
		cmd[6] = 0;
		cmd[7] = 0;

		cmd2[4] = DISPHSTX_VGA_CTRL_STOP;
		cmd2[5] = 0;
		cmd2[6] = 0;
		cmd2[7] = 0;
	}
}

// initialize HSTX VGA mode
void DispHstxVgaInit()
{
	// pointer to videomode state descriptor
	sDispHstxVModeState* v = pDispHstxVMode;

	// prepare videomode state descriptor before start HSTX VGA mode
	DispHstxVgaPrepare(v);

	// serial output config:
	// - clock period 5 or 10 cycles
	// - pop from command expander every 5 cycles
	// - shift output shift reg by 16 every cycle
	hstx_ctrl_hw->csr = 0;			// disable HSTX
	hstx_ctrl_hw->csr =
			(5 << 28) |		// clock period of the generated clock
			(5 << 16) |		// number of times to shift the shift register before refilling
			(16 << 8) |		// number of bits to right-rotate shift register by each clock cycle
			(1 << 1) |		// enable command expander
			1;			// enable HSTX

// VGA output:
//	GPIO12	blue0	SEL_P=0, SEL_N=1
//	GPIO13	blue1	SEL_P=2, SEL_N=3
//	GPIO14	green0	SEL_P=4, SEL_N=5
//	GPIO15	green1	SEL_P=6, SEL_N=7
//	GPIO16	red0	SEL_P=8, SEL_N=9
//	GPIO17	red1	SEL_P=10, SEL_N=11
//	GPIO18	HSYNC	SEL_P=12, SEL_N=12
//	GPIO19	VSYNC	SEL_P=13, SEL_N=13

	// setup HSTX outputs
#define HSTX_BIT(bit) ((bit) | (((bit)+1)<<8))
#define HSTX_BITS(bit) ((bit) | ((bit)<<8))
// Output word:
//	31  29  27  25  23  21  19  17  15  13  11   9   7   5   3   1
//	  30  28  26  24  22  20  18  16  14  12  10   8   6   4   2   0
//
//	 .   V  R1  R0  G1  G0  B1  B0   .   V  R1  R0  G1  G0  B1  B0  
//	   .   H  R1  R0  G1  G0  B1  B0   .   H  R1  R0  G1  G0  B1  B0
	hstx_ctrl_hw->bit[DISPHSTX_VGA_B0-12] = HSTX_BIT(0);	// (GPIO12) ... B0
	hstx_ctrl_hw->bit[DISPHSTX_VGA_B1-12] = HSTX_BIT(2);	// (GPIO13) ... B1
	hstx_ctrl_hw->bit[DISPHSTX_VGA_G0-12] = HSTX_BIT(4);	// (GPIO14) ... G0
	hstx_ctrl_hw->bit[DISPHSTX_VGA_G1-12] = HSTX_BIT(6);	// (GPIO15) ... G1
	hstx_ctrl_hw->bit[DISPHSTX_VGA_R0-12] = HSTX_BIT(8);	// (GPIO16) ... R0
	hstx_ctrl_hw->bit[DISPHSTX_VGA_R1-12] = HSTX_BIT(10);	// (GPIO17) ... R1
	hstx_ctrl_hw->bit[DISPHSTX_VGA_HSYNC-12] = HSTX_BITS(12) | (v->vtime.hpol ? 0 : B16);	// (GPIO18) ... HSYNC
	hstx_ctrl_hw->bit[DISPHSTX_VGA_VSYNC-12] = HSTX_BITS(13) | (v->vtime.vpol ? 0 : B16);	// (GPIO19) ... VSYNC

#undef HSTX_BIT
#undef HSTX_BITS

	// set GPIOs to HSTX mode
	GPIO_FncMask(PinRangeMask(12, 19), GPIO_FNC_HSTX);

	// set output strength
	int i;
	for (i = 12; i <= 19; i++) GPIO_Drive8mA(i); // 2mA, 4mA, 8mA, 12mA

	// prepare next buffer
	v->line = 0;
	v->bufnext = v->vsync_off_cmd;
	v->bufinx = 0;

	// setup DMA command channel (no trigger yet)
	DMA_Config(
		DISPHSTX_DMA_CMD,			// channel
		v->vsync_off_cmd,			// read address
		&dma_hw->ch[DISPHSTX_DMA_DATA].al3_ctrl, // write address = DMA data channel (alias: CTRL, WRITE, CNT, READ)
		4,					// number of transfers = 4 (4x u32)
			// DMA_CTRL_SNIFF |		// not sniff
			// DMA_CTRL_BSWAP |		// not byte swap
			// DMA_CTRL_QUIET |		// not quiet
			DMA_CTRL_TREQ_FORCE |		// permanent request
			DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) | // disable chaining
			DMA_CTRL_RING_WRITE |		// wrap ring on write
			DMA_CTRL_RING_SIZE(4) | 	// ring size = 16 bytes
			// DMA_CTRL_INC_WRITE_REV |	// write address is 1 = decremented, 0 = incremented
			DMA_CTRL_INC_WRITE |		// increment write
			// DMA_CTRL_INC_READ_REV |	// read address is 1 = decremented, 0 = incremented
			DMA_CTRL_INC_READ |		// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) |	// data size 32 bits
			DMA_CTRL_HIGH_PRIORITY |	// high priority
			DMA_CTRL_EN);			// enable DMA

	// setup DMA IRQ
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);	// clear pending IRQ
	DMA_IRQ1Enable(DISPHSTX_DMA_DATA);	// enable IRQ from data DMA on NULL
	SetHandler(IRQ_DMA_1, DispHstxVgaIrqHandler); // set DMA IRQ handler
	NVIC_IRQEnable(IRQ_DMA_1);		// enable IRQ
	NVIC_IRQPrio(IRQ_DMA_1, IRQ_PRIO_REALTIME); // set highest IRQ priority

	// flag - HSTX VGA mode is active
	DispHstxVgaRunning = True;

	// start first DMA transfer
	DispHstxVgaIrqHandler();
}

// exchange videomode state descriptor, without terminating HSTX VGA mode
// - HSTX VGA mode must be started using DispHstxVgaInit()
// - videomode state descriptor must be prepared using DispHstxVgaPrepare()
void DispHstxVgaExchange(sDispHstxVModeState* v)
{
	// get pointer to old descriptor
	sDispHstxVModeState* vold = pDispHstxVMode;

	// get current scanline
	int line, bufinx, vactive;
	u32* bufnext;

	// disable interrupts
	di();

	do {
		ei();

		line = vold->line;
		v->line = line;

		// get buffer index
		bufinx = vold->bufinx;
		v->bufinx = bufinx;

		// prepare pointer to next buffer
		if ((line >= v->vtime.vactive + v->vtime.vfront) && (line < v->vtime.vtotal - v->vtime.vback))
		{
			// VSYNC
			bufnext = v->vsync_on_cmd;
		}
		else
		{
			// active line, front porch or back porch
			bufnext = v->vsync_off_cmd;
		}

		// set new next buffer
		v->bufnext = bufnext;

		di();

	} while (line != vold->line);

	// set new descriptor
	pDispHstxVMode = v;

	// re-enable interrupts
	ei();
}

// terminate HSTX VGA mode
void DispHstxVgaTerm()
{
	// driver is not active
	if (!DispHstxVgaRunning) return;

	// abort DMA channels (interrupt data channel first and then control channel)
	DMA_Abort(DISPHSTX_DMA_DATA); // pre-abort, could be chaining right now
	DMA_Abort(DISPHSTX_DMA_CMD);
	DMA_Abort(DISPHSTX_DMA_DATA);

	// disable IRQ1 from DMA
	NVIC_IRQDisable(IRQ_DMA_1);
	DMA_IRQ1Disable(DISPHSTX_DMA_DATA);

	// disable DMA channels
	DMA_Disable(DISPHSTX_DMA_DATA);
	DMA_Disable(DISPHSTX_DMA_CMD);

	// Clear the interrupt request for DMA control channel
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);

	// disable HSTX
	hstx_ctrl_hw->csr = 0;
	WaitUs(100);
	ResetPeriphery(RESET_HSTX);

	// set GPIOs to default mode
	GPIO_ResetMask(PinRangeMask(12, 19));

	// flag - HSTX VGA mode is not active
	DispHstxVgaRunning = False;
}

void (* volatile DispHstxVgaCore1Fnc)() = NULL; // core 1 remote function

#define DISPHSTX_VGA_REQNO		0	// request - no
#define DISPHSTX_VGA_REQINIT		1	// request - init
#define DISPHSTX_VGA_REQTERM		2	// request - terminate
volatile int DispHstxVgaReq = DISPHSTX_VGA_REQNO; // current VGA request

// VGA core
void NOFLASH(DispHstxVgaCore)()
{
	void (*fnc)();
	int req;

	// infinite loop
	while (True)
	{
		// data memory barrier
		dmb();

		// initialize/terminate VGA
		req = DispHstxVgaReq;
		if (req != DISPHSTX_VGA_REQNO)
		{
			if (req == DISPHSTX_VGA_REQINIT)
			{
				DispHstxVgaInit(); // initialize
			}
			else
			{
				DispHstxVgaTerm(); // terminate
			}

			DispHstxVgaReq = DISPHSTX_VGA_REQNO;
		}

		// execute remote function
		fnc = DispHstxVgaCore1Fnc;
		if (fnc != NULL)
		{
			fnc();
			dmb();
			DispHstxVgaCore1Fnc = NULL;
		}
	}
}

// execute core 1 remote function
void DispHstxVgaCore1Exec(void (*fnc)())
{
	dmb();
	DispHstxVgaCore1Fnc = fnc;
	dmb();
}

// check if core 1 is busy (executing remote function)
Bool DispHstxVgaCore1Busy()
{
	dmb();
	return DispHstxVgaCore1Fnc != NULL;
}

// wait if core 1 is busy (executing remote function)
void DispHstxVgaCore1Wait()
{
	while (DispHstxVgaCore1Busy()) {}
}

// start VGA on core 1 from core 0 (must be paired with DispHstxVgaStop())
void DispHstxVgaStart()
{
	dmb();
	if (DispHstxVgaRunning) return;

	// run VGA core
	Core1Exec(DispHstxVgaCore);

	// initialize VGA
	DispHstxVgaReq = DISPHSTX_VGA_REQINIT;
	dmb();
	while (DispHstxVgaReq != DISPHSTX_VGA_REQNO) { dmb(); }
}

// terminate VGA on core 1 from core 0 (must be paired with DispHstxVgaStart())
void DispHstxVgaStop()
{
	dmb();
	if (!DispHstxVgaRunning) return;

	// terminate VGA
	DispHstxVgaReq = DISPHSTX_VGA_REQTERM;
	dmb();
	while (DispHstxVgaReq != DISPHSTX_VGA_REQNO) { dmb(); }

	// core 1 reset
	Core1Reset();
}

#endif // DISPHSTX_USE_VGA		// 1=use VGA support (0=or only DVI)

#endif // USE_DISPHSTX
