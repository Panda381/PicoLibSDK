
// ****************************************************************************
//
//                               Input Grabber
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

	.syntax unified
	.section .time_critical.Grabber, "ax"
	.cpu cortex-m0plus
	.thumb

.extern FrameBuf				// frame buffer
.extern DispBGR					// use BGR data mode instead of RGB
.extern DispLittleEndian			// use Little Endian data (LSB first)

#define WIDTH		320			// display width
#define HEIGHT		240			// display height

// ports
#define SPI_PIO		0				// SPI grabber PIO
#define SPI_SM		0				// SPI grabber state machine

#define PIO0_BASE	0x50200000			// PIO 0 base
//#define PIO1_BASE	0x50300000			// PIO 1 base

#define PIO_BASE	(PIO0_BASE + SPI_PIO*0x100000)	// PIO base
#define PIO_FSTAT_OFF	4				// offset of FIFO status register
#define PIO_RXF_OFF	(0x20 + SPI_SM*4)		// offset of RX FIFO

#define PIO_EMPTY_BIT	(SPI_SM + 8)			// test if RX FIFO is empty

// LCD commands
#define ST7789_CASET	0x2A				// set start and end column of draw window
#define ST7789_RASET	0x2B				// set start and end row of draw window
#define ST7789_RAMWR	0x2C				// start write data to draw window
#define ST7789_MADCTL	0x36				// set rotation mode
#define ST7789_RAMCTRL	0xB0				// RAM control, data 2 bytes

//#define ST7789_MADCTL_RGB 0x08			// B3: RGB/BGR order (0=RGB, 1=BGR)

// local registers
#define REG_IN		R0		// input data from PIO, output to memory
#define REG_IN2		R1		// input data 2 from PIO
#define REG_PIO		R2		// register with base address of the PIO (=PIO_BASE)
#define REG_TST		R3		// test value if RX FIFO is empty (=1<<PIO_EMPTY_BIT)
#define REG_DST		R4		// destination pointer
#define REG_LOOPY	R5		// loop counter of rows
#define REG_LOOPX	R6		// loop counter of pixels
#define REG_WIDTH	R7		// display width

#define REG_X		R8		// start X coordinate
#define REG_Y		R9		// start Y coordinate
#define REG_W		R10		// width of window
#define REG_H		R11		// height of window
#define REG_WB		R12		// increment of row

#define REG_TMP		R14		// temporary register (= LR)

// macro - receive data from PIO
.macro recv reg

	// wait for data
	// ... while (PioRxIsEmpty(SPI_PIO, SPI_SM)) {}
