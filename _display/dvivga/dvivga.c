
// ****************************************************************************
//
//                              DVI (HDMI) + VGA
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

#include "../../global.h"	// globals

#if USE_DVIVGA					// use DVI (HDMI) + VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

#include "dvivga.h"
#include "../../_sdk/inc/sdk_dma.h"
#include "../../_sdk/inc/sdk_pwm.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_pio.h"
#include "../../_sdk/inc/sdk_irq.h"
#include "../../_sdk/inc/sdk_interp.h"
#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_multicore.h"
#include "dvivga.pio.h"

#define DVI_PIO_OFF	0	// offset of DVI program in PIO memory (must be 0!)
#define DVI_LANES	3	// number of TMDS lanes (0: blue, 1: green, 2: red, 3: clock)
#define DVI_SYNC_LANE	0	// synchronization lane (= blue)
#define DVI_INTERP	0	// interpolator used to encode pixels

#define VGA_PIO_OFF	2	// offset of VGA program in PIO memory (2 instructions are reserve for HDMI program)

// PIO command (jmp=program address, num=loop counter)
#define VGACMD(jmp, num) ( ((u32)((jmp)+VGA_PIO_OFF)<<27) | (u32)(num) )

#define VGA_GPIO_VSYNC	(VGA_GPIO_HSYNC+1) // VGA VSYNC GPIO (VGA requires VSYNC = HSYNC+1)

#define DVICLK_SLICE	PWM_GPIOTOSLICE(DVI_GPIO_CLK) // PWM slice index

#define DBUF_SIZE	(DVI_HACT/2*4)	// size of one line buffer in bytes (640/2=320 pixels, 1 pixels sent as 2 pixels in one u32 word)

#define DVI_SM1		(DVI_SM0+1) 	// DVI state machine for lane 1
#define DVI_SM2		(DVI_SM0+2) 	// DVI state machine for lane 2

#define DVI_SM(lane)	(DVI_SM0+(lane)) // DVI state machine for specified lane

#define DVI_DMA_CB0	(DVI_DMA+0)	// DVI control DMA channel for lane 0
#define DVI_DMA_DB0	(DVI_DMA+1)	// DVI data DMA channel for lane 0
#define DVI_DMA_CB1	(DVI_DMA+2)	// DVI control DMA channel for lane 1
#define DVI_DMA_DB1	(DVI_DMA+3)	// DVI data DMA channel for lane 1
#define DVI_DMA_CB2	(DVI_DMA+4)	// DVI control DMA channel for lane 2
#define DVI_DMA_DB2	(DVI_DMA+5)	// DVI data DMA channel for lane 2

#define VGA_DMA_CB	(VGA_DMA+0)	// VGA DMA channel - control block of base layer
#define VGA_DMA_PIO	(VGA_DMA+1)	// VGA DMA channel - copy data of base layer to PIO (raises IRQ0 on quiet)

#define DVI_DMA_CB(lane) (DVI_DMA+(lane)*2)	// DVI control DMA channel for specified lane
#define DVI_DMA_DB(lane) (DVI_DMA+(lane)*2+1)	// DVI data DMA channel for specified lane

// frame buffer
#if USE_FRAMEBUF	// use default display frame buffer
ALIGNED FRAMETYPE FrameBuf[FRAMESIZE];
#endif

// back buffer
#if USE_FRAMEBUF && (BACKBUFSIZE > 0)	// size of back buffer strip
ALIGNED FRAMETYPE BackBuf[BACKBUFSIZE]; // back buffer strip
#endif

// display setup
#if USE_DVIVGA == 2 // use full back buffer
FRAMETYPE* pDrawBuf = BackBuf;	// current draw buffer
int DispStripInx = 0;		// current index of back buffer strip (-1 = use full FrameBuf)
#else
FRAMETYPE* pDrawBuf = FrameBuf;	// current draw buffer
int DispStripInx = -1;		// current index of back buffer strip (-1 = use full FrameBuf)
#endif

int DispMinY = 0;		// minimal Y; base of back buffer strip
int DispMaxY = HEIGHT;		// maximal Y + 1; end of back buffer strip

// dirty window to update (used only with full back buffer, USE_MINIVGA = 2)
int DispDirtyX1 = 0;
int DispDirtyX2 = WIDTH;
int DispDirtyY1 = 0;
int DispDirtyY2 = HEIGHT;

// TMDS control symbols (every symbol if twice, 2 x 10 bits in one 32-bit word)
//   bit 0: HSYNC, bit 1: VSYNC ... sent to sync lane 0 (= blue)
u32 DviCtrlSyms[5] = // don't use "const" to keep the table in faster RAM
{
	// negative polarity
	0xaaeab,	// 0: (0x2AB << 10) | 0x2AB, no sync
	0x55154,	// 1: (0x154 << 10) | 0x154, HSYNC
	0x2acab,	// 2: (0x0AB << 10) | 0x0AB, VSYNC
	0xd5354,	// 3: (0x354 << 10) | 0x354, HSYNC + VSYNC

	0x7fd00,	// 4: (0x1ff << 10) | 0x100, dark line
};

// TMDS data table
u32 TmdsTable[64] = // don't use "const" to keep the table in faster RAM
{
#include "dvivga_tab.h"
};

// data
volatile int DviScanLine;	// current scan line 1...
volatile u32 DviFrame;		// frame counter
volatile int DviBufInx;		// current data buffer index (0 or 1)
//volatile Bool DviVSync;		// current scan line is vsync or dark

#if DVI_IRQTIME				// debug flag - measure delta time of DVI service
volatile u32 DviTimeIn;			// time in interrupt service, in [us]
volatile u32 DviTimeOut;		// time out interrupt service, in [us]
volatile u32 DviTimeIn2;		// time in interrupt service, in [us]
volatile u32 DviTimeOut2;		// time out interrupt service, in [us]
u32 DviTimeTmp;
#endif // DVI_IRQTIME

// data buffers to decode graphics lines (= 640/2*4*3*2 = 7680 bytes)
u32 DviLineBuf[DBUF_SIZE/4*DVI_LANES*2]; // even and odd line buffer for 3 lanes

