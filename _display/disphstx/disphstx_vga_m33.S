// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                         HSTX VGA driver - ARM assembler
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

#if DISPHSTX_ARM_ASM && !RISCV && DISPHSTX_USE_VGA	// 1 = use ARM assembler for render functions

	.section .time_critical.hstx.vga, "ax"
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
#define DISPHSTXSLOT_PALVGA		0x28	// offset of u32* sDispHstxVSlot.palvga ... pointer to palettes for VGA
#define DISPHSTXSLOT_FONT		0x2C	// offset of const u8* font ... pointer to font (width 8 pixels)

#define DISPHSTXSLOT_VCOLOR		0x60	// offset of copy of pixel color format descriptor
#define DISPHSTXCOLOR_PALNUM		(DISPHSTXSLOT_VCOLOR+0x0A) // offset of number of palette entries

#define TIMER0_BASE			0x400b0000	// timer 0 base
#define TIMER0_RAWL			0x28		// timer 0 - raw read LOW offset

// ----------------------------------------------------------------------------
//                          do-nothing render
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_No(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_No
DispHstxVgaRender_No:
	bx	lr

#if DISPHSTX_USE_FORMAT_1 || DISPHSTX_USE_FORMAT_1_PAL
// ----------------------------------------------------------------------------
//                    1 bit per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_1(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// R0 = temporary register
// R1 = palette mapping
// R2 = source sample with 32 pixels
// R5 = destination buffer
// R6 = temporary register 2
.macro store1 dst bit
	ubfx	r0,r2,#\bit+1,#1	// R0 <- extract bit from the sample
	ubfx	r6,r2,#\bit,#1		// R6 <- extract bit from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxVgaRender_1
DispHstxVgaRender_1:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+1		// R1 <- palette + odd*2

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
//  IP = source buffer

// byte 0, destination index 0..7, source bit 7..0

	store1	0,6
	store1	2,4
	store1	4,2
	store1	6,0

// byte 1, destination index 8..15, source bit 15..8

	store1	8,14
	store1	10,12
	store1	12,10
	store1	14,8

// byte 2, destination index 16..23, source bit 23..16

	store1	16,22
	store1	18,20
	store1	20,18
	store1	22,16

// byte 3, destination index 24..31, source bit 31..24

	store1	24,30
	store1	26,28
	store1	28,26
	store1	30,24

	// shift destination address
	adds	r5,#32*4		// R5 <- shift destination address

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_2 || DISPHSTX_USE_FORMAT_2_PAL
// ----------------------------------------------------------------------------
//                   2 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_2(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// macro - store 2x 2-bit pixel
//   dst ... destination index 0..30
//   bit ... source 2-bit index 0..28
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 16 pixels
// R5 = destination buffer
// R6 = temporary register 2
.macro store2 dst bit
	ubfx	r0,r2,#\bit+2,#2	// R0 <- extract bits from the sample
	ubfx	r6,r2,#\bit,#2		// R6 <- extract bits from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- mapping[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxVgaRender_2
DispHstxVgaRender_2:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes in u32 format
	add	r1,r1,r3,lsl #2+2		// R1 <- palette + odd*4

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,r3,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

	.align 2,,				// align address to 4-byte boundary

	// load next source word -> R2
2:	ldr	r2,[ip],#4			// R2 <- [IP], increment IP + 4

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 16 pixels
//  R3 = width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  IP = source buffer

// byte 0, destination index 0..3, source bits 7..0

	store2	0,4
	store2	2,0

// byte 1, destination index 4..7, source bits 15..8

	store2	4,12
	store2	6,8

// byte 2, destination index 8..11, source bits 23..16

	store2	8,20
	store2	10,16

// byte 3, destination index 12..15, source bits 31..24

	store2	12,28
	store2	14,24

	// shift destination address
	adds	r5,#16*4		// R5 <- shift destination address

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
	.align 2,,			// align address to 4-byte boundary
	rev	r2,r2			// reverze byte order

	// process one pixel
6:	lsrs	r0,r2,#30		// R0 <- extract bits 30+31 from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_3 || DISPHSTX_USE_FORMAT_3_PAL
// ----------------------------------------------------------------------------
//                             3 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_3(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// macro - store 2x 3-bit pixel
//   dst ... destination index 0..18
//   bit ... source 3-bit index 0..24
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 10 pixels
// R5 = destination buffer
.macro store3 dst bit
	ubfx	r0,r2,#\bit,#3		// R0 <- extract bits from the sample
	ubfx	r6,r2,#\bit+3,#3	// R6 <- extract bits from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- mapping[bit << 2]
	str	r0,[r5,#\dst*4-80]	// store pixel
	str	r6,[r5,#\dst*4+4-80]	// store pixel
.endm

.macro store3B dst bit
	ubfx	r0,r2,#\bit,#3		// R0 <- extract bits from the sample
	ubfx	r6,r2,#\bit+3,#3	// R6 <- extract bits from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- mapping[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxVgaRender_3
DispHstxVgaRender_3:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes in u32 format
	add	r1,r1,r3,lsl #2+3		// R1 <- palette + odd*8

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
2:	ldr	r2,[ip],#8			// R2 <- [IP], increment IP + 8

	// shift destination address
	adds	r5,#20*4			// R5 <- shift destination address

	// decrement loop counter
	subs	r4,#1				// decrement loop counter

// Here is:
//  R0 = temporary register
//  R1 = palette mapping
//  R2 = sample with 10 pixels
//  R3 = remaining width of the slot
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 2
//  IP = source buffer

	store3	0,0
	store3	2,6
	store3	4,12
	store3	6,18
	store3	8,24

	ubfx	r0,r2,#24,#3		// R0 <- extract bits from the sample
	ubfx	r6,r2,#27,#3		// R6 <- extract bits from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- mapping[bit << 2]
	str	r0,[r5,#8*4-80]		// store pixel

	// load next source word -> R2
	ldr	r2,[ip,#-4]		// R2 <- [IP-4]
	str	r6,[r5,#9*4+4-80]	// store pixel

	store3	10,0
	store3	12,6
	store3	14,12
	store3	16,18
	store3	18,24

	// loop counter
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

	.align 2,,				// align address to 4-byte boundary

	// load next source word -> R2
	ldr	r2,[ip]			// R2 <- [IP]

	// check remaining width
	cmp	r3,#10			// check remaining width
	blt	6f			// skip if width < 10

	// middle loop - process 10 pixels
	store3B	0,0
	store3B	2,6
	store3B	4,12
	store3B	6,18
	store3B	8,24

	// shift middle loop
	sub	r3,#10			// subtract 10 pixels
	beq	8f			// no pixels left
	adds	r5,#10*4		// R5 <- shift destination address
	ldr	r2,[ip,#4]		// R2 <- [IP+4]

	// process one pixel
6:	and	r0,r2,#7		// R0 <- extract lowest 3 bits
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	lsrs	r2,r2,#3		// shift sample rigth by 3 bits
	subs	r3,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_4 || DISPHSTX_USE_FORMAT_4_PAL
// ----------------------------------------------------------------------------
//                             4 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_4(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// macro - store 4-bit pixel
//   dst ... destination index 0..31
//   bit ... source 4-bit index 0..24
// R0 = temporary register
// R1 = palette mapping address
// R2 = source sample with 8 pixels
// R5 = destination buffer
.macro store4 dst bit
	ubfx	r0,r2,#\bit+4,#4	// R0 <- extract bits from the sample
	ubfx	r6,r2,#\bit,#4		// R6 <- extract bits from the sample
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- mapping[bit << 2]
	str	r0,[r5,#\dst*4-32]	// store pixel
	str	r6,[r5,#\dst*4+4-32]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxVgaRender_4
DispHstxVgaRender_4:

	// push registers R4-R6
	push	{r4-r6}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes in u32 format
	add	r1,r1,r3,lsl #2+4		// R1 <- palette + odd*16

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 8 pixels)

	// prepare counter of big loop (by 8 pixels) -> R4
	lsrs	r4,r3,#3			// R4 <- width / 8
	beq	5f				// skip if width/8 is 0

	// load next source word -> R2
2:	ldr	r2,[ip],#4			// R2 <- [IP], increment IP + 4

	// shift destination address
	adds	r5,#8*4				// R5 <- shift destination address

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

	store4	0,0
	store4	2,8
	store4	4,16
	store4	6,24

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	lsls	r2,r2,#4		// shift sample left by 4 bits
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R6 and return
8:	pop	{r4-r6}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_6 || DISPHSTX_USE_FORMAT_6_PAL
// ----------------------------------------------------------------------------
//                           6 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_6(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r2,#\bit,#6		// R0 <- extract bits from the sample
	str	r6,[r5,#\dst*4]		// store pixel
.endm

.macro store6B dst bit
	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r7,#\bit,#6		// R0 <- extract bits from the sample
	str	r6,[r5,#\dst*4]		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_6
DispHstxVgaRender_6:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes in u32 format
	add	r1,r1,r3,lsl #2+6		// R1 <- palette + odd*64

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

	and	r0,r2,#0x3f			// R0 <- first pixel

	store6	0,6
	store6	1,12
	store6	2,18
	store6	3,24

	store6B	4,0

	store6B	5,6
	store6B	6,12
	store6B	7,18
	store6B	8,24

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ldrd	r2,r7,[ip,#-8]		// R2 <- [IP+8], R7 <- [IP+12]
	str	r6,[r5,#9*4]		// store pixel

	and	r0,r2,#0x3f			// R0 <- first pixel

	store6	10,6
	store6	11,12
	store6	12,18
	store6	13,24

	store6B	14,0

	store6B	15,6
	store6B	16,12
	store6B	17,18
	store6B	18,24

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	adds	r5,#20*4		// R5 <- shift destination address
	str	r6,[r5,#19*4-80]	// store pixel

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
	and	r0,r2,#0x3f		// R0 <- first pixel

	store6	0,6
	store6	1,12
	store6	2,18
	store6	3,24

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	add	ip,4			// IP <- shift source address
	str	r6,[r5,#4*4]		// store pixel

	// shift middle loop
	adds	r5,#5*4			// R5 <- shift destination address
	subs	r3,#5			// subtract 5 pixels
	bne	1b
	b	8f

	// process one pixel
6:	and	r0,r2,#0x3f		// R0 <- extract lowest 6 bits
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	lsrs	r2,r2,#6		// shift sample rigth by 6 bits
	subs	r3,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_8 || DISPHSTX_USE_FORMAT_8_PAL
// ----------------------------------------------------------------------------
//                           8 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxVgaRender_8
DispHstxVgaRender_8:

	// push registers R4-R7
	push	{r4-r7}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes in u32 format
	add	r1,r1,r3,lsl #2+8		// R1 <- palette + odd*256

	// prepare width of the slot -> R3
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

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

2:	ldrd	r2,r7,[ip,#0]		// R2 <- [IP], R7 <- [IP+4], load next 2 samples
	add	ip,#4*4			// IP <- shift source address

	uxtb	r0,r2			// R0 <- extract bits 0-7 from the sample

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r2,#8,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#0*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r2,#16,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#1*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	lsrs	r0,r2,#24		// R0 <- extract bits 24-31 from the sample
	str	r6,[r5,#2*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	uxtb	r0,r7			// R0 <- extract bits 0-7 from the sample
	str	r6,[r5,#3*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r7,#8,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#4*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r7,#16,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#5*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	lsrs	r0,r7,#24		// R0 <- extract bits 24-31 from the sample
	str	r6,[r5,#6*4]		// store pixel

	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	ldrd	r2,r7,[ip,#-8]		// R2 <- [IP+8], R7 <- [IP+12], load next 2 samples
	str	r0,[r5,#7*4]		// store pixel

	uxtb	r0,r2			// R0 <- extract bits 0-7 from the sample

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r2,#8,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#8*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r2,#16,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#9*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	lsrs	r0,r2,#24		// R0 <- extract bits 24-31 from the sample
	str	r6,[r5,#10*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	uxtb	r0,r7			// R0 <- extract bits 0-7 from the sample
	str	r6,[r5,#11*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r7,#8,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#12*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	ubfx	r0,r7,#16,#8		// R0 <- extract bits from the sample
	str	r6,[r5,#13*4]		// store pixel

	ldr	r6,[r1,r0,lsl #2]	// R6 <- mapping[bit << 2]
	lsrs	r0,r7,#24		// R0 <- extract bits 24-31 from the sample
	str	r6,[r5,#14*4]		// store pixel

	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	str	r0,[r5,#15*4]		// store pixel

	// shift destination address
	adds	r5,#16*4		// R5 <- shift destination address

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_12
// ----------------------------------------------------------------------------
//                           12 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_12(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_12
DispHstxVgaRender_12:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare mapping, blue mapping -> R8, red-green mapping -> R7
	ldr	r8,=DispHstxVgaLev4		// R8 <- blue mapping
	ldr	r7,=DispHstxPalVgaRG12		// R7 <- red-green mapping
	add	r8,r8,r3,lsl #2+4		// R8 <- palette + odd*16
	add	r7,r7,r3,lsl #2+8		// R7 <- palette + odd*256

	// prepare width of the slot -> LR
	ldrh	lr,[r0,#DISPHSTXSLOT_W]		// LR <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 8 pixels)

	// prepare counter of big loop (by 8 pixels) -> R4
	lsrs	r4,lr,#3			// R4 <- width / 8
	beq	5f				// skip if width/8 is 0

// Here is:
//  R0 = temporary register
//  R1 = temporary register 2
//  R2 = sample 1 or 3
//  R3 = sample 2
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = temporary register 3
//  R7 = red-green mapping
//  R8 = blue mapping
//  LR = width of the slot
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

//	[2222 2222 1111 1111 1111 0000 0000 0000]
//	[5555 4444 4444 4444 3333 3333 3333 2222]
//	[7777 7777 7777 6666 6666 6666 5555 5555]

	// load 2 samples
2:	ldrd	r2,r3,[ip],#3*4		// R2 <- [IP], R3 <- [IP+4], load 2 samples, shift source address

	// pixel 0
	and	r0,r2,#0x0f		// R0 <- blue
	ubfx	r1,r2,#4,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#12,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#0*4]		// store pixel

	// pixel 1
	ubfx	r1,r2,#16,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#24,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#1*4]		// store pixel

	// pixel 2
	lsrs	r1,r2,#28		// R1 <- red-green 1
	bfi	r1,r3,#4,#4		// R1 <- insert red-green 2
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r3,#4,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#2*4]		// store pixel

	// pixel 3
	ubfx	r1,r3,#8,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r3,#16,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#3*4]		// store pixel

	// pixel 4
	ubfx	r1,r3,#20,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	lsrs	r0,r3,#28		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel

	// load 3rd sample -> R2
	ldr	r2,[ip,#-4]		// R2 <- [IP+8]
	str	r1,[r5,#4*4]		// store pixel

	// pixel 5
	ands	r1,r2,#0xff		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#8,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#5*4]		// store pixel

	// pixel 6
	ubfx	r1,r2,#12,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#20,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#6*4]		// store pixel

	// pixel 7
	lsrs	r1,r2,#24		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	adds	r5,#8*4			// shift destination address
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#7*4-8*4]	// store pixel

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,lr,#0x07		// R4 <- width low 3 bits
	beq	8f			// skip if width & 0x07 is 0

	// load 2 samples
	ldrd	r2,r3,[ip],#3*4		// R2 <- [IP], R3 <- [IP+4], load 2 samples, shift source address

	// pixel 0
	and	r0,r2,#0x0f		// R0 <- blue
	ubfx	r1,r2,#4,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#12,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#0*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 1
	ubfx	r1,r2,#16,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#24,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#1*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 2
	lsrs	r1,r2,#28		// R1 <- red-green 1
	bfi	r1,r3,#4,#4		// R1 <- insert red-green 2
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r3,#4,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#2*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 3
	ubfx	r1,r3,#8,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r3,#16,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#3*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 4
	ubfx	r1,r3,#20,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	lsrs	r0,r3,#28		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel

	// load 3rd sample -> R2
	ldr	r2,[ip,#-4]		// R2 <- [IP+8]
	str	r1,[r5,#4*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 5
	ands	r1,r2,#0xff		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#8,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#5*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 6
	ubfx	r1,r2,#12,#8		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ubfx	r0,r2,#20,#4		// R0 <- blue
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#6*4]		// store pixel

	subs	r4,#1			// R4 <- decrement loop counter
	beq	8f			// end

	// pixel 7
	lsrs	r1,r2,#24		// R1 <- red-green
	ldr	r6,[r8,r0,lsl #2]	// R6 <- blue mapping
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green mapping
	ldrd	r2,r3,[ip],#3*4		// R2 <- [IP], R3 <- [IP+4], load 2 samples, shift source address
	orrs	r1,r6			// R1 <- compose pixel
	str	r1,[r5,#7*4]		// store pixel

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_15
// ----------------------------------------------------------------------------
//                           15 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_15(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// convert 4 pixels
//  dst = first destination index 0, 4, 8 or 12
//  R0 = temporary register
//  R1 = sample with 2 pixels
//  R2 = next sample with 2 pixels
//  R3 = temporary register 2
//  R5 = destination buffer
//  R6 = blue mapping
//  R7 = temporary register 3
//  R8 = red-green mapping
.macro store15 dst
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r3,r1,#5,#10		// R3 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
	ubfx	r7,r1,#16,#5		// R7 <- blue
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#\dst*4+0*4]	// store pixel

	ubfx	r3,r1,#21,#10		// R3 <- red-green
	ldr	r0,[r6,r7,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
	and	r7,r2,#0x1f		// R7 <- blue
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#\dst*4+1*4]	// store pixel

	ubfx	r3,r2,#5,#10		// R3 <- red-green
	ldr	r0,[r6,r7,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
	ubfx	r7,r2,#16,#5		// R7 <- blue
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#\dst*4+2*4]	// store pixel

	ubfx	r3,r2,#21,#10		// R3 <- red-green
	ldr	r0,[r6,r7,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
//	orrs	r0,r3			// R0 <- compose pixel
//	str	r0,[r5,#\dst*4+3*4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_15
DispHstxVgaRender_15:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare mapping, blue mapping -> R6, red-green mapping -> R8
	ldr	r6,=DispHstxVgaLev5		// R6 <- blue mapping
	ldr	r8,=DispHstxPalVgaRG		// R8 <- red-green mapping
	add	r6,r6,r3,lsl #2+5		// R6 <- palette + odd*32
	add	r8,r8,r3,lsl #2+10		// R8 <- palette + odd*1024

	// prepare width of the slot -> LR
	ldrh	lr,[r0,#DISPHSTXSLOT_W]		// LR <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,lr,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

// Here is:
//  R0 = temporary register
//  R1 = sample with 2 pixels
//  R2 = next sample with 2 pixels
//  R3 = temporary register 2
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = blue mapping
//  R7 = temporary register 3
//  R8 = red-green mapping
//  LR = width of the slot
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

	// load 2 samples
2:	ldrd	r1,r2,[ip],#8*4		// R1 <- [IP], R2 <- [IP+4], load 4 pixels, shift source address

	// pixels 0-3
	store15	0

	// pixels 4-7
	ldrd	r1,r2,[ip,#2*4-8*4]	// R1 <- [IP], R2 <- [IP+4], load 4 pixels
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#3*4]		// store pixel
	store15	4

	// pixels 8-11
	ldrd	r1,r2,[ip,#4*4-8*4]	// R1 <- [IP], R2 <- [IP+4], load 4 pixels
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#7*4]		// store pixel
	store15	8

	// pixels 12-15
	ldrd	r1,r2,[ip,#6*4-8*4]	// R1 <- [IP], R2 <- [IP+4], load 4 pixels
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#11*4]		// store pixel
	store15	12

	// loop counter
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#15*4]		// store pixel
	adds	r5,#16*4		// R5 <- shift destination address
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,lr,#0x0f		// R4 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

// Here is:
//  R0 = temporary register
//  R1 = sample with 1 pixel
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = blue mapping
//  R8 = red-green mapping
//  IP = source buffer

	// process one pixel
6:	ldrh	r1,[ip],#2		// R1 <- [IP], load 1 pixels, shift source address
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r1,r1,#5,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r8,r1,lsl #2]	// R1 <- red-green
	orrs	r0,r1			// R0 <- compose pixel
	str	r0,[r5],#4		// store pixel, increment pointer
	subs	r4,#1			// decrement loop counter
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_16
// ----------------------------------------------------------------------------
//                           16 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_16(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

// convert 4 pixels
//  dst = first destination index 0, 4, 8 or 12
//  R0 = temporary register
//  R1 = sample with 2 pixels
//  R2 = next sample with 2 pixels
//  R3 = temporary register 2
//  R5 = destination buffer
//  R6 = blue mapping
//  R7 = temporary register 3
//  R8 = red-green mapping
.macro store16 dst
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r3,r1,#6,#10		// R3 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
	ubfx	r7,r1,#16,#5		// R7 <- blue
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#\dst*4+0*4]	// store pixel

	lsrs.w	r3,r1,#22		// R3 <- red-green
	ldr	r0,[r6,r7,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
	and	r7,r2,#0x1f		// R7 <- blue
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#\dst*4+1*4]	// store pixel

	ubfx	r3,r2,#6,#10		// R3 <- red-green
	ldr	r0,[r6,r7,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
	ubfx	r7,r2,#16,#5		// R7 <- blue
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#\dst*4+2*4]	// store pixel

	lsrs.w	r3,r2,#22		// R3 <- red-green
	ldr	r0,[r6,r7,lsl #2]	// R0 <- blue
	ldr	r3,[r8,r3,lsl #2]	// R3 <- red-green
//	orrs	r0,r3			// R0 <- compose pixel
//	str	r0,[r5,#\dst*4+3*4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_16
DispHstxVgaRender_16:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source address -> IP
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	ip,r1,r5,r4			// IP <- line*pitch+base

	// prepare mapping, blue mapping -> R6, red-green mapping -> R8
	ldr	r6,=DispHstxVgaLev5		// R6 <- blue mapping
	ldr	r8,=DispHstxPalVgaRG		// R8 <- red-green mapping
	add	r6,r6,r3,lsl #2+5		// R6 <- palette + odd*32
	add	r8,r8,r3,lsl #2+10		// R8 <- palette + odd*1024

	// prepare width of the slot -> LR
	ldrh	lr,[r0,#DISPHSTXSLOT_W]		// LR <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> R4
	lsrs	r4,lr,#4			// R4 <- width / 16
	beq	5f				// skip if width/16 is 0

// Here is:
//  R0 = temporary register
//  R1 = sample with 2 pixels
//  R2 = next sample with 2 pixels
//  R3 = temporary register 2
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = blue mapping
//  R7 = temporary register 3
//  R8 = red-green mapping
//  LR = width of the slot
//  IP = source buffer

	.align 2,,			// align address to 4-byte boundary

	// load 2 samples
2:	ldrd	r1,r2,[ip],#8*4		// R1 <- [IP], R2 <- [IP+4], load 4 pixels, shift source address

	// pixels 0-3
	store16	0

	// pixels 4-7
	ldrd	r1,r2,[ip,#2*4-8*4]	// R1 <- [IP], R2 <- [IP+4], load 4 pixels
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#3*4]		// store pixel
	store16	4

	// pixels 8-11
	ldrd	r1,r2,[ip,#4*4-8*4]	// R1 <- [IP], R2 <- [IP+4], load 4 pixels
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#7*4]		// store pixel
	store16	8

	// pixels 12-15
	ldrd	r1,r2,[ip,#6*4-8*4]	// R1 <- [IP], R2 <- [IP+4], load 4 pixels
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#11*4]		// store pixel
	store16	12

	// loop counter
	orrs	r0,r3			// R0 <- compose pixel
	str	r0,[r5,#15*4]		// store pixel
	adds	r5,#16*4		// R5 <- shift destination address
	subs	r4,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	ands	r4,lr,#0x0f		// R4 <- width low 4 bits
	beq	8f			// skip if width & 0x0f is 0

// Here is:
//  R0 = temporary register
//  R1 = sample with 1 pixel
//  R4 = loop counter
//  R5 = destination buffer
//  R6 = blue mapping
//  R8 = red-green mapping
//  IP = source buffer

	// process one pixel
6:	ldrh	r1,[ip],#2		// R1 <- [IP], load 1 pixels, shift source address
	and	r0,r1,#0x1f		// R0 <- blue
	lsrs	r1,r1,#6		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r8,r1,lsl #2]	// R1 <- red-green
	orrs	r0,r1			// R0 <- compose pixel
	str	r0,[r5],#4		// store pixel, increment pointer
	subs	r4,#1			// decrement loop counter
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}
.ltorg
#endif

#if DISPHSTX_USE_FORMAT_COL
// ----------------------------------------------------------------------------
//       multiply color lines (palette color per every line, modulo fonth)
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_COL(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_COL
DispHstxVgaRender_COL:

	push	{r4}

	ldr	r4,[r2]			// R4 <- destination buffer
	ldrsh	r2,[r0,#DISPHSTXSLOT_FONTH] // R2 <- fonth (modulo lines)
	ldr	r0,[r0,#DISPHSTXSLOT_PALVGA] // R0 <- palettes
	sdiv	ip,r1,r2		// IP <- line / fonth
	mls	r1,ip,r2,r1		// R1 <- line - line/fonth*fonth = line % fonth
	mla	r1,r3,r2,r1		// R1 <- odd*fonth + line%fonth
	ldr	r3,[r0,r1,lsl #2]	// R3 <- load palette
	str	r3,[r4]			// store palette

	pop	{r4}
	bx	lr
#endif

#if DISPHSTX_USE_FORMAT_MTEXT
// ----------------------------------------------------------------------------
// monocolor text, font width 8 pixels, colors of every row as 2 palette colors (BG,FG)
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_MTEXT(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxVgaRender_MTEXT
DispHstxVgaRender_MTEXT:

	// push registers R4-R8
	push	{r4-r8,lr}
	mov	r6,r3				// R6 <- save odd flag

	// load font height -> R3
	ldrsh	r3,[r0,#DISPHSTXSLOT_FONTH]	// R3 fonth <- height of font

	// get row index -> R4, subline -> R1
	sdiv	r4,r1,r3			// R4 row <- line/fonth
	mls	r1,r3,r4,r1			// R1 subline <- line - row*fonth

	// prepare cursor position -> R8
	mov	r8,0				// R8 <- invalid cursor
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
	ldrb	r8,[r0,#DISPHSTXSLOT_CURPOS]	// R8 <- cursor position
	adds	r8,#1				// R8 <- correction + 1

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
	ldr	r5,[r0,#DISPHSTXSLOT_PALVGA]	// R5 pal <- pointer to palettes in u32 format
	add	r4,r5,r4,lsl #3			// R4 map <- row*4*2+pal, pointer to entries in palette buffer
	ldrh	r7,[r0,#DISPHSTXCOLOR_PALNUM]	// R7 palnum <- number of palettes
	mul	r6,r6,r7			// R6 <- palnum*odd
	add	r4,r4,r6,lsl #2			// R4 map <- palette + palnum*odd*4

	// width -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 w <- width of slot in graphics pixels

// ---- big loop (by 8 pixels of 1 character)

	lsrs	lr,r5,#3			// LR <- width / 8
	beq	5f				// skip if width/8 is 0

// Here is:
//  R0 = temporary
//  R1 = temporary 2
//  R2 = (dst) pointer to destination buffer
//  R3 = (font) pointer to line in the font
//  R4 = (map) pointer to palettes in u32 format
//  R5 = (w) width of slot in graphics pixels
//  R6 = temporary 3
//  R7 = (k) font sample with 8 pixels
//  R8 = (curpos) cursor position
//  IP = (src) pointer to source buffer
//  LR = (i) loop counter

	.align 2,,			// align address to 4-byte boundary

	// load font sample -> R7
2:	ldrb	r7,[ip],#1		// R7 <- load source byte (character), shift source address
	subs	r8,#1			// R8 <- decrement cursor position
	ldrb	r7,[r3,r7]		// R7 <- load font sample
	beq	9f			// skip if cursor
3:	add	r2,#8*4			// R2 <- shift destination address

	// pixels 0+1, bits 7+6
	ubfx	r0,r7,#7,#1		// R0 <- extract bit 7
	ubfx	r1,r7,#6,#1		// R1 <- extract bit 6
	ldr	r0,[r4,r0,lsl #2]	// R0 <- load pixel
	ldr	r1,[r4,r1,lsl #2]	// R1 <- load pixel
	ubfx	r6,r7,#5,#1		// R6 <- extract bit 5
	strd	r0,r1,[r2,#0*4-8*4]	// store pixels 0 and 1

	// pixels 2+3, bits 5+4
	ubfx	r1,r7,#4,#1		// R1 <- extract bit 4
	ldr	r0,[r4,r6,lsl #2]	// R0 <- load pixel
	ldr	r1,[r4,r1,lsl #2]	// R1 <- load pixel
	ubfx	r6,r7,#3,#1		// R1 <- extract bit 3
	strd	r0,r1,[r2,#2*4-8*4]	// store pixels 2 and 3

	// pixels 4+5, bits 3+2
	ubfx	r1,r7,#2,#1		// R1 <- extract bit 2
	ldr	r0,[r4,r6,lsl #2]	// R0 <- load pixel
	ldr	r1,[r4,r1,lsl #2]	// R1 <- load pixel
	ubfx	r6,r7,#1,#1		// R1 <- extract bit 1
	strd	r0,r1,[r2,#4*4-8*4]	// store pixels 4 and 5

	// pixels 6+7, bits 1+0
	and	r1,r7,#1		// R1 <- extract bit 0
	ldr	r0,[r4,r6,lsl #2]	// R0 <- load pixel
	ldr	r1,[r4,r1,lsl #2]	// R1 <- load pixel
	subs	lr,#1			// decrement loop counter
	strd	r0,r1,[r2,#6*4-8*4]	// store pixels 6 and 7

	// loop counter
	bne	2b

// ---- small loop (by 1 pixel)

5:	ands	r6,r5,#7		// R6 <- width & 7, small loop counter
	beq	8f			// skip small loop (if loop counter = 0)

	// load font sample -> R7
	ldrb	r7,[ip]			// R7 <- load source byte (character)
	subs	r8,#1			// R8 <- decrement cursor position
	ldrb	r7,[r3,r7]		// R7 <- load font sample
	beq	10f			// skip if cursor

	.align 2,,			// align address to 4-byte boundary

// Here is:
//  R0 = temporary
//  R2 = (dst) pointer to destination buffer
//  R4 = (map) pointer to palettes in u32 format
//  R6 = (i) loop counter
//  R7 = (k) font sample with 8 pixels
//  R8 = (curpos) cursor position

6:	ubfx	r0,r7,#7,#1		// R0 <- extract bit 7
	ldr	r0,[r4,r0,lsl #2]	// R0 <- load pixel
	lsls	r7,r7,#1		// R7 <- shift sample 1 bit left
	subs	r6,#1			// decrement loop counter
	str	r0,[r2],#4		// store pixel, increment pointer
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_ATEXT(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxVgaRender_ATEXT
DispHstxVgaRender_ATEXT:

	// push registers R4-R9,LR
	push	{r4-r9,lr}
	mov	r6,r3				// R6 <- save odd flag

	// reserve space in stack for local variables
	sub	sp,#8				// reserve space in stack for local variable: u32 map2[2]

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
	ldr	r4,[r0,#DISPHSTXSLOT_PALVGA]	// R4 pal <- pointer to palettes in u32 format map[]
	add	r4,r4,r6,lsl #2+4		// R4 pal <- add odd*4*16

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
//  R4 = (map) pointer to palettes in u32 format
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

// 1st character

	// prepare palettes map2[2] in the stack
	ubfx	r0,r7,#12,#4		// R0 <- extract bits of background attribute
	ldr	r0,[r4,r0,lsl #2]	// R0 <- background color
	ubfx	r1,r7,#8,#4		// R1 <- extract bits of foreground attribute
	ldr	r1,[r4,r1,lsl #2]	// R1 <- foreground color
	subs	r9,#1			// R9 <- decrement cursor position
	strd	r0,r1,[sp]		// map2[0] <- color0, map2[1] <- color1

	// load font sample -> R6
	uxtb	r6,r7			// R6 <- 1st character
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	beq	9f			// skip if cursor

	// pixels 0+1, bits 6+7
3:	ubfx	r0,r6,#7,#1		// R0 <- extract bit 7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#6,#1		// R1 <- extract bit 6
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#5,#1		// R5 <- extract bit 5
	strd	r0,r1,[r2,#0*4]		// store pixels

	// pixels 2+3, bits 4+5
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#4,#1		// R1 <- extract bit 4
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#3,#1		// R5 <- extract bit 3
	strd	r0,r1,[r2,#2*4]		// store pixels

	// pixels 4+5, bits 2+3
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#2,#1		// R1 <- extract bit 2
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#1,#1		// R5 <- extract bit 1
	strd	r0,r1,[r2,#4*4]		// store pixels

	// pixels 6+7, bits 0+1
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	and	r1,r6,#1		// R1 <- extract bit 
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	lsrs	r5,r7,#28		// R5 <- extract bits of background attribute
	strd	r0,r1,[r2,#6*4]		// store pixels

// 2nd character

	// prepare palettes map2[2] in the stack
	ldr	r0,[r4,r5,lsl #2]	// R0 <- background color
	ubfx	r1,r7,#24,#4		// R1 <- extract bits of foreground attribute
	ldr	r1,[r4,r1,lsl #2]	// R1 <- foreground color
	ubfx	r6,r7,#16,#8		// R6 <- 2nd character
	strd	r0,r1,[sp]		// map2[0] <- color0, map2[1] <- color1

	// load font sample -> R6
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	add	r2,#16*4		// R2 <- shift destination address
	subs	r9,#1			// R9 <- decrement cursor position
	beq	10f			// skip if cursor

	// pixels 8+9, bits 6+7
4:	ubfx	r0,r6,#7,#1		// R0 <- extract bit 7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#6,#1		// R1 <- extract bit 6
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#5,#1		// R1 <- extract bit 5
	strd	r0,r1,[r2,#8*4-16*4]	// store pixels

	// pixels 10+11, bits 4+5
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#4,#1		// R1 <- extract bit 4
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#3,#1		// R1 <- extract bit 3
	strd	r0,r1,[r2,#10*4-16*4]	// store pixels

	// pixels 12+13, bits 2+3
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#2,#1		// R1 <- extract bit 2
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#1,#1		// R1 <- extract bit 1
	strd	r0,r1,[r2,#12*4-16*4]	// store pixels

	// pixels 14+15, bits 0+1
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	and	r1,r6,#1		// R1 <- extract bit 
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	subs	r8,#1			// decrement loop counter
	strd	r0,r1,[r2,#14*4-16*4]	// store pixels

	// loop counter
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
//  R4 = (map) pointer to palettes in u32 format
//  R5 = temporary 3
//  R6 = temporary 4, (k) font sample
//  R7 = (kk) font sample with 2 characters, 16 pixels
//  R8 = (i) loop counter
//  R9 = (curpos) cursor position
//  LR = (w) width of slot in graphics pixels
//  IP = (src) pointer to source buffer
//  [SP] = (map2) double-palettes

	// prepare palettes map2[2] in the stack
	ubfx	r0,r7,#12,#4		// R0 <- extract bits of background attribute
	ldr	r0,[r4,r0,lsl #2]	// R0 <- background color
	ubfx	r1,r7,#8,#4		// R1 <- extract bits of foreground attribute
	ldr	r1,[r4,r1,lsl #2]	// R1 <- foreground color
	uxtb	r6,r7			// R6 <- 1st character
	strd	r0,r1,[sp]		// map2[0] <- color0, map2[1] <- color1

	// load font sample -> R6
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	subs	r9,#1			// R9 <- decrement cursor position
	it	eq
	mvneq	r6,r6			// R6 <- invert sample

	// counter >= 8 ?
	cmp	r8,#8
	blt	6f			// count < 8

	.align 2,,			// align address to 4-byte boundary

	// pixels 0+1, bits 6+7
	ubfx	r0,r6,#7,#1		// R0 <- extract bit 7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#6,#1		// R1 <- extract bit 6
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#5,#1		// R5 <- extract bit 5
	strd	r0,r1,[r2,#0*4]		// store pixels

	// pixels 2+3, bits 4+5
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#4,#1		// R1 <- extract bit 4
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#3,#1		// R5 <- extract bit 3
	strd	r0,r1,[r2,#2*4]		// store pixels

	// pixels 4+5, bits 2+3
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	ubfx	r1,r6,#2,#1		// R1 <- extract bit 2
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	ubfx	r5,r6,#1,#1		// R5 <- extract bit 1
	strd	r0,r1,[r2,#4*4]		// store pixels

	// pixels 6+7, bits 0+1
	ldr	r0,[sp,r5,lsl #2]	// R0 <- load pixel
	and	r1,r6,#1		// R1 <- extract bit 
	ldr	r1,[sp,r1,lsl #2]	// R1 <- load pixel
	strd	r0,r1,[r2,#6*4]		// store pixels

	// shift counter
	subs	r8,#8			// subtract 8 pixels
	beq	8f			// end

	// prepare palettes map2[2] in the stack
	lsrs	r5,r7,#28		// R5 <- extract bits of background attribute
	ldr	r0,[r4,r5,lsl #2]	// R0 <- background color
	ubfx	r1,r7,#24,#4		// R1 <- extract bits of foreground attribute
	ldr	r1,[r4,r1,lsl #2]	// R1 <- foreground color
	ubfx	r6,r7,#16,#8		// R6 <- 2nd character
	strd	r0,r1,[sp]		// map2[0] <- color0, map2[1] <- color1

	// load font sample -> R6
	ldrb	r6,[r3,r6]		// R6 <- load font sample
	subs	r9,#1			// R9 <- decrement cursor position
	it	eq
	mvneq	r6,r6			// R6 <- invert sample

	// shift destination address
	add	r2,#8*4			// R2 <- shift destination address

	// small loop
6:	ubfx	r0,r6,#7,#1		// R0 <- extract bit 7
	ldr	r0,[sp,r0,lsl #2]	// R0 <- load pixel, 
	lsls	r6,r6,#1		// R6 <- shift sample 1 bit left
	str	r0,[r2],#4		// store pixel, increment pointer

	// loop counter
	subs	r8,#1			// decrement loop counter
	bne	6b

	// restore stack
8:	add	sp,#8

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

#if DISPHSTX_USE_FORMAT_TILE4_8 || DISPHSTX_USE_FORMAT_TILE4_8_PAL
// ----------------------------------------------------------------------------
//                               tiles 4x4 pixels
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_TILE4_8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_TILE4_8
DispHstxVgaRender_TILE4_8:

	// push registers R4-R7
	push	{r4-r7,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PALVGA]	// R7 <- pointer to palettes in u32 format
	add	r7,r7,r3,lsl #2+8		// R7 <- palette + odd*256

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

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

	.align 2,,			// align address to 4-byte boundary

// 1st tile

	// load sample with tile index -> R6
2:	ldrb	r6,[ip]			// R6 <- load tile index

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	str	r0,[lr,#0*4]		// store pixel

	// pixel 1
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	str	r0,[lr,#1*4]		// store pixel

	// pixel 2
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#24,#8		// R1 <- pixel 3
	str	r0,[lr,#2*4]		// store pixel

	// pixel 3
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel

// 2nd tile

	// load sample with tile index -> R6
	ldrb	r6,[ip,#1]		// R6 <- load tile index
	str	r0,[lr,#3*4]		// store pixel

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	str	r0,[lr,#4*4]		// store pixel

	// pixel 1
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	str	r0,[lr,#5*4]		// store pixel

	// pixel 2
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#24,#8		// R1 <- pixel 3
	str	r0,[lr,#6*4]		// store pixel

	// pixel 3
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel

// 3rd tile

	// load sample with tile index -> R6
	ldrb	r6,[ip,#2]		// R6 <- load tile index
	str	r0,[lr,#7*4]		// store pixel

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	str	r0,[lr,#8*4]		// store pixel

	// pixel 1
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	str	r0,[lr,#9*4]		// store pixel

	// pixel 2
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#24,#8		// R1 <- pixel 3
	str	r0,[lr,#10*4]		// store pixel

	// pixel 3
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel

// 4th tile

	// load sample with tile index -> R6
	ldrb	r6,[ip,#3]		// R6 <- load tile index
	str	r0,[lr,#11*4]		// store pixel

	// prepare pointer to tile graphics -> R1
	mul	r1,r6,r5		// R1 <- tile offset (index*delta)

	// load first 4 pixels -> R6
	ldr	r6,[r1,r3]		// R6 <- load pixels

	// pixel 0
	uxtb	r1,r6			// R1 <- pixel 0
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	str	r0,[lr,#12*4]		// store pixel

	// pixel 1
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	str	r0,[lr,#13*4]		// store pixel

	// pixel 2
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	lsrs	r1,r6,#24		// R1 <- pixel 3
	subs	r2,#1			// decrement loop counter
	str	r0,[lr,#14*4]		// store pixel

	// pixel 3
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	add	ip,#4			// IP <- shift source address
	str	r0,[lr,#15*4]		// store pixel

	// shift destination address
	add	lr,#16*4		// LR <- shift destination address

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
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#8,#8		// R1 <- pixel 1
	str	r0,[lr,#0*4]		// store pixel

	// pixel 1
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	ubfx	r1,r6,#16,#8		// R1 <- pixel 2
	str	r0,[lr,#1*4]		// store pixel

	// pixel 2
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	lsrs	r1,r6,#24		// R1 <- pixel 3
	subs	r4,#RENDER_TILEW	// decrement counter
	str	r0,[lr,#2*4]		// store pixel

	// pixel 3
	ldr	r0,[r7,r1,lsl #2]	// R0 <- remap pixel
	add	lr,#4*4			// LR <- shift destination address
	str	r0,[lr,#3*4-4*4]	// store pixel

	// shift address and counter
	beq	8f			// no pixels left
	cmp	r4,#RENDER_TILEW	// whole 1 tile left?
	bge	1b	

// ---- small loop (by 1 pixel)
	
5:	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldr	r0,[r7,r0,lsl #2]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	str	r0,[lr],#4		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R7 and return
8:	pop	{r4-r7,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8 || DISPHSTX_USE_FORMAT_TILE8_8_PAL
// ----------------------------------------------------------------------------
//                            tiles 8x8 pixels
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_TILE8_8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_TILE8_8
DispHstxVgaRender_TILE8_8:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PALVGA]	// R7 <- pointer to palettes in u32 format
	add	r7,r7,r3,lsl #2+8		// R7 <- palette + odd*256

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

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

	// Instructions of the loop are 32-bits, they should be aligned to speed-up
	.align 2,,			// align address to 4-byte boundary

	// load sample with tile index -> R6
2:	ldrb.w	r6,[ip]			// R6 <- load tile index

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]			// R6 <- load pixels

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#0*4]		// store pixel

	// pixel 1
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#1*4]		// store pixel

	// pixel 2
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#2*4]		// store pixel

	// pixel 3
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr.w	r6,[r1,#4]		// load next 4 pixels -> R6
	str.w	r0,[lr,#3*4]		// store pixel

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#4*4]		// store pixel

	// pixel 5
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#5*4]		// store pixel

	// pixel 6
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#6*4]		// store pixel

	// load sample with tile index -> R6
	ldrb.w	r6,[ip,#1]		// R6 <- load tile index

	// pixel 7
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]
	str.w	r0,[lr,#7*4]		// store pixel

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#8*4]		// store pixel

	// pixel 1
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#9*4]		// store pixel

	// pixel 2
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#10*4]		// store pixel

	// pixel 3
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr	r6,[r1,#4]		// load next 4 pixels -> R6
	subs	r2,#1			// decrement loop counter
	str.w	r0,[lr,#11*4]		// store pixel

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#12*4]		// store pixel

	// pixel 5
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#13*4]		// store pixel

	// pixel 6
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#14*4]		// store pixel

	// pixel 7
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	add	ip,#2			// IP <- shift source address

	// shift destination address
	str.w	r0,[lr,#15*4]		// store pixel
	add	lr,#16*4		// LR <- shift destination address

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
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#0*4]		// store pixel

	// pixel 1
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#1*4]		// store pixel

	// pixel 2
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#2*4]		// store pixel

	// pixel 3
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr.w	r6,[r1,#4]		// load next 4 pixels -> R6
	str.w	r0,[lr,#3*4]		// store pixel

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#4*4]		// store pixel

	// pixel 5
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#5*4]		// store pixel

	// pixel 6
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#6*4]		// store pixel

	// pixel 7
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	subs	r4,#RENDER_TILEW	// decrement counter
	str.w	r0,[lr,#7*4]		// store pixel

	// shift address and counter
	add	lr,#8*4			// LR <- shift destination address
	ble	8f			// no pixels left

// ---- small loop (by 1 pixel)
	
5:	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldr	r0,[r7,r0,lsl #2]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	str	r0,[lr],#4		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8 || DISPHSTX_USE_FORMAT_TILE16_8_PAL
// ----------------------------------------------------------------------------
//                            tiles 16x16 pixels
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_TILE16_8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_TILE16_8
DispHstxVgaRender_TILE16_8:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PALVGA]	// R7 <- pointer to palettes in u32 format
	add	r7,r7,r3,lsl #2+8		// R7 <- palette + odd*256

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

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

	// Instructions of the loop are 32-bits, they should be aligned to speed-up
	.align 2,,			// align address to 4-byte boundary

	// load sample with tile index -> R6
2:	ldrb.w	r6,[ip],#1		// R6 <- load tile index, increment source address

	// prepare pointer to tile graphics -> R1
	mla.w	r1,r6,r5,r3		// R1 <- tile pointer (index*delta + tiles)

// pixesl 0-3

	// load first 4 pixels -> R6
	ldr.w	r6,[r1]			// R6 <- load pixels

	// pixel 0
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#0*4]		// store pixel

	// pixel 1
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#1*4]		// store pixel

	// pixel 2
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#2*4]		// store pixel

	// pixel 3
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr.w	r6,[r1,#4]		// load next 4 pixels -> R6
	str.w	r0,[lr,#3*4]		// store pixel

// pixesl 4-7

	// pixel 4
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#4*4]		// store pixel

	// pixel 5
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#5*4]		// store pixel

	// pixel 6
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#6*4]		// store pixel

	// pixel 7
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr.w	r6,[r1,#8]		// load next 4 pixels -> R6
	str.w	r0,[lr,#7*4]		// store pixel

// pixesl 8-11

	// pixel 8
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#8*4]		// store pixel

	// pixel 9
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#9*4]		// store pixel

	// pixel 10
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#10*4]		// store pixel

	// pixel 11
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr.w	r6,[r1,#12]		// load next 4 pixels -> R6
	str.w	r0,[lr,#11*4]		// store pixel

// pixesl 12-15

	// pixel 12
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#12*4]		// store pixel

	// pixel 13
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#13*4]		// store pixel

	// pixel 14
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#14*4]		// store pixel

	// pixel 15
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel

	// shift destination address
	add	lr,#16*4		// LR <- shift destination address
	str.w	r0,[lr,#15*4 - 16*4]	// store pixel

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

4:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldr	r0,[r7,r0,lsl #2]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	str	r0,[lr],#4		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8 || DISPHSTX_USE_FORMAT_TILE32_8_PAL
// ----------------------------------------------------------------------------
//                             tiles 32x32 pixels
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_TILE32_8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

// macro - store 4 pixels
//   dst ... destination first index 0, 4, 8, 12, 16, 20, 24
//   src ... next source index 1, 2, 3, 4, 5, 6
.macro store_32_4 dst src
	ubfx.w	r8,r6,#0,#8		// R8 <- pixel 0
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#\dst*4+0]	// store pixel

	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#\dst*4+4]	// store pixel

	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#\dst*4+8]	// store pixel

	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ldr.w	r6,[r1,#\src*4]		// load next 4 pixels -> R6
	str.w	r0,[lr,#\dst*4+12]	// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_TILE32_8
DispHstxVgaRender_TILE32_8:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare source pointer -> IP (src), get row index -> R4
	ldrh	r5,[r0,#DISPHSTXSLOT_PITCH]	// R5 pitch <- pitch of the line
	ldr	r6,[r0,#DISPHSTXSLOT_BUF]	// R6 src <- source buffer base address
	asrs	r4,r1,#RENDER_TILEW_BIT		// R4 row <- line / tile_height
	mla	ip,r4,r5,r6			// IP src <- row*pitch + buf

	// destination buffer -> LR (dst)
	ldr	lr,[r2]				// LR dst <- destination buffer

	// prepare palette mapping -> R7 (map)
	ldr	r7,[r0,#DISPHSTXSLOT_PALVGA]	// R7 <- pointer to palettes in u32 format
	add	r7,r7,r3,lsl #2+8		// R7 <- palette + odd*256

	// prepare width of the slot -> R4 (w)
	ldrh	r4,[r0,#DISPHSTXSLOT_W]		// R4 <- width of slot in graphics pixels

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

	// Instructions of the loop are 32-bits, they should be aligned to speed-up
	.align 2,,			// align address to 4-byte boundary

	// load sample with tile index -> R6
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
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#8,#8		// R8 <- pixel 1
	str.w	r0,[lr,#28*4]		// store pixel

	// pixel 29
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#16,#8		// R8 <- pixel 2
	str.w	r0,[lr,#29*4]		// store pixel

	// pixel 30
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel
	ubfx.w	r8,r6,#24,#8		// R8 <- pixel 3
	str.w	r0,[lr,#30*4]		// store pixel

	// pixel 31
	ldr.w	r0,[r7,r8,lsl #2]	// R0 <- remap pixel

	// shift destination address
	add	lr,#32*4		// LR <- shift destination address
	str.w	r0,[lr,#31*4 - 32*4]	// store pixel

	// loop counter
	subs	r2,#1			// decrement loop counter
	bne	2b

// ---- small loop (by 1 pixel)

4:	ands	r4,#RENDER_TILEW-1	// R4 <- remaining pixels
	beq	8f			// skip small loop (if loop counter = 0)
	
	ldrb	r6,[ip]			// R6 <- load tile index
	mla	r1,r6,r5,r3		// R1 <- tile pointer
6:	ldrb	r0,[r1],#1		// R6 <- load pixel, shift source address
	ldr	r0,[r7,r0,lsl #2]	// R0 <- remap pixel
	subs	r4,#1			// decrement loop counter
	str	r0,[lr],#4		// store pixel, shift destination address
	bne	6b

	// pop registers R4-R8 and return
8:	pop	{r4-r8,pc}

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_HSTX_15
// ----------------------------------------------------------------------------
//                       HSTX RLE compression 15-bit
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_HSTX_15(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_HSTX_15
DispHstxVgaRender_HSTX_15:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare mapping, blue mapping -> R6, red-green mapping -> R7
	ldr	r6,=DispHstxVgaLev5		// R6 <- blue mapping
	ldr	r7,=DispHstxPalVgaRG		// R7 <- red-green mapping
	add	r6,r6,r3,lsl #2+5		// R6 <- palette + odd*32
	add	r7,r7,r3,lsl #2+10		// R7 <- palette + odd*1024

	// prepare width of the slot -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 <- width of slot in graphics pixels

	// prepare destination address -> R4
	ldr	r4,[r2,#0]			// R4 <- destination address of render buffer

	// prepare source address -> IP
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 <- pointer to array of line offsets
	ldr	r3,[r3,r1,lsl #2]		// R1 <- load line offset
	ldr	r1,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	add	ip,r1,r3,lsl #1			// IP <- line start address

// ---- main loop

// Here is:
//  R0 = temporary
//  R1 = temporary 2
//  R2 = small loop counter
//  R3 = (n) data length 
//  R4 = (dst) destination buffer
//  R5 = (w) width of the slot (loop counter)
//  R6 = (mapB) blue mapping
//  R7 = (mapRG) red-green mapping
//  R8 = temporary 3
//  IP = (src) source buffer
//  LR = temporary 4

	.align 2,,			// align address to 4-byte boundary

	// get token -> R0
2:	ldrh	r0,[ip],#2		// load token, increase source pointer

	// data length -> R3
	ubfx	r3,r0,#0,#12		// R3 (n) <- data length
	sub	r5,r3			// R5 (w) <- subtract loop counter

	// check repeat mode
	lsrs	r0,r0,#12		// R0 <- token >> 12
	cmp	r0,#3			// HSTX_CMD_TMDS_REPEAT token?
	bne	6f			// skip if not repeat mode

// repeat mode

	// get color -> R0
	ldrh	r1,[ip],#2		// R1 (k) <- load color, increase source pointer
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r1,r1,#5,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R8 <- red-green

	// prepare loop counter -> R2
	lsrs	r2,r3,#2		// R2 <- data length / 4
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	beq	4f			// skip if zero

	// store color by 4 pixels
3:	strd	r0,r0,[r4],#16
	strd	r0,r0,[r4,#8-16]
	subs	r2,#1			// decrement loop counter
	bne	3b

	// prepare loop counter -> R2
4:	ands	r2,r3,#3		// R2 <- data length % 4
	beq	9f			// continue main loop

	// store color by 1 pixel
5:	str	r0,[r4],#4
	subs	r2,#1			// decrement loop counter
	bne	5b
	b	9f			// continue main loop

// raw mode

	.align 2,,			// align address to 4-byte boundary

	// prepare loop counter -> R2
6:	lsrs	r2,r3,#2		// R2 <- data length / 4
	beq	8f			// skip if zero

	// transfer by 4 pixels
7:	ldrh	r1,[ip],#8		// R1 (k) <- load color, shift source address
	add	r4,#16			// R4 <- shift destination address
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r1,r1,#5,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green

	ldrh	r8,[ip,#2-8]		// R8 (k) <- load color
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	and	r1,r8,#0x1f		// R1 <- blue
	ubfx	r8,r8,#5,#10		// R8 <- red-green
	ldr	r1,[r6,r1,lsl #2]	// R1 <- blue
	ldr	r8,[r7,r8,lsl #2]	// R8 <- red-green
	ldrh	lr,[ip,#4-8]		// LR (k) <- load color
	orr	r1,r1,r8		// R1 (col) <- compose pixel
	strd	r0,r1,[r4,#0-16]	// store 2 pixels

	and	r0,lr,#0x1f		// R0 <- blue
	ubfx	r1,lr,#5,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R8 <- red-green

	ldrh	r8,[ip,#6-8]		// R8 (k) <- load color
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	and	r1,r8,#0x1f		// R0 <- blue
	ubfx	r8,r8,#5,#10		// R1 <- red-green
	ldr	r1,[r6,r1,lsl #2]	// R0 <- blue
	ldr	r8,[r7,r8,lsl #2]	// R8 <- red-green
	subs	r2,#1			// decrement loop counter
	orr	r1,r1,r8		// R1 (col) <- compose pixel
	strd	r0,r1,[r4,#8-16]	// store 2 pixels

	bne	7b

	// prepare loop counter -> R2
8:	ands	r2,r3,#3		// R2 <- data length % 4
	beq	9f			// continue main loop

	.align 2,,			// align address to 4-byte boundary

	// transfer by 1 pixel
1:	ldrh	r1,[ip],#2		// R1 (k) <- load color, shift source address
	add	r4,#4			// R4 <- shift destination address
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r1,r1,#5,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green
	subs	r2,#1			// decrement loop counter
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	str	r0,[r4,#-4]
	bne	1b

	// loop (check remaining width)
9:	cmp	r5,#0
	bgt	2b

	// pop registers R4-R8 and return
	pop	{r4-r8,pc}

.ltorg
#endif

#if DISPHSTX_USE_FORMAT_HSTX_16
// ----------------------------------------------------------------------------
//                       HSTX RLE compression 16-bit
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_HSTX_16(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_HSTX_16
DispHstxVgaRender_HSTX_16:

	// push registers R4-R8
	push	{r4-r8,lr}

	// prepare mapping, blue mapping -> R6, red-green mapping -> R7
	ldr	r6,=DispHstxVgaLev5		// R6 <- blue mapping
	ldr	r7,=DispHstxPalVgaRG		// R7 <- red-green mapping
	add	r6,r6,r3,lsl #2+5		// R6 <- palette + odd*32
	add	r7,r7,r3,lsl #2+10		// R7 <- palette + odd*1024

	// prepare width of the slot -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 <- width of slot in graphics pixels

	// prepare destination address -> R4
	ldr	r4,[r2,#0]			// R4 <- destination address of render buffer

	// prepare source address -> IP
	ldr	r3,[r0,#DISPHSTXSLOT_FONT]	// R3 <- pointer to array of line offsets
	ldr	r3,[r3,r1,lsl #2]		// R1 <- load line offset
	ldr	r1,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	add	ip,r1,r3,lsl #1			// IP <- line start address

// ---- main loop

// Here is:
//  R0 = temporary
//  R1 = temporary 2
//  R2 = small loop counter
//  R3 = (n) data length 
//  R4 = (dst) destination buffer
//  R5 = (w) width of the slot (loop counter)
//  R6 = (mapB) blue mapping
//  R7 = (mapRG) red-green mapping
//  R8 = temporary 3
//  IP = (src) source buffer
//  LR = temporary 4

	.align 2,,			// align address to 4-byte boundary

	// get token -> R0
2:	ldrh	r0,[ip],#2		// load token, increase source pointer

	// data length -> R3
	ubfx	r3,r0,#0,#12		// R3 (n) <- data length
	sub	r5,r3			// R5 (w) <- subtract loop counter

	// check repeat mode
	lsrs	r0,r0,#12		// R0 <- token >> 12
	cmp	r0,#3			// HSTX_CMD_TMDS_REPEAT token?
	bne	6f			// skip if not repeat mode

// repeat mode

	// get color -> R0
	ldrh	r1,[ip],#2		// R1 (k) <- load color, increase source pointer
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r1,r1,#6,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R8 <- red-green

	// prepare loop counter -> R2
	lsrs	r2,r3,#2		// R2 <- data length / 4
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	beq	4f			// skip if zero

	// store color by 4 pixels
3:	strd	r0,r0,[r4],#16
	strd	r0,r0,[r4,#8-16]
	subs	r2,#1			// decrement loop counter
	bne	3b

	// prepare loop counter -> R2
4:	ands	r2,r3,#3		// R2 <- data length % 4
	beq	9f			// continue main loop

	// store color by 1 pixel
5:	str	r0,[r4],#4
	subs	r2,#1			// decrement loop counter
	bne	5b
	b	9f			// continue main loop

// raw mode

	.align 2,,			// align address to 4-byte boundary

	// prepare loop counter -> R2
6:	lsrs	r2,r3,#2		// R2 <- data length / 4
	beq	8f			// skip if zero

	// transfer by 4 pixels
7:	ldrh	r1,[ip],#8		// R1 (k) <- load color, shift source address
	add	r4,#16			// R4 <- shift destination address
	and	r0,r1,#0x1f		// R0 <- blue
	ubfx	r1,r1,#6,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green

	ldrh	r8,[ip,#2-8]		// R8 (k) <- load color
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	and	r1,r8,#0x1f		// R1 <- blue
	ubfx	r8,r8,#6,#10		// R8 <- red-green
	ldr	r1,[r6,r1,lsl #2]	// R1 <- blue
	ldr	r8,[r7,r8,lsl #2]	// R8 <- red-green
	ldrh	lr,[ip,#4-8]		// LR (k) <- load color
	orr	r1,r1,r8		// R1 (col) <- compose pixel
	strd	r0,r1,[r4,#0-16]	// store 2 pixels

	and	r0,lr,#0x1f		// R0 <- blue
	ubfx	r1,lr,#6,#10		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R8 <- red-green

	ldrh	r8,[ip,#6-8]		// R8 (k) <- load color
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	and	r1,r8,#0x1f		// R0 <- blue
	ubfx	r8,r8,#6,#10		// R1 <- red-green
	ldr	r1,[r6,r1,lsl #2]	// R0 <- blue
	ldr	r8,[r7,r8,lsl #2]	// R8 <- red-green
	subs	r2,#1			// decrement loop counter
	orr	r1,r1,r8		// R1 (col) <- compose pixel
	strd	r0,r1,[r4,#8-16]	// store 2 pixels

	bne	7b

	// prepare loop counter -> R2
8:	ands	r2,r3,#3		// R2 <- data length % 4
	beq	9f			// continue main loop

	.align 2,,			// align address to 4-byte boundary

	// transfer by 1 pixel
1:	ldrh	r1,[ip],#2		// R1 (k) <- load color, shift source address
	add	r4,#4			// R4 <- shift destination address
	and	r0,r1,#0x1f		// R0 <- blue
	lsrs	r1,r1,#6		// R1 <- red-green
	ldr	r0,[r6,r0,lsl #2]	// R0 <- blue
	ldr	r1,[r7,r1,lsl #2]	// R1 <- red-green
	subs	r2,#1			// decrement loop counter
	orr	r0,r0,r1		// R0 (col) <- compose pixel
	str	r0,[r4,#-4]
	bne	1b

	// loop (check remaining width)
9:	cmp	r5,#0
	bgt	2b

	// pop registers R4-R8 and return
	pop	{r4-r8,pc}

.ltorg
#endif

#if DISPHSTX_USE_FORMAT_PAT_8 || DISPHSTX_USE_FORMAT_PAT_8_PAL
// ----------------------------------------------------------------------------
//                       repeated pattern, 8-bit format
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Pat_8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 2,,		// align address to 4-byte boundary
.thumb_func
.global DispHstxVgaRender_Pat_8
DispHstxVgaRender_Pat_8:

	// push registers R4-R10
	push	{r4-r9,sl,lr}

	// line modulo pattern height -> R1
	ldrsh	r5,[r0,#DISPHSTXSLOT_FONTH] // R5 <- fonth (modulo lines)
	sdiv	r4,r1,r5		// R4 <- line / fonth
	mls	r1,r4,r5,r1		// R1 <- line - lin/fonth*fonth = line % fonth

	// prepare source address -> LR
	ldrh	r8,[r0,#DISPHSTXSLOT_PITCH]	// R8 <- load pitch of the line
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 <- load buffer base address
	mla	lr,r1,r8,r4			// LR <- line*pitch+base

	// prepare palette mapping address -> R1
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes in u32 format
	add	r1,r1,r3,lsl #2+8		// R1 <- palette + odd*256

	// prepare width of the slot -> R9
	ldrh	r9,[r0,#DISPHSTXSLOT_W]		// R9 <- width of slot in graphics pixels

	// prepare destination address -> R5
	ldr	r5,[r2,#0]			// R5 <- destination address of render buffer

// ---- start next pattern

DispHstxVgaRender_Pat_8_2:

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
//  SL = temporary register 3
//  IP = source buffer
//  LR = original source buffer

	.align 2,,			// align address to 4-byte boundary

// sample 0, destination index 0..3

2:	ldrd	r2,r6,[ip,#0]		// R2 <- [IP], R6 <- [IP+4], load next 2 samples
	adds	r5,#16*4		// R5 <- shift destination address

	uxtb	r7,r2			// R7 <- extract bits 0-7 from the sample
	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	ubfx	r7,r2,#8,#8		// R7 <- extract bits from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	ubfx	r7,r2,#16,#8		// R7 <- extract bits from the sample
	strd	r0,sl,[r5,#0*4-16*4]	// store 2 pixela

	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	lsrs	r7,r2,#24		// R7 <- extract bits 24-31 from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	uxtb	r7,r6			// R7 <- extract bits 0-7 from the sample
	strd	r0,sl,[r5,#2*4-16*4]	// store 2 pixels

// sample 1, destination index 4..7

	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	ubfx	r7,r6,#8,#8		// R7 <- extract bits from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	ubfx	r7,r6,#16,#8		// R7 <- extract bits from the sample
	strd	r0,sl,[r5,#4*4-16*4]	// store 2 pixels

	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	lsrs.w	r7,r6,#24		// R7 <- extract bits 24-31 from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	ldrd	r2,r6,[ip,#8]		// R2 <- [IP+8], R6 <- [IP+12], load next 2 samples
	strd	r0,sl,[r5,#6*4-16*4]	// store 2 pixels

// sample 2, destination index 8..11

	uxtb.w	r7,r2			// R7 <- extract bits 0-7 from the sample
	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	ubfx	r7,r2,#8,#8		// R7 <- extract bits from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	ubfx	r7,r2,#16,#8		// R7 <- extract bits from the sample
	strd	r0,sl,[r5,#8*4-16*4]	// store 2 pixels

	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	lsrs	r7,r2,#24		// R7 <- extract bits 24-31 from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	uxtb	r7,r6			// R7 <- extract bits 0-7 from the sample
	strd	r0,sl,[r5,#10*4-16*4]	// store 2 pixels

// sample 3, destination index 12..15

	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	ubfx	r7,r6,#8,#8		// R7 <- extract bits from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	ubfx	r7,r6,#16,#8		// R7 <- extract bits from the sample
	strd	r0,sl,[r5,#12*4-16*4]	// store 2 pixels

	ldr	r0,[r1,r7,lsl #2]	// R0 <- mapping[bit << 2]
	lsrs	r7,r6,#24		// R7 <- extract bits 24-31 from the sample
	ldr	sl,[r1,r7,lsl #2]	// SL <- mapping[bit << 2]
	adds	ip,#4*4			// IP <- shift source address
	strd	r0,sl,[r5,#14*4-16*4]	// store 2 pixels

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- mapping[bit << 2]
	str	r0,[r5],#4		// store pixel, increment pointer

	// loop counter
	subs	r4,#1			// decrement loop counter
	bne	6b

	// check if all is done
8:	cmp	r9,#0			// check remaining width
	bne	DispHstxVgaRender_Pat_8_2

	// pop registers R4-R10 and return
	pop	{r4-r9,sl,pc}
#endif

#if DISPHSTX_USE_FORMAT_RLE8 || DISPHSTX_USE_FORMAT_RLE8_PAL
// ----------------------------------------------------------------------------
//                           RLE8 compression
// ----------------------------------------------------------------------------
// INPUT: R0 = pointer to slot descriptor
//	  R1 = current graphics line
//	  R2 = pointer to command buffer entry with pointer to render buffer
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Rle8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

.align 4,,		// align address to 16-byte boundary (cache line)

.thumb_func
.global DispHstxVgaRender_Rle8
DispHstxVgaRender_Rle8:

	// push registers R4-R7
	push	{r4-r7}

	// prepare palette mapping address -> R6
	ldr	r6,[r0,#DISPHSTXSLOT_PALVGA]	// R6 <- pointer to palettes in u32 format
	add	r6,r6,r3,lsl #2+8		// R6 <- palette + odd*256

	// prepare destination address -> R3
	ldr	r3,[r2,#0]			// R3 <- destination address of render buffer

	// prepare width of the slot -> R5
	ldrh	r5,[r0,#DISPHSTXSLOT_W]		// R5 <- width of slot in graphics pixels

	// prepare source address -> R1
	ldr	r2,[r0,#DISPHSTXSLOT_FONT]	// R2 (font) <- pointer to font
	ldr	r4,[r0,#DISPHSTXSLOT_BUF]	// R4 (buf) <- source buffer base address
	ldr	r2,[r2,r1,lsl #2]		// R2 <- line offset font[line]
	adds	r1,r4,r2			// R1 (src) <- source address

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
	ldr	r0,[r6,r0,lsl #2]	// R0 <- map color

	// prepare big loop counter -> R4
	lsrs	r4,r2,#2		// R4 <- data length / 4
	beq	4f			// skip if counter is zero

	// store color by 4 pixels
3:	str	r0,[r3,#0]		// store pixel
	str	r0,[r3,#4]		// store pixel
	str	r0,[r3,#8]		// store pixel
	str	r0,[r3,#12]		// store pixel
	adds	r3,#16			// incrrement address
	subs	r4,#1			// decrement loop counter
	bne	3b			// loop

	// prepare small loop counter -> R2
4:	ands	r2,#3			// R2 <- loop counter
	beq	9f			// continue main loop

	// store color by 1 pixel (count = 1..3)
	str	r0,[r3,#0]
	cmp	r2,#1
	beq	1f
	str	r0,[r3,#4]
	cmp	r2,#2
	beq	1f
	str	r0,[r3,#8]

	// shift destination address
1:	cmp	r5,#0			// here for align
	add	r3,r3,r2,lsl #2

	// main loop (check remaining width)
	bgt	2b

	// pop registers R4-R7 and return
	pop	{r4-r7}
	bx	lr

.align 4,,		// align address to 16-byte boundary (cache line)
	nop

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

	// get data length -> R2
6:	adds	r2,r0,#1		// R2 <- length = token - 128 + 3
	cmp	r2,r5			// check remaining width
	it	ge			// true if >=
	movge	r2,r5			// limit length
	subs	r5,r2			// R5 <- decrease width

	// prepare big loop counter -> R4
	lsrs	r4,r2,#2		// R4 <- data length / 4
	beq	8f			// skip if counter is zero

	.align 2,,			// align address to 4-byte boundary

	// transfer by 4 pixels
7:	ldrb	r0,[r1],#4		// R0 <- get color, increment address
	ldrb	r7,[r1,#1-4]		// R7 <- get color 2
	ldr	r0,[r6,r0,lsl #2]	// R0 <- map color
	ldr	r7,[r6,r7,lsl #2]	// R7 <- map color 2
	str	r0,[r3],#16		// store pixel, increment address
	str	r7,[r3,#4-16]		// store pixel

	ldrb	r0,[r1,#2-4]		// R0 <- get color
	ldrb	r7,[r1,#3-4]		// R7 <- get color 2
	ldr	r0,[r6,r0,lsl #2]	// R0 <- map color
	ldr	r7,[r6,r7,lsl #2]	// R7 <- map color 2
	str	r0,[r3,#8-16]		// store pixel
	str	r7,[r3,#12-16]		// store pixel

	subs	r4,#1
	bne	7b

	// prepare small loop counter -> R2
8:	ands	r2,#3			// R2 <- loop counter
	beq	9f			// continue main loop

	// transfer by 1 pixel (count = 1..3)
	ldrb	r0,[r1,#0]
	cmp	r2,#1
	ldr	r0,[r6,r0,lsl #2]	// R0 <- map color
	ldrb	r7,[r1,#1]		// R7 <- get color 2
	str	r0,[r3]
	beq	10f

	ldr	r7,[r6,r7,lsl #2]	// R0 <- map color
	cmp	r2,#2
	str	r7,[r3,#4]
	beq	10f
	ldrb	r0,[r1,#2]
	ldr	r0,[r6,r0,lsl #2]	// R0 <- map color
	str	r0,[r3,#8]

	// increment address
10:	add	r1,r2			// shift source address
	add	r3,r3,r2,lsl #2		// shift destination address

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr1(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_Attr1
DispHstxVgaRender_Attr1:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+4		// R1 <- palette + odd*16

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#6*4]		// store pixels
	str	r6,[r5,#7*4]		// store pixels

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#24*4]		// store pixels
	str	r6,[r5,#25*4]		// store pixels

	attr1_1	26,28
	attr1_1	28,26
	attr1_1	30,24

	// shift destination address
	adds	r5,#32*4		// R5 <- shift destination address

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#6*4]		// store pixels
	str	r6,[r5,#7*4]		// store pixels

	// next loop
	adds	r5,#8*4			// R5 <- shift destination address
	lsrs	lr,lr,#8		// LR <- shift attribute word
	lsrs	r2,r2,#8		// R2 <- shift sample word
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr2(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxVgaRender_Attr2
DispHstxVgaRender_Attr2:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+4		// R1 <- palette + odd*16

	// prepare width of the slot -> R3 (saves flags)
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// jump to odd line
	bne	DispHstxVgaRender_Attr2_odd	// jump to odd line service

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
	adds	r5,#16*4		// R5 <- shift destination address

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
	adds	r5,#4*4			// R5 <- shift destination address
	lsrs	lr,lr,#8		// LR <- shift attribute word
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

	// loop counter
	bne	7b

	// restore stack
8:	add	sp,#8

	// pop registers R4-R7 and return
	pop	{r4-r7,pc}

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

DispHstxVgaRender_Attr2_odd:

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
	adds	r5,#16*4		// R5 <- shift destination address

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
	adds	r5,#4*4			// R5 <- shift destination address
	lsrs	lr,lr,#8		// LR <- shift attribute word
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr3(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_Attr3
DispHstxVgaRender_Attr3:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+4		// R1 <- palette + odd*16

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
	adds	r5,#16*4		// R5 <- shift destination address

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

	adds	r5,#8*4			// R5 <- shift destination address
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	subs	r3,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
// void DispHstxVgaRender_Attr4(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_Attr4
DispHstxVgaRender_Attr4:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+4		// R1 <- palette + odd*16

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
	adds	r5,#16*4		// R5 <- shift destination address

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
	adds	r5,#4*4			// R5 <- shift destination address
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr5(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[pix << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[pix << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.thumb_func
.global DispHstxVgaRender_Attr5
DispHstxVgaRender_Attr5:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+8		// R1 <- palette + odd*256

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#6*4]		// store pixels
	str	r6,[r5,#7*4]		// store pixels

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

	adds.w	r7,#2			// R7 <- shift attributes

// byte 3, destination index 24..31, source bit 31..24

	lsrs.w	r0,r2,#31		// R0 <- extract bit from the sample
	ubfx	r6,r2,#30,#1		// R6 <- extract bit from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldrb	r6,[r7,r6]		// R6 <- pix[bit]
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#24*4]		// store pixels
	str	r6,[r5,#25*4]		// store pixels

	attr5_1	26,28
	attr5_1	28,26
	attr5_1	30,24

	adds	r7,#2			// R7 <- shift attributes

	// shift destination address
	adds	r5,#32*4		// R5 <- shift destination address

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#6*4]		// store pixels
	str	r6,[r5,#7*4]		// store pixels

	adds	r7,#2			// R7 <- shift attributes
	adds	r5,#8*4			// R5 <- shift destination address
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr6(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[pix << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[pix << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.thumb_func
.global DispHstxVgaRender_Attr6
DispHstxVgaRender_Attr6:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+8		// R1 <- palette + odd*256

	// prepare width of the slot -> R3 (saves flags)
	ldrh	r3,[r0,#DISPHSTXSLOT_W]		// R3 <- width of slot in graphics pixels

	// jump to odd line
	bne	DispHstxVgaRender_Attr6_odd	// jump to odd line service

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
	adds	r5,#16*4		// R5 <- shift destination address

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
	adds	r5,#4*4			// R5 <- shift destination address
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

DispHstxVgaRender_Attr6_odd:

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
	adds	r5,#16*4		// R5 <- shift destination address

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
	adds	r5,#4*4			// R5 <- shift destination address
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#1		// shift sample left by 1 bit
	subs	r4,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr7(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[pix << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[pix << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxVgaRender_Attr7
DispHstxVgaRender_Attr7:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+8		// R1 <- palette + odd*256

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#2*4]		// store pixels
	str	r6,[r5,#3*4]		// store pixels

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#12*4]		// store pixels
	str	r6,[r5,#13*4]		// store pixels

	attr7_1	14,24
	adds	r7,#4			// R7 <- shift attributes

	// shift destination address
	adds	r5,#16*4		// R5 <- shift destination address

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
	adds	r5,#8*4			// R5 <- shift destination address
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	subs	r3,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

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
//	  R3 = 0=even frame, 1=odd frame
// ----------------------------------------------------------------------------
// void DispHstxVgaRender_Attr8(sDispHstxVSlot* slot, int line, u32* cmd, int odd);

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[pix << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[pix << 2]
	str	r0,[r5,#\dst*4]		// store pixel
	str	r6,[r5,#\dst*4+4]	// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.thumb_func
.global DispHstxVgaRender_Attr8
DispHstxVgaRender_Attr8:

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
	ldr	r1,[r0,#DISPHSTXSLOT_PALVGA]	// R1 <- pointer to palettes for VGA
	add	r1,r1,r3,lsl #2+8		// R1 <- palette + odd*256

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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#2*4]		// store pixels
	str	r6,[r5,#3*4]		// store pixels
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
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	ldr	r6,[r1,r6,lsl #2]	// R6 <- map[bit << 2]
	str	r0,[r5,#12*4]		// store pixels
	str	r6,[r5,#13*4]		// store pixels

	attr8_1	14,24
	adds	r7,#4			// R7 <- shift attributes

	// shift destination address
	adds	r5,#16*4		// R5 <- shift destination address

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

	// middle loop
6:	attr8_1	0,28
	attr8_1	2,24

	adds	r7,#4			// R7 <- shift attributes
	adds	r5,#4*4			// R5 <- shift destination address
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

	// process one pixel
7:	lsrs	r0,r2,#30		// R0 <- extract 2 bits from the sample
	ldrb	r0,[r7,r0]		// R0 <- pix[bit]
	ldr	r0,[r1,r0,lsl #2]	// R0 <- map[bit << 2]
	lsls	r2,r2,#2		// shift sample left by 2 bits
	subs	r3,#1			// decrement loop counter
	str	r0,[r5],#4		// store pixel, increment pointer

	// loop counter
	bne	7b

	// pop registers R4-R7 and return
8:	pop	{r4-r7}
	bx	lr
#endif

#endif // DISPHSTX_ARM_ASM && !RISCV && DISPHSTX_USE_VGA

#endif // USE_DISPHSTX		// 1=use HSTX Display driver
