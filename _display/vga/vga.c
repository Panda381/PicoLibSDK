
// ****************************************************************************
//
//                                  VGA
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

// VGA resolution:
// - 640x480 pixels
// - vertical frequency 60 Hz
// - horizontal frequency 31.4685 kHz
// - pixel clock 25.175 MHz
//
// QVGA resolution:
// - 320x240 pixels
// - vertical double image scanlines
// - vertical frequency 60 Hz
// - horizontal frequency 31.4685 kHz
// - pixel clock 12.5875 MHz
//
// VGA vertical timings:
// - 525 scanlines total
// - line 1,2: (2) vertical sync
// - line 3..35: (33) dark
// - line 36..515: (480) image lines 0..479
// - line 516..525: (10) dark
//
// VGA horizontal timings:
// - 31.77781 total scanline in [us] (800 pixels, QVGA 400 pixels)
// - 0.63556 H front porch (after image, before HSYNC) in [us] (16 pixels, QVGA 8 pixels)
// - 3.81334 H sync pulse in [us] (96 pixels, QVGA 48 pixels)
// - 1.90667 H back porch (after HSYNC, before image) in [us] (48 pixels, QVGA 24 pixels)
// - 25.42224 H full visible in [us] (640 pixels, QVGA 320 pixels)
// - 0.0397222625 us per pixel at VGA, 0.079444525 us per pixel at QVGA
//
// We want reach 25.175 pixel clock (at 640x480). Default system clock is 125 MHz, which is
// approx. 5x pixel clock. We need 25.175*5 = 125.875 MHz. We use nearest frequency 126 MHz.
//	126000, 1512000, 126, 6, 2,     // 126.00MHz, VC0=1512MHz, FBDIV=126, PD1=6, PD2=2
//	126000, 504000, 42, 4, 1,       // 126.00MHz, VC0=504MHz, FBDIV=42, PD1=4, PD2=1
//	sysclk=126.000000 MHz, vco=504 MHz, fbdiv=42, pd1=4, pd2=1
//	sysclk=126.000000 MHz, vco=504 MHz, fbdiv=42, pd1=2, pd2=2
//	sysclk=126.000000 MHz, vco=756 MHz, fbdiv=63, pd1=6, pd2=1
//	sysclk=126.000000 MHz, vco=756 MHz, fbdiv=63, pd1=3, pd2=2
//	sysclk=126.000000 MHz, vco=1008 MHz, fbdiv=84, pd1=4, pd2=2 !!!!!
//	sysclk=126.000000 MHz, vco=1260 MHz, fbdiv=105, pd1=5, pd2=2
//	sysclk=126.000000 MHz, vco=1512 MHz, fbdiv=126, pd1=6, pd2=2
//	sysclk=126.000000 MHz, vco=1512 MHz, fbdiv=126, pd1=4, pd2=3
// Pixel clock is now:
//      5 system clock ticks per pixel at VGA ... pixel clock = 25.2 MHz, 0.039683 us per pixel
//     10 system clock ticks per pixel at QVGA ... pixel clock = 12.6 MHz, 0.079365 us per pixel
//
// - active image is 640*5=3200 clock ticks = 25.3968 us (QVGA: 1600 clock ticks)
// - total scanline is 126*31.77781=4004 clock ticks (QVGA: 2002 clock ticks)
// - H front porch = 82 clock ticks (QVGA: 41 clock ticks)
// - H sync pulse = 480 clock ticks (QVGA: 240 clock ticks)
// - H back porch = 242 clock ticks (QVGA: 121 clock ticks)

// in case of system clock = 125 MHz
// - PIO clock = system clock / 2
// - 5 PIO clocks per pixel = 10 system clocks per pixel
// - PIO clock = 62.5 MHz
// - pixel clock = 12.5 MHz
// - active image (320 pixels): 320*5 = 1600 PIO clocks = 3200 system ticks = 25.6 us (2.2 pixels stays invisible)
// - total scanline: 125*31.77781 = 3972 system clocks = 1986 PIO clocks
// - H front porch = 34 PIO clk
// - H sync = 238 PIO clk
// - H back = 114 PIO clk

#include "../../global.h"	// globals

#if USE_VGA		// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)

#include "vga.h"
#include "vga.pio.h"
#include "../../_sdk/inc/sdk_dma.h"
#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_pio.h"
#include "../../_sdk/inc/sdk_irq.h"
#include "../../_sdk/inc/sdk_gpio.h"
#include "../../_sdk/inc/sdk_timer.h"
#include "../../_sdk/inc/sdk_multicore.h"

// PIO command (jmp=program address, num=loop counter)
#define VGACMD(jmp, num) ( ((u32)((jmp)+VGA_PIO_OFF)<<27) | (u32)(num))