// DVI control buffers with DMA command list
// - lane 0 control buffers contain 4 DMA command segments: front porch, HSYNC, back porch + IRQ, data
// - lane 1+2 control buffers contain 2 DMA command segments: front+hsync+back porch, data
// - every DMA command segment requires 4 u32 entries (4 registers: read, write, count and control)
// - The IRQ interrupt occurs from the data DMA channel at the end of the back porch, during data
//   segment activation. This provides a sufficient time reserve for possible IRQ service delays.
u32 DviLineBufSync[4*4];	// lane 0 vertical sync (front+VSYNC, HSYNC+VSYNC, back+VSYNC+IRQ, dark+VSYNC)
u32 DviLineBufDark0[4*4];	// lane 0 dark line (front, HSYNC, back+IRQ, dark)
u32 DviLineBufDark12[2*4 * 2];	// lane 1+2 dark line (front+hsync+back, dark)
u32 DviLineBufImg0[4*4 * 2];	// lane 0 image line, 2 buffers (even and odd line; front, HSYNC, back+IRQ, image)
u32 DviLineBufImg12[2*4 * 4];	// lane 1+2 image lines, 2 buffers (even and odd line; front+hsync+back, image)

// next DVI control buffer
u32*	DviCtrlBufNext[DVI_LANES];

// VGA data buffers (don't use "const" to keep the table in faster RAM)
u32 VgaVSyncDB[4] = {	// V-sync data buffer
	VGACMD(minivga_offset_vsync, VGA_HFRONT-3-3),	// front porch + V-sync
	VGACMD(minivga_offset_vhsync, VGA_HSYNC-3),	// H-sync + V-sync
	VGACMD(minivga_offset_vsync, VGA_HBACK-3),	// back porch + V-sync
	VGACMD(minivga_offset_voutput, WIDTH-1),	// dark + V-sync
};

u32 VgaNoSyncDB[4] = {	// no V-sync data buffer
	VGACMD(minivga_offset_dark, VGA_HFRONT-4-3),	// front porch
	VGACMD(minivga_offset_hsync, VGA_HSYNC-3),	// H-sync
	VGACMD(minivga_offset_dark, VGA_HBACK-4),	// back porch
	VGACMD(minivga_offset_output, WIDTH-1),		// dark or image
};

u32 VgaDarkDB = 0; // dark pixels

// VGA control buffers
u32 VgaVSyncCB[2*4];	// vertical sync
u32 VgaDarkCB[2*4];	// dark
u32 VgaImgCB[2*4];	// image

u32* VgaCBNext;		// next control buffer

// TMDS pins
const u8 DviPins[3] = {
	DVI_GPIO_D0,
	DVI_GPIO_D1,
	DVI_GPIO_D2,
};

// VGA pins output strength
const u8 VgaDrivemA[VGA_GPIO_NUM] = {
	GPIO_DRIVE_2MA,		// B0
	GPIO_DRIVE_4MA,		// B1
	GPIO_DRIVE_4MA,		// B2
	GPIO_DRIVE_8MA,		// B3
	GPIO_DRIVE_12MA,	// B4

	GPIO_DRIVE_2MA,		// G0
	GPIO_DRIVE_2MA,		// G1
	GPIO_DRIVE_4MA,		// G2
	GPIO_DRIVE_4MA,		// G3
	GPIO_DRIVE_8MA,		// G4
	GPIO_DRIVE_12MA,	// G5

	GPIO_DRIVE_2MA,		// R0
	GPIO_DRIVE_4MA,		// R1
	GPIO_DRIVE_4MA,		// R2
	GPIO_DRIVE_8MA,		// R3
	GPIO_DRIVE_12MA,	// R4
};

