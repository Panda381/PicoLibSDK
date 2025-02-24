// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                         HSTX DVI driver - ARM assembler
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

// ASM notes:
//  - scratch registers R0..R3, R12 (IP)
//  - save registers R4-R9, R10 (SL), R11 (FP), R14 (LR)
//  - u32 instructions should be aligned to u32
//  - after ldr insert another instruction before using the data
//  - align functions and loops to 16-byte boundary due cache-line

#include "config.h"			// project configuration
#include "_config.h"			// default configuration

#if USE_DISPHSTX		// 1=use HSTX Display driver

#if DISPHSTX_ARM_ASM && !RISCV && DISPHSTX_USE_DVI	// 1=use DVI (HDMI) support (0=or only VGA)

	.section .time_critical.hstx.dvi, "ax"
	.syntax unified
	.cpu cortex-m33
	.thumb
	.p2align 2,,

// Structures
// - Structure definitions must match definitions in file disphstx_vmode.h

#define DISPHSTXSLOT_CURPOS		0x02	// offset of u8 curpos ... text cursor position
#define DISPHSTXSLOT_CURROW		0x03	// offset of u8 currow ... text cursor row
#define DISPHSTXSLOT_CURBEG		0x04	// offset of u8 curbeg ... text cursor begin line
#define DISPHSTXSLOT_CUREND		0x05	// offset of u8 curend ... text cursor end line
#define DISPHSTXSLOT_CURSPEED		0x06	// offset of u8 curspeed ... text cursor speed - number of shifts of [us] timer

#define DISPHSTXSLOT_FONTH		0x0A	// offset of s16 fonth ... height of font or tile in graphics lines
#define DISPHSTXSLOT_W			0x12	// offset of u16 sDispHstxVSlot.w ... width of slot in graphics pixels
#define DISPHSTXSLOT_PITCH		0x14	// offset of u16 sDispHstxVSlot.pitch ... length of line or row in bytes
#define DISPHSTXSLOT_BUF		0x20	// offset of u8* sDispHstxVSlot.buf ... frame buffer base address
#define DISPHSTXSLOT_PAL		0x24	// offset of u16* sDrvVSlot.pal ... pointer to palettes in RGB565 format
#define DISPHSTXSLOT_FONT		0x2C	// offset of const u8* font ... pointer to font (width 8 pixels)

#define TIMER0_BASE			0x400b0000	// timer 0 base
#define TIMER0_RAWL			0x28		// timer 0 - raw read LOW offset

// ----------------------------------------------------------------------------
//                          do-nothing render
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_No(sDispHstxVSlot* slot, int line, u32* cmd);
// 4 bytes

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_No
DispHstxDviRender_No:
	bx	lr

