
// ****************************************************************************
//
//                                Mini-VGA
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

#if USE_MINIVGA					// use mini-VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

#include "minivga.h"
#include "../../_sdk/inc/sdk_dma.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_pio.h"
#include "../../_sdk/inc/sdk_irq.h"
#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_multicore.h"

#if VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)

#if (COLBITS == 16) || (COLBITS == 15)
#include "minivga16csync.pio.h"
#elif COLBITS == 8
#include "minivga8csync.pio.h"
#elif COLBITS == 4
#include "minivga4csync.pio.h"
#else
#error Invalid COLBITS!
#endif

#else // VGA_USECSYNC

#if (COLBITS == 16) || (COLBITS == 15)
#include "minivga16.pio.h"
#elif COLBITS == 8
#include "minivga8.pio.h"
#elif COLBITS == 4
#include "minivga4.pio.h"
#else
#error Invalid COLBITS!
#endif

#endif // VGA_USECSYNC

#ifndef VGA_PIO_OFF
#define VGA_PIO_OFF	2	// offset of VGA program in PIO memory (2 instructions are reserve for HDMI program)
#endif

// swap bytes of command
#define BYTESWAP(n) ((((n)&0xff)<<24)|(((n)&0xff00)<<8)|(((n)&0xff0000)>>8)|(((n)&0xff000000)>>24))

// PIO command (jmp=program address, num=loop counter) ... 4-bit mode uses left shift
#if COLBITS == 4
#define VGACMD(jmp, num) BYTESWAP( (u32)((jmp)+VGA_PIO_OFF) | ((u32)(num)<<5) )
#else
#define VGACMD(jmp, num) ( ((u32)((jmp)+VGA_PIO_OFF)<<27) | (u32)(num) )
#endif

// pins
#define VGA_GPIO_VSYNC	(VGA_GPIO_HSYNC+1) // VGA VSYNC GPIO (VGA requires VSYNC = HSYNC+1)

// DMA
#define VGA_DMA_CB	(VGA_DMA+0)	// VGA DMA channel - control block of base layer
#define VGA_DMA_PIO	(VGA_DMA+1)	// VGA DMA channel - copy data of base layer to PIO (raises IRQ0 on quiet)

// frame buffer
#if USE_FRAMEBUF	// use default display frame buffer
ALIGNED FRAMETYPE FrameBuf[FRAMESIZE];
#endif

// back buffer
#if USE_FRAMEBUF && (BACKBUFSIZE > 0)	// size of back buffer strip
ALIGNED FRAMETYPE BackBuf[BACKBUFSIZE]; // back buffer strip
#endif

// display setup
#if USE_MINIVGA == 2 // use full back buffer
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

// last system time of auto update
u32 DispAutoUpdateLast;

// Scanline data buffers (commands sent to PIO)
u32 ScanLineImg[3];	// image: HSYNC ... back porch ... image command
u32 ScanLineFp;		// front porch (after image data)
u32 ScanLineDark[2];	// dark: HSYNC ... back porch + dark + front porch
u32 ScanLineSync[2];	// vertical sync: VHSYNC ... VSYNC(back porch + dark + front porch)

// Scanline control buffers
#define CB_MAX 8	// size of one scanline control buffer (1 link to data buffer requires 2x u32)
u32 ScanLineCB[2*CB_MAX]; // 2 control buffers
int VgaBufInx;		// current running control buffer
u32* ScanLineCBNext;	// next control buffer

// handler variables
volatile int VgaScanLine; // current processed scan line 0... (next displayed scan line)
volatile u32 VgaFrame;	// frame counter

#if MINIVGA_IRQTIME			// debug flag - measure delta time of VGA service
volatile u32 VgaTimeIn;			// time in interrupt service, in [us]
volatile u32 VgaTimeOut;		// time out interrupt service, in [us]
u32 VgaTimeTmp;
#endif // MINIVGA_IRQTIME

#ifdef VGA_KEYSCAN_GPIO		// scan keyboard
volatile u32 VgaKeyScan = 0;	// key scan
int VgaKeyScanStart = 100;	// start delay to prevent false keystrokes
#endif