// pins
#define VGA_GPIO_VSYNC	(VGA_GPIO_HSYNC+1) // VGA VSYNC GPIO (VGA requires VSYNC = HSYNC+1)

// DMA
#define VGA_DMA_CB	(VGA_DMA+0)	// VGA DMA channel - control block of base layer
#define VGA_DMA_PIO	(VGA_DMA+1)	// VGA DMA channel - copy data of base layer to PIO (raises IRQ0 on quiet)

#if USE_FRAMEBUF	// use default display frame buffer
// frame buffer in BGR565 pixel format
ALIGNED u16 FrameBuf[FRAMESIZE]; // display buffer
#endif // USE_FRAMEBUF

#if USE_VGA > 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
ALIGNED u16 BackBuf[BACKBUFSIZE]; // back buffer strip (77 KB)
#endif

// dirty window to update
//u8 DispRot = 1;	// current display rotation
//u16 DispWidth = WIDTH;
//u16 DispHeight = HEIGHT; // current display size
int DispDirtyX1, DispDirtyX2, DispDirtyY1, DispDirtyY2;

// strip of back buffer
u16* pDrawBuf = FrameBuf; // current draw buffer
int DispStripInx = -1;	// current index of back buffer strip (-1 = use full FrameBuf)
int DispMinY = 0;	// minimal Y; base of back buffer strip
int DispMaxY = HEIGHT;	// maximal Y + 1; end of back buffer strip

// Scanline data buffers (commands sent to PIO)
u32 ScanLineImg[3];	// image: HSYNC ... back porch ... image command
u32 ScanLineFp;		// front porch
u32 ScanLineDark[2];	// dark: HSYNC ... back porch + dark + front porch
u32 ScanLineSync[2];	// vertical sync: VHSYNC ... VSYNC(back porch + dark + front porch)

// Scanline control buffers
#define CB_MAX 8	// size of one scanline control buffer (1 link to data buffer requires 2x u32)
u32 ScanLineCB[2*CB_MAX]; // 2 control buffers
int VgaBufInx;		// current running control buffer
u32* ScanLineCBNext;	// next control buffer

// Scanline data buffer (only BGR mode)
#if VGA_MODE	// used VGA output mode: 0=RGB, 1=BGR (with conversion on output)
ALIGNED u32 ScanLineBuf[WIDTH/2*2]; // scanline 1 and 2 image data buffer
#endif

// handler variables
volatile int VgaScanLine; // current processed scan line 0... (next displayed scan line)
volatile u32 VgaFrame;	// frame counter

#if VGA_MODE	// used VGA output mode: 0=RGB, 1=BGR (with conversion on output)
// convert image to data buffer (only BGR mode)
void NOFLASH(VgaLineRender)(u32* dst, const u32* src)
{
	int i;
	u32 k;
	u32 maskgreen = (0x3f << 5) | (0x3f << (5+16));
	u32 maskblue = 0x1f | (0x1f << 16);
	for (i = WIDTH/2/8; i > 0; i--)
	{
		k = src[0]; dst[0] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[1]; dst[1] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[2]; dst[2] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[3]; dst[3] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[4]; dst[4] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[5]; dst[5] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[6]; dst[6] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		k = src[7]; dst[7] = (k & maskgreen) | ((k & maskblue) << 11) | ((k >> 11) & maskblue);
		src += 8;
		dst += 8;
	}
}
#endif

// VGA DMA handler - called on end of every scanline
void NOFLASH(VgaLine)()
{
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
	if (line >= VGA_VTOT) // last scanline?
	{
		VgaFrame++;	// increment frame counter
		line = 0; 	// restart scanline
	}
	VgaScanLine = line;	// store new scanline

	// check scanline
	line -= VGA_VSYNC;
	if (line < 0)
	{
		// VSYNC
		*cb++ = 2;
		*cb++ = (u32)&ScanLineSync[0];
	}
	else
	{
		// front porch and back porch
		line -= VGA_VBACK;
		if ((line < 0) || (line >= VGA_VACT))
		{
			// dark line
			*cb++ = 2;
			*cb++ = (u32)&ScanLineDark[0];
		}

		// image scanlines
		else
		{
			// prepare image line
			line >>= 1;

			// HSYNC ... back porch ... image command
			*cb++ = 3;
			*cb++ = (u32)&ScanLineImg[0];

			// image data
			*cb++ = WIDTH/2;
#if VGA_MODE	// used VGA output mode: 0=RGB, 1=BGR (with conversion on output)
			u32* db = &ScanLineBuf[bufinx*(WIDTH/2)]; // scanline image data buffer
			*cb++ = (u32)db;
			VgaLineRender(db, (u32*)&FrameBuf[line*WIDTH]);
#else
			*cb++ = (u32)&FrameBuf[line*WIDTH];
#endif

			// front porch
			*cb++ = 1;
			*cb++ = (u32)&ScanLineFp;
		}
	}

	// end mark
	*cb++ = 0;
	*cb = 0;
}

