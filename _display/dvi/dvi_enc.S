
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

.extern TmdsTable	// TMDS table

#define SIO_BASE		0xd0000000	// SIO registers (does not support aliases for atomic access!)

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
#define INTERP1         (SIO_INTERP1_ACCUM0_OFFSET     - SIO_INTERP0_ACCUM0_OFFSET) // = 64

	.syntax unified
	.section .time_critical.DviEnc, "ax"
	.cpu cortex-m0plus
	.thumb			// use 16-bit instructions

// extern "C" void DviEncRG(u16* inbuf, u32* outbufR, u32* outbufG, int count);

// decode scanline data - Red and Green components
//  R0: inbuf ... input buffer (u16), must be u32 aligned
//  R1: outbufR ... output buffer, red component (u32), interpolator 0
//  R2: outbufG ... output buffer, green component (u32), interpolator 1
//  R3: count ... number of pixels (must be multiply of 16)

// Local registers:
#define REG_SRC		R0		// source pointer
#define REG_DST_R	R1		// destination pointer, red component
#define REG_DST_G	R2		// destination pointer, green component
#define REG_BASE	R3		// interpolator 0 base address
#define REG_TMP1	R4		// temporary 1, input and output
#define REG_TMP2	R5		// temporary 2, output
#define REG_TMP3	R6		// temporary 3, input and output
#define REG_TMP4	R7		// temporary 4, output
#define REG_END		IP		// end source pointer

.thumb_func
.global DviEncRG
DviEncRG:

	// push registers
	push	{r4-r7,lr}

	// prepare registers
	lsls	r3,#1					// convert number of pixels to number of bytes in source buffer
	add	r3,r0					// R3 <- end of source buffer
	mov	REG_END,r3				// REG_END <- end of source buffer
	ldr	REG_BASE,=(SIO_BASE+SIO_INTERP0_ACCUM0_OFFSET) // pointer to interpolator 0

.align 2						// to optimise loop

// 42*4+2=170 clock cycles per 16 pixels, total 170*20=3400 clock cycles ... on 252 MHz = 3400/252=13.5 us

DviEncRG_Loop:

.rept	4	// repeat loop to minimise weight of "loop" command