// encode data
void NOFLASH(DviEncode)(int line, int bufinx)
{
#if !DVIVGA_INTERPNOSAVE	// 1=not saving interpolator during interrupt (more optimisation)
	// save interpolators
	interp_hw_save_t save0, save1;
	InterpSave(0, &save0);
	InterpSave(1, &save1);
#endif // !DVIVGA_INTERPNOSAVE

	// reset interpolators
	InterpReset(0);
	InterpReset(1);

// ==== prepare RED channel (interpolator 0)

	// prepare channel position - red
#define CHANNEL_LSB	11	// red channel least significant bit
#define CHANNEL_MSB	15	// red channel most significant bit
#define PIXEL_WIDTH	16	// pixel width in bits
#define INDEX_SHIFT	2	// convert index to offset in LUT table
#define PIXEL_LSB	0	// least significant bit of the pixel
#define LUT_INDEX_WIDTH	6	// number of bits per index in LUT table (= 64 entries)
#define INDEX_MSB	(INDEX_SHIFT + LUT_INDEX_WIDTH - 1) // most significant bit of the index

	// shift channel to offset in the LUT table, red
	//  PIXEL_LSB ... >> bit offset of the pixel in input u32, this will normalize pixel to base bit position
	//  channel_msb ... >> this will shift last bit of the channel to bit position 0
	//  +1 ... >> this will shift whole channel under bit position 0
	//  -LUT_INDEX_WIDTH ... << this will shift channel that only usable bits will be visible = index in LUT table
	//  -INDEX_SHIFT ... << this will convert pixel index to the offset in the LUT table
	//  +channel_preshift ... << this will pre-shift blue channel 3 bits left to correct negatibe position
#define SHIFT_CHANNEL_TO_INDEX (PIXEL_LSB + CHANNEL_MSB + 1 - LUT_INDEX_WIDTH - INDEX_SHIFT)

	// red: 0 + 15 + 1 - 6 - 2 = 8 (delete 5+6 bits without 2 bits)

	// setup lane 0 for 1st pixel, red
	InterpShift(0, 0, SHIFT_CHANNEL_TO_INDEX); // set shift for 1st pixel
	InterpMask(0, 0, INDEX_MSB - (CHANNEL_MSB - CHANNEL_LSB), INDEX_MSB); // mask least and most significant bit

	// setup lane 1 for 2nd pixel, red
	InterpShift(0, 1, PIXEL_WIDTH + SHIFT_CHANNEL_TO_INDEX); // set shift for 2nd pixel
	InterpMask(0, 1, INDEX_MSB - (CHANNEL_MSB - CHANNEL_LSB), INDEX_MSB); // mask least and most significant bit
	InterpCrossInput(0, 1, True); // feed lane's 0 accumulator into this lane's shift+mask input

	// setup base
	InterpBase(0, 0, (u32)TmdsTable); // set LUT table as base of lane 0
	InterpBase(0, 1, (u32)TmdsTable); // set LUT table as base of lane 1

#undef CHANNEL_LSB
#undef CHANNEL_MSB
#undef SHIFT_CHANNEL_TO_INDEX

// ==== prepare GREEN channel (interpolator 1)

	// prepare channel position - green
#define CHANNEL_LSB	5	// green channel least significant bit
#define CHANNEL_MSB	10	// green channel most significant bit

	// shift channel to offset in the LUT table, green
	//  PIXEL_LSB ... >> bit offset of the pixel in input u32, this will normalize pixel to base bit position
	//  channel_msb ... >> this will shift last bit of the channel to bit position 0
	//  +1 ... >> this will shift whole channel under bit position 0
	//  -LUT_INDEX_WIDTH ... << this will shift channel that only usable bits will be visible = index in LUT table
	//  -INDEX_SHIFT ... << this will convert pixel index to the offset in the LUT table
	//  +channel_preshift ... << this will pre-shift blue channel 3 bits left to correct negatibe position
#define SHIFT_CHANNEL_TO_INDEX (PIXEL_LSB + CHANNEL_MSB + 1 - LUT_INDEX_WIDTH - INDEX_SHIFT)

	// green: 0 + 10 + 1 - 6 - 2 = 3 (delete 5 bits without 2 bits)

	// setup lane 0 for 1st pixel, green
	InterpShift(1, 0, SHIFT_CHANNEL_TO_INDEX); // set shift for 1st pixel
	InterpMask(1, 0, INDEX_MSB - (CHANNEL_MSB - CHANNEL_LSB), INDEX_MSB); // mask least and most significant bit

	// setup lane 1 for 2nd pixel, green
	InterpShift(1, 1, PIXEL_WIDTH + SHIFT_CHANNEL_TO_INDEX); // set shift for 2nd pixel
	InterpMask(1, 1, INDEX_MSB - (CHANNEL_MSB - CHANNEL_LSB), INDEX_MSB); // mask least and most significant bit
	InterpCrossInput(1, 1, True); // feed lane's 0 accumulator into this lane's shift+mask input

	// setup base
	InterpBase(1, 0, (u32)TmdsTable); // set LUT table as base of lane 0
	InterpBase(1, 1, (u32)TmdsTable); // set LUT table as base of lane 1

// ==== convert RED and GREEN channels

	// pointer to source data line
	u16* data = &FrameBuf[line*WIDTHLEN];

	// destination buffers
	u32* dstR = &DviLineBuf[(2*2+bufinx)*DBUF_SIZE/4]; // pointer to destination buffer - red component
	u32* dstG = &DviLineBuf[(1*2+bufinx)*DBUF_SIZE/4]; // pointer to destination buffer - green component

	// decode scanline data - Red and Green components
	//  inbuf ... input buffer (u16), must be u32 aligned
	//  outbufR ... output buffer, red component (u32)
	//  outbufG ... output buffer, green component (u32)
	//  count ... number of pixels (must be multiply of 8)
	DviVgaEncRG(data, dstR, dstG, WIDTH);

// ==== convert BLUE channel

	// destination buffer
	u32* dst = &DviLineBuf[(0*2+bufinx)*DBUF_SIZE/4]; // pointer to destination buffer - blue component

	// encode data of lane 0 (blue channel)
	DviVgaEncB(data, dst, WIDTH);

#if !DVIVGA_INTERPNOSAVE	// 1=not saving interpolator during interrupt (more optimisation)
	// restore interpolators
	InterpLoad(1, &save1);
	InterpLoad(0, &save0);
#endif // !DVIVGA_INTERPNOSAVE
}

// DVI DMA handler - called on end of every scanline
// - The IRQ interrupt occurs from the data DMA channel at the end of the back porch, during data
//   segment activation. This provides a sufficient time reserve for possible IRQ service delays.
void NOFLASH(DviLine)()
{
#if DVI_IRQTIME				// debug flag - measure delta time of DVI service
	u32 t1 = Time();		// start time
#endif // DVI_IRQTIME

	// Clear the interrupt request for DMA control channel
	DMA_IRQ1Clear(DVI_DMA_DB0);

	// wait to fill data DMA by data segment
	int lane;
	for (lane = 0; lane < DVI_LANES; lane++)
	{
		// "NEXT count" of data DMA must be equal to "WIDTH" on last command segment
		while (DMA_Next(DVI_DMA_DB(lane)) != DVI_HACT/2) {}

		// set next control buffer
		DMA_SetRead(DVI_DMA_CB(lane), DviCtrlBufNext[lane]);
	}

	// set next VGA control buffer
	while (DMA_Next(VGA_DMA_PIO) != WIDTH/2) {}
	DMA_SetRead(VGA_DMA_CB, VgaCBNext);

	// increment scanline (1..)
	int line = DviScanLine; // current scanline
	line++; 		// new current scanline
	if (line >= DVI_VTOTAL) // last scanline?
	{
		DviFrame++;	// increment frame counter
		line = 0; 	// restart scanline
	}
	DviScanLine = line;	// store new scanline

	// VSYNC line
	line -= DVI_VSYNC;
	if (line < 0)
	{
		DviCtrlBufNext[0] = DviLineBufSync; // lane 0 vertical sync
		DviCtrlBufNext[1] = DviLineBufDark12; // lane 1 dark line
		DviCtrlBufNext[2] = &DviLineBufDark12[2*4]; // lane 2 dark line
		VgaCBNext = VgaVSyncCB;
	}
	else
	{
		// front porch and back porch (dark line)
		line -= DVI_VBACK;
		if ((line < 0) || (line >= DVI_VACT))
		{
			DviCtrlBufNext[0] = DviLineBufDark0; // lane 0 dark line
			DviCtrlBufNext[1] = DviLineBufDark12; // lane 1 dark line
			DviCtrlBufNext[2] = &DviLineBufDark12[2*4]; // lane 2 dark line
			VgaCBNext = VgaDarkCB;
		}

		// image scanlines
		else
		{
			// current buffer index
			int bufinx = DviBufInx;

			// lines are duplicated, so only even lines need to be encoded
			if ((line & 1) == 0)
			{
				// encode data
				DviEncode(line/2, bufinx);
			}
			else
			{
				// switch current data buffer index (bufinx = current preparing buffer, DviBufInx = current running buffer)
				DviBufInx = bufinx ^ 1;
			}

			// set next control buffer
			DviCtrlBufNext[0] = &DviLineBufImg0[4*4 * bufinx]; // lane 0 image line
			DviCtrlBufNext[1] = &DviLineBufImg12[2*4 * bufinx]; // lane 1 image line
			DviCtrlBufNext[2] = &DviLineBufImg12[2*4 * (bufinx+2)]; // lane 2 image line
			VgaCBNext = VgaImgCB;
			VgaImgCB[5] = (u32)&FrameBuf[line/2*WIDTH];
		}
	}

#if DVI_IRQTIME					// debug flag - measure delta time of DVI service
	if (line == 100)
	{
		u32 t2 = Time();		// stop time
		DviTimeIn = t2 - t1;		// time in interrupt service
		DviTimeTmp = t2;
	}

	if (line == 101)
	{
		DviTimeOut = t1 - DviTimeTmp;	// time out interrupt service

		u32 t2 = Time();		// stop time
		DviTimeIn2 = t2 - t1;		// time in interrupt service
		DviTimeTmp = t2;
	}

	if (line == 102)
	{
		DviTimeOut2 = t1 - DviTimeTmp;	// time out interrupt service
	}
#endif // DVI_IRQTIME
}

