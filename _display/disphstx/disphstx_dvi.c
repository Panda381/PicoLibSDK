
// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                             HSTX DVI driver
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

#if DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

// TMDS data table - all values with popcount (10-bit token of level 0..255)
const u16 TmdsTable[256*2] =
{
#include "disphstx_dvi_tab.h"
};

// TMDS data table - even (10-bit token of level 0..63)
const u16 TmdsTable1[64] =
{
#include "disphstx_dvi_tab1.h"
};

// TMDS data table - odd (10-bit token of level 0..63)
const u16 TmdsTable2[64] =
{
#include "disphstx_dvi_tab2.h"
};

// TMDS commands (10 bits)
#define TMDS_CTRL_00	0x0354		// VSYNC=0, HSYNC=0
#define TMDS_CTRL_01	0x00ab		// VSYNC=0, HSYNC=1
#define TMDS_CTRL_10	0x0154		// VSYNC=1, HSYNC=0
#define TMDS_CTRL_11	0x02ab		// VSYNC=1, HSYNC=1

// DVI synchronization commands (3 x 10 = 30 bits, lanes 0=blue+SYNC 1=green 2=red)
#define HDMI_SYNC_V0_H0	(TMDS_CTRL_00 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=0, HSYNC=0
#define HDMI_SYNC_V0_H1	(TMDS_CTRL_01 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=0, HSYNC=1
#define HDMI_SYNC_V1_H0	(TMDS_CTRL_10 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=1, HSYNC=0
#define HDMI_SYNC_V1_H1	(TMDS_CTRL_11 | (TMDS_CTRL_00 << 10) | (TMDS_CTRL_00 << 20))	// VSYNC=1, HSYNC=1

#define HDMI_BLACK (0x1f0 | (0x1f0 << 10) | (0x1f0 << 20)) // black color, DC balanced

// flag - HSTX DVI mode is active
Bool DispHstxDviRunning = False;

// find nearest higher TMDS code DC balanced
u16 DispHstxTmdsDC(int val)
{
	if (val > 239) val = 239;
	while (TmdsTable[val*2+1] != 5) val++;
	return TmdsTable[val*2];
}

// convert color in RGB565 format to TMDS 30-bit word - DC balanced
u32 DispHstxRgbToTmdsDC(u16 col)
{
	// color components in rande 0..255
	int b = (col & 0x1f) << 3;
	int g = ((col >> 5) & 0x3f) << 2;
	int r = ((col >> 11) & 0x1f) << 3;
	return DispHstxTmdsDC(b) + ((u32)DispHstxTmdsDC(g) << 10) + ((u32)DispHstxTmdsDC(r) << 20);
}

// convert color in RGB565 format to TMDS 30-bit word - even (lower value)
u32 DispHstxRgbToTmds1(u16 col)
{
	int b = (col & 0x1f) << 1;
	int g = (col >> 5) & 0x3f;
	int r = ((col >> 11) & 0x1f) << 1;
	return TmdsTable1[b] + ((u32)TmdsTable1[g] << 10) + ((u32)TmdsTable1[r] << 20);
}

// convert color in RGB565 format to TMDS 30-bit word - odd (higher value)
u32 DispHstxRgbToTmds2(u16 col)
{
	int b = (col & 0x1f) << 1;
	int g = (col >> 5) & 0x3f;
	int r = ((col >> 11) & 0x1f) << 1;
	return TmdsTable2[b] + ((u32)TmdsTable2[g] << 10) + ((u32)TmdsTable2[r] << 20);
}

// DVI DMA IRQ handler
void FASTCODE NOFLASH(DispHstxDviIrqHandler)()
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
	if (line < vactive) // check line of active image
	{
		// pre-set default = blank scanline
		bufnext = v->vsync_blank_cmd;

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
					if (slot->w > 0) slot->vcolor.render_dvi(slot, line/slot->vdbl, bufnext + slot->data_inx);

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
#define DISPHSTX_DVI_CTRL_SYNC (			\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ(DREQ_HSTX) |		\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_INC_READ |			\
		DMA_CTRL_SIZE(DMA_SIZE_32) |		\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// DMA control word to stop transfer
#define DISPHSTX_DVI_CTRL_STOP (			\
		DMA_CTRL_QUIET |			\
		DMA_CTRL_TREQ_FORCE |			\
		DMA_CTRL_CHAIN(DISPHSTX_DMA_CMD) |	\
		DMA_CTRL_HIGH_PRIORITY |		\
		DMA_CTRL_EN )