// ----------------------------------------------------------------------------
//                default render - read from frame buffer
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Default(sDispHstxVSlot* slot, int line, u32* cmd);
// 16 bytes

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_Default
DispHstxDviRender_Default:
	ldrh.w	ip,[r0,#DISPHSTXSLOT_PITCH]	// IP <- load pitch of the line
	ldr	r3,[r0,#DISPHSTXSLOT_BUF]	// R3 <- load buffer base address
	mla	r3,r1,ip,r3			// R3 <- line*pitch+base
	str	r3,[r2,#0]			// save data address to the command buffer
	bx	lr

#if DISPHSTX_USE_FORMAT_12
// ----------------------------------------------------------------------------
//                           12 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_12(sDispHstxVSlot* slot, int line, u32* cmd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_12
DispHstxDviRender_12:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare width of the slot -> R6
	ldrh	r6,[r0,#DISPHSTXSLOT_W]		// R6 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 8 pixels)

	// prepare counter of big loop (by 8 pixels) -> R4
	lsrs	r4,r6,#3			// R4 <- width / 8
	beq	5f				// skip if width/8 is 0

// Here is:
//  R0 = temporary register
//  R1 = temporary register 2
//  R2 = sample 1 or 3
//  R3 = sample 2
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = width of the slot
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

//	[2222 2222 1111 1111 1111 0000 0000 0000]
//	[5555 4444 4444 4444 3333 3333 3333 2222]
//	[7777 7777 7777 6666 6666 6666 5555 5555]

	// load 2 samples
2:	ldrd	r2,r3,[ip],#3*4		// R2 <- [IP], R3 <- [IP+4], load 2 samples, shift source address

	// get pixels 0+1 -> R2
	lsrs	r0,r2,#12		// R0 <- pixel 1
	lsrs	r1,r2,#24		// R1 <- pixel 2, bits 0-7
	bfi	r2,r0,#16,#12		// R2 <- pixels 0 + 1

	// get pixels 2+3 -> R1
	bfi	r1,r3,#8,#4		// R1 <- pixel 2
	lsrs	r0,r3,#4		// R0 <- pixel 3
	bfi	r1,r0,#16,#12		// R1 <- pixels 2 + 3
	strd	r2,r1,[r5]		// store pixels 0-3

	// get pixels 4+5 -> R0
	ldr	r2,[ip,#-4]		// R2 <- [IP+8 -12] sample 3
	lsrs	r0,r3,#16		// R0 <- pixel 4
	lsrs	r3,r3,#28		// R3 <- pixel 5, bits 0-3
	bfi	r0,r3,#16,#4		// R0 <- add pixel 5, bits 0-3
	bfi	r0,r2,#20,#8		// R0 <- pixels 4 + 5

	// get pixels 6+7 -> R2
	lsrs	r1,r2,#20		// R1 <- pixel 7
	lsrs	r2,r2,#8		// R2 <- pixel 6
	bfi	r2,r1,#16,#12		// R1 <- pixels 6 + 7
	strd	r0,r2,[r5,#4*2]		// store pixels 4-7

	// loop counter
	adds	r5,#8*2			// shift destination address
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r6,#0x07		// R4 <- width low 3 bits
	beq	8f			// skip if width & 0x07 is 0

	// load 2 samples
	ldrd	r2,r3,[ip],#3*4		// R2 <- [IP], R3 <- [IP+4], load 2 samples, shift source address

	// pixel 0
	str	r2,[r5]			// store pixel 0
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 1
	lsrs	r0,r2,#12		// R0 <- pixel 1
	str	r0,[r5,#1*2]		// store pixel 1
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 2
	lsrs	r2,r2,#24		// R2 <- pixel 2, bits 0..7
	bfi	r2,r3,#8,#4		// R2 <- pixel 2
	str	r2,[r5,#2*2]		// store pixel 2
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 3
	lsrs	r0,r3,#4		// R0 <- pixel 3
	str	r0,[r5,#3*2]		// store pixel 3
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 4
	lsrs	r0,r3,#16		// R0 <- pixel 4
	str	r0,[r5,#4*2]		// store pixel 4
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// load 3rd sample -> R2
	ldr	r2,[ip,#-4]		// R2 <- [IP+8]

	// pixel 5
	lsrs	r3,r3,#28		// R3 <- pixel 5, bits 0..3
	bfi	r3,r2,#4,#8		// R3 <- pixel 5
	str	r3,[r5,#5*2]		// store pixel 5
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 6
	lsrs	r0,r2,#8		// R0 <- pixel 6
	str	r0,[r5,#6*2]		// store pixel 6
	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 7
	lsrs	r2,r2,#20		// R0 <- pixel 7
	str	r2,[r5,#7*2]		// store pixel 7

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr

#endif

#if DISPHSTX_USE_FORMAT_1_PAL
// ----------------------------------------------------------------------------
//                    1 bit per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_1_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 1-bit double-pixel
//   dst ... destination index 0..14
//   bit ... source bit index 0..28
// R0 = temporary register
// R2 = source sample with 32 pixels
// R5 = destination buffer
// R6 = temporary register 2
// [SP] = palette mapping
.macro store1 dst bit
	ubfx	r0,r2,#\bit+2,#2	// R0 <- extract 2 bits from the sample
	ubfx	r6,r2,#\bit,#2		// R6 <- extract 2 bits from the sample
	ldr	r0,[sp,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldr	r6,[sp,r6,lsl #2]	// R6 <- mapping[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixels
	str	r6,[r5,#\dst*4+4]	// store pixels
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxDviRender_1_PAL
DispHstxDviRender_1_PAL:

	// push registers R4-R6
	push	{r4-r6}

	// reserve space in stack for local variables
	sub	sp,#16				// reserve space in stack for local variable: u32 map2[4]

	// prepare source address -> IP
	ldrh	r3,[r0,#DISPHSTXSLOT_PITCH]	// R3 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r3,r4			// IP <- line*pitch+base

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare palettes map2[4] in the stack
	ldrh	r0,[r1,#0]		// R0 <- color0
	ldrh	r2,[r1,#2]		// R2 <- color1
	add	r4,r0,r0,lsl #16	// R0 <- color0 | (color0 << 16)
	str	r4,[sp,#0*4]		// map2[0] <- color0 | (color0 << 16)
	add	r4,r0,r2,lsl #16	// R0 <- color0 | (color1 << 16)
	str	r4,[sp,#1*4]		// map2[1] <- color0 | (color1 << 16)
	add	r4,r2,r0,lsl #16	// R0 <- color1 | (color0 << 16)
	str	r4,[sp,#2*4]		// map2[2] <- color1 | (color0 << 16)
	add	r4,r2,r2,lsl #16	// R0 <- color1 | (color1 << 16)
	str	r4,[sp,#3*4]		// map2[3] <- color1 | (color1 << 16)

// ---- big loop (by 32 pixels)

	// prepare counter of big loop (by 32 pixels) -> R4
	lsrs	r4,r3,#5		// R4 <- width / 32
	beq	5f			// skip if width/32 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  IP = source buffer
//  [SP] = (map2) double-palettes

// byte 0, destination index 0..3, source bit 7..0

	store1	0,4
	store1	2,0

// byte 1, destination index 4..7, source bit 15..8

	store1	4,12
	store1	6,8

// byte 2, destination index 8..11, source bit 23..16

	store1	8,20
	store1	10,16

// byte 3, destination index 12..15, source bit 31..24

	store1	12,28
	store1	14,24

	// shift destination address
	adds	r5,#16*4		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r3,#0x1f		// R4 <- width low 5 bits
	beq	8f			// skip if width & 0x1f is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	.align 2,,			// align address to 4-byte boundary
	rev	r2,r2			// reverze byte order

	// process one pixel
6:	lsrs	r0,r2,#31		// R0 <- extract bit 31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	6b

	// restore stack
8:	add	sp,#16

	// pop registers R4-R6 and return
	pop	{r4-r6}
	bx	lr

#endif

#if DISPHSTX_USE_FORMAT_2_PAL
// ----------------------------------------------------------------------------
//                   2 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_2_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2-bit pixel
//   dst ... destination index 0..31
//   bit ... source 2-bit index 0..30
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 16 pixels
// R5 = destination buffer
.macro store2 dst bit
	ubfx	r0,r2,#\bit,#2		// R0 <- extract bits from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_2_PAL
DispHstxDviRender_2_PAL:

	// push registers R4-R5
	push	{r4,r5}

	// prepare source address -> IP
	ldrh	r3,[r0,#DISPHSTXSLOT_PITCH]	// R3 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r3,r4			// IP <- line*pitch+base

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

	// load next source word -> R2
2:	ldr	r2,[ip],#4			// R2 <- [IP], increment IP + 4

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

// byte 0, destination index 0..3, source bits 7..0

	store2	0,6
	store2	1,4
	store2	2,2

	// process pixel 3, bits 0+1
	and	r0,r2,#3		// R0 <- extract bits 0+1 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#3*2]		// store pixel

// byte 1, destination index 4..7, source bits 15..8

	store2	4,14
	store2	5,12
	store2	6,10
	store2	7,8

// byte 2, destination index 8..11, source bits 23..16

	store2	8,22
	store2	9,20
	store2	10,18
	store2	11,16

// byte 3, destination index 12..15, source bits 31..24

	// process pixel 12, bits 30+31
	lsrs	r0,r2,#30		// R0 <- extract bits 30+31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#12*2]		// store pixel

	store2	13,28
	store2	14,26
	store2	15,24

	// shift destination address
	adds	r5,#32			// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r3,#0x0f		// R4 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

	// load next source word -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	rev	r2,r2			// reverze byte order

	// process one pixel
6:	lsrs	r0,r2,#30		// R0 <- extract bits 30+31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R5 and return
8:	pop	{r4,r5}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_3_PAL
// ----------------------------------------------------------------------------
//                   3 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_3_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 3-bit pixel
//   dst ... destination index 0..19
//   bit ... NEXT source 3-bit index 0..27
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 10 pixels
// R5 = destination buffer
.macro store3 dst bit
	ldrh	r6,[r1,r0,lsl #1]
	ubfx	r0,r2,#\bit,#3
	strh	r6,[r5,#\dst*2]
.endm

.macro store3B dst bit
	ldrh	r6,[r1,r0,lsl #1]
	ubfx	r0,r7,#\bit,#3
	strh	r6,[r5,#\dst*2]
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w
	nop

.thumb_func
.global DispHstxDviRender_3_PAL
DispHstxDviRender_3_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// split width to big (-> R4) and small (-> R3) loop counter
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels
	ldr	r6,=0xcccccccd			// R6 <- divider = (1/20) << (4+32) round up = 3435973837 = 0xCCCCCCCD
	mov	r2,#-20				// R2 <- -20
	umull	r6,r4,r6,r3			// R6 <- LOW(width * divider), R4 <- HIGH(width * divider)
	lsrs	r4,r4,#4			// R4 <- big loop counter = width/20
	mla	r3,r2,r4,r3			// R3 <- small loop counter = width - width/20*20

// ---- big loop (by 20 pixels)

	beq	5f				// skip if width/20 is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 10 pixels
//  R3 = remaining width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = sample with next 10 pixels
//  IP = source buffer

	// load next source word -> R2
2:	ldrd	r2,r7,[ip]			// R2 <- [IP], R7 <- [IP+4]
	add	ip,#2*4				// shift source address

	and	r0,r2,#7			// R0 <- first pixel

	store3	0,3
	store3	1,6
	store3	2,9
	store3	3,12
	store3	4,15
	store3	5,18
	store3	6,21
	store3	7,24
	store3	8,27

	store3B	9,0
	store3B	10,3
	store3B	11,6
	store3B	12,9
	store3B	13,12
	store3B	14,15
	store3B	15,18
	store3B	16,21
	store3B	17,24
	store3B	18,27

	ldrh	r6,[r1,r0,lsl #1]
	adds	r5,#20*2			// R5 <- shift destination address
	strh	r6,[r5,19*2-40]

	// loop counter
	subs	r4,#1				// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// check remaining width
5:	cmp	r3,#0			// check small loop counter
	beq	8f			// skip if no pixels remain

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 10 pixels
//  R3 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  IP = source buffer

	// load next source word -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// check remaining width
	cmp	r3,#10			// check remaining width
	blt	6f			// skip if width < 10

	// middle loop - process 10 pixels
	and	r0,r2,#7		// R0 <- first pixel
	store3	0,3
	store3	1,6
	store3	2,9
	store3	3,12
	store3	4,15
	store3	5,18
	store3	6,21
	store3	7,24
	store3	8,27

	ldrh	r6,[r1,r0,lsl #1]
	subs	r3,#10			// subtract 10 pixels
	strh	r6,[r5,9*2]

	// shift middle loop
	beq	8f			// no pixels left
	adds	r5,#10*2		// R5 <- shift destination address
	ldr	r2,[ip,#4]		// R2 <- [IP+4]

	// process one pixel
6:	and	r0,r2,#7		// R0 <- extract lowest 3 bits
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	lsrs	r2,r2,#3		// shift sample rigth by 3 bits
	subs	r3,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_4_PAL
// ----------------------------------------------------------------------------
//                   4 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_4_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 4-bit pixel
//   dst ... destination index 0..31
//   bit ... NEXT source 4-bit index 0..28
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 8 pixels
// R5 = destination buffer
// R6 = temporary register 2
.macro store4 dst bit
	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	ubfx	r0,r2,#\bit,#4		// R0 <- extract bits from the sample
	strh	r6,[r5,#\dst*2-16]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.thumb_func
.global DispHstxDviRender_4_PAL
DispHstxDviRender_4_PAL:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r3,[r0,#DISPHSTXSLOT_PITCH]	// R3 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r3,r4			// IP <- line*pitch+base

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

// ---- big loop (by 8 pixels)

	// prepare counter of big loop (by 8 pixels) -> R4
	lsrs	r4,r3,#3			// R4 <- width / 8
	beq	5f				// skip if width/8 is 0

	// load next source word -> R2
2:	ldr	r2,[ip],#4			// R2 <- [IP], increment IP + 4
	adds	r5,#16				// R5 <- shift destination address
	subs	r4,#1				// decrement loop counter

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 8 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  IP = source buffer

	ubfx	r0,r2,#4,#4		// R0 <- first pixel

	store4	0,0
	store4	1,12
	store4	2,8
	store4	3,20
	store4	4,16
	store4	5,28
	store4	6,24

	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	strh	r6,[r5,#7*2-16]		// store pixel

	// loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r3,#7		// R4 <- width low 3 bits
	beq	8f			// skip if width & 7 is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 8 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

	// load next source word -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	rev	r2,r2			// reverze byte order

	// process one pixel
6:	lsrs	r0,r2,#28		// R0 <- extract bits 28-31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	lsls	r2,r2,#4		// shift sample left by 4 bits
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_6_PAL
// ----------------------------------------------------------------------------
//                           6 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_6_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 6-bit pixel
//   dst ... destination index 0..19
//   bit ... NEXT source 6-bit index 0..24
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 5 pixels
// R5 = destination buffer
// R6 = temporary register 2
// R7 = next source sample with 5 pixels
.macro store6 dst bit
	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	ubfx	r0,r2,#\bit,#6		// R0 <- extract bits from the sample
	strh	r6,[r5,#\dst*2]		// store pixel
.endm

.macro store6B dst bit
	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	ubfx	r0,r7,#\bit,#6		// R0 <- extract bits from the sample
	strh	r6,[r5,#\dst*2]		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_6_PAL
DispHstxDviRender_6_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// split width to big (-> R4) and small (-> R3) loop counter
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels
	ldr	r6,=0xcccccccd			// R6 <- divider = (1/20) << (4+32) round up = 3435973837 = 0xCCCCCCCD
	mov	r2,#-20				// R2 <- -20
	umull	r6,r4,r6,r3			// R6 <- LOW(width * divider), R4 <- HIGH(width * divider)
	lsrs	r4,r4,#4			// R4 <- big loop counter = width/20
	mla	r3,r2,r4,r3			// R3 <- small loop counter = width - width/20*20

// ---- big loop (by 20 pixels)

	beq	5f				// skip if width/20 is 0

	.align 2,,				// align address to 4-byte boundary

	// load next source word -> R2
2:	ldrd	r2,r7,[ip]			// R2 <- [IP], R7 <- [IP+4]
	add	ip,4*4				// IP <- shift source address

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 5 pixels
//  R3 = remaining width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = next source sample with 5 pixels
//  IP = source buffer

	and	r0,r2,0x3f			// R0 <- first pixel

	store6	0,6
	store6	1,12
	store6	2,18
	store6	3,24

	store6B	4,0

	store6B	5,6
	store6B	6,12
	store6B	7,18
	store6B	8,24

	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	ldrd	r2,r7,[ip,#-8]		// R2 <- [IP+8], R7 <- [IP+12]
	strh	r6,[r5,#9*2]		// store pixel

	and	r0,r2,0x3f		// R0 <- first pixel

	store6	10,6
	store6	11,12
	store6	12,18
	store6	13,24

	store6B	14,0

	store6B	15,6
	store6B	16,12
	store6B	17,18
	store6B	18,24

	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	adds	r5,#20*2		// R5 <- shift destination address
	strh	r6,[r5,#19*2-40]	// store pixel

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// check remaining width
5:	cmp	r3,#0			// check small loop counter
	beq	8f			// skip if no pixels remain

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 5 pixels
//  R3 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  IP = source buffer

	// load next source word -> R2
1:	ldr	r2,[ip]			// R2 <- [IP]

	// check remaining width
	cmp	r3,#5			// check remaining width
	blt	6f			// skip if width < 5

	// middle loop - process 5 pixels
	and	r0,r2,0x3f		// R0 <- first pixel

	store6	0,6
	store6	1,12
	store6	2,18
	store6	3,24

	ldrh	r6,[r1,r0,lsl #1]	// R6 <- mapping[bit << 1]
	add	ip,4			// IP <- shift source address
	strh	r6,[r5,#4*2]		// store pixel

	// shift middle loop
	adds	r5,#5*2			// R5 <- shift destination address
	subs	r3,#5			// subtract 5 pixels
	bne	1b
	b	8f

	// process one pixel
6:	and	r0,r2,#0x3f		// R0 <- extract lowest 6 bits
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	lsrs	r2,r2,#6		// shift sample rigth by 6 bits
	subs	r3,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_8_PAL
// ----------------------------------------------------------------------------
//                   8 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 8-bit pixel
//   dst ... destination index 0..31
//   bit ... source 8-bit index 0..24
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 4 pixels
// R5 = destination buffer
.macro store8 dst bit
	ubfx	r0,r2,#\bit,#8		// R0 <- extract bits from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
.endm

// R6 = 2nd source sample with next 4 pixels
.macro store8B dst bit
	ubfx	r0,r6,#\bit,#8		// R0 <- extract bits from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxDviRender_8_PAL
DispHstxDviRender_8_PAL:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r3,[r0,#DISPHSTXSLOT_PITCH]	// R3 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r3,r4			// IP <- line*pitch+base

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 4 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = 2nd sample with next 4 pixels
//  IP = source buffer

// sample 0, destination index 0..3

	// load next 2 samples
2:	ldrd	r2,r6,[ip,#0]		// R2 <- [IP], R6 <- [IP+4], load next 2 samples

	// process pixel 0, bits 0-7
	uxtb	r0,r2			// R0 <- extract bits 0-7 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#0*2]		// store pixel

	store8	1,8
	store8	2,16

	// process pixel 3, bits 24-31
	lsrs	r0,r2,#24		// R0 <- extract bits 24-31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#3*2]		// store pixel

// sample 1, destination index 4..7

	// process pixel 4, bits 0-7
	uxtb	r0,r6			// R0 <- extract bits 0-7 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#4*2]		// store pixel

	store8B	5,8
	store8B	6,16

	// process pixel 3, bits 24-31
	lsrs	r0,r6,#24		// R0 <- extract bits 24-31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#7*2]		// store pixel

// sample 2, destination index 8..11

	// load next 2 samples
	ldrd	r2,r6,[ip,#8]		// R2 <- [IP+8], R6 <- [IP+12], load next 2 samples

	// process pixel 8, bits 0-7
	uxtb	r0,r2			// R0 <- extract bits 0-7 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#8*2]		// store pixel

	store8	9,8
	store8	10,16

	// process pixel 11, bits 24-31
	lsrs	r0,r2,#24		// R0 <- extract bits 24-31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#11*2]		// store pixel

// sample 3, destination index 12..15

	// process pixel 12, bits 0-7
	uxtb	r0,r6			// R0 <- extract bits 0-7 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#12*2]		// store pixel

	store8B	13,8
	store8B	14,16

	// process pixel 15, bits 24-31
	lsrs	r0,r6,#24		// R0 <- extract bits 24-31 from the sample
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5,#15*2]		// store pixel

	// shift destination and source address
	adds	r5,#16*2		// R5 <- shift destination address
	adds	ip,#4*4			// IP <- shift source address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r3,#0x0f		// R4 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

	// process one pixel
6:	ldrb	r0,[ip],#1		// R0 <- [IP], load source byte
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	6b

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_COL
// ----------------------------------------------------------------------------
//        simple color (color RGB565 per line, index modulo fonth)
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_COL(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_COL
DispHstxDviRender_COL:
	ldrsh	r3,[r0,#DISPHSTXSLOT_FONTH]	// R3 <- height of font, color modulo
	ldr	r0,[r0,#DISPHSTXSLOT_PAL]	// R0 <- pointer to palettes in RGB565 format
	sdiv	ip,r1,r3			// IP <- line/fonth
	mls	r1,ip,r3,r1			// R1 <- line - (line/fonth)*fonth (= line % fonth)
	ldr	r2,[r2,#0]			// R2 <- destination address of render buffer
	ldrh	r3,[r0,r1,lsl #1]		// R3 <- load color
	strh	r3,[r2,#0]			// write pixel
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_MTEXT
// ----------------------------------------------------------------------------
// monocolor text, font width 8 pixels, colors of every row as 2 palette colors RGB565 (BG,FG)
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_MTEXT(sDispHstxVSlot* slot, int line, u32* cmd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_MTEXT
DispHstxDviRender_MTEXT:

	// push registers R4-R7
	push	{r4-r7,lr}

	// reserve space in stack for local variables
	sub	sp,#16				// reserve space in stack for local variable: u32 map2[4]

	// load font height -> R3
	ldrsh	r3,[r0,#DISPHSTXSLOT_FONTH]	// R3 fonth <- height of font

	// get row index -> R4, subline -> R1
	sdiv	r4,r1,r3			// R4 row <- line/fonth
	mls	r1,r3,r4,r1			// R1 subline <- line - row*fonth

	// prepare cursor position -> LR
	mov	lr,0				// LR <- invalid cursor
	ldrb	r3,[r0,#DISPHSTXSLOT_CURROW]	// R3 <- cursor row
	cmp	r3,r4				// check cursor row
	bne	1f				// not cursor row
	ldrb	r3,[r0,#DISPHSTXSLOT_CURBEG]	// R3 <- cursor begin
	cmp	r1,r3				// check cursor begin
	blt	1f				// not cursor
	ldrb	r3,[r0,#DISPHSTXSLOT_CUREND]	// R3 <- cursor end
	cmp	r1,r3				// check cursor end
	bgt	1f				// not cursor
	ldr	r3,=TIMER0_BASE			// R3 <- Timer0 base
	ldrb	r5,[r0,#DISPHSTXSLOT_CURSPEED]	// R5 <- cursor speed
	ldr	r3,[r3,#TIMER0_RAWL]		// R3 <- Timer LOW
	lsrs	r3,r5				// R3 <- shift timer
	lsls	r3,r3,#31			// R3 <- get bit 0
	bpl	1f				// bit 0 of the timer is not set
	ldrb	lr,[r0,#DISPHSTXSLOT_CURPOS]	// LR <- cursor position
	adds	lr,#1				// LR <- correction + 1

	// prepare pointer to font -> R3
1:	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 <- pointer to font
	add	r3,r3,r1,lsl #8			// R3 font <- font + subline*256

	// destination buffer -> R2
	ldr	r2,[r2]				// R2 dst <- destination buffer

	// prepare source pointer -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r1,[r0,#DISPHSTXSLOT_BUF]	// R1 src <- source buffer base address
	mla	ip,r4,r5,r1			// R1 src <- row*pitch + buf

	// prepare pointer to palettes -> A4
	ldr	r5,[r0,#DISPHSTXSLOT_PAL]	// R5 pal <- pointer to palettes in RGB565 format map[2]
	add	r4,r5,r4,lsl #2			// A4 map <- row*4+pal, pointer to entries in palette buffer

	// width -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 w <- width of slot in graphics pixels

	// prepare palettes map2[4] in the stack
	ldrh	r6,[r4,#0]			// R6 <- color0
	ldrh	r7,[r4,#2]			// R7 <- color1
	add	r0,r6,r6,lsl #16		// R0 <- color0 | (color0 << 16)
	str	r0,[sp,#0*4]			// map2[0] <- color0 | (color0 << 16)
	add	r0,r6,r7,lsl #16		// R0 <- color0 | (color1 << 16)
	str	r0,[sp,#1*4]			// map2[1] <- color0 | (color1 << 16)
	add	r0,r7,r6,lsl #16		// R0 <- color1 | (color0 << 16)
	str	r0,[sp,#2*4]			// map2[2] <- color1 | (color0 << 16)
	add	r0,r7,r7,lsl #16		// R0 <- color1 | (color1 << 16)
	str	r0,[sp,#3*4]			// map2[3] <- color1 | (color1 << 16)

// ---- big loop (by 8 pixels of 1 character)

	lsrs	r6,r5,#3			// R6 <- width / 8
	beq	5f				// skip if width/8 is 0

// Here is:
//  R0 = temporary
//  R1 = temporary 2
//  R2 = (dst) pointer to destination buffer
//  R3 = (font) pointer to line in the font
//  R4 = (map) pointer to palettes in RGB565 format
//  R5 = (w) width of slot in graphics pixels
//  R6 = (i) loop counter
//  R7 = (k) font sample with 8 pixels
//  IP = (src) pointer to source buffer
//  LR = (curpos) cursor position
//  [SP] = (map2) double-palettes

	.align 2,,			// align address to 4-byte boundary

	// load font sample -> R7
2:	ldrb	r7,[ip],#1		// R7 <- load source byte (character), shift source address
	subs	lr,#1			// LR <- decrement cursor position
	ldrb	r7,[r3,r7]		// R7 <- load font sample
	beq	9f			// skip if cursor

	// pixels 0+1, bits 6+7
3:	ubfx	r0,r7,#6,#2		// R0 <- extract bits 6+7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixels

	// pixels 2+3, bits 4+5
	ubfx	r1,r7,#4,#2		// R1 <- extract bits 4+5
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixels
	strd	r0,r1,[r2,#0]		// store pixels

	// pixels 4+5, bits 2+3
	ubfx	r0,r7,#2,#2		// R0 <- extract bits 2+3
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixels

	// pixels 6+7, bits 0+1
	and	r1,r7,#3		// R1 <- extract bits 0+1
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixels
	strd	r0,r1,[r2,#8]		// store pixels

	// shift destination address
	add	r2,#16			// R2 <- shift destination address

	// loop counter
	subs	r6,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

5:	ands	r6,r5,#7		// R6 <- width & 7, small loop counter
	beq	8f			// skip small loop (if loop counter = 0)

	// load font sample -> R7
	ldrb	r7,[ip]			// R7 <- load source byte (character)
	subs	lr,#1			// LR <- decrement cursor position
	ldrb	r7,[r3,r7]		// R7 <- load font sample
	beq	10f			// skip if cursor

// Here is:
//  R0 = temporary
//  R2 = (dst) pointer to destination buffer
//  R4 = (map) pointer to palettes in RGB565 format
//  R6 = (i) loop counter
//  R7 = (k) font sample with 8 pixels
//  LR = (curpos) cursor position

6:	ubfx	r0,r7,#7,#1		// R0 <- extract bit 7
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- load pixel, 
	lsls	r7,r7,#1		// R7 <- shift sample 1 bit left
	subs	r6,#1			// decrement loop counter
	strh	r0,[r2],#2		// store pixel, increment pointer
	bne	6b

	// restore stack
8:	add	sp,#16

	// pop registers R4-R7 and return
	pop	{r4-r7,pc}

	.align 2,,			// align address to 4-byte boundary

	// cursor shape
9:	mvn	r7,r7			// R7 <- invert sample
	b	3b

	.align 2,,			// align address to 4-byte boundary

	// cursor shape
10:	mvn	r7,r7			// R7 <- invert sample
	b	6b
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_ATEXT
// ----------------------------------------------------------------------------
// attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit
//             attribute low=FG high=BG index of color
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_ATEXT(sDispHstxVSlot* slot, int line, u32* cmd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_ATEXT
DispHstxDviRender_ATEXT:

	// push registers R4-R9,LR
	push	{r4-r9,lr}

	// reserve space in stack for local variables
	sub	sp,#16				// reserve space in stack for local variable: u32 map2[4] or u16 map3[2]

	// load font height -> R3
	ldrsh	r3,[r0,#DISPHSTXSLOT_FONTH]	// R3 fonth <- height of font

	// get row index -> R4, subline -> R1
	sdiv	r4,r1,r3			// R4 row <- line/fonth
	mls	r1,r3,r4,r1			// R1 subline <- line - row*fonth

	// prepare cursor position -> R9
	mov	r9,0				// R9 <- invalid cursor
	ldrb	r3,[r0,#DISPHSTXSLOT_CURROW]	// R3 <- cursor row
	cmp	r3,r4				// check cursor row
	bne	1f				// not cursor row
	ldrb	r3,[r0,#DISPHSTXSLOT_CURBEG]	// R3 <- cursor begin
	cmp	r1,r3				// check cursor begin
	blt	1f				// not cursor
	ldrb	r3,[r0,#DISPHSTXSLOT_CUREND]	// R3 <- cursor end
	cmp	r1,r3				// check cursor end
	bgt	1f				// not cursor
	ldr	r3,=TIMER0_BASE			// R3 <- Timer0 base
	ldrb	r5,[r0,#DISPHSTXSLOT_CURSPEED]	// R5 <- cursor speed
	ldr	r3,[r3,#TIMER0_RAWL]		// R3 <- Timer LOW
	lsrs	r3,r5				// R3 <- shift timer
	lsls	r3,r3,#31			// R3 <- get bit 0
	bpl	1f				// bit 0 of the timer is not set
	ldrb	r9,[r0,#DISPHSTXSLOT_CURPOS]	// R9 <- cursor position
	adds	r9,#1				// R9 <- correction + 1

	// prepare pointer to font -> R3
1:	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 <- pointer to font
	add	r3,r3,r1,lsl #8			// R3 font <- font + subline*256

	// destination buffer -> R2
	ldr	r2,[r2]				// R2 dst <- destination buffer

	// prepare source pointer -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r1,[r0,#DISPHSTXSLOT_BUF]	// R1 src <- source buffer base address
	mla	ip,r4,r5,r1			// R1 src <- row*pitch + buf

	// prepare pointer to palettes -> A4
	ldr	r4,[r0,#DISPHSTXSLOT_PAL]	// R4 pal <- pointer to palettes in RGB565 format map[]

	// width -> LR
	ldrh	lr,[r0,#DISPHSTXSLOT_W]		// LR w <- width of slot in graphics pixels

// ---- big loop (by 16 pixels of 2 characters)

	lsrs	r8,lr,#4			// R8 <- width / 16
	beq	5f				// skip if width/16 is 0

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (dst) pointer to destination buffer
//  R3 = (font) pointer to line in the font
//  R4 = (map) pointer to palettes in RGB565 format
//  R5 = temporary 3
//  R6 = temporary 4, (k) font sample
//  R7 = (kk) font sample with 2 characters, 16 pixels
//  R8 = (i) loop counter
//  R9 = (curpos) cursor position
//  LR = (w) width of slot in graphics pixels
//  IP = (src) pointer to source buffer
//  [SP] = (map2) double-palettes

	.align 2,,			// align address to 4-byte boundary

	// load 2 characters with attributes -> R7
2:	ldr	r7,[ip],#4		// R7 <- load source data, shift source address
	subs	r9,#1			// R9 <- decrement cursor position

// 1st character

	// prepare palettes map2[4] in the stack
	ubfx	r0,r7,#12,#4		// R0 <- extract bits of background attribute
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- background color
	ubfx	r1,r7,#8,#4		// R1 <- extract bits of foreground attribute
	ldrh	r1,[r4,r1,lsl #1]	// R1 <- foreground color
	add	r5,r0,r0,lsl #16	// R5 <- color0 | (color0 << 16)
	add	r6,r0,r1,lsl #16	// R6 <- color0 | (color1 << 16)
	strd	r5,r6,[sp,#0*4]		// map2[0] <- color0 | (color0 << 16), map2[1] <- color0 | (color1 << 16)
	add	r5,r1,r0,lsl #16	// R5 <- color1 | (color0 << 16)
	add	r6,r1,r1,lsl #16	// R6 <- color1 | (color1 << 16)
	strd	r5,r6,[sp,#2*4]		// map2[2] <- color1 | (color0 << 16), map2[3] <- color1 | (color1 << 16)

	// load font sample -> R6
	uxtb	r6,r7			// R6 <- 1st character
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	beq	9f			// skip if cursor

	// pixels 0+1, bits 6+7
3:	ubfx	r0,r6,#6,#2		// R0 <- extract bits 6+7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixels

	// pixels 2+3, bits 4+5
	ubfx	r1,r6,#4,#2		// R1 <- extract bits 4+5
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixels
	strd	r0,r1,[r2,#0*8]		// store pixels

	// pixels 4+5, bits 2+3
	ubfx	r0,r6,#2,#2		// R0 <- extract bits 2+3
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixels

	// pixels 6+7, bits 0+1
	and	r1,r6,#3		// R1 <- extract bits 0+1
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixels
	strd	r0,r1,[r2,#1*8]		// store pixels

// 2nd character

	// prepare palettes map2[4] in the stack
	lsrs	r0,r7,#28		// R0 <- extract bits of background attribute
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- background color
	ubfx	r1,r7,#24,#4		// R1 <- extract bits of foreground attribute
	ldrh	r1,[r4,r1,lsl #1]	// R1 <- foreground color
	add	r5,r0,r0,lsl #16	// R5 <- color0 | (color0 << 16)
	add	r6,r0,r1,lsl #16	// R6 <- color0 | (color1 << 16)
	strd	r5,r6,[sp,#0*4]		// map2[0] <- color0 | (color0 << 16), map2[1] <- color0 | (color1 << 16)
	add	r5,r1,r0,lsl #16	// R5 <- color1 | (color0 << 16)
	add	r6,r1,r1,lsl #16	// R6 <- color1 | (color1 << 16)
	strd	r5,r6,[sp,#2*4]		// map2[2] <- color1 | (color0 << 16), map2[3] <- color1 | (color1 << 16)

	// load font sample -> R6
	ubfx	r6,r7,#16,#8		// R6 <- 2nd character
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	subs	r9,#1			// R9 <- decrement cursor position
	beq	10f			// skip if cursor

	// pixels 0+1, bits 6+7
4:	ubfx	r0,r6,#6,#2		// R0 <- extract bits 6+7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixels

	// pixels 2+3, bits 4+5
	ubfx	r1,r6,#4,#2		// R1 <- extract bits 4+5
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixels
	strd	r0,r1,[r2,#2*8]		// store pixels

	// pixels 4+5, bits 2+3
	ubfx	r0,r6,#2,#2		// R0 <- extract bits 2+3
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixels

	// pixels 6+7, bits 0+1
	and	r1,r6,#3		// R1 <- extract bits 0+1
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixels
	strd	r0,r1,[r2,#3*8]		// store pixels

	// shift destination address
	add	r2,#4*8			// R2 <- shift destination address

	// loop counter
	subs	r8,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

5:	ands	r8,lr,#0x0f		// R8 <- width & 0x0f, small loop counter
	beq	8f			// skip small loop (if loop counter = 0)

	// load 2 characters with attributes -> R7
	ldr	r7,[ip]			// R7 <- load source data

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (dst) pointer to destination buffer
//  R3 = (font) pointer to line in the font
//  R4 = (map) pointer to palettes in RGB565 format
//  R5 = temporary 3
//  R6 = temporary 4, (k) font sample
//  R7 = (kk) font sample with 2 characters, 16 pixels
//  R8 = (i) loop counter
//  R9 = (curpos) cursor position
//  LR = (w) width of slot in graphics pixels
//  IP = (src) pointer to source buffer
//  [SP] = (map2) double-palettes

	// prepare palettes map3[2] in the stack
	ubfx	r0,r7,#12,#4		// R0 <- extract bits of background attribute
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- background color
	strh	r0,[sp,#0*2]		// map3[0] <- color0
	ubfx	r0,r7,#8,#4		// R0 <- extract bits of foreground attribute
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- foreground color
	strh	r0,[sp,#1*2]		// map3[1] <- color1

	// load font sample -> R6
	uxtb	r6,r7			// R6 <- load character
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	subs	r9,#1			// R9 <- decrement cursor position
	it	eq
	mvneq	r6,r6			// R6 <- invert sample

	// counter >= 8 ?
	cmp	r8,#8
	blt	6f			// count < 8

	// pixel 0, bit 7
	ubfx	r0,r6,#7,#1		// R0 <- extract bit 7
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#0*2]		// store pixel

	// pixel 1, bit 6
	ubfx	r0,r6,#6,#1		// R0 <- extract bit 6
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#1*2]		// store pixel

	// pixel 2, bit 5
	ubfx	r0,r6,#5,#1		// R0 <- extract bit 5
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#2*2]		// store pixel

	// pixel 3, bit 4
	ubfx	r0,r6,#4,#1		// R0 <- extract bit 4
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#3*2]		// store pixel

	// pixel 4, bit 3
	ubfx	r0,r6,#3,#1		// R0 <- extract bit 3
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#4*2]		// store pixel

	// pixel 5, bit 2
	ubfx	r0,r6,#2,#1		// R0 <- extract bit 2
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#5*2]		// store pixel

	// pixel 6, bit 1
	ubfx	r0,r6,#1,#1		// R0 <- extract bit 1
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#6*2]		// store pixel

	// pixel 7, bit 0
	and	r0,r6,#1		// R0 <- extract bit 0
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel
	strh	r0,[r2,#7*2]		// store pixel

	// shift counter
	subs	r8,#8			// subtract 8 pixels
	beq	8f			// end

	// prepare palettes map3[2] in the stack
	lsrs	r0,r7,#28		// R0 <- extract bits of background attribute
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- background color
	strh	r0,[sp,#0*2]		// map3[0] <- color0
	ubfx	r0,r7,#24,#4		// R0 <- extract bits of foreground attribute
	ldrh	r0,[r4,r0,lsl #1]	// R0 <- foreground color
	strh	r0,[sp,#1*2]		// map3[1] <- color1

	// load font sample -> R6
	ubfx	r6,r7,#16,#8		// R6 <- load character
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	subs	r9,#1			// R9 <- decrement cursor position
	it	eq
	mvneq	r6,r6			// R6 <- invert sample

	// shift destination address
	add	r2,#8*2			// R2 <- shift destination address

	// small loop
6:	ubfx	r0,r6,#7,#1		// R0 <- extract bit 7
	ldrh	r0,[sp,r0,lsl #1]	// R0 <- load pixel, 
	strh	r0,[r2],#2		// store pixel, increment pointer

	// shift sample
	lsls	r6,r6,#1		// R6 <- shift sample 1 bit left

	// loop counter
	subs	r8,#1			// decrement loop counter
	bne	6b

	// restore stack
8:	add	sp,#16

	// pop registers R4-R9 and return
	pop	{r4-r9,pc}

	.align 2,,			// align address to 4-byte boundary

	// cursor shape
9:	mvn	r6,r6			// R6 <- invert sample
	b	3b

	.align 2,,			// align address to 4-byte boundary

	// cursor shape
10:	mvn	r6,r6			// R6 <- invert sample
	b	4b
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8
// ----------------------------------------------------------------------------
//                 tiles 4x4 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE4_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE4_8
DispHstxDviRender_TILE4_8:

	// push registers R4-R6
	push	{r4-r6,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT+2	// R2 big loop counter <- w >> (TILEW_B+2)
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1, (s) temporary source
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 4 tile indices
//  LR = (dst) destination
//  IP = (src) source

	// load sample with 4 tile indices -> R6
2:	ldr	r6,[ip],#4		// R6 <- load source data, shift source address

	// 1st tile
	uxtb	r0,r6			// R0 <- tile index
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldr	r0,[r0]			// R0 <- load pixels
	str	r0,[lr,#0*4]		// store pixels

	// 2nd tile
	ubfx	r0,r6,#8,#8		// R0 <- extract bits
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldr	r0,[r0]			// R0 <- load pixels
	str	r0,[lr,#1*4]		// store pixels

	// 3rd tile
	ubfx	r0,r6,#16,#8		// R0 <- extract bits
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldr	r0,[r0]			// R0 <- load pixels
	str	r0,[lr,#2*4]		// store pixels
	
	// 4th tile
	lsrs	r0,r6,#24		// R0 <- extract bits
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldr	r0,[r0]			// R0 <- load pixels
	str	r0,[lr,#3*4]		// store pixels

	// shift destination address
	add	lr,#4*4			// LR <- shift destination address

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- check remaining pixels

4:	ands	r4,#4*RENDER_TILEW-1	// R4 <- small loop counter
	beq	8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	lsrs	r2,r4,#RENDER_TILEW_BIT	// R2 <- number of tiles
	beq	6f			// no whole tiles left

// Here is:
//  R0 = temporary
//  R1 = (s) temporary source
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) remaining width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 1 tile index
//  LR = (dst) destination
//  IP = (src) source

5:	ldrb	r6,[ip],#1		// R6 <- load tile index, shift source address
	mla	r0,r6,r5,r3		// R0 <- tile pointer
	ldr	r0,[r0]			// R0,R1 <- load pixels
	str	r0,[lr],#4		// store pixels, shift destination address
	subs	r2,#1			// decrement loop counter
	bne	5b

// ---- small loop (by 1 pixel)

6:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
7:	ldrb	r0,[r1],#1		// R0 <- load pixel, shift source address
	strb	r0,[lr],#1		// store pixel, shift destination address
	subs	r4,#1			// decrement loop counter
	bne	7b

	// pop registers R4-R6 and return
8:	pop	{r4-r6,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8
// ----------------------------------------------------------------------------
//                 tiles 8x8 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE8_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE8_8
DispHstxDviRender_TILE8_8:

	// push registers R4-R6
	push	{r4-r6,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT+2	// R2 big loop counter <- w >> (TILEW_B+2)
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 4 tile indices
//  LR = (dst) destination
//  IP = (src) source

	// load sample with 4 tile indices -> R6
2:	ldr	r6,[ip],#4		// R6 <- load source data, shift source address

	// 1st tile
	uxtb	r0,r6			// R0 <- tile index
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldrd	r0,r1,[r0]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#0*8]		// store pixels

	// 2nd tile
	ubfx	r0,r6,#8,#8		// R0 <- extract bits
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldrd	r0,r1,[r0]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#1*8]		// store pixels

	// 3rd tile
	ubfx	r0,r6,#16,#8		// R0 <- extract bits
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldrd	r0,r1,[r0]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#2*8]		// store pixels
	
	// 4th tile
	lsrs	r0,r6,#24		// R0 <- extract bits
	mla	r0,r0,r5,r3		// R0 <- tile pointer
	ldrd	r0,r1,[r0]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#3*8]		// store pixels

	// shift destination address
	add	lr,#4*8			// LR <- shift destination address

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- check remaining pixels

4:	ands	r4,#4*RENDER_TILEW-1	// R4 <- small loop counter
	beq	8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	lsrs	r2,r4,#RENDER_TILEW_BIT	// R2 <- number of tiles
	beq	6f			// no whole tiles left

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) remaining width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 1 tile index
//  LR = (dst) destination
//  IP = (src) source

5:	ldrb	r6,[ip],#1		// R6 <- load tile index, shift source address
	mla	r0,r6,r5,r3		// R0 <- tile pointer
	ldrd	r0,r1,[r0]		// R0,R1 <- load pixels
	strd	r0,r1,[lr],#8		// store pixels, shift destination address
	subs	r2,#1			// decrement loop counter
	bne	5b

// ---- small loop (by 1 pixel)

6:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
7:	ldrb	r0,[r1],#1		// R0 <- load pixel, shift source address
	strb	r0,[lr],#1		// store pixel, shift destination address
	subs	r4,#1			// decrement loop counter
	bne	7b

	// pop registers R4-R6 and return
8:	pop	{r4-r6,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8
// ----------------------------------------------------------------------------
//                 tiles 16x16 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE16_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE16_8
DispHstxDviRender_TILE16_8:

	// push registers R4-R7
	push	{r4-r7,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT+2	// R2 big loop counter <- w >> (TILEW_B+2)
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 4 tile indices
//  R7 = (s) temporary source
//  LR = (dst) destination
//  IP = (src) source

	// load sample with 4 tile indices -> R6
2:	ldr	r6,[ip],#4		// R6 <- load source data, shift source address

	// 1st tile
	uxtb	r0,r6			// R0 <- tile index
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#0*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#1*8]		// store pixels

	// 2nd tile
	ubfx	r0,r6,#8,#8		// R0 <- extract bits
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#2*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#3*8]		// store pixels

	// 3rd tile
	ubfx	r0,r6,#16,#8		// R0 <- extract bits
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#4*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#5*8]		// store pixels
	
	// 4th tile
	lsrs	r0,r6,#24		// R0 <- extract bits
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#6*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#7*8]		// store pixels

	// shift destination address
	add	lr,#8*8			// LR <- shift destination address

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- check remaining pixels

4:	ands	r4,#4*RENDER_TILEW-1	// R4 <- small loop counter
	beq	8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	lsrs	r2,r4,#RENDER_TILEW_BIT	// R2 <- number of tiles
	beq	6f			// no whole tiles left

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) remaining width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 1 tile index
//  R7 = (s) temporary source
//  LR = (dst) destination
//  IP = (src) source

5:	ldrb	r6,[ip],#1		// R6 <- load tile index, shift source address
	mla	r7,r6,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#0*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#1*8]		// store pixels
	add	lr,#2*8			// LR <- shift destination address
	subs	r2,#1			// decrement loop counter
	bne	5b

// ---- small loop (by 1 pixel)

6:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
7:	ldrb	r0,[r1],#1		// R0 <- load pixel, shift source address
	strb	r0,[lr],#1		// store pixel, shift destination address
	subs	r4,#1			// decrement loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8
// ----------------------------------------------------------------------------
//                 tiles 32x32 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE32_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE32_8
DispHstxDviRender_TILE32_8:

	// push registers R4-R7
	push	{r4-r7,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT+2	// R2 big loop counter <- w >> (TILEW_B+2)
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 4 tile indices
//  R7 = (s) temporary source
//  LR = (dst) destination
//  IP = (src) source

	// load sample with 4 tile indices -> R6
2:	ldr	r6,[ip],#4		// R6 <- load source data, shift source address

	// 1st tile
	uxtb	r0,r6			// R0 <- tile index
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#0*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#1*8]		// store pixels
	ldrd	r0,r1,[r7,#2*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#2*8]		// store pixels
	ldrd	r0,r1,[r7,#3*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#3*8]		// store pixels

	// 2nd tile
	ubfx	r0,r6,#8,#8		// R0 <- extract bits
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#4*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#5*8]		// store pixels
	ldrd	r0,r1,[r7,#2*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#6*8]		// store pixels
	ldrd	r0,r1,[r7,#3*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#7*8]		// store pixels

	// 3rd tile
	ubfx	r0,r6,#16,#8		// R0 <- extract bits
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#8*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#9*8]		// store pixels
	ldrd	r0,r1,[r7,#2*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#10*8]	// store pixels
	ldrd	r0,r1,[r7,#3*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#11*8]	// store pixels
	
	// 4th tile
	lsrs	r0,r6,#24		// R0 <- extract bits
	mla	r7,r0,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#12*8]	// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#13*8]	// store pixels
	ldrd	r0,r1,[r7,#2*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#14*8]	// store pixels
	ldrd	r0,r1,[r7,#3*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#15*8]	// store pixels

	// shift destination address
	add	lr,#16*8		// LR <- shift destination address

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- check remaining pixels

4:	ands	r4,#4*RENDER_TILEW-1	// R4 <- small loop counter
	beq	8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	lsrs	r2,r4,#RENDER_TILEW_BIT	// R2 <- number of tiles
	beq	6f			// no whole tiles left

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) remaining width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with 1 tile index
//  R7 = (s) temporary source
//  LR = (dst) destination
//  IP = (src) source

5:	ldrb	r6,[ip],#1		// R6 <- load tile index, shift source address
	mla	r7,r6,r5,r3		// R7 <- tile pointer
	ldrd	r0,r1,[r7]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#0*8]		// store pixels
	ldrd	r0,r1,[r7,#1*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#1*8]		// store pixels
	ldrd	r0,r1,[r7,#2*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#2*8]		// store pixels
	ldrd	r0,r1,[r7,#3*8]		// R0,R1 <- load pixels
	strd	r0,r1,[lr,#3*8]		// store pixels
	add	lr,#4*8			// LR <- shift destination address
	subs	r2,#1			// decrement loop counter
	bne	5b

// ---- small loop (by 1 pixel)

6:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
7:	ldrb	r0,[r1],#1		// R0 <- load pixel, shift source address
	strb	r0,[lr],#1		// store pixel, shift destination address
	subs	r4,#1			// decrement loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 4x4 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE4_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE4_8_PAL
DispHstxDviRender_TILE4_8_PAL:

	// push registers R4-R7
	push	{r4-r7,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PAL]	// R7 <- pointer to palettes in RGB565 format

// ---- big loop (by 2 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT+2	// R2 big loop counter <- w >> TILEW_B+2
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = temporary 2
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with tile index, (kk) sample with 4 pixels
//  R7 = (map) palette mapping
//  R8 = temporary 2
//  LR = (dst) destination
//  IP = (src) source

	// load sample with tile index -> R6
	.align 2,,			// align address to 4-byte boundary

// 1st tile

2:	ldrb	r6,[ip]			// R6 <- load tile index

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	strh	r0,[lr,#0*2]		// store pixel

	// pixel 1
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	strh	r0,[lr,#1*2]		// store pixel

	// pixel 2
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#24,#8		// R1 <- pixel 3
	strh	r0,[lr,#2*2]		// store pixel

	// pixel 3
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel

// 2nd tile

	// load sample with tile index -> R6
	ldrb	r6,[ip,#1]		// R6 <- load tile index
	strh	r0,[lr,#3*2]		// store pixel

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	strh	r0,[lr,#4*2]		// store pixel

	// pixel 1
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	strh	r0,[lr,#5*2]		// store pixel

	// pixel 2
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#24,#8		// R1 <- pixel 3
	strh	r0,[lr,#6*2]		// store pixel

	// pixel 3
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel

// 3rd tile

	// load sample with tile index -> R6
	ldrb	r6,[ip,#2]		// R6 <- load tile index
	strh	r0,[lr,#7*2]		// store pixel

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	strh	r0,[lr,#8*2]		// store pixel

	// pixel 1
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	strh	r0,[lr,#9*2]		// store pixel

	// pixel 2
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#24,#8		// R1 <- pixel 3
	strh	r0,[lr,#10*2]		// store pixel

	// pixel 3
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel

// 4th tile

	// load sample with tile index -> R6
	ldrb	r6,[ip,#3]		// R6 <- load tile index
	strh	r0,[lr,#11*2]		// store pixel

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	strh	r0,[lr,#12*2]		// store pixel

	// pixel 1
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	strh	r0,[lr,#13*2]		// store pixel

	// pixel 2
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	lsrs	r1,r6,#24		// R1 <- pixel 3
	subs	r2,#1			// decrement loop counter
	strh	r0,[lr,#14*2]		// store pixel

	// pixel 3
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	add	ip,#4			// IP <- shift source address
	strh	r0,[lr,#15*2]		// store pixel

	// shift destination address
	add	lr,#16*2		// LR <- shift destination address

	// loop counter
	bne	2b

// ---- middle loop (1 tile)

4:	ands	r4,#4*RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)

	cmp	r4,#RENDER_TILEW	// whole 1 tile left?
	blt	5f			// skip if less than 1 tile

	// load sample with tile index -> R6
	.align 2,,			// align address to 4-byte boundary
1:	ldrb	r6,[ip],#1		// R6 <- load tile index, increase pointer

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	strh	r0,[lr,#0*2]		// store pixel

	// pixel 1
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	strh	r0,[lr,#1*2]		// store pixel

	// pixel 2
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	lsrs	r1,r6,#24		// R1 <- pixel 3
	subs	r4,#RENDER_TILEW	// decrement counter
	strh	r0,[lr,#2*2]		// store pixel

	// pixel 3
	ldrh	r0,[r7,r1,lsl #1]	// R0 <- remap pixel
	add	lr,#4*2			// LR <- shift destination address
	strh	r0,[lr,#3*2-4*2]	// store pixel

	// shift address and counter
	beq	8f			// no pixels left
	cmp	r4,#RENDER_TILEW	// whole 1 tile left?
	bge	1b	

// ---- small loop (by 1 pixel)
	
5:	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldrh	r0,[r7,r0,lsl #1]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	strh	r0,[lr],#2		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R7 and return
8:	pop	{r4-r7,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 8x8 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE8_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE8_8_PAL
DispHstxDviRender_TILE8_8_PAL:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PAL]	// R7 <- pointer to palettes in RGB565 format

// ---- big loop (by 2 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT+1	// R2 big loop counter <- w >> TILEW_B+1
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = (s) temporary source
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with tile index, (kk) sample with 4 pixels
//  R7 = (map) palette mapping
//  R8 = temporary 2
//  LR = (dst) destination
//  IP = (src) source

	// load sample with tile index -> R6
	// - Instructions of the loop are 32-bits, they should be aligned to speed-up
	.align 2,,			// align address to 4-byte boundary
2:	ldrb.w	r6,[ip]			// R6 <- load tile index

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]			// R6 <- load pixels

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#0*2]		// store pixel

	// pixel 1
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#1*2]		// store pixel

	// pixel 2
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#2*2]		// store pixel

	// pixel 3
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr.w	r6,[r1,#4]		// load next 4 pixels -> R6
	strh.w	r0,[lr,#3*2]		// store pixel

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#4*2]		// store pixel

	// pixel 5
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#5*2]		// store pixel

	// pixel 6
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#6*2]		// store pixel

	// load sample with tile index -> R6
	ldrb.w	r6,[ip,#1]		// R6 <- load tile index

	// pixel 7
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]
	strh.w	r0,[lr,#7*2]		// store pixel

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#8*2]		// store pixel

	// pixel 1
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#9*2]		// store pixel

	// pixel 2
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#10*2]		// store pixel

	// pixel 3
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr	r6,[r1,#4]		// load next 4 pixels -> R6
	subs	r2,#1			// decrement loop counter
	strh.w	r0,[lr,#11*2]		// store pixel

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#12*2]		// store pixel

	// pixel 5
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#13*2]		// store pixel

	// pixel 6
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#14*2]		// store pixel

	// pixel 7
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	add	ip,#2			// IP <- shift source address

	// shift destination address
	strh.w	r0,[lr,#15*2]		// store pixel
	add	lr,#16*2		// LR <- shift destination address

	// loop counter
	bne	2b

// ---- middle loop (1 tile)

4:	ands	r4,#2*RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)

	cmp	r4,#RENDER_TILEW	// whole 1 tile left?
	blt	5f			// skip if less than 1 tile

	// load sample with tile index -> R6
	.align 2,,			// align address to 4-byte boundary
	ldrb.w	r6,[ip],#1		// R6 <- load tile index, increase pointer

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]			// R6 <- load pixels

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#0*2]		// store pixel

	// pixel 1
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#1*2]		// store pixel

	// pixel 2
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#2*2]		// store pixel

	// pixel 3
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr.w	r6,[r1,#4]		// load next 4 pixels -> R6
	strh.w	r0,[lr,#3*2]		// store pixel

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#4*2]		// store pixel

	// pixel 5
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#5*2]		// store pixel

	// pixel 6
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#6*2]		// store pixel

	// pixel 7
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	subs	r4,#RENDER_TILEW	// decrement counter
	strh.w	r0,[lr,#7*2]		// store pixel

	// shift address and counter
	add	lr,#8*2			// LR <- shift destination address
	ble	8f			// no pixels left

// ---- small loop (by 1 pixel)
	
5:	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldrh	r0,[r7,r0,lsl #1]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	strh	r0,[lr],#2		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 16x16 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE16_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE16_8_PAL
DispHstxDviRender_TILE16_8_PAL:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PAL]	// R7 <- pointer to palettes in RGB565 format

// ---- big loop (by 2 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT		// R2 big loop counter <- w >> TILEW_B
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = (s) temporary source
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with tile index, (kk) sample with 4 pixels
//  R7 = (map) palette mapping
//  R8 = temporary 2
//  LR = (dst) destination
//  IP = (src) source

	// load sample with tile index -> R6
	// - Instructions of the loop are 32-bits, they should be aligned to speed-up
	.align 2,,			// align address to 4-byte boundary
2:	ldrb.w	r6,[ip],#1		// R6 <- load tile index, increment source address

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

// pixesl 0-3

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]			// R6 <- load pixels

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#0*2]		// store pixel

	// pixel 1
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#1*2]		// store pixel

	// pixel 2
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#2*2]		// store pixel

	// pixel 3
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr.w	r6,[r1,#4]		// load next 4 pixels -> R6
	strh.w	r0,[lr,#3*2]		// store pixel

// pixesl 4-7

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#4*2]		// store pixel

	// pixel 5
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#5*2]		// store pixel

	// pixel 6
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#6*2]		// store pixel

	// pixel 7
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr.w	r6,[r1,#8]		// load next 4 pixels -> R6
	strh.w	r0,[lr,#7*2]		// store pixel

// pixesl 8-11

	// pixel 8
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#8*2]		// store pixel

	// pixel 9
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#9*2]		// store pixel

	// pixel 10
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#10*2]		// store pixel

	// pixel 11
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr.w	r6,[r1,#12]		// load next 4 pixels -> R6
	strh.w	r0,[lr,#11*2]		// store pixel

// pixesl 12-15

	// pixel 12
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#12*2]		// store pixel

	// pixel 13
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#13*2]		// store pixel

	// pixel 14
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#14*2]		// store pixel

	// pixel 15
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel

	// shift destination address
	add	lr,#16*2		// LR <- shift destination address
	strh.w	r0,[lr,#15*2 - 16*2]	// store pixel

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

4:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldrh	r0,[r7,r0,lsl #1]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	strh	r0,[lr],#2		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 32x32 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE32_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

// macro - store 4 pixels
//   dst ... destination first index 0, 4, 8, 12, 16, 20, 24
//   src ... next source index 1, 2, 3, 4, 5, 6
.macro store_32_4 dst src
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#\dst*2+0]	// store pixel

	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#\dst*2+2]	// store pixel

	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#\dst*2+4]	// store pixel

	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ldr.w	r6,[r1,#\src*4]		// load next 4 pixels -> R6
	strh.w	r0,[lr,#\dst*2+6]	// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_TILE32_8_PAL
DispHstxDviRender_TILE32_8_PAL:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare pointer to tiles -> R3 (tiles), delta of tiles -> R5 (d)
	ldrsh	r6,[r0,#DISPHSTXSLOT_FONTH]	// R6 fonth <- tile pitch in horizontal row mode
	mov	r5,#RENDER_TILEWB		// R5 <- delta of tiles in horizontal row mode
	cmp	r6,r5				// is tile pitch small?
	itt	le				// if less or equal, use vertical column mode
	movle	r6,r5				// R6 fonth <- tile pitch in vertical column mode
	movle	r5,#RENDER_TILEWB*RENDER_TILEW	// R5 <- delta of tiles in vertical column mode
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 font <- pointer to tiles
	and	r1,#RENDER_TILEW-1		// R1 <- subline
	mla	r3,r1,r6,r3			// R3 tiles <- subline*fonth + font

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PAL]	// R7 <- pointer to palettes in RGB565 format

// ---- big loop (by 2 tiles)

	lsrs	r2,r4,#RENDER_TILEW_BIT		// R2 big loop counter <- w >> TILEW_B
	beq	4f				// skip if loop counter is 0

// Here is:
//  R0 = temporary 1
//  R1 = (s) temporary source
//  R2 = (i) loop counter
//  R3 = (tiles) pointer to tiles
//  R4 = (w) width of slot
//  R5 = (d) delta of tiles
//  R6 = (k) sample with tile index, (kk) sample with 4 pixels
//  R7 = (map) palette mapping
//  R8 = temporary 2
//  LR = (dst) destination
//  IP = (src) source

	// load sample with tile index -> R6
	// - Instructions of the loop are 32-bits, they should be aligned to speed-up
	.align 2,,			// align address to 4-byte boundary
2:	ldrb.w	r6,[ip],#1		// R6 <- load tile index, increment source address

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]			// R6 <- load pixels

	// pixels 0-3
	store_32_4 0,1

	// pixels 4-7
	store_32_4 4,2

	// pixels 8-11
	store_32_4 8,3

	// pixels 12-15
	store_32_4 12,4

	// pixels 16-19
	store_32_4 16,5

	// pixels 20-23
	store_32_4 20,6

	// pixels 24-27
	store_32_4 24,7

// pixesl 28-31

	// pixel 28
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	strh.w	r0,[lr,#28*2]		// store pixel

	// pixel 29
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	strh.w	r0,[lr,#29*2]		// store pixel

	// pixel 30
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	strh.w	r0,[lr,#30*2]		// store pixel

	// pixel 31
	ldrh.w	r0,[r7,r8,lsl #1]	// R0 <- remap pixel

	// shift destination address
	add	lr,#32*2		// LR <- shift destination address
	strh.w	r0,[lr,#31*2 - 32*2]	// store pixel

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

4:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldrh	r0,[r7,r0,lsl #1]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	strh	r0,[lr],#2		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_HSTX_15 || DISPHSTX_USE_FORMAT_HSTX_16
// ----------------------------------------------------------------------------
//                            HSTX RLE compression
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_HSTX(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_HSTX
DispHstxDviRender_HSTX:

	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 (font) <- pointer to font
	ldr	r0,[r0,#DISPHSTXSLOT_BUF]	// R0 (buf) <- source buffer base address
	add	r3,r3,r1,lsl #2			// R3 (font+line) <- offset in array of line offsets
	ldr	ip,[r3]				// IP (off) <- current line offset font[line]
	ldr	r3,[r3,#4]			// R3 <- next line offset font[line+1]
	add	r0,r0,ip,lsl #1			// R0 <- source address = buf + off
	subs	r3,ip				// R3 <- length of the line
	strd	r3,r0,[r2,#-4]			// store length and address of the line
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_PAT_8
// ----------------------------------------------------------------------------
//                      repeated pattern, 8-bit format
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Pat_8(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_Pat_8
DispHstxDviRender_Pat_8:

	// push registers R4-R9
	push	{r4-r9,lr}

	// line modulo pattern height -> R1
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH] // R5 <- fonth (modulo lines)
	sdiv	r4,r1,r5		// R4 <- line / fonth
	mls	r1,r4,r5,r1		// R1 <- line - lin/fonth*fonth = line % fonth

	// prepare source address -> LR
	ldrh	r8,[r0,#DISPHSTXSLOT_PITCH]	// R8 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	lr,r1,r8,r4			// LR <- line*pitch+base

	// prepare width of the slot -> R9
	ldrh	r9,[r0,#DISPHSTXSLOT_W]		// R9 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- start next pattern

DispHstxDviRender_Pat_8_2:

	mov	ip,lr				// IP <- source
	mov	r3,r8				// R3 <- width of the pattern
	cmp	r3,r9				// check remaining width
	ble	1f				// width is OK
	mov	r3,r9				// limit width
1:	sub	r9,r3				// decrease total width

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

// Here is:
//  R0 = temporary register
//  R2 = sample with 4 pixels
//  R3 = width of pattern to render
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = 2nd sample with next 4 pixels
//  R7 = temporary register 2
//  R8 = original pitch (pattern width)
//  R9 = original width
//  IP = source buffer
//  LR = original source buffer

2:	ldrd	r2,r6,[ip,#0]		// R2,R6 <- load 8 pixels
	adds	ip,#4*4			// IP <- shift source address
	strd	r2,r6,[r5,#0]		// store 8 pixels
	ldrd	r2,r6,[ip,#8-16]	// R2,R6 <- load 8 pixels
	adds	r5,#4*4			// R5 <- shift destination address
	strd	r2,r6,[r5,#8-16]	// store 8 pixels
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r3,#0x0f		// R4 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

// Here is:
//  R0 = temporary register
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
6:	ldrb	r0,[ip],#1		// R0 <- [IP], load source byte
	subs	r4,#1			// decrement loop counter
	strb	r0,[r5],#1		// store pixel, increment pointer

	// loop counter
	bne	6b

	// check if all is done
8:	cmp	r9,0			// check remaining width
	bne	DispHstxDviRender_Pat_8_2

	// pop registers R4-R9 and return
	pop	{r4-r9,pc}
#endif

#if DISPHSTX_USE_FORMAT_PAT_8_PAL
// ----------------------------------------------------------------------------
//                 repeated pattern, 8-bit format, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Pat_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_Pat_8_PAL
DispHstxDviRender_Pat_8_PAL:

	// push registers R4-R9
	push	{r4-r9,lr}

	// line modulo pattern height -> R1
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH] // R5 <- fonth (modulo lines)
	sdiv	r4,r1,r5		// R4 <- line / fonth
	mls	r1,r4,r5,r1		// R1 <- line - lin/fonth*fonth = line % fonth

	// prepare source address -> LR
	ldrh	r8,[r0,#DISPHSTXSLOT_PITCH]	// R8 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	lr,r1,r8,r4			// LR <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes in RGB565 format

	// prepare width of the slot -> R9
	ldrh	r9,[r0,#DISPHSTXSLOT_W]		// R9 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- start next pattern

DispHstxDviRender_Pat_8_PAL_2:

	mov	ip,lr				// IP <- source
	mov	r3,r8				// R3 <- width of the pattern
	cmp	r3,r9				// check remaining width
	ble	1f				// width is OK
	mov	r3,r9				// limit width
1:	sub	r9,r3				// decrease total width

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 4 pixels
//  R3 = width of pattern to render
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = 2nd sample with next 4 pixels
//  R7 = temporary register 2
//  R8 = original pitch (pattern width)
//  R9 = original width
//  IP = source buffer
//  LR = original source buffer

// sample 0, destination index 0..3

2:	ldrd	r2,r6,[ip,#0]		// R2 <- [IP], R6 <- [IP+4], load next 2 samples
	adds	r5,#16*2		// R5 <- shift destination address

	uxtb	r7,r2			// R7 <- extract bits 0-7 from the sample
	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r2,#8,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#0*2-16*2]	// store 2 pixela

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r2,#16,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#1*2-16*2]	// store 2 pixela

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	lsrs	r7,r2,#24		// R7 <- extract bits 24-31 from the sample
	strh	r0,[r5,#2*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	uxtb	r7,r6			// R7 <- extract bits 0-7 from the sample
	strh	r0,[r5,#3*2-16*2]	// store 2 pixels

// sample 1, destination index 4..7

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r6,#8,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#4*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r6,#16,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#5*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	lsrs	r7,r6,#24		// R7 <- extract bits 24-31 from the sample
	strh	r0,[r5,#6*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ldrd	r2,r6,[ip,#8]		// R2 <- [IP+8], R6 <- [IP+12], load next 2 samples
	strh	r0,[r5,#7*2-16*2]	// store 2 pixels

// sample 2, destination index 8..11

	uxtb	r7,r2			// R7 <- extract bits 0-7 from the sample
	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r2,#8,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#8*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r2,#16,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#9*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	lsrs	r7,r2,#24		// R7 <- extract bits 24-31 from the sample
	strh	r0,[r5,#10*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	uxtb	r7,r6			// R7 <- extract bits 0-7 from the sample
	strh	r0,[r5,#11*2-16*2]	// store 2 pixels

// sample 3, destination index 12..15

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r6,#8,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#12*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	ubfx	r7,r6,#16,#8		// R7 <- extract bits from the sample
	strh	r0,[r5,#13*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	lsrs	r7,r6,#24		// R7 <- extract bits 24-31 from the sample
	strh	r0,[r5,#14*2-16*2]	// store 2 pixels

	ldrh	r0,[r1,r7,lsl #1]	// R0 <- mapping[bit << 1]
	adds	ip,#4*4			// IP <- shift source address
	strh	r0,[r5,#15*2-16*2]	// store 2 pixels

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,r3,#0x0f		// R4 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R4 = loop counter
//  R5 = destination buffer
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
6:	ldrb	r0,[ip],#1		// R0 <- [IP], load source byte
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- mapping[bit << 1]
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	6b

	// check if all is done
8:	cmp	r9,0			// check remaining width
	bne	DispHstxDviRender_Pat_8_PAL_2

	// pop registers R4-R9 and return
	pop	{r4-r9,pc}
#endif

#if DISPHSTX_USE_FORMAT_RLE8
// ----------------------------------------------------------------------------
//                            RLE8 compression
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Rle8(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_Rle8
DispHstxDviRender_Rle8:

	// push registers R4-R5
	push	{r4-r5}

	// prepare destination address -> R3
	ldr	r3,[r2,#0]			// R3 <- destination address of render buffer

	// prepare source address -> R1
	ldr	r2,[r0,#DISPHSTXSLOT_FONT]	// R2 (font) <- pointer to font
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 (buf) <- source buffer base address
	ldr	r2,[r2,r1,lsl #2]		// R2 <- line offset font[line]
	adds	r1,r4,r2			// R1 (src) <- source address

	// prepare width of the slot -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 <- width of slot in graphics pixels

// ---- main loop

// Here is:
//  R0 = temporary
//  R1 = (src) source address
//  R2 = (n) data length
//  R3 = (dst) destination buffer
//  R4 = loop counter
//  R5 = (w) width of the slot

	.align 2,,			// align address to 4-byte boundary

	// get token -> R0
2:	ldrb	r0,[r1],#1		// R0 <- load token, increment address

	// check repeat mode
	cmp	r0,#128			// check if token is >= 128 (= negative)
	blt	6f			// skip if token < 128

// repeat mode

	// get data length -> R2
	subs	r2,r0,#128 - 3		// R2 <- length = token - 128 + 3
	cmp	r2,r5			// check remaining width
	it	ge			// true if >=
	movge	r2,r5			// limit length
	subs	r5,r2			// R5 <- decrease width

	// get color -> R0
	ldrb	r0,[r1],#1		// R0 <- get color, increment address

	// prepare big loop counter -> R4
	lsrs	r4,r2,#2		// R4 <- data length / 4
	beq	4f			// skip if counter is zero

	// pack color -> R0
	bfi	r0,r0,#8,#8
	bfi	r0,r0,#16,#16

	// store color by 4 pixels
3:	str	r0,[r3],#4		// store 4 pixels, increment address
	subs	r4,#1			// decrement loop counter
	bne	3b			// loop

	// prepare small loop counter -> R2
4:	ands	r2,#3			// R2 <- loop counter
	beq.w	9f			// continue main loop

	// store color by 1 pixel (count = 1..3)
	strb	r0,[r3],#1
	subs	r2,#1
	beq	9f
	strb	r0,[r3],#1
	subs	r2,#1
	beq	9f
	strb	r0,[r3],#1

	// main loop (check remaining width)
	cmp	r5,#0
	bgt	2b

	// pop registers R4-R5 and return
	pop	{r4-r5}
	bx	lr

// Here is:
//  R0 = temporary
//  R1 = (src) source address
//  R2 = (n) data length
//  R3 = (dst) destination buffer
//  R4 = loop counter
//  R5 = (w) width of the slot

// raw data

	.align 2,,			// align address to 4-byte boundary

	// get data length -> R2
6:	adds	r2,r0,#1		// R2 <- length = token - 128 + 3
	cmp	r2,r5			// check remaining width
	it	ge			// true if >=
	movge	r2,r5			// limit length
	subs	r5,r2			// R5 <- decrease width

	// prepare big loop counter -> R4
	lsrs	r4,r2,#2		// R4 <- data length / 4
	beq	8f			// skip if counter is zero

	// transfer by 4 pixels
7:	ldr	r0,[r1],#4
	subs	r4,#1
	str	r0,[r3],#4
	bne	7b

	// prepare small loop counter -> R2
8:	ands	r2,#3			// R2 <- loop counter
	beq	9f			// continue main loop

	// transfer by 1 pixel (count = 1..3)
	ldrb	r0,[r1],#1
	subs	r2,#1
	strb	r0,[r3],#1
	beq	9f

	ldrb	r0,[r1],#1
	subs	r2,#1
	strb	r0,[r3],#1
	beq	9f

	ldrb	r0,[r1],#1
	strb	r0,[r3],#1

	// main loop (check remaining width)
9:	cmp	r5,#0
	bgt	2b

	// pop registers R4-R5 and return
// Note: if you update pop registers, update them also in pop inside the function
	pop	{r4-r5}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_RLE8_PAL
// ----------------------------------------------------------------------------
//                         RLE8 compression, paletted
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Rle8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxDviRender_Rle8_PAL
DispHstxDviRender_Rle8_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare destination address -> R3
	ldr	r3,[r2,#0]			// R3 <- destination address of render buffer

	// prepare source address -> R1
	ldr	r2,[r0,#DISPHSTXSLOT_FONT]	// R2 (font) <- pointer to font
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 (buf) <- source buffer base address
	ldr	r2,[r2,r1,lsl #2]		// R2 <- line offset font[line]
	adds	r1,r4,r2			// R1 (src) <- source address

	// prepare width of the slot -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 <- width of slot in graphics pixels

	// prepare palette mapping address -> R6
	ldr	r6,[r0,#DISPHSTXSLOT_PAL]	// R6 <- pointer to palettes in RGB565 format

// ---- main loop

// Here is:
//  R0 = temporary
//  R1 = (src) source address
//  R2 = (n) data length
//  R3 = (dst) destination buffer
//  R4 = loop counter
//  R5 = (w) width of the slot
//  R6 = (map) palette mapping
//  R7 = temporary 2

	.align 2,,			// align address to 4-byte boundary

	// get token -> R0
2:	ldrb	r0,[r1],#1		// R0 <- load token, increment address

	// check repeat mode
	cmp	r0,#128			// check if token is >= 128 (= negative)
	blt	6f			// skip if token < 128

// repeat mode

	// get data length -> R2
	subs	r2,r0,#128 - 3		// R2 <- length = token - 128 + 3
	cmp	r2,r5			// check remaining width
	it	ge			// true if >=
	movge	r2,r5			// limit length
	subs	r5,r2			// R5 <- decrease width

	// get color -> R0
	ldrb	r0,[r1],#1		// R0 <- get color, increment address
	ldrh	r0,[r6,r0,lsl #1]	// R0 <- map color

	// prepare big loop counter -> R4
	lsrs	r4,r2,#2		// R4 <- data length / 4
	beq	4f			// skip if counter is zero

	// pack color -> R0
	bfi	r0,r0,#16,#16

	// store color by 4 pixels
3:	str	r0,[r3,#0]		// store 2 pixels
	str	r0,[r3,#4]		// store 2 pixels
	adds	r3,#8			// incrrement address
	subs	r4,#1			// decrement loop counter
	bne	3b			// loop

	// prepare small loop counter -> R2
4:	ands	r2,#3			// R2 <- loop counter
	beq	9f			// continue main loop

	// store color by 1 pixel (count = 1..3)
	strh	r0,[r3,#0]
	cmp	r2,#1
	beq	1f
	strh	r0,[r3,#2]
	cmp	r2,#2
	it	ne
	strhne	r0,[r3,#4]

	// shift destination address
1:	cmp	r5,#0			// here for align
	add	r3,r3,r2,lsl #1

	// main loop (check remaining width)
	bgt	2b

	// pop registers R4-R7 and return
	pop	{r4-r7}
	bx	lr

// Here is:
//  R0 = temporary
//  R1 = (src) source address
//  R2 = (n) data length
//  R3 = (dst) destination buffer
//  R4 = loop counter
//  R5 = (w) width of the slot
//  R6 = (map) palette mapping
//  R7 = temporary 2

// raw data

	.align 2,,			// align address to 4-byte boundary
	nop				// better alignment

	// get data length -> R2
6:	adds	r2,r0,#1		// R2 <- length = token - 128 + 3
	cmp	r2,r5			// check remaining width
	it	ge			// true if >=
	movge	r2,r5			// limit length
	subs	r5,r2			// R5 <- decrease width

	// prepare big loop counter -> R4
	lsrs	r4,r2,#2		// R4 <- data length / 4
	beq	8f			// skip if counter is zero

	// transfer by 4 pixels
7:	ldrb	r0,[r1]			// R0 <- get color
	ldrb	r7,[r1,#1]		// R7 <- get color 2
	ldrh	r0,[r6,r0,lsl #1]	// R0 <- map color
	ldrh	r7,[r6,r7,lsl #1]	// R7 <- map color 2
	adds	r1,#4			// R1 <- increment source address
	bfi	r0,r7,#16,#16		// R0 <- pack colors
	str	r0,[r3]			// store 2 pixels

	ldrb	r0,[r1,#2-4]		// R0 <- get color
	ldrb	r7,[r1,#3-4]		// R7 <- get color 2
	ldrh	r0,[r6,r0,lsl #1]	// R0 <- map color
	ldrh	r7,[r6,r7,lsl #1]	// R7 <- map color 2
	adds	r3,#8			// R3 <- increment destination address
	bfi	r0,r7,#16,#16		// R0 <- pack colors
	str	r0,[r3,#-4]		// store 2 pixels

	subs	r4,#1
	bne	7b

	// prepare small loop counter -> R2
8:	ands	r2,#3			// R2 <- loop counter
	beq	9f			// continue main loop

	// transfer by 1 pixel (count = 1..3)
	ldrb	r0,[r1,#0]
	cmp	r2,#1
	ldrh	r0,[r6,r0,lsl #1]	// R0 <- map color
	ldrb	r7,[r1,#1]		// R7 <- get color 2
	strh	r0,[r3]
	beq	10f

	ldrh	r7,[r6,r7,lsl #1]	// R0 <- map color
	cmp	r2,#2
	strh	r7,[r3,#2]
	beq	10f

	ldrb	r0,[r1,#2]
	ldrh	r0,[r6,r0,lsl #1]	// R0 <- map color
	strh	r0,[r3,#4]

	// increment address
10:	add	r3,r3,r2,lsl #1		// shift destination address
	add	r1,r2			// shift source address

	// main loop (check remaining width)
9:	cmp	r5,#0
	bgt	2b

	// pop registers R4-R7 and return
// Note: if you update pop registers, update them also in pop inside the function
	pop	{r4-r7}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_ATTR1_PAL
// ----------------------------------------------------------------------------
//   attribute compression 1, 4-bit paletted colors, cell 8x8, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr1_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 32 pixels
// R5 = destination buffer
// R6 = temporary register 2
// SP[] = attribute mapping
.macro attr1_1 dst bit
	ubfx	r0,r2,#\bit+1,#1	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#1		// R6 <- extract bit from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_Attr1_PAL
DispHstxDviRender_Attr1_PAL:

	// push registers R4-R7
	push	{r4-r7,lr}

	// reserve space in stack for local variables
	sub	sp,#8				// reserve space in stack for local variable: u8 map2[2]

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#3			// R4 <- line / 8
	mla	r7,r4,r5,r7			// R7 pix <- font[line/8 * fonth], attribute row

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 32 pixels)

	// prepare counter of big loop (by 32 pixels) -> R4
	lsrs	r4,r3,#5		// R4 <- width / 32
	beq	5f			// skip if width/32 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

	// load attributes -> LR
	ldr	lr,[r7],#4		// LR <- attributes, increment address

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

// byte 0, destination index 0..7, source bit 7..0

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	attr1_1	0,6
	attr1_1	2,4
	attr1_1	4,2

	ubfx	r0,r2,#1,#1		// R0 <- extract bit from the sample
	and.w	r6,r2,#1		// R6 <- extract bit 0 from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#6*2]		// store pixels
	strh	r6,[r5,#7*2]		// store pixels

// byte 1, destination index 8..15, source bit 15..8

	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#1]

	attr1_1	8,14
	attr1_1	10,12
	attr1_1	12,10
	attr1_1	14,8

// byte 2, destination index 16..23, source bit 23..16

	ubfx	r0,lr,#20,#4		// R0 <- attribute 4
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 5
	strb	r0,[sp,#1]

	attr1_1	16,22
	attr1_1	18,20
	attr1_1	20,18
	attr1_1	22,16

// byte 3, destination index 24..31, source bit 31..24

	ubfx	r0,lr,#28,#4		// R0 <- attribute 6
	strb	r0,[sp]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 7
	strb	r0,[sp,#1]

	lsrs.w	r0,r2,#31		// R0 <- extract bit from the sample
	ubfx	r6,r2,#30,#1		// R6 <- extract bit from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#24*2]		// store pixels
	strh	r6,[r5,#25*2]		// store pixels

	attr1_1	26,28
	attr1_1	28,26
	attr1_1	30,24

	// shift destination address
	adds	r5,#32*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	ands	r3,r3,#0x1f		// R3 <- width low 5 bits
	beq	8f			// skip if width & 0x1f is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// load attributes -> LR
	ldr	lr,[r7]			// LR <- attributes

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

	// prepare middle loop counter -> R4
	lsrs	r4,r3,#3		// R4 <- width / 8
	beq	6f

	// middle loop
1:	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	attr1_1	0,6
	attr1_1	2,4
	attr1_1	4,2

	ubfx	r0,r2,#1,#1		// R0 <- extract bit from the sample
	and.w	r6,r2,#1		// R6 <- extract bit 0 from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#6*2]		// store pixels
	strh	r6,[r5,#7*2]		// store pixels

	// next loop
	adds	r5,#8*2			// R5 <- shift destination address
	lsrs	r2,r2,#8		// R2 <- shift sample word
	lsrs	lr,lr,#8		// LR <- shift attribute word
	subs	r4,#1			// decrement loop counter
	bne	1b

	// prepare small loop counter -> R4
6:	ands	r4,r3,#7
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// prepare attributes
	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	// process one pixel
7:	ubfx	r0,r2,#7,#1		// R0 <- extract bit 7 from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// restore stack
8:	add	sp,#8

	// pop registers R4-R7 and return
	pop	{r4-r7,pc}
#endif

#if DISPHSTX_USE_FORMAT_ATTR2_PAL
// ----------------------------------------------------------------------------
//   attribute compression 2, 4-bit paletted colors, cell 4x4, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr2_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 32 pixels
// R5 = destination buffer
// R6 = temporary register 2
// SP[] = attribute mapping
.macro attr2_1 dst bit
	ubfx	r0,r2,#\bit+1,#1	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#1		// R6 <- extract bit from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.thumb_func
.global DispHstxDviRender_Attr2_PAL
DispHstxDviRender_Attr2_PAL:

	// push registers R4-R7
	push	{r4-r7,lr}

	// reserve space in stack for local variables
	sub	sp,#8				// reserve space in stack for local variable: u8 map2[2]

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	lsrs	r7,r1,#1			// R7 <- line/2
	mla	ip,r7,r5,r4			// IP <- line/2*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#2			// R4 <- line/4
	mla	r7,r4,r5,r7			// R7 pix <- font[line/4 * fonth], attribute row

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// prepare palette mapping address -> R1
	ands	r4,r1,#1			// check if line is odd
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3 (saves flags)
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// jump to odd line
	bne	DispHstxDviRender_Attr2_PAL_odd	// jump to odd line service

// ---- even line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load attributes -> LR
2:	ldr	lr,[r7],#4		// LR <- attributes, increment address

	// load next source data -> R2
	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

// byte 0, destination index 0..7, source bit 7..0

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	attr2_1	0,6
	attr2_1	2,4

// byte 1, destination index 8..15, source bit 15..8

	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#1]

	attr2_1	4,14
	attr2_1	6,12

// byte 2, destination index 16..23, source bit 23..16

	ubfx	r0,lr,#20,#4		// R0 <- attribute 4
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 5
	strb	r0,[sp,#1]

	attr2_1	8,22
	attr2_1	10,20

// byte 3, destination index 24..31, source bit 31..24

	ubfx	r0,lr,#28,#4		// R0 <- attribute 6
	strb	r0,[sp]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 7
	strb	r0,[sp,#1]

	attr2_1	12,30
	attr2_1	14,28

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- even line - small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x1f is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// load attributes -> LR
	ldr	lr,[r7]			// LR <- attributes

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

	// prepare middle loop counter -> R4
	lsrs	r4,r3,#2		// R4 <- width / 4
	beq	6f

	// middle loop
1:	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	attr2_1	0,6
	attr2_1	2,4

	// next loop
	lsrs	lr,lr,#8		// LR <- shift attribute word
	adds	r5,#4*2			// R5 <- shift destination address
	lsrs	r2,r2,#8		// R2 <- shift sample word
	subs	r4,#1			// decrement loop counter
	bne	1b

	// prepare small loop counter -> R4
6:	ands	r4,r3,#3
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// prepare attributes
	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	// process one pixel
7:	ubfx	r0,r2,#7,#1		// R0 <- extract bit 7 from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// restore stack
8:	add	sp,#8

	// pop registers R4-R7 and return
	pop	{r4-r7,pc}

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

DispHstxDviRender_Attr2_PAL_odd:

// ---- odd line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load attributes -> LR
2:	ldr	lr,[r7],#4		// LR <- attributes, increment address

	// load next source data -> R2
	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

// byte 0, destination index 0..7, source bit 7..0

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	attr2_1	0,2
	attr2_1	2,0

// byte 1, destination index 8..15, source bit 15..8

	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#1]

	attr2_1	4,10
	attr2_1	6,8

// byte 2, destination index 16..23, source bit 23..16

	ubfx	r0,lr,#20,#4		// R0 <- attribute 4
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 5
	strb	r0,[sp,#1]

	attr2_1	8,18
	attr2_1	10,16

// byte 3, destination index 24..31, source bit 31..24

	ubfx	r0,lr,#28,#4		// R0 <- attribute 6
	strb	r0,[sp]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 7
	strb	r0,[sp,#1]

	attr2_1	12,26
	attr2_1	14,24

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- odd line - small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x1f is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// load attributes -> LR
	ldr	lr,[r7]			// LR <- attributes

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

	// prepare middle loop counter -> R4
	lsrs	r4,r3,#2		// R4 <- width / 4
	beq	6f

	// middle loop
1:	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	attr2_1	0,2
	attr2_1	2,0

	// next loop
	lsrs	lr,lr,#8		// LR <- shift attribute word
	adds	r5,#4*2			// R5 <- shift destination address
	lsrs	r2,r2,#8		// R2 <- shift sample word
	subs	r4,#1			// decrement loop counter
	bne	1b

	// prepare small loop counter -> R4
6:	ands	r4,r3,#3
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// prepare attributes
	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]

	// process one pixel
7:	ubfx	r0,r2,#3,#1		// R0 <- extract bit 3 from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// restore stack
8:	add	sp,#8

	// pop registers R4-R7 and return
// Note: if you update pop registers, update them also in pop inside the function
	pop	{r4-r7,pc}
#endif

#if DISPHSTX_USE_FORMAT_ATTR3_PAL
// ----------------------------------------------------------------------------
//   attribute compression 3, 4-bit paletted colors, cell 8x8, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr3_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 2-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 16 pixels
// R5 = destination buffer
// R6 = temporary register 2
// SP[] = attribute mapping
.macro attr3_1 dst bit
	ubfx	r0,r2,#\bit+2,#2	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#2		// R6 <- extract bit from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxDviRender_Attr3_PAL
DispHstxDviRender_Attr3_PAL:

	// push registers R4-R7
	push	{r4-r7,lr}

	// reserve space in stack for local variables
	sub	sp,#8				// reserve space in stack for local variable: u8 map2[4]

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#3			// R4 <- line / 8
	mla	r7,r4,r5,r7			// R7 pix <- font[line/8 * fonth], attribute row

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load attributes -> LR
2:	ldr	lr,[r7],#4		// LR <- attributes, increment address

	// load next source data -> R2
	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

// byte 0, destination index 0..7, source bit 7..0

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	attr3_1	0,4
	attr3_1	2,0

// byte 1, destination index 8..15, source bit 15..8

	attr3_1	4,12
	attr3_1	6,8

// byte 2, destination index 16..23, source bit 23..16

	ubfx	r0,lr,#20,#4		// R0 <- attribute 0
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#28,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	attr3_1 8,20
	attr3_1 10,16

// byte 3, destination index 24..31, source bit 31..24

	attr3_1 12,28
	attr3_1 14,24

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

	// load attributes -> LR
	ldr	lr,[r7]			// LR <- attributes

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	rev	r2,r2			// reverze byte order

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	// check width 8 pixels
	cmp	r3,#8			// at least 8 pixels?
	blt	7f			// < 8 pixels

	// write 8 pixels
	attr3_1	0,28
	attr3_1	2,24
	attr3_1	4,20
	attr3_1	6,16

	adds	r5,#8*2			// R5 <- shift destination address
	lsls	r2,r2,#16		// R2 <- shift sample left by 16 bits

	subs	r3,r3,#8		// decrement width
	beq	8f			// no data left

	ubfx	r0,lr,#20,#4		// R0 <- attribute 0
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#28,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	// process one pixel
7:	lsrs	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	subs	r3,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// restore stack
8:	add	sp,#8

	// pop registers R4-R7 and return
	pop	{r4-r7,pc}
#endif

#if DISPHSTX_USE_FORMAT_ATTR4_PAL
// ----------------------------------------------------------------------------
//   attribute compression 4, 4-bit paletted colors, cell 4x4, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr4_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 2-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 16 pixels
// R5 = destination buffer
// R6 = temporary register 2
// SP[] = attribute mapping
.macro attr4_1 dst bit
	ubfx	r0,r2,#\bit+2,#2	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#2		// R6 <- extract bit from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	ldrb	r6,[sp,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.thumb_func
.global DispHstxDviRender_Attr4_PAL
DispHstxDviRender_Attr4_PAL:

	// push registers R4-R7
	push	{r4-r7,lr}

	// reserve space in stack for local variables
	sub	sp,#8				// reserve space in stack for local variable: u8 map2[4]

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#2			// R4 <- line/2
	mla	r7,r4,r5,r7			// R7 pix <- font[line/2 * fonth], attribute row

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load attributes -> LR
2:	ldr	lr,[r7],#8		// LR <- attributes, increment address

	// load next source data -> R2
	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

// byte 0, destination index 0..7, source bit 7..0

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	attr4_1	0,4
	attr4_1	2,0

// byte 1, destination index 8..15, source bit 15..8

	ubfx	r0,lr,#20,#4		// R0 <- attribute 0
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#28,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]
	ldr	lr,[r7,#-4]		// LR <- attributes

	attr4_1	4,12
	attr4_1	6,8

// byte 2, destination index 16..23, source bit 23..16

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	attr4_1 8,20
	attr4_1 10,16

// byte 3, destination index 24..31, source bit 31..24

	ubfx	r0,lr,#20,#4		// R0 <- attribute 0
	strb	r0,[sp]
	ubfx	r0,lr,#16,#4		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#28,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#24,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	attr4_1 12,28
	attr4_1 14,24

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

	// load attributes -> LR
	ldrh	lr,[r7],#2		// LR <- attributes, increment

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	rev	r2,r2			// reverze byte order

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = pointer to attributes
//  IP = source buffer
//  LR = attribute word with 8 pixels

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	// prepare middle loop counter -> R4
	lsrs	r4,r3,#2		// R4 <- width / 4
	beq	6f

	.align 2,,			// align address to 4-byte boundary

	// middle loop
1:	attr4_1	0,28
	attr4_1	2,24

	ldrh	lr,[r7],#2		// LR <- attributes, increment

	ubfx	r0,lr,#4,#4		// R0 <- attribute 0
	strb	r0,[sp]
	and	r0,lr,#0x0f		// R0 <- attribute 1
	strb	r0,[sp,#1]
	ubfx	r0,lr,#12,#4		// R0 <- attribute 2
	strb	r0,[sp,#2]
	ubfx	r0,lr,#8,#4		// R0 <- attribute 3
	strb	r0,[sp,#3]

	// next loop
	adds	r5,#4*2			// R5 <- shift destination address
	lsls	r2,r2,#8		// R2 <- shift sample left by 8 bits
	subs	r4,#1			// decrement loop counter
	bne	1b

	// prepare small loop counter -> R4
6:	ands	r4,r3,#3
	beq	8f

	// process one pixel
7:	lsrs	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ldrb	r0,[sp,r0]		// R0 <- pix[bit]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// restore stack
8:	add	sp,#8

	// pop registers R4-R7 and return
	pop	{r4-r7,pc}
#endif

#if DISPHSTX_USE_FORMAT_ATTR5_PAL
// ----------------------------------------------------------------------------
//   attribute compression 5, 8-bit paletted colors, cell 8x8, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr5_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 32 pixels
// R5 = destination buffer
// R6 = temporary register 2
// R7 = attributes
.macro attr5_1 dst bit
	ubfx	r0,r2,#\bit+1,#1	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#1		// R6 <- extract bit from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[pix << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[pix << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxDviRender_Attr5_PAL
DispHstxDviRender_Attr5_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#3			// R4 <- line / 8
	mla	r7,r4,r5,r7			// R7 pix <- font[line/8 * fonth], attribute row

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 32 pixels)

	// prepare counter of big loop (by 32 pixels) -> R4
	lsrs	r4,r3,#5		// R4 <- width / 32
	beq	5f			// skip if width/32 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = attributes
//  IP = source buffer

// byte 0, destination index 0..7, source bit 7..0

	attr5_1	0,6
	attr5_1	2,4
	attr5_1	4,2

	ubfx	r0,r2,#1,#1		// R0 <- extract bit from the sample
	and.w	r6,r2,#1		// R6 <- extract bit 0 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#6*2]		// store pixels
	strh	r6,[r5,#7*2]		// store pixels

	adds.w	r7,#2			// R7 <- shift attributes

// byte 1, destination index 8..15, source bit 15..8

	attr5_1	8,14
	attr5_1	10,12
	attr5_1	12,10
	attr5_1	14,8

	adds.w	r7,#2			// R7 <- shift attributes

// byte 2, destination index 16..23, source bit 23..16

	attr5_1	16,22
	attr5_1	18,20
	attr5_1	20,18
	attr5_1	22,16

	adds	r7,#2			// R7 <- shift attributes

// byte 3, destination index 24..31, source bit 31..24

	lsrs	r0,r2,#31		// R0 <- extract bit from the sample
	ubfx	r6,r2,#30,#1		// R6 <- extract bit from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#24*2]		// store pixels
	strh	r6,[r5,#25*2]		// store pixels

	attr5_1	26,28
	attr5_1	28,26
	attr5_1	30,24

	adds	r7,#2			// R7 <- shift attributes

	// shift destination address
	adds	r5,#32*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare remaining width -> R3
5:	ands	r3,r3,#0x1f		// R3 <- width low 5 bits
	beq	8f			// skip if width & 0x1f is 0

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// prepare counter of middle loop (by 8 pixels) -> R4
	lsrs	r4,r3,#3		// R4 <- width / 8
	beq	1f

	.align 2,,			// align address to 4-byte boundary

	// middle loop
6:	attr5_1	0,6
	attr5_1	2,4
	attr5_1	4,2

	ubfx	r0,r2,#1,#1		// R0 <- extract bit from the sample
	and.w	r6,r2,#1		// R6 <- extract bit 0 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#6*2]		// store pixels
	strh	r6,[r5,#7*2]		// store pixels

	adds	r7,#2			// R7 <- shift attributes
	adds	r5,#8*2			// R5 <- shift destination address
	lsrs	r2,r2,#8		// R2 <- shift sample right by 8 bits

	subs	r4,#1			// decrement loop counter
	bne	6b

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = remaining width
//  R4 = loop counter
//  R5 = destination buffer
//  R7 = attributes
//  IP = source buffer

1:	ands	r4,r3,#0x7		// R4 <- width low 3 bits
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
7:	ubfx	r0,r2,#7,#1		// R0 <- extract bit 7 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_ATTR6_PAL
// ----------------------------------------------------------------------------
//   attribute compression 6, 8-bit paletted colors, cell 4x4, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr6_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..14
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 32 pixels
// R5 = destination buffer
// R6 = temporary register 2
// R7 = attributes
.macro attr6_1 dst bit
	ubfx	r0,r2,#\bit+1,#1	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#1		// R6 <- extract bit from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[pix << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[pix << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.thumb_func
.global DispHstxDviRender_Attr6_PAL
DispHstxDviRender_Attr6_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	lsrs	r7,r1,#1			// R7 <- line/2
	mla	ip,r7,r5,r4			// IP <- line/2*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#2			// R4 <- line/4
	mla	r7,r4,r5,r7			// R7 pix <- font[line/4 * fonth], attribute row

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

	// prepare palette mapping address -> R1
	ands	r4,r1,#1			// check if line is odd
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3 (saves flags)
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// jump to odd line
	bne	DispHstxDviRender_Attr6_PAL_odd	// jump to odd line service

// ---- even line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = attributes
//  IP = source buffer

// byte 0, destination index 0..7, source bit 7..0

	attr6_1	0,6
	attr6_1	2,4
	adds	r7,#2			// R7 <- shift attributes

// byte 1, destination index 8..15, source bit 15..8

	attr6_1	4,14
	attr6_1	6,12
	adds	r7,#2			// R7 <- shift attributes

// byte 2, destination index 16..23, source bit 23..16

	attr6_1	8,22
	attr6_1	10,20
	adds	r7,#2			// R7 <- shift attributes

// byte 3, destination index 24..31, source bit 31..24

	attr6_1	12,30
	attr6_1	14,28
	adds	r7,#2			// R7 <- shift attributes

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- even line - small loop (by 1 pixel)

	// prepare remaining width -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// prepare counter of middle loop (by 8 pixels) -> R4
	lsrs	r4,r3,#2		// R4 <- width / 4
	beq	1f

	.align 2,,			// align address to 4-byte boundary

	// middle loop
6:	attr6_1	0,6
	attr6_1	2,4

	adds	r7,#2			// R7 <- shift attributes
	adds	r5,#4*2			// R5 <- shift destination address
	lsrs	r2,r2,#8		// R2 <- shift sample right by 8 bits

	subs	r4,#1			// decrement loop counter
	bne	6b

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = remaining width
//  R4 = loop counter
//  R5 = destination buffer
//  R7 = attributes
//  IP = source buffer

1:	ands	r4,r3,#0x3		// R4 <- width low 2 bits
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
7:	ubfx	r0,r2,#7,#1		// R0 <- extract bit 7 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

DispHstxDviRender_Attr6_PAL_odd:

// ---- odd line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = attributes
//  IP = source buffer

// byte 0, destination index 0..7, source bit 7..0

	attr6_1	0,2
	attr6_1	2,0
	adds	r7,#2			// R7 <- shift attributes

// byte 1, destination index 8..15, source bit 15..8

	attr6_1	4,10
	attr6_1	6,8
	adds	r7,#2			// R7 <- shift attributes

// byte 2, destination index 16..23, source bit 23..16

	attr6_1	8,18
	attr6_1	10,16
	adds	r7,#2			// R7 <- shift attributes

// byte 3, destination index 24..31, source bit 31..24

	attr6_1	12,26
	attr6_1	14,24
	adds	r7,#2			// R7 <- shift attributes

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- odd line - small loop (by 1 pixel)

	// prepare remaining width -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// prepare counter of middle loop (by 8 pixels) -> R4
	lsrs	r4,r3,#2		// R4 <- width / 4
	beq	1f

	.align 2,,			// align address to 4-byte boundary

	// middle loop
6:	attr6_1	0,2
	attr6_1	2,0

	adds	r7,#2			// R7 <- shift attributes
	adds	r5,#4*2			// R5 <- shift destination address
	lsrs	r2,r2,#8		// R2 <- shift sample right by 8 bits

	subs	r4,#1			// decrement loop counter
	bne	6b

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = remaining width
//  R4 = loop counter
//  R5 = destination buffer
//  R7 = attributes
//  IP = source buffer

1:	ands	r4,r3,#0x3		// R4 <- width low 2 bits
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
7:	ubfx	r0,r2,#3,#1		// R0 <- extract bit 3 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
// Note: if you update pop registers, update them also in pop inside the function
8:	pop	{r4-r7}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_ATTR7_PAL
// ----------------------------------------------------------------------------
//   attribute compression 7, 8-bit paletted colors, cell 8x8, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr7_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..14
//   bit ... source bit index 0..28
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 16 pixels
// R5 = destination buffer
// R6 = temporary register 2
// R7 = attributes
.macro attr7_1 dst bit
	ubfx	r0,r2,#\bit+2,#2	// R0 <- extract 2 bits from the sample
	ubfx	r6,r2,#\bit,#2		// R6 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[pix << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[pix << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.thumb_func
.global DispHstxDviRender_Attr7_PAL
DispHstxDviRender_Attr7_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#3			// R4 <- line / 8
	mla	r7,r4,r5,r7			// R7 pix <- font[line/8 * fonth], attribute row

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = attributes
//  IP = source buffer

// byte 0, destination index 0..7, source bit 7..0

	attr7_1	0,4

	ubfx	r0,r2,#2,#2		// R0 <- extract 2 bits from the sample
	and.w	r6,r2,#3		// R6 <- extract bit 0+1 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#2*2]		// store pixels
	strh	r6,[r5,#3*2]		// store pixels

// byte 1, destination index 8..15, source bit 15..8

	attr7_1	4,12
	attr7_1	6,8
	adds.w	r7,#4			// R7 <- shift attributes

// byte 2, destination index 16..23, source bit 23..16

	attr7_1	8,20
	attr7_1	10,16

// byte 3, destination index 24..31, source bit 31..24

	lsrs.w	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ubfx	r6,r2,#28,#2		// R6 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#12*2]		// store pixels
	strh	r6,[r5,#13*2]		// store pixels

	attr7_1	14,24
	adds	r7,#4			// R7 <- shift attributes

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare remaining width -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x1f is 0

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	rev	r2,r2			// reverze byte order

	// check width 8 pixels
	cmp	r3,#8			// at least 8 pixels?
	blt	7f			// < 8 pixels

	.align 2,,			// align address to 4-byte boundary

	// write 8 pixels
	attr7_1	0,28
	attr7_1	2,24
	attr7_1	4,20
	attr7_1	6,16

	adds	r7,#4			// R7 <- shift attributes
	adds	r5,#8*2			// R5 <- shift destination address
	lsls	r2,r2,#16		// R2 <- shift sample left by 16 bits

	subs	r3,r3,#8		// decrement width
	beq	8f			// no data left

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = remaining width
//  R4 = loop counter
//  R5 = destination buffer
//  R7 = attributes
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
7:	lsrs	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	subs	r3,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_ATTR8_PAL
// ----------------------------------------------------------------------------
//   attribute compression 8, 8-bit paletted colors, cell 4x4, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..14
//   bit ... source bit index 0..28
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 16 pixels
// R5 = destination buffer
// R6 = temporary register 2
// R7 = attributes
.macro attr8_1 dst bit
	ubfx	r0,r2,#\bit+2,#2	// R0 <- extract 2 bits from the sample
	ubfx	r6,r2,#\bit,#2		// R6 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[pix << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[pix << 1]
	strh	r0,[r5,#\dst*2]		// store pixel
	strh	r6,[r5,#\dst*2+2]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxDviRender_Attr8_PAL
DispHstxDviRender_Attr8_PAL:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare pixel mapping -> R7
	ldr	r7,[r0,#DISPHSTXSLOT_FONT]	// R7 <- pointer to font
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH]	// R5 fonth <- attribute pitch
	lsrs	r4,r1,#2			// R4 <- line / 4
	mla	r7,r4,r5,r7			// R7 pix <- font[line/4 * fonth], attribute row

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PAL]	// R1 <- pointer to palettes for DVI

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4		// R4 <- width / 16
	beq	5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> R2
2:	ldr	r2,[ip],#4		// R2 <- [IP], increment IP

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  R7 = attributes
//  IP = source buffer

// byte 0, destination index 0..7, source bit 7..0

	attr8_1	0,4

	ubfx	r0,r2,#2,#2		// R0 <- extract 2 bits from the sample
	and.w	r6,r2,#3		// R6 <- extract bit 0+1 from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#2*2]		// store pixels
	strh	r6,[r5,#3*2]		// store pixels
	adds.w	r7,#4			// R7 <- shift attributes

// byte 1, destination index 8..15, source bit 15..8

	attr8_1	4,12
	attr8_1	6,8
	adds.w	r7,#4			// R7 <- shift attributes

// byte 2, destination index 16..23, source bit 23..16

	attr8_1	8,20
	attr8_1	10,16
	adds.w	r7,#4			// R7 <- shift attributes

// byte 3, destination index 24..31, source bit 31..24

	lsrs.w	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ubfx	r6,r2,#28,#2		// R6 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	ldrh	r6,[r1,r6,lsl #1]	// R6 <- map[bit << 1]
	strh	r0,[r5,#12*2]		// store pixels
	strh	r6,[r5,#13*2]		// store pixels

	attr8_1	14,24
	adds	r7,#4			// R7 <- shift attributes

	// shift destination address
	adds	r5,#16*2		// R5 <- shift destination address

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare remaining width -> R3
5:	ands	r3,r3,#0x0f		// R3 <- width low 4 bits
	beq	8f			// skip if width & 0x1f is 0

	// load next source data -> R2
	ldr	r2,[ip]			// R2 <- [IP]
	rev	r2,r2			// reverze byte order

	// prepare counter of middle loop (by 8 pixels) -> R4
	lsrs	r4,r3,#2		// R4 <- width / 4
	beq	1f

	.align 2,,			// align address to 4-byte boundary

	// middle loop
6:	attr8_1	0,28
	attr8_1	2,24

	adds	r7,#4			// R7 <- shift attributes
	adds	r5,#4*2			// R5 <- shift destination address
	lsls	r2,r2,#8		// R2 <- shift sample left by 8 bits

	subs	r4,#1			// decrement loop counter
	bne	6b

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 32 pixels
//  R3 = remaining width
//  R4 = loop counter
//  R5 = destination buffer
//  R7 = attributes
//  IP = source buffer

1:	ands	r4,r3,#0x3		// R4 <- width low 2 bits
	beq	8f

	.align 2,,			// align address to 4-byte boundary

	// process one pixel
7:	lsrs	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	ldrh	r0,[r1,r0,lsl #1]	// R0 <- map[bit << 1]
	subs	r3,#1			// decrement loop counter
	strh	r0,[r5],#2		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
#endif

#endif // DISPHSTX_ARM_ASM && !RISCV && DISPHSTX_USE_DVI

#endif // USE_DISPHSTX