// configure one output pin
void DviPinInit(u8 pin)
{
	GPIO_Drive2mA(pin);	// 2mA drive (options: 2/4/8/12 mA)
	GPIO_Slow(pin);		// use slow slew rate control (options: Slow/Fast)
	GPIO_InDisable(pin);	// input disable
	GPIO_NoPull(pin);	// no pulls
}

// initialize PIO of serializer
void DviPioInit()
{
	int i;
	u8 pin, sm;

	// initialize PIO
	PioInit(DVIVGA_PIO);

// ==== VGA PIO initialize (must be done before DVI, some pins can be overriden by the DVI)

	// load VGA PIO program
#if VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioLoadProg(DVIVGA_PIO, minivga_csync_program_instructions, count_of(minivga_csync_program_instructions), VGA_PIO_OFF);
#else
	PioLoadProg(DVIVGA_PIO, minivga_program_instructions, count_of(minivga_program_instructions), VGA_PIO_OFF);
#endif
	// PIO set wrap address
	PioSetWrap(DVIVGA_PIO, VGA_SM, VGA_PIO_OFF + minivga_wrap_target, VGA_PIO_OFF + minivga_wrap);

	// set start address
	PioSetAddr(DVIVGA_PIO, VGA_SM, VGA_PIO_OFF + minivga_offset_entry);

	// configure GPIOs for use by PIO
	PioSetupGPIO(DVIVGA_PIO, VGA_GPIO_FIRST, VGA_GPIO_NUM);

#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioSetupGPIO(DVIVGA_PIO, VGA_GPIO_HSYNC, 2);	// use HSYNC + VSYNC
#else
	PioSetupGPIO(DVIVGA_PIO, VGA_GPIO_HSYNC, 1);	// use only CSYNC
#endif
	// set pin direction to output
	PioSetPinDir(DVIVGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_NUM, 1);

	// setup current output
	for (i = 0; i < VGA_GPIO_NUM; i++) GPIO_Drive(i+VGA_GPIO_FIRST, VgaDrivemA[i]);

#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioSetPinDir(DVIVGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 2, 1);	// use HSYNC + VSYNC
#else
	PioSetPinDir(DVIVGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 1, 1);	// use only CSYNC
#endif
	// negate HSYNC and VSYNC output
	GPIO_OutOverInvert(VGA_GPIO_HSYNC);
#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	GPIO_OutOverInvert(VGA_GPIO_VSYNC);		// use HSYNC + VSYNC
#endif
	// map state machine's OUT and MOV pins	
	PioSetupOut(DVIVGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_NUM);

	// set sideset pins (HSYNC and VSYNC, 2 bits, no optional, no pindirs)
#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioSetupSideset(DVIVGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 2, False, False);	// use HSYNC + VSYNC
#else
	PioSetupSideset(DVIVGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 1, False, False);	// use only CSYNC
#endif
	// join FIFO to send only
	PioSetFifoJoin(DVIVGA_PIO, VGA_SM, PIO_FIFO_JOIN_TX);

	// PIO clock divider
	PioSetClkdiv(DVIVGA_PIO, VGA_SM, VGA_CLKDIV*256);

	// shift right, autopull, pull threshold
	PioSetOutShift(DVIVGA_PIO, VGA_SM, True, True, 32);

// ==== DVI PIO initialize

	// load DVI PIO program
#if DVI_GPIO_INV
	PioLoadProg(DVIVGA_PIO, dvi_inv_program_instructions, count_of(dvi_inv_program_instructions), DVI_PIO_OFF);
#else
	PioLoadProg(DVIVGA_PIO, dvi_program_instructions, count_of(dvi_program_instructions), DVI_PIO_OFF);
#endif

	// initialize all lanes
	for (i = 0; i < DVI_LANES; i++)
	{
		sm = DVI_SM(i);

		// PIO set wrap address
		PioSetWrap(DVIVGA_PIO, sm, DVI_PIO_OFF + dvi_wrap_target, DVI_PIO_OFF + dvi_wrap);

		// set start address
		PioSetAddr(DVIVGA_PIO, sm, DVI_PIO_OFF);

		// initialize GPIOs
		pin = DviPins[i];
		PioSetupGPIO(DVIVGA_PIO, pin, 2); // setup pins for use by PIO
		DviPinInit(pin);		// setup first pin
		PioSetPin(DVIVGA_PIO, sm, pin, 1, 0); // set pin1 output value to 0
		DviPinInit(pin+1);		// setup second pin
		PioSetPin(DVIVGA_PIO, sm, pin+1, 1, 1); // set pin2 output value to 1 
		PioSetPinDir(DVIVGA_PIO, sm, pin, 2, 1); // set pin direction to output

		// set sideset pins (2 bits, no optional, no pindirs)
		PioSetupSideset(DVIVGA_PIO, sm, pin, 2, False, False);

		// join FIFO to send only
		PioSetFifoJoin(DVIVGA_PIO, sm, PIO_FIFO_JOIN_TX);

		// set PIO clock divider to 1.00
		PioSetClkdiv(DVIVGA_PIO, sm, 1*256);

		// shift right, autopull, pull threshold = 20 bits (2 symbols per 32-bit word)
		PioSetOutShift(DVIVGA_PIO, sm, True, True, 2*10);
	}
}