// prepare videomode state descriptor before start HSTX DVI mode
void DispHstxDviPrepare(sDispHstxVModeState* v)
{
	int n;

	// pre-prepare videomode state descriptor before start video mode
	// - adds missing slots
	DispHstxPrePrepare(v);

	// pointer to videomode timings
	sDispHstxVModeTime* vtime = &v->vtime;

	// prepare control signals (0=signal is OFF, 1=signal is ON)
	u32 v0h0, v0h1, v1h0, v1h1;
	if (vtime->vpol) // VSYNC polarity 1=positive
	{
		if (vtime->hpol) // HSYNC polarity 1=positive
		{
			v0h0 = HDMI_SYNC_V0_H0; // blank
			v0h1 = HDMI_SYNC_V0_H1; // HSYNC
			v1h0 = HDMI_SYNC_V1_H0; // VSYNC
			v1h1 = HDMI_SYNC_V1_H1; // VSYNC+HSYNC
		}

		// HSYNC polarity 0=negative
		else
		{
			v0h0 = HDMI_SYNC_V0_H1; // blank
			v0h1 = HDMI_SYNC_V0_H0; // HSYNC
			v1h0 = HDMI_SYNC_V1_H1; // VSYNC
			v1h1 = HDMI_SYNC_V1_H0; // VSYNC+HSYNC
		}
	}

	// VSYNC polarity 0=negative
	else
	{
		if (vtime->hpol) // HSYNC polarity 1=positive
		{
			v0h0 = HDMI_SYNC_V1_H0; // blank
			v0h1 = HDMI_SYNC_V1_H1; // HSYNC
			v1h0 = HDMI_SYNC_V0_H0; // VSYNC
			v1h1 = HDMI_SYNC_V0_H1; // VSYNC+HSYNC
		}

		// HSYNC polarity 0=negative
		else
		{
			v0h0 = HDMI_SYNC_V1_H1; // blank
			v0h1 = HDMI_SYNC_V1_H0; // HSYNC
			v1h0 = HDMI_SYNC_V0_H1; // VSYNC
			v1h1 = HDMI_SYNC_V0_H0; // VSYNC+HSYNC
		}
	}

	// prepare write address to HSTX FIFO
	u32 w = (u32)&hstx_fifo_hw->fifo;

	// prepare HSTX command "scanline with VSYNC OFF"
	u32* data = v->vsync_off;
	u32* cmd = v->vsync_off_cmd;

	cmd[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
	cmd[1] = w;				// DMA write address
	cmd[2] = 6;				// DMA transfer count
	cmd[3] = (u32)data;			// DMA read address

	cmd[4] = DISPHSTX_DVI_CTRL_STOP;
	cmd[5] = 0;
	cmd[6] = 0;
	cmd[7] = 0;

	data[0] = HSTX_CMD_RAW_REPEAT | (vtime->hactive + vtime->hfront); // active pixels, horizontal front porch
	data[1] = v0h0;					// blank
	data[2] = HSTX_CMD_RAW_REPEAT | vtime->hsync;	// HSYNC
	data[3] = v0h1;					// HSYNC
	data[4] = HSTX_CMD_RAW_REPEAT | vtime->hback;	// horizontal back porch
	data[5] = v0h0;					// blank

	// prepare HSTX command "scanline with VSYNC ON"
	data = v->vsync_on;
	cmd = v->vsync_on_cmd;

	cmd[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
	cmd[1] = w;				// DMA write address
	cmd[2] = 6;				// DMA transfer count
	cmd[3] = (u32)data;			// DMA read address

	cmd[4] = DISPHSTX_DVI_CTRL_STOP;
	cmd[5] = 0;
	cmd[6] = 0;
	cmd[7] = 0;

	data[0] = HSTX_CMD_RAW_REPEAT | (vtime->hactive + vtime->hfront); // active pixels, horizontal front porch
	data[1] = v1h0;					// blank
	data[2] = HSTX_CMD_RAW_REPEAT | vtime->hsync;	// HSYNC
	data[3] = v1h1;					// HSYNC
	data[4] = HSTX_CMD_RAW_REPEAT | vtime->hback;	// horizontal back porch
	data[5] = v1h0;					// blank

	// prepare HSTX command "blank scanline"
	data = v->vsync_blank;
	cmd = v->vsync_blank_cmd;

	cmd[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
	cmd[1] = w;				// DMA write address
	cmd[2] = 8;				// DMA transfer count
	cmd[3] = (u32)data;			// DMA read address

	cmd[4] = DISPHSTX_DVI_CTRL_STOP;
	cmd[5] = 0;
	cmd[6] = 0;
	cmd[7] = 0;

	data[0] = HSTX_CMD_RAW_REPEAT | vtime->hactive; // active pixels
	data[1] = HDMI_BLACK;				// black
	data[2] = HSTX_CMD_RAW_REPEAT | vtime->hfront;	// horizontal front porch
	data[3] = v0h0;					// blank
	data[4] = HSTX_CMD_RAW_REPEAT | vtime->hsync;	// HSYNC
	data[5] = v0h1;					// HSYNC
	data[6] = HSTX_CMD_RAW_REPEAT | vtime->hback;	// horizontal back porch
	data[7] = v0h0;					// blank

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

			// get number of color bits
			int colbits = slot->vcolor.colbits; 		// number of color bits
			if (slot->vcolor.palnum != 0) colbits = 16;	// in palette mode, pixel is always u16

			// width align mode DISPHSTX_WALIGN_*
			int walign = slot->walign;

			// prepare expand_shift register
			u32 reg = slot->hdbl;				// repeat shift = double-pixels
			if (reg <= 1) 	// single-pixel mode
			{
				if (walign == DISPHSTX_WALIGN_32)	// 32-bit width align
				{
					reg = (32 / colbits) & 0x1f;	// number of pixels per u32 word
				}
				else if (walign == DISPHSTX_WALIGN_16)	// 16-bit width align
				{
					reg = 16 / colbits;		// number of pixels per u16 half-word
				}
				else
				{
					reg = 1;
					if (colbits < 8) reg = 8 / colbits; // number of pixels per u8 byte
				}
			}
			int shft = 32 - colbits;
			if ((colbits == 3) || (colbits == 6)) shft = colbits;
			if (colbits == 12) shft = 0;
			reg &= 0x1f;
			reg = (reg << 24) |			// number of times to shift before refilling TMDS
				(shft << 16) |			// number of bits to rotate right on TMDS = color depth
				(1 << 8) |			// number of times to shift before refilling on raw data
				(0 << 0);			// number of bits to rotate right on raw data
			slot->vactive_1[0] = reg;		// setup expand_shift register

			// prepare expand_tmds register
			slot->vactive_1[1] = slot->vcolor.expand_tmds;

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

			// setup expander registers - only if this is first slot, or some register has been changed
			if (	(slot->format != DISPHSTX_FORMAT_NONE) &&
				(slot->w > 0) &&
					((slotprev == NULL) ||
					(slot->vactive_1[0] != slotprev->vactive_1[0]) ||
					(slot->vactive_1[1] != slotprev->vactive_1[1])))
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

				// write address - expand_shift and expand_tmds registers
				reg = (u32)&hstx_ctrl_hw->expand_shift;
				cmd[1] = reg;
				cmd2[1] = reg;

				// transfer count - 2 words u32
				int nn = 2;
				reg = (u32)&slot->vactive_1[0];
				if (slotprev != NULL)
				{
					if (slot->vactive_1[1] == slotprev->vactive_1[1]) nn = 1;

					if (slot->vactive_1[0] == slotprev->vactive_1[0])
					{
						reg = (u32)&hstx_ctrl_hw->expand_tmds;
						cmd[1] = reg;
						cmd2[1] = reg;
						reg = (u32)&slot->vactive_1[1];
						nn = 1;
					}
				}
				cmd[2] = nn;
				cmd2[2] = nn;

				// read address
				cmd[3] = reg;
				cmd2[3] = reg;

				// shift command register
				cmd += 4;
				cmd2 += 4;
			}

			// prepare DMA data size
			// - If double-pixels, transfer unit will be u16 or u8, memory bus will duplicate pixels
			// Note: 1, 2 and 4-bit modes are not supported in double-pixel mode, except for paletted modes.
			u32 data_size = DMA_SIZE_32;
			if (walign != DISPHSTX_WALIGN_32)		// not 32-bit width align
			{
				data_size = DMA_SIZE_16;
				if (walign != DISPHSTX_WALIGN_16)	// not 16-bit width align
				{
					if (colbits < 16) data_size = DMA_SIZE_8;
				}
			}

			// color mode - data size is always u16
			if (slot->vcolor.colmode) data_size = DMA_SIZE_16;

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
				DMA_CTRL_SIZE(data_size) |	// data size
				DMA_CTRL_HIGH_PRIORITY |	// high priority
				DMA_CTRL_EN;			// enable DMA

			// increment on read - only if not simple-color mode
			if (!slot->vcolor.colmode) ctrl_data |= DMA_CTRL_INC_READ; // increment on read

			// use byte swap - only in 8-bit or less modes and not in palette mode
			if ((colbits < 16) && (colbits != 3) && (colbits != 6) && (colbits != 12)) ctrl_data |= DMA_CTRL_BSWAP; // use byte swap (not in 16-bit or palette mode)

			// prepare DMA transfer count
			// Note: 1, 2 and 4-bit modes are not supported in double-pixel mode (but it is OK in palette mode)
			n = slot->w;				// if double pixels, transfer unit is u16 or u8, transfer count = number of graphics pixels
			if (walign == DISPHSTX_WALIGN_32)	// 32-bit width align
			{
				n = n / (32 / colbits);
			}
			else if (walign == DISPHSTX_WALIGN_16)	// 16-bit width align
			{
				n = n / (16 / colbits);
			}
			else if (walign == DISPHSTX_WALIGN_8)	// 8-bit width align
			{
				n = n / (8 / colbits);
			}

			// only if there are some active pixels (width > 0)
			if (n > 0)
			{
				// HSTX command not sent in HSTX RLE compression format
				if (!slot->vcolor.hstxmode)
				{
					// prepare HSTX command "active scanline, prefix"
					slot->vactive_1[2] = HSTX_CMD_TMDS | slot->width; // width in video pixels

					// prepare DMA command to send data prefix
					cmd[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
					cmd[1] = w;				// DMA write address
					cmd[2] = 1;				// DMA transfer count
					cmd[3] = (u32)&slot->vactive_1[2];	// DMA read address

					cmd2[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
					cmd2[1] = w;				// DMA write address
					cmd2[2] = 1;				// DMA transfer count
					cmd2[3] = (u32)&slot->vactive_1[2];	// DMA read address

					cmd += 4;
					cmd2 += 4;
				}

				// prepare DMA command to send data
				cmd[0] = ctrl_data;		// DMA control word
				cmd[1] = w;			// DMA write address
				cmd[2] = n;			// DMA transfer count
				cmd[3] = render;		// DMA read address

				cmd2[0] = ctrl_data;		// DMA control word
				cmd2[1] = w;			// DMA write address
				cmd2[2] = n;			// DMA transfer count
				cmd2[3] = render2;		// DMA read address

				// save data index
				slot->data_inx = &cmd[3] - strip->vactive_cmd;

				// shift address of render buffer
				if (slot->vcolor.userender)
				{
					if ((ctrl_data & DMA_CTRL_INC_READ) == 0) n = 1; // render simple color
					reg = n * sizeof(u32);
					if (data_size == DMA_SIZE_16) reg = n * sizeof(u16);
					if (data_size == DMA_SIZE_8) reg = n * sizeof(u8);
					render += reg;
					render2 += reg;
				}

				// shift command register
				cmd += 4;
				cmd2 += 4;
			}

			// prepare color separator
			data = slot->vactive_2;
			n = slot->gap_len;
			if (n > 0)
			{
				u32 tmds = DispHstxRgbToTmdsDC(slot->gap_col); // get color
				if (n < 4)
				{
					data[0] = HSTX_CMD_RAW_REPEAT | n;
					data[1] = tmds;
					n = 2;
				}
				else if (n < 8)
				{
					data[0] = HSTX_CMD_RAW_REPEAT | ((n+0)/2);
					data[1] = tmds;
					data[2] = HSTX_CMD_RAW_REPEAT | ((n+1)/2);
					data[3] = tmds;
					n = 4;
				}
				else
				{
					data[0] = HSTX_CMD_RAW_REPEAT | ((n+0)/4);
					data[1] = tmds;
					data[2] = HSTX_CMD_NOP;
					data[3] = HSTX_CMD_RAW_REPEAT | ((n+1)/4);
					data[4] = tmds;
					data[5] = HSTX_CMD_RAW_REPEAT | ((n+2)/4);
					data[6] = tmds;
					data[7] = HSTX_CMD_RAW_REPEAT | ((n+3)/4);
					data[8] = tmds;
					n = 9;
				}

				// prepare DMA command to send suffix
				cmd[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
				cmd[1] = w;				// DMA write address
				cmd[2] = n;				// DMA transfer count
				cmd[3] = (u32)data;			// DMA read address

				cmd2[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
				cmd2[1] = w;				// DMA write address
				cmd2[2] = n;				// DMA transfer count
				cmd2[3] = (u32)data;			// DMA read address

				// shift command pointer
				cmd += 4;
				cmd2 += 4;
			}
		}

		// prepare HSYNC suffix
		data = strip->vactive_3;
		data[0] = HSTX_CMD_RAW_REPEAT | vtime->hfront;	// horizontal front porch
		data[1] = v0h0;					// blank
		data[2] = HSTX_CMD_NOP;				// filler
		data[3] = HSTX_CMD_RAW_REPEAT | vtime->hsync;	// HSYNC
		data[4] = v0h1;					// HSYNC
		data[5] = HSTX_CMD_NOP;				// filler
		data[6] = HSTX_CMD_NOP;				// filler
		data[7] = HSTX_CMD_RAW_REPEAT | vtime->hback;	// horizontal back porch
		data[8] = v0h0;					// blank

		cmd[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
		cmd[1] = w;				// DMA write address
		cmd[2] = 9;				// DMA transfer count
		cmd[3] = (u32)data;			// DMA read address

		cmd2[0] = DISPHSTX_DVI_CTRL_SYNC;	// DMA control word
		cmd2[1] = w;				// DMA write address
		cmd2[2] = 9;				// DMA transfer count
		cmd2[3] = (u32)data;			// DMA read address

		// write DMA NULL stop mark
		cmd[4] = DISPHSTX_DVI_CTRL_STOP;
		cmd[5] = 0;
		cmd[6] = 0;
		cmd[7] = 0;

		cmd2[4] = DISPHSTX_DVI_CTRL_STOP;
		cmd2[5] = 0;
		cmd2[6] = 0;
		cmd2[7] = 0;
	}
}

// initialize and start HSTX DVI mode (requires initialized DispHstxVMode descriptor)
// - DVI mode must be deactivated, system clock must be set
void DispHstxDviInit()
{
	// pointer to videomode state descriptor
	sDispHstxVModeState* v = pDispHstxVMode;

	// prepare videomode state descriptor before start HSTX DVI mode
	DispHstxDviPrepare(v);

	// serial output config (output 3 lane TMDS tokens, every token is 10 bits long):
	// - clock period 5 cycles
	// - pop from command expander every 5 cycles
	// - shift output shift reg by 2 every cycle
	hstx_ctrl_hw->csr = 0;			// disable HSTX
	hstx_ctrl_hw->csr =
			(5 << 28) |		// clock period of the generated clock, 5 = generate complete output period (10 half-clocks)
			(5 << 16) |		// number of times to shift the shift register before refilling = 5 (one TMDS word is 2*5=10 bits)
			(2 << 8) |		// number of bits to right-rotate shift register by each clock cycle = 2
			(1 << 1) |		// enable command expander
			1;			// enable HSTX

	// setup HSTX outputs
#define HSTX_BIT(lane) (((lane)*10)|(((lane)*10+1)<<8))

	hstx_ctrl_hw->bit[DISPHSTX_DVI_D2P-12] = HSTX_BIT(2);		// output D2+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D2M-12] = HSTX_BIT(2)|B16;	// output D2-
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D1P-12] = HSTX_BIT(1);		// output D1+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D1M-12] = HSTX_BIT(1)|B16;	// output D1-
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D0P-12] = HSTX_BIT(0);		// output D0+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_D0M-12] = HSTX_BIT(0)|B16;	// output D0-
	hstx_ctrl_hw->bit[DISPHSTX_DVI_CLKP-12] = B17;			// output CLK+
	hstx_ctrl_hw->bit[DISPHSTX_DVI_CLKM-12] = B17|B16;		// output CLK-

#undef HSTX_BIT

	// set GPIOs to HSTX mode
	GPIO_FncMask(PinRangeMask(12, 19), GPIO_FNC_HSTX);

	// set output strength
	int i;
	for (i = 12; i <= 19; i++) GPIO_Drive8mA(i); // 2mA, 4mA, 8mA, 12mA

	// prepare next buffer
	v->line = 0;
	v->bufnext = v->vsync_blank_cmd;
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
	DMA_IRQ1Clear(DISPHSTX_DMA_DATA);		// clear pending IRQ
	DMA_IRQ1Enable(DISPHSTX_DMA_DATA);		// enable IRQ from data DMA on NULL
	SetHandler(IRQ_DMA_1, DispHstxDviIrqHandler);	// set DMA IRQ handler
	NVIC_IRQEnable(IRQ_DMA_1);			// enable IRQ
	NVIC_IRQPrio(IRQ_DMA_1, IRQ_PRIO_REALTIME);	// set highest IRQ priority

	// flag - HSTX DVI mode is active
	DispHstxDviRunning = True;

	// start first DMA transfer
	DispHstxDviIrqHandler();
}

// exchange videomode state descriptor, without terminating HSTX DVI mode
// - HSTX DVI mode must be started using DispHstxDviInit()
// - videomode state descriptor must be prepared using DispHstxDviPrepare()
void DispHstxDviExchange(sDispHstxVModeState* v)
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
		vactive = v->vtime.vactive;	// vertical active scanlines
		if (line < vactive)
		{
			// active image - use blank scanline
			bufnext = v->vsync_blank_cmd;
		}
		else if ((line >= vactive + v->vtime.vfront) && (line < v->vtime.vtotal - v->vtime.vback))
		{
			bufnext = v->vsync_on_cmd;
		}
		// front porch or back porch
		else
		{
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

// terminate HSTX DVI mode
void DispHstxDviTerm()
{
	// driver is not active
	if (!DispHstxDviRunning) return;

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

	// flag - HSTX DVI mode is not active
	DispHstxDviRunning = False;
}

void (* volatile DispHstxDviCore1Fnc)() = NULL; // core 1 remote function

#define DISPHSTX_DVI_REQNO		0	// request - no
#define DISPHSTX_DVI_REQINIT		1	// request - init
#define DISPHSTX_DVI_REQTERM		2	// request - terminate
volatile int DispHstxDviReq = DISPHSTX_DVI_REQNO; // current DVI request

// DVI core
void NOFLASH(DispHstxDviCore)()
{
	void (*fnc)();
	int req;

	// infinite loop
	while (True)
	{
		// data memory barrier
		dmb();

		// initialize/terminate VGA
		req = DispHstxDviReq;
		if (req != DISPHSTX_DVI_REQNO)
		{
			if (req == DISPHSTX_DVI_REQINIT)
			{
				DispHstxDviInit(); // initialize
			}
			else
			{
				DispHstxDviTerm(); // terminate
			}

			DispHstxDviReq = DISPHSTX_DVI_REQNO;
		}

		// execute remote function
		fnc = DispHstxDviCore1Fnc;
		if (fnc != NULL)
		{
			fnc();
			dmb();
			DispHstxDviCore1Fnc = NULL;
		}
	}
}

// execute core 1 remote function
void DispHstxDviCore1Exec(void (*fnc)())
{
	dmb();
	DispHstxDviCore1Fnc = fnc;
	dmb();
}

// check if core 1 is busy (executing remote function)
Bool DispHstxDviCore1Busy()
{
	dmb();
	return DispHstxDviCore1Fnc != NULL;
}

// wait if core 1 is busy (executing remote function)
void DispHstxDviCore1Wait()
{
	while (DispHstxDviCore1Busy()) {}
}

// start DVI on core 1 from core 0 (must be paired with DispHstxDviStop())
void DispHstxDviStart()
{
	dmb();
	if (DispHstxDviRunning) return;

	// run DVI core
	Core1Exec(DispHstxDviCore);

	// initialize DVI
	DispHstxDviReq = DISPHSTX_DVI_REQINIT;
	dmb();
	while (DispHstxDviReq != DISPHSTX_DVI_REQNO) { dmb(); }
}

// terminate DVI on core 1 from core 0 (must be paired with DispHstxDviStart())
void DispHstxDviStop()
{
	dmb();
	if (!DispHstxDviRunning) return;

	// terminate DVI
	DispHstxDviReq = DISPHSTX_DVI_REQTERM;
	dmb();
	while (DispHstxDviReq != DISPHSTX_DVI_REQNO) { dmb(); }

	// core 1 reset
	Core1Reset();
}

#endif // DISPHSTX_USE_DVI		// 1=use DVI (HDMI) support (0=or only VGA)

#endif // USE_DISPHSTX