// ---- [42] encode 4 pixels to 8 symbols, both red and green components

	// [3] load 4 pixels
	ldmia	REG_SRC!,{REG_TMP1, REG_TMP3}		// [3] load 4 pixels

	// [2] feed both interpolators from first 2 pixels
	str	REG_TMP1,[REG_BASE,#ACCUM0_OFFS]	// [1] save 2 pixels to accumulator 0, interpolator 0 (red)
	str	REG_TMP1,[REG_BASE,#(ACCUM0_OFFS+INTERP1)] // [1] save 2 pixels to accumulator 0, interpolator 1 (green)
							// - accumulator 0 is also fed to the lane 1
	// [3] convert 1st pixel, red component
	ldr	REG_TMP1,[REG_BASE,#PEEK0_OFFS]		// [1] load pointer to table of first 2 symbols
	ldr	REG_TMP1,[REG_TMP1]			// [2] load first 2 symbols from the table

	// [3] convert 2nd pixel, red component
	ldr	REG_TMP2,[REG_BASE,#PEEK1_OFFS]		// [1] load pointer to table of next 2 symbols
	ldr	REG_TMP2,[REG_TMP2]			// [2] load next 2 symbols from the table

	// [3] store result 4 symbols, red component
	stmia	REG_DST_R!,{REG_TMP1, REG_TMP2}		// [3] store result 4 symbols R4, R5 -> [R1++]

	// [3] convert 1st pixel, green component
	ldr	REG_TMP1,[REG_BASE,#(PEEK0_OFFS+INTERP1)] // [1] load pointer to table of first 2 symbols
	ldr	REG_TMP1,[REG_TMP1]			// [2] load first 2 symbols from the table

	// [3] convert 2nd pixel, green component
	ldr	REG_TMP2,[REG_BASE,#(PEEK1_OFFS+INTERP1)] // [1] load pointer to table of next 2 symbols
	ldr	REG_TMP2,[REG_TMP2]			// [2] load next 2 symbols from the table

	// [2] feed both interpolators from next 2 pixels
	str	REG_TMP3,[REG_BASE,#ACCUM0_OFFS]	// [1] save 2 pixels to accumulator 0, interpolator 0 (red)
	str	REG_TMP3,[REG_BASE,#(ACCUM0_OFFS+INTERP1)] // [1] save 2 pixels to accumulator 0, interpolator 1 (green)
							// - accumulator 0 is also fed to the lane 1
	// [3] convert 3rd pixel, red component
	ldr	REG_TMP3,[REG_BASE,#PEEK0_OFFS]		// [1] load pointer to table of first 2 symbols
	ldr	REG_TMP3,[REG_TMP3]			// [2] load first 2 symbols from the table

	// [3] convert 4th pixel, red component
	ldr	REG_TMP4,[REG_BASE,#PEEK1_OFFS]		// [1] load pointer to table of next 2 symbols
	ldr	REG_TMP4,[REG_TMP4]			// [2] load next 2 symbols from the table

	// [3] store result 4 symbols, red component
	stmia	REG_DST_R!,{REG_TMP3, REG_TMP4}		// [3] store result 4 symbols R6, R7 -> [R1++]

	// [3] convert 3rd pixel, green component
	ldr	REG_TMP3,[REG_BASE,#(PEEK0_OFFS+INTERP1)] // [1] load pointer to table of first 2 symbols
	ldr	REG_TMP3,[REG_TMP3]			// [2] load first 2 symbols from the table

	// [3] convert 4th pixel, green component
	ldr	REG_TMP4,[REG_BASE,#(PEEK1_OFFS+INTERP1)] // [1] load pointer to table of next 2 symbols
	ldr	REG_TMP4,[REG_TMP4]			// [2] load next 2 symbols from the table

	// [5] store result 8 symbols, green component
	stmia	REG_DST_G!,{REG_TMP1, REG_TMP2, REG_TMP3, REG_TMP4} // [5] store result 8 symbols
.endr

DviEncRG_Cont:

	// [2,3] continue loop
	cmp	REG_SRC,REG_END				// [1] end of data?
	bne	DviEncRG_Loop				// [1,2]

	// pop registers
	pop	{r4-r7,pc}

#undef REG_SRC				// source pointer
#undef REG_DST_R			// destination pointer, red component
#undef REG_DST_G			// destination pointer, green component
#undef REG_BASE				// interpolator 0 base address
#undef REG_TMP1				// temporary 1, input and output
#undef REG_TMP2				// temporary 2, output
#undef REG_TMP3				// temporary 3, input and output
#undef REG_TMP4				// temporary 4, output
#undef REG_END				// end source pointer

// extern "C" void DviEncB(u16* inbuf, u32* outbuf, int count);

// decode scanline data - Blue component
//  R0: inbuf ... input buffer (u16), must be u32 aligned
//  R1: outbuf ... output buffer, blue component (u32)
//  R2: count ... number of pixels (must be multiply of 16)

// Local registers:
#define REG_SRC		R0		// source pointer
#define REG_DST		R1		// destination pointer, red component
#define REG_TAB		R2		// TMDS table
#define REG_MASK	R3		// blue component mask
#define REG_TMP1	R4		// temporary 1, input and output
#define REG_TMP2	R5		// temporary 2, output
#define REG_TMP3	R6		// temporary 3, input and output
#define REG_TMP4	R7		// temporary 4, output
#define REG_END		IP		// end source pointer

.thumb_func
.global DviEncB
DviEncB:

	// push registers
	push	{r4-r7,lr}

	// prepare registers
	lsls	r2,#1					// convert number of pixels to number of bytes in source buffer
	add	r2,r0					// R2 <- end of source buffer
	mov	REG_END,r2				// REGB_END <- end of source buffer
	movs	REG_MASK,#0xf8				// blue component mask
	ldr	REG_TAB,=TmdsTable			// table

.align 2						// to optimise loop

// 24*4+2=98 clock cycles per 16 pixels, total 98*20=1960 clock cycles ... on 252 MHz = 1960/252=7.8 us

DviEncB_Loop:

.rept	4	// repeat loop to minimise weight of "loop" command

// ---- [24] encode 4 pixels to 8 symbols, both red and green components

	// [3] load 4 pixels
	ldmia	REG_SRC!,{REG_TMP2, REG_TMP4}	// [3] load 4 pixels

	// [4]
	lsls	REG_TMP1,REG_TMP2,#3		// [1]
	ands	REG_TMP1,REG_MASK		// [1]
	ldr	REG_TMP1,[REG_TAB,REG_TMP1]	// [2] convert
	
	// [4]
	lsrs	REG_TMP2,REG_TMP2,#13		// [1]
	ands	REG_TMP2,REG_MASK		// [1]
	ldr	REG_TMP2,[REG_TAB,REG_TMP2]	// [2] convert

	// [4]
	lsls	REG_TMP3,REG_TMP4,#3		// [1]
	ands	REG_TMP3,REG_MASK		// [1]
	ldr	REG_TMP3,[REG_TAB,REG_TMP3]	// [2] convert
	
	// [4]
	lsrs	REG_TMP4,REG_TMP4,#13		// [1]
	ands	REG_TMP4,REG_MASK		// [1]
	ldr	REG_TMP4,[REG_TAB,REG_TMP4]	// [2] convert

	// [5] store result 8 symbols
	stmia	REG_DST!,{REG_TMP1, REG_TMP2, REG_TMP3, REG_TMP4} // [5] store result 8 symbols
.endr

DviEncB_Cont:

	// [2,3] continue loop
	cmp	REG_SRC,REG_END				// [1] end of data?
	bne	DviEncB_Loop				// [1,2]

	// pop registers
	pop	{r4-r7,pc}