// VGA DMA handler - called on end of every scanline
void NOFLASH(VgaLine)()
{
#if MINIVGA_IRQTIME			// debug flag - measure delta time of VGA service
	u32 t1 = Time();		// start time
#endif // MINIVGA_IRQTIME

	// Clear the interrupt request for DMA control channel
	DMA_IRQ0Clear(VGA_DMA_PIO);

	// update DMA control channel and run it
	DMA_SetReadTrig(VGA_DMA_CB, ScanLineCBNext);

	// switch current buffer index (bufinx = current preparing buffer, VgaBufInx = current running buffer)
	int bufinx = VgaBufInx;
	VgaBufInx = bufinx ^ 1;

	// prepare control buffer to be processed
	u32* cb = &ScanLineCB[bufinx*CB_MAX];
	ScanLineCBNext = cb;

	// increment scanline (1..)
	int line = VgaScanLine; // current scanline
	line++; 		// new current scanline
	if (line >= MINIVGA_VTOTAL) // last scanline?
	{
		VgaFrame++;	// increment frame counter
		line = 0; 	// restart scanline
	}
	VgaScanLine = line;	// store new scanline

	// scan keyboard
#ifdef VGA_KEYSCAN_GPIO		// scan keyboard
#define VGA_KEYSCAN_LINE	(MINIVGA_VSYNC+6)	// first valid scanline
	if ((line >= VGA_KEYSCAN_LINE) && (line < VGA_KEYSCAN_LINE+8))
	{
		// disable output from PIO
		GPIO_OEOverDisable(VGA_KEYSCAN_GPIO);

		// wait 2 us (do not use WaitUs() from Flash - collision during flash write)
		u32 start = Time();
		while ((u32)(Time() - start) < 2) {}

		// prepare key mask
		u32 m = 1UL << (line - VGA_KEYSCAN_LINE);
		u32 key = VgaKeyScan;
		key &= ~m;

		// input
		if (GPIO_In(VGA_KEYSCAN_GPIO)) key |= m;

		// start delay to prevent false keystrokes
		if (VgaKeyScanStart > 0)
		{
			VgaKeyScanStart--;
			key = 0;
		}
		VgaKeyScan = key;

		// set normal output from PIO
		GPIO_OEOverNormal(VGA_KEYSCAN_GPIO);
	}
#endif

	// check scanline
	line -= MINIVGA_VSYNC;
	if (line < 0)
	{
		// VSYNC
		*cb++ = 2;
		*cb++ = (u32)&ScanLineSync[0];
	}
	else
	{
		// front porch and back porch
		line -= MINIVGA_VBACK;
		if ((line < 0) || (line >= MINIVGA_VACT))
		{
			// dark line
			*cb++ = 2;
			*cb++ = (u32)&ScanLineDark[0];
		}

		// image scanlines
		else
		{
			// prepare image line
			line /= MINIVGA_VMUL;

			// HSYNC ... back porch ... image command
			*cb++ = 3;
			*cb++ = (u32)&ScanLineImg[0];

			// image data
			*cb++ = WIDTHLEN*sizeof(FRAMETYPE)/4;
			*cb++ = (u32)&FrameBuf[line*WIDTHLEN];

			// front porch
			*cb++ = 1;
			*cb++ = (u32)&ScanLineFp;
		}
	}

	// end mark
	*cb++ = 0;
	*cb = 0;

#if MINIVGA_IRQTIME			// debug flag - measure delta time of VGA service
	if (line == 99) VgaTimeTmp = Time();
	if (line == 100)
	{
		u32 t2 = Time();		// stop time
		VgaTimeIn = t2 - t1;		// time in interrupt service
		VgaTimeOut = t1 - VgaTimeTmp;	// time out interrupt service
	}
#endif // MINIVGA_IRQTIME
}

