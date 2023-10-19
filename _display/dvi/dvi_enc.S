
// ****************************************************************************
//
//                             DVI TMDS encoder
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

#define SIO_BASE		0xd0000000	// SIO registers (does not support aliases for atomic access!)

#define DVI_INTERP	0	// interpolator used to encode pixels

#define SIO_INTERP0_ACCUM0_OFFSET	0x00000080
#define SIO_INTERP0_ACCUM1_OFFSET	0x00000084
#define SIO_INTERP0_ACCUM1_ADD_OFFSET	0x000000b8
#define SIO_INTERP0_PEEK_LANE0_OFFSET	0x000000a0
#define SIO_INTERP0_PEEK_LANE1_OFFSET	0x000000a4
#define SIO_INTERP0_PEEK_FULL_OFFSET	0x000000a8
#define SIO_INTERP1_ACCUM0_OFFSET	0x000000c0

#define ACCUM0_OFFS     (SIO_INTERP0_ACCUM0_OFFSET     - SIO_INTERP0_ACCUM0_OFFSET)
#define ACCUM1_OFFS     (SIO_INTERP0_ACCUM1_OFFSET     - SIO_INTERP0_ACCUM0_OFFSET)
#define ACCUM1_ADD_OFFS (SIO_INTERP0_ACCUM1_ADD_OFFSET - SIO_INTERP0_ACCUM0_OFFSET)
#define PEEK0_OFFS      (SIO_INTERP0_PEEK_LANE0_OFFSET - SIO_INTERP0_ACCUM0_OFFSET)
#define PEEK1_OFFS      (SIO_INTERP0_PEEK_LANE1_OFFSET - SIO_INTERP0_ACCUM0_OFFSET)
#define PEEK2_OFFS      (SIO_INTERP0_PEEK_FULL_OFFSET  - SIO_INTERP0_ACCUM0_OFFSET)
//#define INTERP1         (SIO_INTERP1_ACCUM0_OFFSET     - SIO_INTERP0_ACCUM0_OFFSET)

// macro - process two 16-bit pixels
//  r_ibase ... base register of interpolator
//  r_inout0 ... input 2 pixels, output first 2 symbols
//  r_out1 ... output next 2 symbols

.macro do_channel_16bpp r_ibase r_inout0 r_out1
	str	\r_inout0,[\r_ibase,#ACCUM0_OFFS]		// save 2 pixels to accumulator 0
								// - accumulator 0 is also fed to the lane 1
	ldr	\r_inout0,[\r_ibase,#PEEK0_OFFS]		// load pointer to table of first 2 symbols
	ldr	\r_inout0,[\r_inout0]				// load first 2 symbols from the table
	ldr	\r_out1,[\r_ibase,#PEEK1_OFFS]			// load pointer to table of next 2 symbols
	ldr	\r_out1,[\r_out1]				// load next 2 symbols from the table
.endm

	.syntax unified
	.section .time_critical.DviEnc, "ax"
	.cpu cortex-m0plus
	.thumb			// use 16-bit instructions


// extern "C" u32* DviEnc(u16* inbuf, u32* outbuf, int count);

// decode scanline data
//  R0: inbuf ... input buffer (u16), must be u32 aligned
//  R1: outbuf ... output buffer (u32)
//  R2: count ... number of pixels (must be multiply of 8)

.thumb_func
.global DviEnc
DviEnc:

	// push registers
	push	{r4-r7,lr}

	// prepare registers
	lsls	r2,#2		// convert number of pixels to number of bytes
	add	r2,r1		// R2 <- end of output buffer
	mov	ip,r2		// IP <- end of output buffer
	ldr	r2,=(SIO_BASE+SIO_INTERP0_ACCUM0_OFFSET) // pointer to interpolator 0
	b	2f		// start encodinf

.align 2			// to optimise loop

	// encode 4 pixels to 8 symbols
1:	ldmia	r0!,{r4, r6}	// load 4 pixels from [R0++] -> R4, R6
	do_channel_16bpp r2,r4,r5 // process first 2 pixels -> symbols R4, R5
	do_channel_16bpp r2,r6,r7 // process next 2 pixels -> symbols R6, R7
	stmia	r1!,{r4-r7}	// store result 8 symbols R4, R5, R6, R7 -> [R1++]

	// encode 4 pixels to 8 symbols
	ldmia	r0!,{r4, r6}	// load 4 pixels from [R0++] -> R4, R6
	do_channel_16bpp r2,r4,r5 // process first 2 pixels -> symbols R4, R5
	do_channel_16bpp r2,r6,r7 // process next 2 pixels -> symbols R6, R7
	stmia	r1!,{r4-r7}	// store result 8 symbols R4, R5, R6, R7 -> [R1++]

	// continue loop
2:	cmp	r1,ip		// end of data?
	bne	1b

	// pop registers
	pop	{r4-r7,pc}


// extern "C" u32* DviEncShift(u16* inbuf, u32* outbuf, int count);

// decode scanline data with pre-shift pixel 3 bits left (used for blue channel)
//  R0: inbuf ... input buffer (u16), must be u32 aligned
//  R1: outbuf ... output buffer (u32)
//  R2: count ... number of pixels (must be multiply of 8)

.thumb_func
.global DviEncShift
DviEncShift:

	// push registers
	push	{r4-r7,lr}

	// prepare registers
	lsls	r2,#2		// convert number of pixels to number of bytes
	add	r2,r1		// R2 <- end of output buffer
	mov	ip,r2		// IP <- end of output buffer
	ldr	r2,=(SIO_BASE+SIO_INTERP0_ACCUM0_OFFSET) // pointer to interpolator 0
	b	2f		// start encodinf

.align 2			// to optimise loop

	// encode 4 pixels to 8 symbols
1:	ldmia	r0!,{r4, r6}	// load 4 pixels from [R0++] -> R4, R6
	lsls	r4,r4,#3	// pre-shift 3 bits left
	do_channel_16bpp r2,r4,r5 // process first 2 pixels -> symbols R4, R5
	lsls	r6,r6,#3	// pre-shift 3 bits left
	do_channel_16bpp r2,r6,r7 // process next 2 pixels -> symbols R6, R7
	stmia	r1!,{r4-r7}	// store result 8 symbols R4, R5, R6, R7 -> [R1++]

	// encode 4 pixels to 8 symbols
	ldmia	r0!,{r4, r6}	// load 4 pixels from [R0++] -> R4, R6
	lsls	r4,r4,#3	// pre-shift 3 bits left
	do_channel_16bpp r2,r4,r5 // process first 2 pixels -> symbols R4, R5
	lsls	r6,r6,#3	// pre-shift 3 bits left
	do_channel_16bpp r2,r6,r7 // process next 2 pixels -> symbols R6, R7
	stmia	r1!,{r4-r7}	// store result 8 symbols R4, R5, R6, R7 -> [R1++]

	// continue loop
2:	cmp	r1,ip		// end of data?
	bne	1b

	// pop registers
	pop	{r4-r7,pc}