pio_read_\@:
	ldr	\reg,[REG_PIO,#PIO_FSTAT_OFF]		// read status register
	tst	\reg,REG_TST				// check if RX FIFO is empty
	bne	pio_read_\@				// loop if RX FIFO is empty

	// read data
	// ... return PioRead(SPI_PIO, SPI_SM);
	ldr	\reg,[REG_PIO,#PIO_RXF_OFF]		// read data
.endm

// ----------------------------------------------------------------------------
//                            Input grabber
// ----------------------------------------------------------------------------

.thumb_func
.global Grabber
Grabber:

	// push registers R4..R7
	push	{r4-r7,lr}		// push registers R4..R7, LR

	// push registers R8..R11
	mov	r4,r8
	mov	r5,r9
	mov	r6,r10
	mov	r7,r11
	push	{r4-r7}			// push registers R8..R11

	// prepare registers
	movs	REG_IN,#0		// clear register
	mov	REG_X,REG_IN		// start X coordinate
	mov	REG_Y,REG_IN		// start Y coordinate
	movs	REG_WIDTH,WIDTH/2	// width of the display/2
	lsls	REG_WIDTH,REG_WIDTH,#1	// *2, width of the display
	mov	REG_W,REG_WIDTH		// width of window
	movs	REG_IN,#HEIGHT		// height of the display
	mov	REG_H,REG_IN		// height of window
	ldr	REG_PIO,=PIO_BASE	// base address of the PIO
	movs	REG_TST,#1
	lsls	REG_TST,#PIO_EMPTY_BIT	// test value if RX FIFO is empty

// wait for command
Grabber_Wait:

	// wait for command (commands are with code > 255)
	// ... do { k = Proc_PioReadWait(); } while (k < 0x100);
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	ble	Grabber_Wait		// data byte received, wait for a command

// process command
Grabber_Proc:

	// prepare command
	// ... k &= 0xff;
	uxtb	REG_IN,REG_IN		// mask only lower 8 bits of the command

// ==== X: set start and end column of draw window

	// ... if (k == ST7789_CASET)
	cmp	REG_IN,#ST7789_CASET	// check command
	bne	Grabber_NoX

	// receive high byte of start X
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc		// invalid data, command received

	// receive low byte of start X
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB		// invalid data, command received

	// compose start X
	// ... k = (k << 8) | k2;
	lsls	REG_IN,#8		// shift high byte 8 bits left
	orrs	REG_IN,REG_IN2		// compose value

	// limit start X
	// ... if (k >= WIDTH) k = WIDTH-1; x = k;
	cmp	REG_IN,REG_WIDTH	// check max. X coordinate
	blt	2f			// start X is OK
	mov	REG_IN,REG_WIDTH	// limit start X to WIDTH-1
	subs	REG_IN,#1		// X = WIDTH-1
2:	mov	REG_X,REG_IN		// set start X

	// receive high byte of stop X
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc		// invalid data, command received

	// receive low byte of stop X
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB		// invalid data, command received

	// compose stop X
	// ... k = (k << 8) | k2; k++;
	lsls	REG_IN,#8		// shift high byte 8 bits left
	orrs	REG_IN,REG_IN2		// compose value
	adds	REG_IN,#1		// stop X + 1

	// limit stop X
	// ... if (k > WIDTH) k = WIDTH;
	cmp	REG_IN,REG_WIDTH	// check max. X coordinate
	ble	2f			// stop X is OK
	mov	REG_IN,REG_WIDTH	// limit stop X to WIDTH

	// set width
	// ... w = k - x; if (w < 1) w = 1;
2:	mov	REG_IN2,REG_X		// get start X
	subs	REG_IN,REG_IN,REG_IN2	// width
	bgt	3f			// width is OK > 1
	movs	REG_IN,#1		// minimal width = 1
3:	mov	REG_W,REG_IN		// set width

	// increment of row
	// ... wb = WIDTH - w;
	subs	REG_IN,REG_WIDTH,REG_IN	// increment in number of pixels
	lsls	REG_IN,REG_IN,#1	// convert to byte offset
	mov	REG_WB,REG_IN
	b	Grabber_Wait		// get next command

// ==== command is in REG_IN2

Grabber_ProcB:
	mov	REG_IN,REG_IN2
	b	Grabber_Proc

Grabber_NoX:

// ==== Y: set start and end row of draw window

	// ... if (k == ST7789_RASET)
	cmp	REG_IN,#ST7789_RASET	// check command
	bne	Grabber_NoY

	// receive high byte of start Y
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc		// invalid data, command received

	// receive low byte of start Y
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB		// invalid data, command received

	// compose start Y
	// ... k = (k << 8) | k2;
	lsls	REG_IN,#8		// shift high byte 8 bits left
	orrs	REG_IN,REG_IN2		// compose value

	// limit start Y
	// ... if (k >= HEIGHT) k = HEIGHT-1; y = k;
	cmp	REG_IN,#HEIGHT		// check max. Y coordinate
	blt	2f			// start Y is OK
	movs	REG_IN,#(HEIGHT-1)	// limit start Y to HEIGHT-1
2:	mov	REG_Y,REG_IN		// set start Y

	// receive high byte of stop Y
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc		// invalid data, command received

	// receive low byte of stop Y
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB		// invalid data, command received

	// compose stop Y
	// ... k = (k << 8) | k2; k++;
	lsls	REG_IN,#8		// shift high byte 8 bits left
	orrs	REG_IN,REG_IN2		// compose value
	adds	REG_IN,#1		// stop Y + 1

	// limit stop Y
	// ... if (k > HEIGHT) k = HEIGHT;
	cmp	REG_IN,#HEIGHT		// check max. Y coordinate
	ble	2f			// stop Y is OK
	movs	REG_IN,#HEIGHT		// limit stop Y to HEIGHT

	// set height
	// ... h = k - y; if (h < 1) h = 1;
2:	mov	REG_IN2,REG_Y		// get start Y
	subs	REG_IN,REG_IN,REG_IN2	// height
	bgt	3f			// height is OK > 1
	movs	REG_IN,#1		// minimal height = 1
3:	mov	REG_H,REG_IN		// set height
	b	Grabber_Wait		// get next command

Grabber_NoY:

// ==== set memory mode

	// ... if (k == ST7789_MADCTL)
	cmp	REG_IN,#ST7789_MADCTL	// check command
	bne	Grabber_NoMadCtrl

	// receive command parameter
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc		// invalid data, command received

	// set BGR mode
	// DispBGR = ((k & ST7789_MADCTL_RGB) != 0);
	//#define ST7789_MADCTL_RGB 0x08	// B3: RGB/BGR order (0=RGB, 1=BGR)
	lsrs	REG_IN,#3		// get bit 3: 1=use BGR
	movs	REG_IN2,#1		// bit mask
	ands	REG_IN,REG_IN2		// mask flag bit
	ldr	REG_IN2,=DispBGR	// BGR flag variable
	strb	REG_IN,[REG_IN2,#0]	// store BGR flag
	b	Grabber_Wait		// get next command

Grabber_NoMadCtrl:

// ==== set RAM control

	// ... if (k == ST7789_RAMCTRL)
	cmp	REG_IN,#ST7789_RAMCTRL	// check command
	bne	Grabber_NoRamCtrl

	// receive 1st parameter
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
Grabber_Proc2:
	bgt	Grabber_Proc		// invalid data, command received

	// receive 2nd parameter
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
Grabber_ProcB2:
	bgt	Grabber_ProcB		// invalid data, command received

	// use Little Endian data (LSB first)
	// DispLittleEndian = ((k2 & B3) != 0);
	lsrs	REG_IN2,#3		// get bit 3: 1=use Little Endian
	movs	REG_IN,#1		// bit mask
	ands	REG_IN2,REG_IN		// mask flag bit
	ldr	REG_IN,=DispLittleEndian // Little Endian flag variable
	strb	REG_IN2,[REG_IN,#0]	// store Little Endian flag
Grabber_Wait2:
	b	Grabber_Wait		// get next command

Grabber_NoRamCtrl:

// ==== start write data to draw window

	// ... if (k == ST7789_RAMWR)
	cmp	REG_IN,#ST7789_RAMWR	// check command
	bne	Grabber_Wait2

	// destination address
	// ... d = &FrameBuf[x + y*WIDTH];
	mov	REG_IN,REG_Y		// start Y
	muls	REG_IN,REG_WIDTH	// y*WIDTH
	mov	REG_IN2,REG_X		// start X
	add	REG_IN,REG_IN2		// offset in FrameBuf in pixels
	lsls	REG_IN,REG_IN,#1	// offset in FrameBuf in bytes
	ldr	REG_IN2,=FrameBuf	// pointer to frame buffer
	adds	REG_DST,REG_IN,REG_IN2	// destination pointer

	// row counter
	// ... for (i = h; i > 0; i--)
	mov	REG_LOOPY,REG_H		// init row counter

	// use Little Endian data (LSB first)
	// if (DispLittleEndian)
	ldr	REG_IN2,=DispLittleEndian // Little Endian flag variable
	ldrb	REG_IN,[REG_IN2,#0]	// load the flag
	cmp	REG_IN,#0		// use Big Endian?
	beq	Grabber_NoLE		// not using Little Endian

// using Little Endian (LSB first)

	// load data in BGR format, Little Endian
	// if (DispBGR)
	ldr	REG_IN2,=DispBGR	// BGR flag variable
	ldrb	REG_IN,[REG_IN2,#0]	// load the flag
	cmp	REG_IN,#0		// use BGR format?
	beq	Grabber_LERGB		// not using BGR format

// using Little Endian (LSB first), BGR format

	// column counter	
	// ... for (j = w; j > 0; j--)
2:	mov	REG_LOOPX,REG_W		// init column counter

	// receive low byte of data
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
4:	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc2		// invalid data, command received

	// receive high byte of data
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB2		// invalid data, command received

	// compose data word
	// k = (k | (k2 << 8));
	lsls	REG_IN2,#8		// shift high byte
	orrs	REG_IN,REG_IN2		// compose data word

	// convert BGR to RGB
	// k = (k << 11) | (k & 0x07e0) | (k >> 11);
	mov	REG_TMP,REG_IN		// save data word
	lsls	REG_IN,#11		// get RED
	mov	REG_IN2,REG_TMP		// get data
	lsrs	REG_IN2,#11		// get BLUE
	orrs	REG_IN,REG_IN2		// RED + BLUE
	mov	REG_IN2,REG_TMP		// get data
	lsls	REG_IN2,#21		// clear top 16+5 bits
	lsrs	REG_IN2,#26		// clear bottom bits (lefts valid 6 bits)
	lsls	REG_IN2,#5		// return green to its position
	orrs	REG_IN,REG_IN2		// add GREEN

	// store low byte of the pixel
	// ... ((u8*)d)[0] = (u8)k;
	strb	REG_IN,[REG_DST,#0]	// store 8-bit value

	// store high byte of the pixel
	// ... ((u8*)d)[1] = (u8)(k >> 8);
	lsrs	REG_IN,#8		// shift data
	strb	REG_IN,[REG_DST,#1]	// store 8-bit value

	// increase pointer
	// ... d++;
	adds	REG_DST,#2		// shift pointer

	// column counter
	// ... j--
	subs	REG_LOOPX,#1		// decrement loop counter
	bne	4b			// next pixel

	// shift destination address to new row
	// ... d += wb
	add	REG_DST,REG_WB		// shift destination pointer

	// row counter
	// ... i--
	subs	REG_LOOPY,#1		// decrement loop counter
	bne	2b			// next row
	b	Grabber_Wait		// get next command

Grabber_LERGB:

// using Little Endian (LSB first), RGB format

	// column counter	
	// ... for (j = w; j > 0; j--)
2:	mov	REG_LOOPX,REG_W		// init column counter

	// receive low byte of data
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
4:	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc2		// invalid data, command received

	// store low byte of the pixel
	// ... ((u8*)d)[0] = (u8)k;
	strb	REG_IN,[REG_DST,#0]	// store 8-bit value

	// receive high byte of data
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB2		// invalid data, command received

	// store high byte of the pixel
	// ... ((u8*)d)[1] = (u8)k2;
	strb	REG_IN2,[REG_DST,#1]	// store 8-bit value

	// increase pointer
	// ... d++;
	adds	REG_DST,#2		// shift pointer

	// column counter
	// ... j--
	subs	REG_LOOPX,#1		// decrement loop counter
	bne	4b			// next pixel

	// shift destination address to new row
	// ... d += wb
	add	REG_DST,REG_WB		// shift destination pointer

	// row counter
	// ... i--
	subs	REG_LOOPY,#1		// decrement loop counter
	bne	2b			// next row
	b	Grabber_Wait		// get next command

Grabber_NoLE:

// using Big Endian (MSB first)

	// load data in BGR format, Big Endian
	// if (DispBGR)
	ldr	REG_IN2,=DispBGR	// BGR flag variable
	ldrb	REG_IN,[REG_IN2,#0]	// load the flag
	cmp	REG_IN,#0		// use BGR format?
	beq	Grabber_NoLERGB		// not using BGR format

// using Big Endian (MSB first), BGR format

	// column counter	
	// ... for (j = w; j > 0; j--)
2:	mov	REG_LOOPX,REG_W		// init column counter

	// receive high byte of data
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
4:	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
Grabber_ProcB3:
	bgt	Grabber_ProcB2		// invalid data, command received

	// receive low byte of data
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
Grabber_Proc3:
	bgt	Grabber_Proc2		// invalid data, command received

	// compose data word
	// k = (k | (k2 << 8));
	lsls	REG_IN2,#8		// shift high byte
	orrs	REG_IN,REG_IN2		// compose data word

	// convert BGR to RGB
	// k = (k << 11) | (k & 0x07e0) | (k >> 11);
	mov	REG_TMP,REG_IN		// save data word
	lsls	REG_IN,#11		// get RED
	mov	REG_IN2,REG_TMP		// get data
	lsrs	REG_IN2,#11		// get BLUE
	orrs	REG_IN,REG_IN2		// RED + BLUE
	mov	REG_IN2,REG_TMP		// get data
	lsls	REG_IN2,#21		// clear top 16+5 bits
	lsrs	REG_IN2,#26		// clear bottom bits (lefts valid 6 bits)
	lsls	REG_IN2,#5		// return green to its position
	orrs	REG_IN,REG_IN2		// add GREEN

	// store low byte of the pixel
	// ... ((u8*)d)[0] = (u8)k;
	strb	REG_IN,[REG_DST,#0]	// store 8-bit value

	// store high byte of the pixel
	// ... ((u8*)d)[1] = (u8)(k >> 8);
	lsrs	REG_IN,#8		// shift data
	strb	REG_IN,[REG_DST,#1]	// store 8-bit value

	// increase pointer
	// ... d++;
	adds	REG_DST,#2		// shift pointer

	// column counter
	// ... j--
	subs	REG_LOOPX,#1		// decrement loop counter
	bne	4b			// next pixel

	// shift destination address to new row
	// ... d += wb
	add	REG_DST,REG_WB		// shift destination pointer

	// row counter
	// ... i--
	subs	REG_LOOPY,#1		// decrement loop counter
	bne	2b			// next row
	b	Grabber_Wait		// get next command

Grabber_NoLERGB:

// using Big Endian (MSB first), RGB format

	// column counter	
	// ... for (j = w; j > 0; j--)
2:	mov	REG_LOOPX,REG_W		// init column counter

	// receive high byte of data
	// ... k2 = Proc_PioReadWait(); if (k2 >= 0x100) goto COMMAND;
4:	recv	REG_IN2			// receive data from PIO
	cmp	REG_IN2,#255		// check command byte
	bgt	Grabber_ProcB3		// invalid data, command received

	// store high byte of the pixel
	// ... ((u8*)d)[0] = (u8)k2;
	strb	REG_IN2,[REG_DST,#1]	// store 8-bit value

	// receive high byte of data
	// ... k = Proc_PioReadWait(); if (k >= 0x100) goto COMMAND;
	recv	REG_IN			// receive data from PIO
	cmp	REG_IN,#255		// check command byte
	bgt	Grabber_Proc3		// invalid data, command received

	// store high byte of the pixel
	// ... ((u8*)d)[1] = (u8)k;
	strb	REG_IN,[REG_DST,#0]	// store 8-bit value

	// increase pointer
	// ... d++;
	adds	REG_DST,#2		// shift pointer

	// column counter
	// ... j--
	subs	REG_LOOPX,#1		// decrement loop counter
	bne	4b			// next pixel

	// shift destination address to new row
	// ... d += wb
	add	REG_DST,REG_WB		// shift destination pointer

	// row counter
	// ... i--
	subs	REG_LOOPY,#1		// decrement loop counter
	bne	2b			// next row
	b	Grabber_Wait		// get next command

// this function never returns
#if 0

// pop registers R8..R11
	pop	{r4-r7}			// pop registers R8..R11
	mov	r8,r4
	mov	r9,r5
	mov	r10,r6
	mov	r11,r7

	// pop registers R4..R7 and return
	pop	{r4-r7,pc}		// pop registers R4..R7 and return

#endif