// initialize VGA PIO
void VgaPioInit()
{
	// initialize PIO
	PioInit(VGA_PIO);

	// configure main program instructions
	uint16_t ins[32]; // temporary buffer of program instructions
	memcpy(ins, &minivga_program_instructions, count_of(minivga_program_instructions)*sizeof(uint16_t)); // copy program into buffer
	ins[minivga_offset_extra] |= (MINIVGA_CPP-2) << 8; // update waits

	// load PIO program
	PioLoadProg(VGA_PIO, ins, count_of(minivga_program_instructions), VGA_PIO_OFF);

	// PIO set wrap address
	PioSetWrap(VGA_PIO, VGA_SM, VGA_PIO_OFF + minivga_wrap_target, VGA_PIO_OFF + minivga_wrap);

	// set start address
	PioSetAddr(VGA_PIO, VGA_SM, VGA_PIO_OFF + minivga_offset_entry);

	// configure GPIOs for use by PIO
#ifdef VGA_GPIO_SKIP
	PioSetupGPIO(VGA_PIO, VGA_GPIO_FIRST, VGA_GPIO_SKIP-VGA_GPIO_FIRST);
	PioSetupGPIO(VGA_PIO, VGA_GPIO_SKIP+1, VGA_GPIO_FIRST+VGA_GPIO_NUM-VGA_GPIO_SKIP-1);
#else
	PioSetupGPIO(VGA_PIO, VGA_GPIO_FIRST, VGA_GPIO_NUM);
#endif

#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioSetupGPIO(VGA_PIO, VGA_GPIO_HSYNC, 2);	// use HSYNC + VSYNC
#else
	PioSetupGPIO(VGA_PIO, VGA_GPIO_HSYNC, 1);	// use only CSYNC
#endif

	// set pin direction to output
#ifdef VGA_GPIO_SKIP
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_SKIP-VGA_GPIO_FIRST, 1);
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_SKIP+1, VGA_GPIO_FIRST+VGA_GPIO_NUM-VGA_GPIO_SKIP-1, 1);
#else
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_NUM, 1);
#endif

	// high current output
	int i;
	for (i = VGA_GPIO_FIRST; i < VGA_GPIO_FIRST+VGA_GPIO_NUM; i++)
	{
#ifdef VGA_GPIO_SKIP
		if (i != VGA_GPIO_SKIP)
#endif
		GPIO_Drive12mA(i);
	}

#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 2, 1);	// use HSYNC + VSYNC
#else
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 1, 1);	// use only CSYNC
#endif

	// negate HSYNC and VSYNC output
	GPIO_OutOverInvert(VGA_GPIO_HSYNC);
#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	GPIO_OutOverInvert(VGA_GPIO_VSYNC);		// use HSYNC + VSYNC
#endif

	// map state machine's OUT and MOV pins	
	PioSetupOut(VGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_NUM);

	// set sideset pins (HSYNC and VSYNC, 2 bits, no optional, no pindirs)
#if !VGA_USECSYNC	// 1=use only CSYNC signal instead of HSYNC (VSYNC not used)
	PioSetupSideset(VGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 2, False, False);	// use HSYNC + VSYNC
#else
	PioSetupSideset(VGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 1, False, False);	// use only CSYNC
#endif

	// join FIFO to send only
	PioSetFifoJoin(VGA_PIO, VGA_SM, PIO_FIFO_JOIN_TX);

	// PIO clock divider
	PioSetClkdiv(VGA_PIO, VGA_SM, MINIVGA_CLKDIV*256);

#if COLBITS == 4
	// shift left, autopull, pull threshold
	PioSetOutShift(VGA_PIO, VGA_SM, False, True, 32);
#else
	// shift right, autopull, pull threshold
	PioSetOutShift(VGA_PIO, VGA_SM, True, True, 32);
#endif
}

// initialize scanline buffers
void VgaBufInit()
{
	// image scanline data buffer: HSYNC ... back porch ... image command
	ScanLineImg[0] = VGACMD(minivga_offset_hsync, MINIVGA_HSYNC-3); // HSYNC
	ScanLineImg[1] = VGACMD(minivga_offset_dark, MINIVGA_HBACK-4); // back porch
	ScanLineImg[2] = VGACMD(minivga_offset_output, WIDTH-1); // number of pixels

	// front porch
	ScanLineFp = VGACMD(minivga_offset_dark, MINIVGA_HFRONT-4-3); // front porch (3 clocks after image)

	// dark scanline: HSYNC ... back porch + dark + front porch
	ScanLineDark[0] = VGACMD(minivga_offset_hsync, MINIVGA_HSYNC-3); // HSYNC
	ScanLineDark[1] = VGACMD(minivga_offset_dark, MINIVGA_HTOTAL-MINIVGA_HSYNC-4); // back porch + dark + front porch

	// vertical sync: VHSYNC ... VSYNC(back porch + dark + front porch)
	ScanLineSync[0] = VGACMD(minivga_offset_vhsync, MINIVGA_HSYNC-3); // VHSYNC
	ScanLineSync[1] = VGACMD(minivga_offset_vsync, MINIVGA_HTOTAL-MINIVGA_HSYNC-3); // VSYNC(back porch + dark + front porch)

	// control buffer 1 - initialize to VSYNC
	ScanLineCB[0] = 2; // send 2x u32 (send ScanLineSync)
	ScanLineCB[1] = (u32)&ScanLineSync[0]; // VSYNC data buffer
	ScanLineCB[2] = 0; // stop mark
	ScanLineCB[3] = 0; // stop mark

	// control buffer 1 - initialize to VSYNC
	ScanLineCB[CB_MAX+0] = 2; // send 2x u32 (send ScanLineSync)
	ScanLineCB[CB_MAX+1] = (u32)&ScanLineSync[0]; // VSYNC data buffer
	ScanLineCB[CB_MAX+2] = 0; // stop mark
	ScanLineCB[CB_MAX+3] = 0; // stop mark
}