// initialize VGA PIO
void VgaPioInit()
{
	// initialize PIO
	PioInit(VGA_PIO);

	// load PIO program
	PioLoadProg(VGA_PIO, vga_program_instructions, count_of(vga_program_instructions), VGA_PIO_OFF);

	// PIO set wrap address
	PioSetWrap(VGA_PIO, VGA_SM, VGA_PIO_OFF + vga_wrap_target, VGA_PIO_OFF + vga_wrap);

	// set start address
	PioSetAddr(VGA_PIO, VGA_SM, VGA_PIO_OFF + vga_offset_entry);

	// configure GPIOs for use by PIO
#ifdef VGA_GPIO_SKIP
	PioSetupGPIO(VGA_PIO, VGA_GPIO_FIRST, VGA_GPIO_SKIP-VGA_GPIO_FIRST);
	PioSetupGPIO(VGA_PIO, VGA_GPIO_SKIP+1, VGA_GPIO_FIRST+VGA_GPIO_NUM-VGA_GPIO_SKIP-1);
#else
	PioSetupGPIO(VGA_PIO, VGA_GPIO_FIRST, VGA_GPIO_NUM);
#endif
	PioSetupGPIO(VGA_PIO, VGA_GPIO_HSYNC, 2);

	// set pin direction to output
#ifdef VGA_GPIO_SKIP
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_SKIP-VGA_GPIO_FIRST, 1);
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_SKIP+1, VGA_GPIO_FIRST+VGA_GPIO_NUM-VGA_GPIO_SKIP-1, 1);
#else
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_NUM, 1);
#endif
	PioSetPinDir(VGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 2, 1);

	// negate HSYNC and VSYNC output
	GPIO_OutOverInvert(VGA_GPIO_HSYNC);
	GPIO_OutOverInvert(VGA_GPIO_VSYNC);

	// map state machine's OUT and MOV pins	
	PioSetupOut(VGA_PIO, VGA_SM, VGA_GPIO_FIRST, VGA_GPIO_NUM);

	// set sideset pins (HSYNC and VSYNC, 2 bits, no optional, no pindirs)
	PioSetupSideset(VGA_PIO, VGA_SM, VGA_GPIO_HSYNC, 2, False, False);

	// join FIFO to send only
	PioSetFifoJoin(VGA_PIO, VGA_SM, PIO_FIFO_JOIN_TX);

	// PIO clock divider
	PioSetClkdiv(VGA_PIO, VGA_SM, VGA_CLKDIV*256);

	// shift right, autopull, pull threshold
	PioSetOutShift(VGA_PIO, VGA_SM, True, True, 32);
}

// initialize scanline buffers
void VgaBufInit()
{
	// image scanline data buffer: HSYNC ... back porch ... image command
	ScanLineImg[0] = VGACMD(vga_offset_hsync, VGA_HSYNC-3); // HSYNC
	ScanLineImg[1] = VGACMD(vga_offset_dark, VGA_BP-4); // back porch
	ScanLineImg[2] = VGACMD(vga_offset_output, WIDTH-2); // image

	// front porch
	ScanLineFp = VGACMD(vga_offset_dark, VGA_FP-4); // front porch

	// dark scanline: HSYNC ... back porch + dark + front porch
	ScanLineDark[0] = VGACMD(vga_offset_hsync, VGA_HSYNC-3); // HSYNC
	ScanLineDark[1] = VGACMD(vga_offset_dark, VGA_TOTAL-VGA_HSYNC-4); // back porch + dark + front porch

	// vertical sync: VHSYNC ... VSYNC(back porch + dark + front porch)
	ScanLineSync[0] = VGACMD(vga_offset_vhsync, VGA_HSYNC-3); // VHSYNC
	ScanLineSync[1] = VGACMD(vga_offset_vsync, VGA_TOTAL-VGA_HSYNC-3); // VSYNC(back porch + dark + front porch)

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
			// DMA_CTRL_BSWAP |	// not byte swap
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

// initialize VGA (required system clock CLK_SYS to be set to 126 MHz)
void VgaInit()
{
#if USE_FRAMEBUF		// use default display frame buffer
	// clear frame buffers
	memset(FrameBuf, 0, FRAMESIZE*2);
#endif // USE_FRAMEBUF

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
	ScanLineCBNext = &ScanLineCB[CB_MAX]; // send control buffer 2 next

	// enable DMA IRQ
	NVIC_IRQEnable(IRQ_DMA_0);

	// start DMA
	DMA_Start(VGA_DMA_CB);

	// run state machine
	PioSMEnable(VGA_PIO, VGA_SM);
}

// terminate VGA
void VgaTerm()
{
	// abort DMA channels (interrupt data channel first and then control channel)
	DMA_Abort(VGA_DMA_PIO); // pre-abort, could be chaining right now
	DMA_Abort(VGA_DMA_CB);

	// disable IRQ0 from DMA0
	NVIC_IRQDisable(IRQ_DMA_0);
	DMA_IRQ0Disable(VGA_DMA_PIO);

	// Clear the interrupt request for DMA control channel
	DMA_IRQ0Clear(VGA_DMA_PIO);

	// reset PIO
	PioInit(VGA_PIO);
}

void (* volatile VgaCore1Fnc)() = NULL; // core 1 remote function

#define VGA_REQNO 	0	// request - no
#define VGA_REQINIT     1	// request - init
#define VGA_REQTERM	2	// request - terminate
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
			if (req == VGA_REQINIT)
				VgaInit(); // initialize
			else
			{
				VgaTerm(); // terminate
			}

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

// execute core 1 remote function (VGA must be running)
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
	return ((VgaScanLine < VGA_VSYNC+VGA_VBACK) ||
		(VgaScanLine >= VGA_VTOT-VGA_VFRONT));
}