// initialize PWM of serializer
void DviPwmInit()
{
STATIC_ASSERT((DVI_GPIO_CLK & 1) == 0, "DVI_GPIO_CLK must be even!");

	// use PWM slice to drive pixel clock
	PWM_InvEnable(DVICLK_SLICE, 0); // invert channel A
	PWM_InvDisable(DVICLK_SLICE, 1); // non-invert channel B
	PWM_GpioInit(DVI_GPIO_CLK);	// set PWM function of first pin
	PWM_GpioInit(DVI_GPIO_CLK+1);	// set PWM function of second pin
	DviPinInit(DVI_GPIO_CLK);	// setup first pin
	DviPinInit(DVI_GPIO_CLK+1);	// setup second pin

	PWM_Top(DVICLK_SLICE, 9);	// set wrap value to 9 (period = 10)
	PWM_ClkDiv(DVICLK_SLICE, 1*16);	// set clock divider to 1.00
	PWM_Comp(DVICLK_SLICE, 0, 5);	// set compare value of channel A to 5
	PWM_Comp(DVICLK_SLICE, 1, 5);	// set compare value of channel B to 5
}

// prepare one VGA command segment of the control buffer (returns new pointer to control buffer)
//  cb ... pointer to control buffer (fills 4 entries)
//  read ... read address
//  count ... number of transfers
//  ring ... order of read ring size in bytes, 0=no ring, 2=4 bytes
u32* VgaSetCb(u32* cb, const void* read, int count, int ring)
{
	*cb++ = 			// control word
		// DMA_CTRL_SNIFF |	// not sniff
		// DMA_CTRL_BSWAP |	// byte swap
		DMA_CTRL_QUIET |	// quiet if not IRQ
		DMA_CTRL_TREQ(PioGetDreq(DVIVGA_PIO, VGA_SM, True)) | // data request from PIO
		DMA_CTRL_CHAIN(VGA_DMA_CB) | // chain to control DMA channel
		// DMA_CTRL_RING_WRITE | // not wrap ring on write
		DMA_CTRL_RING_SIZE(ring) | // order of ring size in bytes
		// DMA_CTRL_INC_WRITE |	// not increment write
		DMA_CTRL_INC_READ |	// increment read
		DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
		//DMA_CTRL_HIGH_PRIORITY | // high priority
		DMA_CTRL_EN;		// enable DMA
	*cb++ = (u32)read;		// read address from data buffer
	*cb++ = (u32)PIO_TXF(DVIVGA_PIO, VGA_SM); // write address - transfer FIFO of the PIO state machine
	*cb++ = count;			// number of transfers

	return cb;
}

// prepare one DVI command segment of the control buffer (returns new pointer to control buffer)
//  cb ... pointer to control buffer (fills 4 entries)
//  lane ... lane index 0..2
//  read ... read address
//  count ... number of transfers
//  ring ... order of read ring size in bytes, 0=no ring, 2=4 bytes
//  irq ... raise IRQ after finishing this command segment (use after back porch)

// - using alias 1 of DMA channel registers (four u32 registers = 16 bytes)
//   We will test the COUNT_NEXT register to see if the DMA setup was complete,
//   so we need the COUNT to be the last setup.
//	#define DMA_CH_AL1_CTRL		4	// DMA channel control and status
//	#define DMA_CH_AL1_READ		5	// DMA channel read address
//	#define DMA_CH_AL1_WRITE	6	// DMA channel write address
//	#define DMA_CH_AL1_COUNT_TRIG	7	// DMA channel transfer count + trigger (auto reloaded on trigger)

u32* DviSetCb(u32* cb, int lane, const void* read, int count, int ring, Bool irq)
{
	*cb++ = 			// control word
		// DMA_CTRL_SNIFF |	// not sniff
		// DMA_CTRL_BSWAP |	// byte swap
		(irq ? 0 : DMA_CTRL_QUIET) |	// quiet if not IRQ
		DMA_CTRL_TREQ(PioGetDreq(DVIVGA_PIO, DVI_SM(lane), True)) | // data request from PIO
		DMA_CTRL_CHAIN(DVI_DMA_CB(lane)) | // chain to control DMA channel
		// DMA_CTRL_RING_WRITE | // not wrap ring on write
		DMA_CTRL_RING_SIZE(ring) | // order of ring size in bytes
		// DMA_CTRL_INC_WRITE |	// not increment write
		DMA_CTRL_INC_READ |	// increment read
		DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
		//DMA_CTRL_HIGH_PRIORITY | // high priority
		DMA_CTRL_EN;		// enable DMA

	*cb++ = (u32)read;		// read address from data buffer
	*cb++ = (u32)PIO_TXF(DVIVGA_PIO, DVI_SM(lane)); // write address - transfer FIFO of the PIO state machine
	*cb++ = count;			// number of transfers

	return cb;
}