// initialize VGA DMA
//   control blocks aliases:
//                  +0x0        +0x4          +0x8          +0xC (Trigger)
// 0x00 (alias 0):  READ_ADDR   WRITE_ADDR    TRANS_COUNT   CTRL_TRIG
// 0x10 (alias 1):  CTRL        READ_ADDR     WRITE_ADDR    TRANS_COUNT_TRIG
// 0x20 (alias 2):  CTRL        TRANS_COUNT   READ_ADDR     WRITE_ADDR_TRIG
// 0x30 (alias 3):  CTRL        WRITE_ADDR    TRANS_COUNT   READ_ADDR_TRIG ... we use this!
void VgaDmaInit()
{

// ==== prepare DMA control channel

	DMA_Config(VGA_DMA_CB,		// channel
		&ScanLineCB[0],		// read address - as first, control buffer 1 will be sent out
		&DMA_CHAN(VGA_DMA_PIO)[DMA_CH_AL3_COUNT], // write address to COUNT and READ_TRIG
		2,			// number of transfers = 2 * u32
			// DMA_CTRL_SNIFF |	// not sniff
			// DMA_CTRL_BSWAP |	// not byte swap
			// DMA_CTRL_QUIET |	// not quiet
			DMA_CTRL_TREQ_FORCE |	// permanent request
			DMA_CTRL_CHAIN(VGA_DMA_CB) | // disable chaining
			DMA_CTRL_RING_WRITE |	// wrap ring on write
			DMA_CTRL_RING_SIZE(3) | // ring size = 8 bytes
			DMA_CTRL_INC_WRITE |	// increment write
			DMA_CTRL_INC_READ |	// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY | // not high priority
			DMA_CTRL_EN);		// enable DMA

// ==== prepare DMA data channel

	DMA_Config(VGA_DMA_PIO,	// channel
		NULL,			// read address
		PIO_TXF(VGA_PIO, VGA_SM), // write address - PIO port
		0,			// number of transfers
			// DMA_CTRL_SNIFF |	// not sniff
#if COLBITS == 4
			DMA_CTRL_BSWAP |	// byte swap
#endif
			DMA_CTRL_QUIET |	// quiet
			DMA_CTRL_TREQ(PioGetDreq(VGA_PIO, VGA_SM, True)) | // data request from PIO
			DMA_CTRL_CHAIN(VGA_DMA_CB) | // chain to control DMA channel
			// DMA_CTRL_RING_WRITE | // not wrap ring on write
			// DMA_CTRL_RING_SIZE(3) | // not ring
			// DMA_CTRL_INC_WRITE |	// not increment write
			DMA_CTRL_INC_READ |	// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
			DMA_CTRL_HIGH_PRIORITY | // high priority
			DMA_CTRL_EN);		// enable DMA

// ==== initialize IRQ0, raised from DMA data channel

	// enable DMA channel IRQ0
	DMA_IRQ0Enable(VGA_DMA_PIO);

	// set DMA IRQ handler
	SetHandler(IRQ_DMA_0, VgaLine);

	// set highest IRQ priority
	NVIC_IRQPrio(IRQ_DMA_0, IRQ_PRIO_REALTIME);
}