// wait for VSync scanline
void VgaWaitVSync()
{
	// wait for end of VSync
	while (VgaIsVSync()) { dmb(); }

	// wait for start of VSync
	while (!VgaIsVSync()) { dmb(); }
}

// start VGA on CPU 1 (must be paired with VgaStop())
void VgaStart()
{
	// run VGA core
	Core1Exec(VgaCore);

	// initialize VGA
	VgaReq = VGA_REQINIT;
	dmb();
	while (VgaReq != VGA_REQNO) { dmb(); }
}

// terminate VGA on CPU 1 (must be paired with VgaStart())
void VgaStop()
{
	// terminate VGA
	VgaReq = VGA_REQTERM;
	dmb();
	while (VgaReq != VGA_REQNO) { dmb(); }

	// core 1 reset
	Core1Reset();
}

// set strip of back buffer
void DispSetStrip(int inx)
{
#if USE_VGA == 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)

	// no back buffer
	pDrawBuf = FrameBuf;	// current draw buffer
	DispStripInx = -1;	// current index of back buffer strip
	DispMinY = 0;		// minimal Y; base of back buffer strip
	DispMaxY = HEIGHT;	// maximal Y + 1; end of back buffer strip

#elif USE_VGA == 2

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
		// 1/2 back buffer
		if (inx >= 2) inx = 0;
		pDrawBuf = BackBuf;	// current draw buffer
		DispStripInx = inx;	// current index of back buffer strip
		DispMinY = inx * (HEIGHT/2);	// minimal Y; base of back buffer strip
		DispMaxY = DispMinY + HEIGHT/2;	// maximal Y + 1; end of back buffer strip
	}
#else

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
		// 1/4 back buffer
		if (inx >= 4) inx = 0;
		pDrawBuf = BackBuf;	// current draw buffer
		DispStripInx = inx;	// current index of back buffer strip
		DispMinY = inx * (HEIGHT/4);	// minimal Y; base of back buffer strip
		DispMaxY = DispMinY + HEIGHT/4;	// maximal Y + 1; end of back buffer strip
	}
#endif

}

// load strip of back buffer from frame buffer
#if USE_VGA > 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
void DispLoad()
{
	// back buffer 77 KB: DMA 154 us, memcpy 539 us
	// back buffer 38 KB: DMA 77 us, memcpy 266 us
	if (DispStripInx >= 0) memcpy(BackBuf, &FrameBuf[DispMinY*WIDTH], BACKBUFSIZE*2);
//	if (DispStripInx >= 0) DMA_MemCopy(BackBuf, &FrameBuf[DispMinY*WIDTH], BACKBUFSIZE*2);
}
#endif // USE_VGA > 1

// update - send dirty window to display (or write back buffer to frame buffer)
#if USE_VGA > 1	// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
void DispUpdate()
{
	// back buffer 77 KB: DMA 154 us, memcpy 539 us
	// back buffer 38 KB: DMA 77 us, memcpy 266 us
	if (DispStripInx >= 0) memcpy(&FrameBuf[DispMinY*WIDTH], BackBuf, BACKBUFSIZE*2);
//	if (DispStripInx >= 0) DMA_MemCopy(&FrameBuf[DispMinY*WIDTH], BackBuf, BACKBUFSIZE*2);
}
#endif // USE_VGA > 1

#endif // USE_VGA		// use VGA display 320x240/16; 1=use 1 frame buffer 153 KB, 2=add 1/2 back buffer 230 KB, 3=add 1/4 back buffer 192 KB (vga.c, vga.h)