// initialize control buffers
void DviBufInit()
{
// command segments: 1) front porch, 2) HSYNC, 3) back porch+IRQ, 4) data

// ==== DVI buffers initialize

	u32 *cb, *db;
	int i;

	// DVI lane 0 vertical sync
	cb = DviSetCb(DviLineBufSync, 0, &DviCtrlSyms[2], DVI_HFRONT/2, 2, False); // front porch + VSYNC
	cb = DviSetCb(cb, 0, &DviCtrlSyms[3], DVI_HSYNC/2, 2, False); // HSYNC + VSYNC
	cb = DviSetCb(cb, 0, &DviCtrlSyms[2], DVI_HBACK/2, 2, True); // back porch + VSYNC + IRQ
	DviSetCb(cb, 0, &DviCtrlSyms[2], DVI_HACT/2, 2, False); // dark + VSYNC

	// DVI lane 0 dark line
	cb = DviSetCb(DviLineBufDark0, 0, &DviCtrlSyms[0], DVI_HFRONT/2, 2, False); // front porch
	cb = DviSetCb(cb, 0, &DviCtrlSyms[1], DVI_HSYNC/2, 2, False); // HSYNC
	cb = DviSetCb(cb, 0, &DviCtrlSyms[0], DVI_HBACK/2, 2, True); // back porch + IRQ
	DviSetCb(cb, 0, &DviCtrlSyms[0], DVI_HACT/2, 2, False); // dark

	// DVI lane 1 dark line
	cb = DviSetCb(DviLineBufDark12, 1, &DviCtrlSyms[3], (DVI_HFRONT+DVI_HSYNC+DVI_HBACK)/2, 2, False); // front+hsync+back porch
	cb = DviSetCb(cb, 1, &DviCtrlSyms[3], DVI_HACT/2, 2, False); // dark

	// DVI lane 2 dark line
	cb = DviSetCb(cb, 2, &DviCtrlSyms[3], (DVI_HFRONT+DVI_HSYNC+DVI_HBACK)/2, 2, False); // front+hsync+back porch
	DviSetCb(cb, 2, &DviCtrlSyms[3], DVI_HACT/2, 2, False); // dark

	// DVI lane 0 image lines
	cb = DviLineBufImg0;
	db = DviLineBuf;
	for (i = 2; i > 0; i--)
	{
		cb = DviSetCb(cb, 0, &DviCtrlSyms[0], DVI_HFRONT/2, 2, False); // front porch
		cb = DviSetCb(cb, 0, &DviCtrlSyms[1], DVI_HSYNC/2, 2, False); // HSYNC
		cb = DviSetCb(cb, 0, &DviCtrlSyms[0], DVI_HBACK/2, 2, True); // back porch + IRQ
		cb = DviSetCb(cb, 0, db, DVI_HACT/2, 0, False); // image
		db += DBUF_SIZE/4;
	}

	// DVI lane 1+2 image lines
	cb = DviLineBufImg12;
	for (i = 0; i < 4; i++)
	{
		cb = DviSetCb(cb, i/2+1, &DviCtrlSyms[3], (DVI_HFRONT+DVI_HSYNC+DVI_HBACK)/2, 2, False); // front+hsync+back porch
		cb = DviSetCb(cb, i/2+1, db, DVI_HACT/2, 0, False); // image
		db += DBUF_SIZE/4;
	}

// ==== VGA buffers initialize

	// VGA vertical sync
	cb = VgaSetCb(VgaVSyncCB, VgaVSyncDB, count_of(VgaVSyncDB), 0);
	cb = VgaSetCb(cb, &VgaDarkDB, WIDTH/2, 2);

	// VGA dark
	cb = VgaSetCb(VgaDarkCB, VgaNoSyncDB, count_of(VgaNoSyncDB), 0);
	cb = VgaSetCb(cb, &VgaDarkDB, WIDTH/2, 2);

	// VGA image
	cb = VgaSetCb(VgaImgCB, VgaNoSyncDB, count_of(VgaNoSyncDB), 0);
	cb = VgaSetCb(cb, FrameBuf, WIDTH/2, 0);
}

// DVI initialize DMA
// initialize VGA DMA
//   control blocks aliases:
//                  +0x0        +0x4          +0x8          +0xC (Trigger)
// 0x00 (alias 0):  READ_ADDR   WRITE_ADDR    TRANS_COUNT   CTRL_TRIG ... we use this!
// 0x10 (alias 1):  CTRL        READ_ADDR     WRITE_ADDR    TRANS_COUNT_TRIG
// 0x20 (alias 2):  CTRL        TRANS_COUNT   READ_ADDR     WRITE_ADDR_TRIG
// 0x30 (alias 3):  CTRL        WRITE_ADDR    TRANS_COUNT   READ_ADDR_TRIG
void DviDmaInit()
{
	// initialize VGA DMA control channel
	DMA_Config(VGA_DMA_CB,		// channel
		VgaVSyncCB,		// read address
		&DMA_CHAN(VGA_DMA_PIO)[DMA_CH_AL1_CTRL], // write address to CTRL register of alias 1
		4,			// number of transfers = 4 * u32 (= 1 command segment)
			// DMA_CTRL_SNIFF |	// not sniff
			// DMA_CTRL_BSWAP |	// not byte swap
			// DMA_CTRL_QUIET |	// not quiet
			DMA_CTRL_TREQ_FORCE |	// permanent request
			DMA_CTRL_CHAIN(VGA_DMA_CB) | // disable chaining
			DMA_CTRL_RING_WRITE |	// wrap ring on write
			DMA_CTRL_RING_SIZE(4) | // ring size = 16 bytes
			DMA_CTRL_INC_WRITE |	// increment write
			DMA_CTRL_INC_READ |	// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY | // not high priority
			DMA_CTRL_EN);		// enable DMA

	// initialize DVI DMA control channels (they will start DMA data channels)
	int lane;
	for (lane = 0; lane < DVI_LANES; lane++)
	{
		// prepare DMA control channel
		DMA_Config(DVI_DMA_CB(lane),		// channel
			(lane == 0) ? DviLineBufSync : ((lane == 1) ? DviLineBufDark12 : &DviLineBufDark12[2*4]), // read address
			&DMA_CHAN(DVI_DMA_DB(lane))[DMA_CH_AL1_CTRL], // write address to CTRL register of alias 1
			4,			// number of transfers = 4 * u32 (= 1 command segment)
				// DMA_CTRL_SNIFF |	// not sniff
				// DMA_CTRL_BSWAP |	// not byte swap
				// DMA_CTRL_QUIET |	// not quiet
				DMA_CTRL_TREQ_FORCE |	// permanent request
				DMA_CTRL_CHAIN(DVI_DMA_CB(lane)) | // disable chaining
				DMA_CTRL_RING_WRITE |	// wrap ring on write
				DMA_CTRL_RING_SIZE(4) | // ring size = 16 bytes
				DMA_CTRL_INC_WRITE |	// increment write
				DMA_CTRL_INC_READ |	// increment read
				DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
				// DMA_CTRL_HIGH_PRIORITY | // not high priority
				DMA_CTRL_EN);		// enable DMA
	}

	// enable DMA channel IRQ1
	DMA_IRQ1Enable(DVI_DMA_DB0);

	// set DMA IRQ handler
	SetHandler(IRQ_DMA_1, DviLine);

	// set highest IRQ priority
	NVIC_IRQPrio(IRQ_DMA_1, IRQ_PRIO_REALTIME);
}