// initialize VGA (required system clock CLK_SYS to be set to PLL_KHZ) ... use VgaStart() to start on core 1
void VgaInit()
{
	// clear frame buffer
#if USE_FRAMEBUF		// use default display frame buffer
	memset(FrameBuf, 0, FRAMESIZE*sizeof(FRAMETYPE));
#endif

	// clear back buffer
#if USE_FRAMEBUF && (BACKBUFSIZE > 0)
	memset(BackBuf, 0, BACKBUFSIZE*sizeof(FRAMETYPE));
#endif

	dmb();

	// initialize PIO
	VgaPioInit();

	// initialize scanline buffers
	VgaBufInit();

	// initialize DMA
	VgaDmaInit();

	// initialize parameters
	VgaScanLine = 0; // currently processed scanline
	VgaBufInx = 0; // at first, control buffer 1 will be sent out
	VgaFrame = 0; // current frame
	ScanLineCBNext = &ScanLineCB[CB_MAX]; // control buffer 2 will be sent next

	// enable DMA IRQ
	NVIC_IRQEnable(IRQ_DMA_0);

	// start DMA
	DMA_Start(VGA_DMA_CB);

#if MINIVGA_IRQTIME			// debug flag - measure delta time of VGA service
	VgaTimeTmp = Time();
#endif // MINIVGA_IRQTIME

	// run state machine
	PioSMEnable(VGA_PIO, VGA_SM);
}

// terminate VGA ... use VgaStop() to stop on core 1
void VgaTerm()
{
	// abort DMA channels (interrupt data channel first and then control channel)
	DMA_Abort(VGA_DMA_PIO); // pre-abort, could be chaining right now
	DMA_Abort(VGA_DMA_CB);
	DMA_Abort(VGA_DMA_PIO);

	// disable IRQ0 from DMA0
	NVIC_IRQDisable(IRQ_DMA_0);
	DMA_IRQ0Disable(VGA_DMA_PIO);

	// Clear the interrupt request for DMA control channel
	DMA_IRQ0Clear(VGA_DMA_PIO);

	// reset PIO
	PioInit(VGA_PIO);
}

#if USE_DOUBLE		// use double support

// required frequency to retune (in Hz)
volatile int VgaReqFreq = PLL_KHZ*1000;
int VgaCurFreq = PLL_KHZ*1000; // current frequency

// request to retune VGA to different system frequency
void VgaRetuneReq()
{
	// check current frequency
	int f = VgaReqFreq;
	if (f == VgaCurFreq) return;
	VgaCurFreq = f;

	// retuning coefficient
	double k = (double)f/PLL_KHZ/1000;

	// clock divider * 256 (CPP will be = 2)
	int clkdiv = (int)(256*k*MINIVGA_CLKDIV*MINIVGA_CPP/2 + 0.5);
	if (clkdiv < 256) clkdiv = 256;
	double clk = (double)clkdiv/256;

	// horizontal
	int hsync = (int)(k*MINIVGA_HSYNC*MINIVGA_CLKDIV/clk + 0.5);
	int hback = (int)(k*MINIVGA_HBACK*MINIVGA_CLKDIV/clk + 0.5);
	int hfront = (int)(k*MINIVGA_HFRONT*MINIVGA_CLKDIV/clk + 0.5);
	int htotal = hsync + hback + hfront + WIDTH*2;

	// disable IRQ
	IRQ_LOCK;

	// pause SM	
	PioSMDisable(VGA_PIO, VGA_SM);

	// load PIO program (with default CPP = 2)
	PioLoadProg(VGA_PIO, minivga_program_instructions, count_of(minivga_program_instructions), VGA_PIO_OFF);

	// set PIO clock divider
	PioSetClkdiv(VGA_PIO, VGA_SM, clkdiv);

	// image scanline data buffer: HSYNC ... back porch ... image command
	ScanLineImg[0] = VGACMD(minivga_offset_hsync, hsync-3); // HSYNC
	ScanLineImg[1] = VGACMD(minivga_offset_dark, hback-4); // back porch

	// front porch
	ScanLineFp = VGACMD(minivga_offset_dark, hfront-4-3); // front porch (3 clocks after image)

	// dark scanline: HSYNC ... back porch + dark + front porch
	ScanLineDark[0] = VGACMD(minivga_offset_hsync, hsync-3); // HSYNC
	ScanLineDark[1] = VGACMD(minivga_offset_dark, htotal-hsync-4); // back porch + dark + front porch

	// vertical sync: VHSYNC ... VSYNC(back porch + dark + front porch)
	ScanLineSync[0] = VGACMD(minivga_offset_vhsync, hsync-3); // VHSYNC
	ScanLineSync[1] = VGACMD(minivga_offset_vsync, htotal-hsync-3); // VSYNC(back porch + dark + front porch)

	// unpause state machine
	PioSMEnable(VGA_PIO, VGA_SM);

	// enable IRQ
	IRQ_UNLOCK;
}

#else

INLINE void VgaRetuneReq() {}

#endif

void (* volatile VgaCore1Fnc)() = NULL; // core 1 remote function

#define VGA_REQNO 	0	// request - no
#define VGA_REQINIT     1	// request - init
#define VGA_REQTERM	2	// request - terminate
#define VGA_REQRETUNE	3	// request - retune frequency
volatile int VgaReq = VGA_REQNO;	// current VGA request

// VGA core
void NOFLASH(VgaCore)()
{
	void (*fnc)();
	int req;

	// infinite loop
	while (True)
	{
		// data memory barrier
		dmb();

		// initialize/terminate VGA
		req = VgaReq;
		if (req != VGA_REQNO)
		{
			if (req == VGA_REQRETUNE)
				VgaRetuneReq(); // returne
			else if (req == VGA_REQINIT)
				VgaInit(); // initialize
			else
				VgaTerm(); // terminate

			VgaReq = VGA_REQNO;
		}		

		// execute remote function
		fnc = VgaCore1Fnc;
		if (fnc != NULL)
		{
			fnc();
			dmb();
			VgaCore1Fnc = NULL;
		}
	}
}

// execute core 1 remote function
void VgaCore1Exec(void (*fnc)())
{
	dmb();
	VgaCore1Fnc = fnc;
	dmb();
}

// check if core 1 is busy (executing remote function)
Bool VgaCore1Busy()
{
	dmb();
	return VgaCore1Fnc != NULL;
}

// wait if core 1 is busy (executing remote function)
void VgaCore1Wait()
{
	while (VgaCore1Busy()) {}
}

// check VSYNC
Bool VgaIsVSync()
{
	return ((VgaScanLine < MINIVGA_VSYNC+MINIVGA_VBACK) ||
		(VgaScanLine >= MINIVGA_VTOTAL-MINIVGA_VFRONT));
}

// wait for VSync scanline
void VgaWaitVSync()
{
	// wait for end of VSync
	while (VgaIsVSync()) { dmb(); }

	// wait for start of VSync
	while (!VgaIsVSync()) { dmb(); }
}

// start VGA on core 1 from core 0 (must be paired with VgaStop())
void VgaStart()
{
	// run VGA core
	Core1Exec(VgaCore);

	// initialize VGA
	VgaReq = VGA_REQINIT;
	dmb();
	while (VgaReq != VGA_REQNO) { dmb(); }
}

// terminate VGA on core 1 from core 0 (must be paired with VgaStart())
void VgaStop()
{
	// terminate VGA
	VgaReq = VGA_REQTERM;
	dmb();
	while (VgaReq != VGA_REQNO) { dmb(); }

	// core 1 reset
	Core1Reset();
}

#if USE_DOUBLE		// use double support

// retune VGA to different system frequency (freq = new system frequency in Hz)
void VgaRetune(int freq)
{
	// required frequency
	VgaReqFreq = freq;

	// request to retune
	VgaReq = VGA_REQRETUNE;
	dmb();
	while (VgaReq != VGA_REQNO) { dmb(); }
}

#endif

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
#if USE_MINIVGA == 2

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
#endif // USE_MINIVGA == 2

// update - send dirty window to display (or write back buffer to frame buffer)
void DispUpdate()
{
	// use back buffer
	if (DispStripInx >= 0)
	{

// use full back buffer
#if USE_MINIVGA == 2

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
#else // USE_MINIVGA == 2

		memcpy(&FrameBuf[DispMinY*WIDTHLEN], BackBuf, BACKBUFSIZE*sizeof(FRAMETYPE));

#endif // USE_MINIVGA == 2

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

#endif // USE_MINIVGA