// DVI output enable
void DviEnable()
{
	// interrupt disable
	IRQ_LOCK;

// clock and data do not have to be exactly
// synchronized, DVI allows some phase offset

	// enable clock PWM
	PWM_Enable(DVICLK_SLICE);

	// enable state machines
	PioSMEnableMaskSync(DVIVGA_PIO, B0+B1+B2+B3);

	// interrupt enable
	IRQ_UNLOCK;
}

// DVI output disable
void DviDisable()
{
	// disable state machines
	PioSMDisableMask(DVIVGA_PIO, B0+B1+B2+B3);

	// disable clock PWM
	PWM_Disable(DVICLK_SLICE);
}

// initialize DVI ... use DviStart() to start on core 1
// - system clock must be set to 252 MHz
void DviInit()
{
	// clear frame buffer
#if USE_FRAMEBUF		// use default display frame buffer
	memset(FrameBuf, 0, FRAMESIZE*sizeof(FRAMETYPE));
#endif

	// clear back buffer
#if USE_FRAMEBUF && (BACKBUFSIZE > 0)
	memset(BackBuf, 0, BACKBUFSIZE*sizeof(FRAMETYPE));
#endif

	// clear data buffer with gray color
	int i;
	u32* d = DviLineBuf;
	for (i = count_of(DviLineBuf); i > 0; i--) *d++ = 0x5fd80;

	// initialize parameters
	DviScanLine = 0; // currently processed scanline
	DviBufInx = 0; // at first, control buffer 1 will be sent out
	DviFrame = 0; // current frame

	// next control buffer
	DviCtrlBufNext[0] = DviLineBufSync;
	DviCtrlBufNext[1] = DviLineBufDark12;
	DviCtrlBufNext[2] = &DviLineBufDark12[2*4];
	VgaCBNext = VgaVSyncCB;

	// initialize PIO
	DviPioInit();

	// initialize PWM
	DviPwmInit();

	// initialize control buffers
	DviBufInit();

	// DVI initialize DMA
	DviDmaInit();

	// enable DMA IRQ
	NVIC_IRQEnable(IRQ_DMA_1);

	// start DMA
	DMA_Start(DVI_DMA_CB0);
	DMA_Start(DVI_DMA_CB1);
	DMA_Start(DVI_DMA_CB2);
	DMA_Start(VGA_DMA_CB);

#if DVI_IRQTIME				// debug flag - measure delta time of DVI service
	DviTimeTmp = Time();
#endif // DVI_IRQTIME

	// DVI output enable
	DviEnable();
}

// terminate DVI ... use DviStop() to stop on core 1
void DviTerm()
{
	// DVI output disable
	DviDisable();

	// abort DVI DMA channels (interrupt data channel first and then control channel)
	DMA_Abort(DVI_DMA_DB0); // pre-abort, could be chaining right now
	DMA_Abort(DVI_DMA_CB0);
	DMA_Abort(DVI_DMA_DB0);
	DMA_Abort(DVI_DMA_DB1);
	DMA_Abort(DVI_DMA_CB1);
	DMA_Abort(DVI_DMA_DB1);
	DMA_Abort(DVI_DMA_DB2);
	DMA_Abort(DVI_DMA_CB2);
	DMA_Abort(DVI_DMA_DB2);

	// abort VGA DMA channels (interrupt data channel first and then control channel)
	DMA_Abort(VGA_DMA_PIO); // pre-abort, could be chaining right now
	DMA_Abort(VGA_DMA_CB);
	DMA_Abort(VGA_DMA_PIO);

	// disable IRQ1 from DMA0
	NVIC_IRQDisable(IRQ_DMA_1);
	DMA_IRQ1Disable(DVI_DMA_DB0);

	// Clear the interrupt request for DMA control channel
	DMA_IRQ1Clear(DVI_DMA_DB0);

	// reset PIO
	PioInit(DVIVGA_PIO);
}

void (* volatile DviCore1Fnc)() = NULL; // core 1 remote function

#define DVI_REQNO 	0	// request - no
#define DVI_REQINIT     1	// request - init
#define DVI_REQTERM	2	// request - terminate
volatile int DviReq = DVI_REQNO;	// current DVI request

// DVI core
void NOFLASH(DviCore)()
{
	void (*fnc)();
	int req;

	// infinite loop
	while (True)
	{
		// data memory barrier
		dmb();

		// initialize/terminate VGA
		req = DviReq;
		if (req != DVI_REQNO)
		{
			if (req == DVI_REQINIT)
			{
				DviInit(); // initialize
			}
			else
			{
				DviTerm(); // terminate
			}

			DviReq = DVI_REQNO;
		}

		// execute remote function
		fnc = DviCore1Fnc;
		if (fnc != NULL)
		{
			fnc();
			dmb();
			DviCore1Fnc = NULL;
		}

	}
}

// execute core 1 remote function
void DviCore1Exec(void (*fnc)())
{
	dmb();
	DviCore1Fnc = fnc;
	dmb();
}

// check if core 1 is busy (executing remote function)
Bool DviCore1Busy()
{
	dmb();
	return DviCore1Fnc != NULL;
}

// wait if core 1 is busy (executing remote function)
void DviCore1Wait()
{
	while (DviCore1Busy()) {}
}

// check VSYNC
Bool DviIsVSync()
{
	return ((DviScanLine < DVI_VSYNC+DVI_VBACK) ||
		(DviScanLine >= DVI_VTOTAL-DVI_VFRONT));
}

// wait for VSync scanline
void DviWaitVSync()
{
	// wait for end of VSync
	while (DviIsVSync()) { dmb(); }

	// wait for start of VSync
	while (!DviIsVSync()) { dmb(); }
}

// start DVI on core 1 from core 0 (must be paired with DviStop())
// - system clock must be set to 252 MHz
void DviStart()
{
	// run DVI core
	Core1Exec(DviCore);

	// initialize DVI
	DviReq = DVI_REQINIT;
	dmb();
	while (DviReq != DVI_REQNO) { dmb(); }
}

// terminate DVI on core 1 from core 0 (must be paired with DviStart())
void DviStop()
{
	// terminate DVI
	DviReq = DVI_REQTERM;
	dmb();
	while (DviReq != DVI_REQNO) { dmb(); }

	// core 1 reset
	Core1Reset();
}

// use back buffer
#if BACKBUFSIZE > 0

// set strip of back buffer (-1 = use full FrameBuffer)
void DispSetStrip(int inx)
{
	if (inx < 0)
	{
		// no back buffer
		pDrawBuf = FrameBuf;	// current draw buffer
		DispStripInx = -1;	// current index of back buffer strip
		DispMinY = 0;		// minimal Y; base of back buffer strip
		DispMaxY = HEIGHT;	// maximal Y + 1; end of back buffer strip
	}
	else
	{
		// use back buffer
		pDrawBuf = BackBuf;	// current draw buffer
		if (inx >= DISP_STRIP_NUM) inx = 0;
		DispStripInx = inx;	// current index of back buffer strip
		DispMinY = inx * DISP_STRIP_HEIGHT;	// minimal Y; base of back buffer strip
		DispMaxY = DispMinY + DISP_STRIP_HEIGHT; // maximal Y + 1; end of back buffer strip
	}
}

// load back buffer from frame buffer
void DispLoad()
{
	if (DispStripInx >= 0) memcpy(BackBuf, &FrameBuf[DispMinY*WIDTHLEN], BACKBUFSIZE*sizeof(FRAMETYPE));
}

// use full back buffer
#if USE_DVIVGA == 2

// set dirty all frame buffer
void DispDirtyAll()
{
	DispDirtyX1 = 0;
	DispDirtyX2 = WIDTH;
	DispDirtyY1 = 0;
	DispDirtyY2 = HEIGHT;
}

// set dirty none (clear after update)
void DispDirtyNone()
{
	DispDirtyX1 = WIDTH;
	DispDirtyX2 = 0;
	DispDirtyY1 = HEIGHT;
	DispDirtyY2 = 0;
}

// update dirty area by rectangle (check valid range)
void DispDirtyRect(int x, int y, int w, int h)
{
	// use back buffer
	if (DispStripInx >= 0)
	{
		if (x < 0)
		{
			w += x;
			x = 0;
		}
		if (x + w > WIDTH) w = WIDTH - x;
		if (w <= 0) return;

		if (y < 0)
		{
			h += y;
			y = 0;
		}
		if (y + h > HEIGHT) h = HEIGHT - y;
		if (h <= 0) return;

		if (x < DispDirtyX1) DispDirtyX1 = x;
		if (x + w > DispDirtyX2) DispDirtyX2 = x + w;
		if (y < DispDirtyY1) DispDirtyY1 = y;
		if (y + h > DispDirtyY2) DispDirtyY2 = y + h;
	}
}

// update dirty area by pixel (check valid range)
void DispDirtyPoint(int x, int y)
{
	// use back buffer
	if (DispStripInx >= 0)
	{
		if (((u32)x < (u32)WIDTH) && ((u32)y < (u32)HEIGHT))
		{
			if (x < DispDirtyX1) DispDirtyX1 = x;
			if (x + 1 > DispDirtyX2) DispDirtyX2 = x + 1;
			if (y < DispDirtyY1) DispDirtyY1 = y;
			if (y + 1 > DispDirtyY2) DispDirtyY2 = y + 1;
		}
	}
}

// use full back buffer
#endif // USE_DVIVGA == 2

// update - send dirty window to display (or write back buffer to frame buffer)
void DispUpdate()
{
	// use back buffer
	if (DispStripInx >= 0)
	{

// use full back buffer
#if USE_DVIVGA == 2

		if ((DispDirtyX1 < DispDirtyX2) && (DispDirtyY1 < DispDirtyY2))
		{
			// height of segment
			int i = DispDirtyY2 - DispDirtyY1;

			// align
			int x1 = FRAME_ALIGN_DN(DispDirtyX1) >> FRAME_SHIFTX;
			int x2 = FRAME_ALIGN_UP(DispDirtyX2) >> FRAME_SHIFTX;
			int w = (x2 - x1)*sizeof(FRAMETYPE);

			// copy data
			int off = x1 + DispDirtyY1*WIDTHLEN;
			FRAMETYPE* d = &FrameBuf[off];
			FRAMETYPE* s = &BackBuf[off];
			for (; i > 0; i--)
			{
				memcpy(d, s, w);
				s += WIDTHLEN;
				d += WIDTHLEN;
			}

			// set dirty none
			DispDirtyNone();
		}

// use strip back buffer
#else // USE_DVIVGA == 2

		memcpy(&FrameBuf[DispMinY*WIDTHLEN], BackBuf, BACKBUFSIZE*sizeof(FRAMETYPE));

#endif // USE_DVIVGA == 2

	}
}

// auto update after delta time in [ms] of running program
void DispAutoUpdate(u32 ms)
{
	// interval in [us]
	u32 us = ms*1000;

	// check interval from last update
	if ((u32)(Time() - DispAutoUpdateLast) >= us)
	{
		// update display
		DispUpdate();

		// start measure new time interval of running program
		DispAutoUpdateLast = Time();
	}
}

// refresh update all display
void DispUpdateAll()
{
	// set dirty all frame buffer
	DispDirtyAll();

	// update - send dirty window to display
	DispUpdate();
}

#endif // BACKBUFSIZE > 0

#endif // USE_DVIVGA
