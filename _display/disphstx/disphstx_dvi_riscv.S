// ****************************************************************************
//
//      HDMI and VGA display driver for Pico2 RP2350 over HSTX interface
//                       HSTX DVI driver - RISC-V assembler
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

// RISC-V ASM notes:
//  - scratch registers t0-t6, a0-a7
//  - after load from memory insert another instruction before using the data (no need before save data to memory)
//  - to prefer use compressed instructions: use registers s0-s1 a0-a5, regdst = regsrc1, offset multiply of 4,
//	lw, sw, srli, srai, andi, sub, xor, or, and, beqz, bnez

#include "config.h"			// project configuration
#include "_config.h"			// default configuration

#if USE_DISPHSTX		// 1=use HSTX Display driver

#if DISPHSTX_RISCV_ASM && RISCV && DISPHSTX_USE_DVI	// 1 = use RISC-V assembler for render functions

	.section .time_critical.hstx.dvi, "ax"
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

#define __hazard3_extension_xh3bextm 100 // comment this to not use bextmi instruction (Hazard3 extension)

// h3.bextmi: Extract up to 8 consecutive bits from register rs1, with the
// first bit index and the number of bits both configured by immediate
// values. I-format instruction. Pseudocode:
//
//     rd = (rs1 >> shamt) & ~(-1 << nbits)
//
// rd ... destination register
// rs1 ... source register
// shamt ... start bit position
// nbits ... number of bits to extract

.macro h3.bextmi rd rs1 shamt nbits
#ifdef __hazard3_extension_xh3bextm
	.insn	i 0x0b, 0x4, \rd, \rs1, (\shamt & 0x1f) | (((\nbits - 1) & 0x7 ) << 6)
#else
	srli	\rd,\rs1,\shamt			// RD <- RS1 >> shamt ... get bits from RS1, starting with bit shamt
	andi	\rd,\rd,((1 << \nbits) - 1)	// mask required number of bits
#endif
.endm

.macro nop.w
	and	t6,t6,t6
.endm

// ----------------------------------------------------------------------------
//                          do-nothing render
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_No(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_No
DispHstxDviRender_No:
	ret

// ----------------------------------------------------------------------------
//                default render - read from frame buffer
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Default(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Default
DispHstxDviRender_Default:
	lhu	a4,DISPHSTXSLOT_PITCH(a0)	// A4 <- load pitch of the line
	lw	a5,DISPHSTXSLOT_BUF(a0)		// A5 <- load buffer base address
	mul	a4,a4,a1			// A4 <- line*pitch = offset in buffer
	add	a5,a5,a4			// A5 <- address in frame buffer
	sw	a5,0(a2)			// save data address to the command buffer
	ret

#if DISPHSTX_USE_FORMAT_12
// ----------------------------------------------------------------------------
//                          12 bits per pixel
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_12(sDispHstxVSlot* slot, int line, u32* cmd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.global DispHstxDviRender_12
DispHstxDviRender_12:

	// prepare source address -> A4
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a4,t1,t0		// A4 <- address in frame buffer

	// prepare destination address -> A5
	lw	a5,0(a2)		// A5 <- load data address from the command buffer

	// prepare width of the slot -> T3
	lhu	t3,DISPHSTXSLOT_W(a0)	// T3 <- width of slot in graphics pixels

// ---- big loop (by 8 pixels)

	// prepare counter of big loop (by 8 pixels) -> T2
	srli	t2,t3,3			// T2 <- width/8
	beqz	t2,5f			// skip if width/8 is 0
	slli	t2,t2,4			// T2 <- counter * 16
	add	t2,t2,a5		// T2 <- end destination address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = sample 1 and 3
//  A3 = sample 2
//  A4 = source buffer
//  A5 = destination buffer
//  T0 = temporary register 3
//  T2 = loop counter, end source address
//  T3 = width of the slot

	.align 2,,			// align address to 4-byte boundary

//	[2222 2222 1111 1111 1111 0000 0000 0000] A2
//	[5555 4444 4444 4444 3333 3333 3333 2222] A3
//	[7777 7777 7777 6666 6666 6666 5555 5555] A2

	// pixel 0
2:	lw	a2,0(a4)		// A2 <- load sample 1
	sh	a2,0*2(a5)		// store pixel 0

	// pixel 1
	srli	a2,a2,12		// A2 <- get pixel 1
	lw	a3,1*4(a4)		// A3 <- load sample 2
	sh	a2,1*2(a5)		// store pixel 1

	// pixel 2
	srli	a2,a2,12		// A2 <- get pixel 2 bits 0-7
	slli	a0,a3,8			// A0 <- get pixel 2 bits 8-11
	or	a0,a0,a2		// A0 <- compose pixel 2
	sh	a0,2*2(a5)		// store pixel 2

	// pixel 3
	srli	a3,a3,4			// A3 <- get pixel 3
	sh	a3,3*2(a5)		// store pixel 3

	// pixel 4
	srli	a3,a3,12		// A3 <- get pixel 4
	lw	a2,2*4(a4)		// A2 <- load sample 3
	sh	a3,4*2(a5)		// store pixel 4

	// pixel 5
	srli	a3,a3,12		// A2 <- get pixel 5 bits 0-3
	slli	a0,a2,4			// A0 <- get pixel 5 bits 4-11
	or	a0,a0,a3		// A0 <- compose pixel 5
	sh	a0,5*2(a5)		// store pixel 5

	// pixel 6
	srli	a2,a2,8			// A2 <- get pixel 6
	sh	a2,6*2(a5)		// store pixel 6

	// pixel 7
	srli	a2,a2,12		// A2 <- get pixel 7
	sh	a2,7*2(a5)		// store pixel 7

	// shift pointers
	addi	a4,a4,3*4		// A4 <- shift source address
	addi	a5,a5,8*2		// A5 <- shift destination address

	// loop
	bne	a5,t2,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R4
5:	andi	t2,t3,7			// T2 <- width % 8
	beqz	t2,8f			// skip if width % 8 is 0

	// pixel 0
	lw	a2,0(a4)		// A2 <- load sample 1
	sh	a2,0*2(a5)		// store pixel 0

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 1
	srli	a2,a2,12		// A2 <- get pixel 1
	lw	a3,1*4(a4)		// A3 <- load sample 2
	sh	a2,1*2(a5)		// store pixel 1

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 2
	srli	a2,a2,12		// A2 <- get pixel 2 bits 0-7
	slli	a0,a3,8			// A0 <- get pixel 2 bits 8-11
	or	a0,a0,a2		// A0 <- compose pixel 2
	sh	a0,2*2(a5)		// store pixel 2

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 3
	srli	a3,a3,4			// A3 <- get pixel 3
	sh	a3,3*2(a5)		// store pixel 3

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 4
	srli	a3,a3,12		// A3 <- get pixel 4
	lw	a2,2*4(a4)		// A2 <- load sample 3
	sh	a3,4*2(a5)		// store pixel 4

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 5
	srli	a3,a3,12		// A2 <- get pixel 5 bits 0-3
	slli	a0,a2,4			// A0 <- get pixel 5 bits 4-11
	or	a0,a0,a3		// A0 <- compose pixel 5
	sh	a0,5*2(a5)		// store pixel 5

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 6
	srli	a2,a2,8			// A2 <- get pixel 6
	sh	a2,6*2(a5)		// store pixel 6

	addi	t2,t2,-1
	beqz	t2,8f

	// pixel 7
	srli	a2,a2,12		// A2 <- get pixel 7
	sh	a2,7*2(a5)		// store pixel 7

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_1_PAL
// ----------------------------------------------------------------------------
//                    1 bit per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_1_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 1-bit double-pixel
//   dst ... destination index 0..31
//   bit ... source 2-bit index 0..30
// A0 = temporary register
// A2 = destination buffer
// A6 = source sample with 32 pixels
// [SP] = palette mapping
.macro store1 dst bit
	h3.bextmi a0,a6,\bit,2		// A0 <- extract bits from the sample
	sh2add	a0,a0,sp		// A0 <- bit*2 + map2, offset in palette mapping
	lw	a0,(a0)			// A0 <- load 2 pixels
	sw	a0,\dst*4(a2)		// store 2 pixels
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.global DispHstxDviRender_1_PAL
DispHstxDviRender_1_PAL:

	// reserve space in stack for local variables
	add	sp,sp,-16		// reserve space in stack for local variable: u32 map2[4]

	// prepare source address -> A1
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a1,t1,t0		// A1 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0)	// A3 <- pointer to palettes in RGB565 format

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palettes map2[4] in the stack
	lhu	t0,0(a3)		// T0 <- color0
	lhu	t1,2(a3)		// T1 <- color1
	slli	t2,t0,16		// T2 <- color0 << 16
	slli	t3,t1,16		// T3 <- color1 << 16
	or	t4,t0,t2		// T4 <- color0 | (color0 << 16)
	sw	t4,0*4(sp)		// map2[0] <- color0 | (color0 << 16)
	or	t4,t0,t3		// T4 <- color0 | (color1 << 16)
	sw	t4,1*4(sp)		// map2[1] <- color0 | (color1 << 16)
	or	t4,t1,t2		// T4 <- color1 | (color0 << 16)
	sw	t4,2*4(sp)		// map2[2] <- color1 | (color0 << 16)
	or	t4,t1,t3		// T4 <- color1 | (color1 << 16)
	sw	t4,3*4(sp)		// map2[3] <- color1 | (color1 << 16)

// ---- big loop (by 32 pixels)

	// prepare counter of big loop (by 32 pixels) -> A5
	srli	a5,a4,5			// A5 <- width / 32
	beqz	a5,5f			// skip if width/32 is 0
	sh2add	t1,a5,a1		// T1 <- end of source address

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> A6
2:	lw	a6,(a1)			// A6 <- load word
	addi	a1,a1,4			// increment source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter
//  A6 = sample with 32 pixels
//  T1 = end of source address
//  [SP] = (map2) double-palettes

// byte 0, destination index 0..3, source bit 7..0

	store1	0,6
	store1	1,4
	store1	2,2
	store1	3,0

// byte 1, destination index 4..7, source bit 15..8

	store1	4,14
	store1	5,12
	store1	6,10
	store1	7,8

// byte 2, destination index 8..11, source bit 23..16

	store1	8,22
	store1	9,20
	store1	10,18
	store1	11,16

// byte 3, destination index 12..15, source bit 31..24

	store1	12,30
	store1	13,28
	store1	14,26
	store1	15,24

	// shift destination address
	addi	a2,a2,16*4		// A2 <- shift destination address

	// loop counter
	bne	a1,t1,2b		// next step

// ---- small loop (1 pixel)

	// prepare counter of small loop (by 1 pixel) -> A5
5:	andi	a5,a4,0x1f		// A5 <- width low 5 bits
	beqz	a5,8f			// skip if width & 0x1f is 0

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter
//  A6 = sample with 32 pixels

	// load next source data -> A6
	lw	a6,(a1)			// A6 <- load word
	rev8	a6,a6			// reverze byte order

	// process 1 pixel
6:	srli	a0,a6,31		// A0 <- extract bit 31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	sh	a0,(a2)			// store pixel

	// shift sample
	slli	a6,a6,1			// shift sample left by 1 bit

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// loop counter
	addi	a5,a5,-1		// drecrement loop counter
	bnez	a5,6b			// next step

	// restore stack
8:	add	sp,sp,16
	ret
#endif

#if DISPHSTX_USE_FORMAT_2_PAL
// ----------------------------------------------------------------------------
//                   2 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_2_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2-bit pixel
//   dst ... destination index 0..31
//   bit ... source bit index 0..30
// A0 = temporary register
// A2 = destination buffer
// A3 = palette mapping address
// A6 = source sample with 16 pixels
.macro store2 dst bit
	h3.bextmi a0,a6,\bit,2		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.global DispHstxDviRender_2_PAL
DispHstxDviRender_2_PAL:

	// prepare source address -> A1
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a1,t1,t0		// A1 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0)	// A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> A6
2:	lw	a6,0(a1)		// A6 <- load word
	addi	a1,a1,4			// A1 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter
//  A6 = sample with 16 pixels

// byte 0, destination index 0..3, source bits 7..0

	store2	0,6
	store2	1,4
	store2	2,2
	store2	3,0

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

	store2	12,30
	store2	13,28
	store2	14,26
	store2	15,24

	// shift destination address
	addi	a2,a2,32		// A2 <- shift destination address

	// loop counter
	addi	a5,a5,-1		// drecrement loop counter
	bnez	a5,2b			// next step

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> A5
5:	andi	a5,a4,0x0f		// A5 <- width low 4 bits
	beqz	a5,8f			// skip if counter is 0

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A5 = loop counter
//  A6 = sample with 16 pixels

	// load next source data -> A6
	lw	a6,0(a1)		// A6 <- load word
	rev8	a6,a6			// reverze byte order

	// process one pixel
6:	srli	a0,a6,30		// A0 <- extract bits 30+31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0(a2)		// store pixel

	// shift sample
	slli	a6,a6,2			// shift sample left by 2 bits

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// loop counter
	addi	a5,a5,-1		// drecrement loop counter
	bnez	a5,6b			// next step

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_3_PAL
// ----------------------------------------------------------------------------
//                        3 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_3_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 3-bit pixel
//   dst ... destination index 0..31
//   pix ... source pixel index 0..9
// A0 = temporary register
// A2 = destination buffer
// A3 = palette mapping address
// A6 = source sample with 8 pixels
.macro store3 dst pix
	h3.bextmi a0,a6,\pix*3,3	// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.global DispHstxDviRender_3_PAL
DispHstxDviRender_3_PAL:

	// prepare source address -> A1
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a1,t1,t0		// A1 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// split width to big (-> A5) and small (-> A4) loop counter
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels
	li	t0,20			// T0 <- 20
	div	a5,a4,t0		// A5 <- big loop counter = width/20
	mul	t1,a5,t0		// T1 <- width/20*20
	sub	a4,a4,t1		// A4 <- small loop counter = width - width/20*20 = width % 20

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 20 pixels)

	beqz	a5,5f			// skip if width/20 is 0
	sh3add	a5,a5,a1		// A5 <- end of source address

	.align 2,,			// align address to 4-byte boundary

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaining width of the slot
//  A5 = loop counter, end of source address
//  A6 = sample with 8 pixels

	// pixels 0..9
2:	lw	a6,0(a1)		// A6 <- load sample
	addi	a1,a1,8			// A1 <- increment source address

	store3	0,0
	store3	1,1
	store3	2,2
	store3	3,3
	store3	4,4
	store3	5,5
	store3	6,6
	store3	7,7
	store3	8,8
	store3	9,9

	// pixels 10..19
	lw	a6,-4(a1)		// A6 <- load sample
	addi	a2,a2,20*2		// A2 <- shift destination address

	store3	(10-20),0
	store3	(11-20),1
	store3	(12-20),2
	store3	(13-20),3
	store3	(14-20),4
	store3	(15-20),5
	store3	(16-20),6
	store3	(17-20),7
	store3	(18-20),8
	store3	(19-20),9

	// loop counter
	bne	a1,a5,2b		// next step

// ---- small loop (by 1 pixel)

5:	beqz	a4,8f			// skip if counter is 0

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = loop counter, end of source address
//  A6 = sample with 8 pixels

	lw	a6,0(a1)		// A6 <- load sample

	// middle loop (by 10 pixels)
	li	t0,10
	blt	a4,t0,6f		// skip if number of pixels < 10

	store3	0,0
	store3	1,1
	store3	2,2
	store3	3,3
	store3	4,4
	store3	5,5
	store3	6,6
	store3	7,7
	store3	8,8
	store3	9,9

	sub	a4,a4,t0		// remaining width A4 -= 10
	beqz	a4,8f			// no pixel remain

	lw	a6,4(a1)		// A6 <- load next sample
	addi	a2,a2,10*2		// A2 <- shift destination address

	// process one pixel
6:	sh1add	a4,a4,a2		// A4 <- end of destination

7:	andi	a0,a6,7			// A0 <- extract bits 0-2 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0(a2)		// store pixel

	// shift sample
	srli	a6,a6,3			// shift sample right by 3 bits

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// loop counter
	bne	a2,a4,7b		// next step

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_4_PAL
// ----------------------------------------------------------------------------
//                   4 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_4_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 4-bit pixel
//   dst ... destination index 0..31
//   bit ... source bit index 0..28
// A0 = temporary register
// A2 = destination buffer
// A3 = palette mapping address
// A6 = source sample with 8 pixels
.macro store4 dst bit
	h3.bextmi a0,a6,\bit,4		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.global DispHstxDviRender_4_PAL
DispHstxDviRender_4_PAL:

	// prepare source address -> A1
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a1,t1,t0		// A1 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0)	// A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 8 pixels)

	// prepare counter of big loop (by 8 pixels) -> A5
	srli	a5,a4,3			// A5 <- width / 8
	beqz	a5,5f			// skip if width/8 is 0
	sh2add	a5,a5,a1		// A5 <- end of source address

	.align 2,,			// align address to 4-byte boundary

	// load next source data -> A6
2:	lw	a6,0(a1)		// A6 <- load word
	addi	a1,a1,4			// A1 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of source address
//  A6 = sample with 8 pixels

	store4	0,4
	store4	1,0

	store4	2,12
	store4	3,8

	store4	4,20
	store4	5,16

	store4	6,28
	store4	7,24

	// shift destination address
	addi	a2,a2,8*2		// A2 <- shift destination address

	// loop counter
	bne	a1,a5,2b		// next step

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> A5
5:	andi	a5,a4,7			// A5 <- width low 3 bits
	beqz	a5,8f			// skip if counter is 0

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A5 = loop counter
//  A6 = sample with 8 pixels

	// load next source data -> A6
	lw	a6,0(a1)		// A6 <- load word
	rev8	a6,a6			// reverze byte order
	sh1add	a5,a5,a2		// A5 <- end of destination address

	// process one pixel
6:	srli	a0,a6,28		// A0 <- extract bits 28-31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0(a2)		// store pixel

	// shift sample
	slli	a6,a6,4			// shift sample left by 4 bits

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// loop counter
	bne	a2,a5,6b		// next step

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_6_PAL
// ----------------------------------------------------------------------------
//                      6 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_6_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 6-bit pixel
//   dst ... destination index 0..31
//   pix ... source pixel index 0..4
// A0 = temporary register
// A2 = destination buffer
// A3 = palette mapping address
// A6 = source sample with 5 pixels
.macro store6 dst pix
	h3.bextmi a0,a6,\pix*6,6	// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)

.global DispHstxDviRender_6_PAL
DispHstxDviRender_6_PAL:

	// prepare source address -> A1
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a1,t1,t0		// A1 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// split width to big (-> A5) and small (-> A4) loop counter
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels
	li	t0,20			// T0 <- 20
	div	a5,a4,t0		// A5 <- big loop counter = width/20
	mul	t1,a5,t0		// T1 <- width/20*20
	sub	a4,a4,t1		// A4 <- small loop counter = width - width/20*20 = width % 20

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 20 pixels)

	beqz	a5,5f			// skip if width/20 is 0
	slli	a5,a5,4			// A5 <- number of loops * 4*4
	add	a5,a5,a1		// A5 <- end of source address

	.align 2,,			// align address to 4-byte boundary

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaining width of the slot
//  A5 = loop counter, end of source address
//  A6 = sample with 5 pixels

// sample 0, destination index 0..3

	// pixels 0..4
2:	lw	a6,0(a1)		// A6 <- load word 0
	addi	a1,a1,4*4		// A1 <- increment source address

	store6	0,0
	store6	1,1
	store6	2,2
	store6	3,3

	h3.bextmi a0,a6,4*6,6		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lw	a6,1*4-4*4(a1)		// A6 <- load word 1
	sh	a0,4*2(a2)		// store pixel

	// pixels 5..9
	store6	5,0
	store6	6,1
	store6	7,2
	store6	8,3

	h3.bextmi a0,a6,4*6,6		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lw	a6,2*4-4*4(a1)		// A6 <- load word 2
	sh	a0,9*2(a2)		// store pixel

	// pixels 10..14
	store6	10,0
	store6	11,1
	store6	12,2
	store6	13,3

	h3.bextmi a0,a6,4*6,6		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lw	a6,3*4-4*4(a1)		// A6 <- load word 3
	sh	a0,14*2(a2)		// store pixel

	// pixels 15..19
	store6	15,0
	store6	16,1
	store6	17,2
	store6	18,3

	h3.bextmi a0,a6,4*6,6		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	addi	a2,a2,20*2		// A2 <- increment destination address
	sh	a0,19*2-20*2(a2)	// store pixel

	// loop counter
	bne	a1,a5,2b		// next step

// ---- small loop (by 1 pixel)

5:	beqz	a4,8f			// skip if counter is 0

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = loop counter
//  A6 = sample with 5 pixels

	li	t0,5
1:	lw	a6,0(a1)		// A6 <- load word 0

	// middle loop (by 5 pixels)
	blt	a4,t0,6f		// skip if number of pixels < 5

	store6	0,0
	store6	1,1
	store6	2,2
	store6	3,3
	store6	4,4

	addi	a1,a1,4			// A1 <- increment source address
	addi	a2,a2,5*2		// A2 <- shift destination address
	sub	a4,a4,t0		// A4 <- decrement width
	bnez	a4,1b			// go next loop
	ret

	// process one pixel
6:	sh1add	a4,a4,a2		// A4 <- end destination

7:	andi	a0,a6,0x3f		// A0 <- extract bits 0-5 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0(a2)		// store pixel

	// shift sample
	srli	a6,a6,6			// shift sample right by 6 bits

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// loop counter
	bne	a4,a2,7b		// next step

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_8_PAL
// ----------------------------------------------------------------------------
//                   8 bits per pixel, paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 8-bit pixel
//   dst ... destination index 0..31
//   bit ... source bit index 0..24
// A0 = temporary register
// A2 = destination buffer
// A3 = sample with 4 pixels
// A6 = palette mapping address
.macro store8 dst bit
	h3.bextmi a0,a3,\bit,8		// A0 <- extract bits from the sample
	sh1add	a0,a0,a6		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
.endm

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w
	nop.w

.global DispHstxDviRender_8_PAL
DispHstxDviRender_8_PAL:

	// prepare source address -> A1
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	a1,t1,t0		// A1 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A6
	lw	a6,DISPHSTXSLOT_PAL(a0)	// A6 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	slli	a5,a5,4			// A5 <- number of loops * 4*4
	add	a5,a5,a1		// A5 <- end of source address

	.align 2,,			// align address to 4-byte boundary

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = sample with 4 pixels
//  A4 = width of the slot
//  A5 = loop counter, end of source address
//  A6 = palette mapping

// sample 0, destination index 0..3

	// pixel 0..3
2:	lw	a3,0(a1)		// A3 <- load word 0
	addi	a1,a1,4*4		// A1 <- increment source address

	store8	0,0
	store8	1,8
	store8	2,16
	srli	a3,a3,24		// A3 <- extract bits 24-31 from the sample
	sh1add	a0,a3,a6		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	lw	a3,1*4-4*4(a1)		// A3 <- load word 1
	sh	a0,3*2(a2)		// store pixel

	// pixel 4..7
	store8	4,0
	store8	5,8
	store8	6,16
	srli	a3,a3,24		// A3 <- extract bits 24-31 from the sample
	sh1add	a0,a3,a6		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	lw	a3,2*4-4*4(a1)		// A3 <- load word 2
	sh	a0,7*2(a2)		// store pixel

	// pixel 8..11
	store8	8,0
	store8	9,8
	store8	10,16
	srli	a3,a3,24		// A3 <- extract bits 24-31 from the sample
	sh1add	a0,a3,a6		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	lw	a3,3*4-4*4(a1)		// A3 <- load word 3
	sh	a0,11*2(a2)		// store pixel

	// pixel 12..15
	store8	12,0
	store8	13,8
	store8	14,16
	srli	a3,a3,24		// A3 <- extract bits 24-31 from the sample
	sh1add	a0,a3,a6		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,15*2(a2)		// store pixel

	// loop counter
	addi	a2,a2,16*2		// A2 <- increment destination address
	bne	a5,a1,2b		// next step

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> A5
5:	andi	a5,a4,0x0f		// A5 <- width low 4 bits
	beqz	a5,8f			// skip if counter is 0

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = sample with 4 pixels
//  A5 = loop counter
//  A6 = palette mapping

	// process one pixel
	sh2add	a5,a5,a2		// A5 <- end destination
6:	lbu	a0,0(a1)		// A0 <- load byte
	addi	a1,a1,1			// A1 <- increment source address
	sh1add	a0,a0,a6		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0(a2)		// store pixel
	addi	a2,a2,2			// A2 <- shift destination address
	bne	a5,a2,6b		// next step

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_COL
// ----------------------------------------------------------------------------
//        simple color (color RGB565 per line, index modulo fonth)
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_COL(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_COL
DispHstxDviRender_COL:

	lh	a3,DISPHSTXSLOT_FONTH(a0) // A3 <- height of font, color modulo
	lw	a0,DISPHSTXSLOT_PAL(a0)	// A0 <- pointer to palettes in RGB565 format
	rem	a1,a1,a3		// A1 <- line % fonth
	lw	a3,(a2)			// A3 <- load data address from the command buffer
	sh1add	a1,a1,a0		// A1 <- address in palettes
	lhu	a0,(a1)			// A0 <- load color
	sh	a0,(a3)			// write pixel
	ret
#endif

#if DISPHSTX_USE_FORMAT_MTEXT
// ----------------------------------------------------------------------------
// monocolor text, font width 8 pixels, colors of every row as 2 palette colors RGB565 (BG,FG)
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_MTEXT(sDispHstxVSlot* slot, int line, u32* cmd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation
	nop.w

.global DispHstxDviRender_MTEXT
DispHstxDviRender_MTEXT:

	// reserve space in stack for local variables
	add	sp,sp,-16		// reserve space in stack for local variable: u32 map2[4]

	// load data
	lw	a2,(a2)			// A2 dst <- destination buffer
	lw	a3,DISPHSTXSLOT_FONT(a0) // A3 font <- pointer to font
	lw	a4,DISPHSTXSLOT_PAL(a0)	// A4 pal <- pointer to palettes in RGB565 format map[2]
	lw	a5,DISPHSTXSLOT_BUF(a0)	// A5 src <- source buffer base address
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 pitch <- pitch of the line
	lh	t4,DISPHSTXSLOT_FONTH(a0) // T4 fonth <- height of font

	// get row index -> T1
	div	t1,a1,t4		// T1 row <- line/fonth

	// prepare pointer to palettes -> A4
	sh2add	a4,t1,a4		// A4 map <- row*4+pal, pointer to entries in palette buffer

	// prepare source pointer -> A5
	mul	t0,t0,t1		// T0 <- pitch*row, source offset
	add	a5,a5,t0		// A5 src <- buf + pitch*row, source pointer

	// get subline index -> T2
	mul	t2,t1,t4		// T2 <- row*fonth
	sub	t2,a1,t2		// T2 subline <- line - row*fonth

	// prepare cursor position -> T5
	li	t5,0			// T5 <- invalid cursor
	lbu	t0,DISPHSTXSLOT_CURROW(a0) // T0 <- cursor row
	bne	t1,t0,1f		// skip if not cursor row
	lbu	t0,DISPHSTXSLOT_CURBEG(a0) // T0 <- cursor begin
	blt	t2,t0,1f		// skip if not cursor
	lbu	t0,DISPHSTXSLOT_CUREND(a0) // T0 <- cursor end
	bgt	t2,t0,1f		// skip if not cursor
	li	t0,TIMER0_BASE		// T0 <- Timer0 base
	lw	t0,TIMER0_RAWL(t0)	// T0 <- load current time LOW
	lbu	t3,DISPHSTXSLOT_CURSPEED(a0) // T3 <- current speed
	bext	t0,t0,t3		// T0 <- extract time bit
	beqz	t0,1f			// skip if not cursor
	lbu	t5,DISPHSTXSLOT_CURPOS(a0) // T5 <- cursor position
	addi	t5,t5,1			// T5 <- cursor position + 1

	// prepare pointer to font -> A3
1:	slli	t2,t2,8			// T2 <- subline*256 (offset of font line)
	add	a3,a3,t2		// A3 font <- pointer to line in the font

	// prepare palettes map2[4] in the stack
	lhu	t0,0(a4)		// T0 <- color0
	lhu	t1,2(a4)		// T1 <- color1
	pack	t4,t0,t0		// T4 <- color0 | (color0 << 16)
	sw	t4,0*4(sp)		// map2[0] <- color0 | (color0 << 16)
	pack	t4,t0,t1		// T4 <- color0 | (color1 << 16)
	sw	t4,1*4(sp)		// map2[1] <- color0 | (color1 << 16)
	pack	t4,t1,t0		// T4 <- color1 | (color0 << 16)
	sw	t4,2*4(sp)		// map2[2] <- color1 | (color0 << 16)
	pack	t4,t1,t1		// T4 <- color1 | (color1 << 16)
	sw	t4,3*4(sp)		// map2[3] <- color1 | (color1 << 16)

// ---- big loop (by 8 pixels of 1 character)

	srli	a1,a6,3			// A1 <- width/8, big loop counter
	beqz	a1,5f			// skip big loop (if loop counter = 0)
	add	t1,a5,a1		// T1 <- end of source address

// Here is:
//  A0 = temporary
//  A1 = (i) loop counter
//  A2 = (dst) pointer to destination buffer
//  A3 = (font) pointer to line in the font
//  A4 = (map) pointer to palettes in RGB565 format
//  A5 = (src) pointer to source buffer
//  A6 = (w) width of slot in graphics pixels
//  A7 = (k) font sample with 8 pixels
//  T1 = end of source address
//  T5 = (curpos) cursor position
//  [SP] = (map2) double-palettes

	.align 2,,			// align address to 4-byte boundary

	// load font sample -> A7
2:	lbu	a7,(a5)			// A7 <- load source byte (character)
	addi	a5,a5,1			// shift source address
	add	a7,a3,a7		// A7 <- address in font
	lbu	a7,(a7)			// A7 <- load font sample

	// cursor
	addi	t5,t5,-1		// T5 <- decrement cursor position
	beqz	t5,9f			// cursor

	// pixels 0+1, bits 6+7
3:	h3.bextmi a0,a7,6,2		// A0 <- extract bits 6+7
	sh2add	a0,a0,sp		// A0 <- bits*4 + sp, pointer to map2[]
	lw	a0,(a0)			// A0 <- load pixels
	sw	a0,0*4(a2)		// store pixels

	// pixels 2+3, bits 4+5
	h3.bextmi a0,a7,4,2		// A0 <- extract bits 4+5
	sh2add	a0,a0,sp		// A0 <- bits*4 + sp, pointer to map2[]
	lw	a0,(a0)			// A0 <- load pixels
	sw	a0,1*4(a2)		// store pixels

	// pixels 4+5, bits 2+3
	h3.bextmi a0,a7,2,2		// A0 <- extract bits 2+3
	sh2add	a0,a0,sp		// A0 <- bits*4 + sp, pointer to map2[]
	lw	a0,(a0)			// A0 <- load pixels
	sw	a0,2*4(a2)		// store pixels

	// pixels 6+7, bits 0+1
	andi	a0,a7,3			// A0 <- extract bits 0+1
	sh2add	a0,a0,sp		// A0 <- bits*4 + sp, pointer to map2[]
	lw	a0,(a0)			// A0 <- load pixels
	sw	a0,3*4(a2)		// store pixels

	// shift destination address
	addi	a2,a2,16		// A2 <- shift destination by 16 (= 8 pixels)

	// loop
	bne	a5,t1,2b

// ---- small loop (by 1 pixel)

5:	andi	a1,a6,7			// A1 <- width & 7, small loop counter
	beqz	a1,8f			// skip small loop (if loop counter = 0)

	// load font sample -> A7
	lbu	a7,(a5)			// A7 <- load source byte (character)
	add	a7,a3,a7		// A7 <- address in font
	lbu	a7,(a7)			// A7 <- load font sample

	// cursor
	addi	t5,t5,-1		// T5 <- decrement cursor position
	bnez	t5,6f			// skip if not cursor
	not	a7,a7			// invert font sample

// Here is:
//  A0 = temporary
//  A1 = (i) loop counter
//  A2 = (dst) pointer to destination buffer
//  A4 = (map) pointer to palettes in RGB565 format
//  A7 = (k) font sample with 8 pixels
//  T5 = (curpos) cursor position

6:	bexti	a0,a7,7			// A0 <- extract bit 7
	sh1add	a0,a0,a4		// A0 <- bit*2 + a4, pointer to map[]
	lhu	a0,(a0)			// A0 <- load pixel
	sh	a0,(a2)			// store pixel

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// shift sample
	slli	a7,a7,1			// A7 <- shift sample 1 bit left

	// loop
	addi	a1,a1,-1		// decrement loop counter
	bnez	a1,6b

	// restore stack
8:	add	sp,sp,16
	ret

	// cursor shape
9:	not	a7,a7			// invert font sample
	j	3b
#endif

#if DISPHSTX_USE_FORMAT_ATEXT
// ----------------------------------------------------------------------------
// attribute text, font width 8 pixels, 16 palette entries, character + 2x4 bit
//             attribute low=FG high=BG index of color
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_ATEXT(sDispHstxVSlot* slot, int line, u32* cmd);

.align 4,,		// align address to 16-byte boundary (cache line)
	nop.w		// alignment optimisation

.global DispHstxDviRender_ATEXT
DispHstxDviRender_ATEXT:

	// reserve space in stack for local variables
	add	sp,sp,-16		// reserve space in stack for local variable: u32 map2[4] or u16 map3[2]

	// load data
	lw	a2,(a2)			// A2 dst <- destination buffer
	lw	a3,DISPHSTXSLOT_FONT(a0) // A3 font <- pointer to font
	lw	a4,DISPHSTXSLOT_PAL(a0)	// A4 pal <- pointer to palettes in RGB565 format map[2]
	lw	a5,DISPHSTXSLOT_BUF(a0)	// A5 src <- source buffer base address
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 pitch <- pitch of the line
	lh	t4,DISPHSTXSLOT_FONTH(a0) // T4 fonth <- height of font

	// get row index -> T1
	div	t1,a1,t4		// T1 row <- line/fonth

	// prepare source pointer -> A5
	mul	t0,t0,t1		// T0 <- pitch*row, source offset
	add	a5,a5,t0		// A5 src <- buf + pitch*row, source pointer

	// get subline index -> T2
	mul	t2,t1,t4		// T2 <- row*fonth
	sub	t2,a1,t2		// T2 subline <- line - row*fonth

	// prepare cursor position -> T5
	li	t5,0			// T5 <- invalid cursor
	lbu	t0,DISPHSTXSLOT_CURROW(a0) // T0 <- cursor row
	bne	t1,t0,1f		// skip if not cursor row
	lbu	t0,DISPHSTXSLOT_CURBEG(a0) // T0 <- cursor begin
	blt	t2,t0,1f		// skip if not cursor
	lbu	t0,DISPHSTXSLOT_CUREND(a0) // T0 <- cursor end
	bgt	t2,t0,1f		// skip if not cursor
	li	t0,TIMER0_BASE		// T0 <- Timer0 base
	lw	t0,TIMER0_RAWL(t0)	// T0 <- load current time LOW
	lbu	t3,DISPHSTXSLOT_CURSPEED(a0) // T3 <- current speed
	bext	t0,t0,t3		// T0 <- extract time bit
	beqz	t0,1f			// skip if not cursor
	lbu	t5,DISPHSTXSLOT_CURPOS(a0) // T5 <- cursor position
	addi	t5,t5,1			// T5 <- cursor position + 1

	// prepare pointer to font -> A3
1:	slli	t2,t2,8			// T2 <- subline*256 (offset of font line)
	add	a3,a3,t2		// A3 font <- pointer to line in the font

// ---- big loop (by 8 pixels of 1 character)

	srli	a1,a6,4			// A1 <- width/16, big loop counter
	beqz	a1,5f			// skip big loop (if loop counter = 0)
	sh2add	t4,a1,a5		// T4 <- end of source address

// Here is:
//  A0 = (k) font sample, temporary
//  A1 = (i) loop counter
//  A2 = (dst) pointer to destination buffer
//  A3 = (font) pointer to line in the font
//  A4 = (map) pointer to palettes in RGB565 format
//  A5 = (src) pointer to source buffer
//  A6 = (w) width of slot in graphics pixels
//  A7 = (kk) font sample with 2 characters, 16 pixels
//  T0,T1,T2,T3 = temporary
//  T4 = end of source address
//  T5 = (curpos) cursor position
//  [SP] = (map2) double-palettes

	.align 2,,			// align address to 4-byte boundary

	// load 2 characters with attributes -> R7
2:	lw	a7,(a5)			// load source data
	addi	a5,a5,4			// shift source address

// 1st character

	// prepare palettes map2[4] in the stack
	h3.bextmi t0,a7,12,4		// T0 <- extract bits of background attribute
	sh1add	t0,t0,a4		// T0 <- address in palettes
	lhu	t0,0(t0)		// T0 <- color0
	h3.bextmi t1,a7,8,4		// T1 <- extract bits of foreground attribute
	sh1add	t1,t1,a4		// T1 <- address in palettes
	lhu	t1,0(t1)		// T1 <- color1
	pack	a0,t0,t0		// A0 <- color0 | (color0 << 16)
	sw	a0,0*4(sp)		// map2[0] <- color0 | (color0 << 16)
	pack	a0,t0,t1		// A0 <- color0 | (color1 << 16)
	sw	a0,1*4(sp)		// map2[1] <- color0 | (color1 << 16)
	pack	a0,t1,t0		// A0 <- color1 | (color0 << 16)
	sw	a0,2*4(sp)		// map2[2] <- color1 | (color0 << 16)
	pack	a0,t1,t1		// A0 <- color1 | (color1 << 16)
	sw	a0,3*4(sp)		// map2[3] <- color1 | (color1 << 16)

	// load font sample -> A0
	zext.b	a0,a7			// A0 <- 1st character
	add	a0,a3,a0		// A0 <- address in font
	lbu	a0,(a0)			// A0 <- load font sample

	// cursor
	addi	t5,t5,-1		// T5 <- decrement cursor position
	beqz	t5,9f			// cursor

	// pixels 0+1, bits 6+7
3:	h3.bextmi t0,a0,6,2		// T0 <- extract bits 6+7
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,0*4(a2)		// store pixels

	// pixels 2+3, bits 4+5
	h3.bextmi t0,a0,4,2		// T0 <- extract bits 4+5
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,1*4(a2)		// store pixels

	// pixels 4+5, bits 2+3
	h3.bextmi t0,a0,2,2		// T0 <- extract bits 2+3
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,2*4(a2)		// store pixels

	// pixels 6+7, bits 0+1
	andi	t0,a0,3			// T0 <- extract bits 0+1
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,3*4(a2)		// store pixels

// 2nd character

	// prepare palettes map2[4] in the stack
	srli	t0,a7,28		// T0 <- extract bits of background attribute
	sh1add	t0,t0,a4		// T0 <- address in palettes
	lhu	t0,0(t0)		// T0 <- color0
	h3.bextmi t1,a7,24,4		// T1 <- extract bits of foreground attribute
	sh1add	t1,t1,a4		// T1 <- address in palettes
	lhu	t1,0(t1)		// T1 <- color1
	pack	a0,t0,t0		// A0 <- color0 | (color0 << 16)
	sw	a0,0*4(sp)		// map2[0] <- color0 | (color0 << 16)
	pack	a0,t0,t1		// A0 <- color0 | (color1 << 16)
	sw	a0,1*4(sp)		// map2[1] <- color0 | (color1 << 16)
	pack	a0,t1,t0		// A0 <- color1 | (color0 << 16)
	sw	a0,2*4(sp)		// map2[2] <- color1 | (color0 << 16)
	pack	a0,t1,t1		// A0 <- color1 | (color1 << 16)
	sw	a0,3*4(sp)		// map2[3] <- color1 | (color1 << 16)

	// load font sample -> A0
	h3.bextmi a0,a7,16,8		// A0 <- 2nd character
	add	a0,a3,a0		// A0 <- address in font
	lbu	a0,(a0)			// A0 <- load font sample

	// cursor
	addi	t5,t5,-1		// T5 <- decrement cursor position
	beqz	t5,10f			// cursor

	// pixels 0+1, bits 6+7
4:	h3.bextmi t0,a0,6,2		// T0 <- extract bits 6+7
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,4*4(a2)		// store pixels

	// pixels 2+3, bits 4+5
	h3.bextmi t0,a0,4,2		// T0 <- extract bits 4+5
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,5*4(a2)		// store pixels

	// pixels 4+5, bits 2+3
	h3.bextmi t0,a0,2,2		// T0 <- extract bits 2+3
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,6*4(a2)		// store pixels

	// pixels 6+7, bits 0+1
	andi	t0,a0,3			// T0 <- extract bits 0+1
	sh2add	t0,t0,sp		// T0 <- bits*4 + sp, pointer to map2[]
	lw	t0,(t0)			// T0 <- load pixels
	sw	t0,7*4(a2)		// store pixels

	// shift destination address
	addi	a2,a2,8*4		// A2 <- shift destination by 32 (= 16 pixels)

	// loop
	bne	a5,t4,2b

// ---- small loop (by 1 pixel)

5:	andi	a1,a6,0x0f		// A1 <- width & 0x0f, small loop counter
	beqz	a1,8f			// skip small loop (if loop counter = 0)

	// load 2 characters with attributes -> A7
	lw	a7,(a5)			// load source data

// Here is:
//  A0 = (k) font sample, temporary
//  A1 = (i) loop counter
//  A2 = (dst) pointer to destination buffer
//  A3 = (font) pointer to line in the font
//  A4 = (map) pointer to palettes in RGB565 format
//  A5 = (src) pointer to source buffer
//  A6 = (w) width of slot in graphics pixels
//  A7 = (kk) font sample with 2 characters, 16 pixels
//  T0,T1,T2,T3 = temporary
//  T4 = end of source address
//  T5 = (curpos) cursor position
//  [SP] = (map2) double-palettes

	// prepare palettes map3[2] in the stack
	h3.bextmi t0,a7,12,4		// T0 <- extract bits of background attribute
	sh1add	t0,t0,a4		// T0 <- address in palettes
	lhu	t0,0(t0)		// T0 <- color0
	sh	t0,0*2(sp)		// map3[0] <- color0
	h3.bextmi t0,a7,8,4		// T0 <- extract bits of foreground attribute
	sh1add	t0,t0,a4		// T0 <- address in palettes
	lhu	t0,0(t0)		// T0 <- color1
	sh	t0,1*2(sp)		// map3[1] <- color1

	// load font sample -> A0
	zext.b	a0,a7			// A0 <- 1st character
	add	a0,a3,a0		// A0 <- address in font
	lbu	a0,(a0)			// A0 <- load font sample

	// cursor
	addi	t5,t5,-1		// T5 <- decrement cursor position
	bnez	t5,11f			// skip if not cursor
	not	a0,a0			// invert font sample

	// counter >= 8 ?
11:	li	t0,8
	blt	a1,t0,6f		// count < 8

	// pixel 0, bit 7
	bexti	t0,a0,7			// T0 <- extract bit 7
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,0*2(a2)		// store pixel

	// pixel 1, bit 6
	bexti	t0,a0,6			// T0 <- extract bit 6
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,1*2(a2)		// store pixel

	// pixel 2, bit 5
	bexti	t0,a0,5			// T0 <- extract bit 5
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,2*2(a2)		// store pixel

	// pixel 3, bit 4
	bexti	t0,a0,4			// T0 <- extract bit 4
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,3*2(a2)		// store pixel

	// pixel 4, bit 3
	bexti	t0,a0,3			// T0 <- extract bit 3
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,4*2(a2)		// store pixel

	// pixel 5, bit 2
	bexti	t0,a0,2			// T0 <- extract bit 2
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,5*2(a2)		// store pixel

	// pixel 6, bit 1
	bexti	t0,a0,1			// T0 <- extract bit 1
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,6*2(a2)		// store pixel

	// pixel 7, bit 0
	andi	t0,a0,1			// T0 <- extract bit 0
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,7*2(a2)		// store pixel

	// shift counter
	addi	a1,a1,-8		// subtract 8 pixels
	beqz	a1,8f			// end

	// prepare palettes map3[2] in the stack
	h3.bextmi t0,a7,28,4		// T0 <- extract bits of background attribute
	sh1add	t0,t0,a4		// T0 <- address in palettes
	lhu	t0,0(t0)		// T0 <- color0
	sh	t0,0*2(sp)		// map3[0] <- color0
	h3.bextmi t0,a7,24,4		// T0 <- extract bits of foreground attribute
	sh1add	t0,t0,a4		// T0 <- address in palettes
	lhu	t0,0(t0)		// T0 <- color1
	sh	t0,1*2(sp)		// map3[1] <- color1

	// load font sample -> A0
	h3.bextmi a0,a7,16,8		// T0 <- extract bits of background attribute
	add	a0,a3,a0		// A0 <- address in font
	lbu	a0,(a0)			// A0 <- load font sample

	// shift destination address
	addi	a2,a2,8*2		// A2 <- shift destination address

	// cursor
	addi	t5,t5,-1		// T5 <- decrement cursor position
	bnez	t5,6f			// skip if not cursor
	not	a0,a0			// invert font sample

	// small loop
6:	bexti	t0,a0,7			// T0 <- extract bit 7
	sh1add	t0,t0,sp		// T0 <- bit*2 + sp, pointer to map3[]
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,(a2)			// store pixel

	// shift destination address
	addi	a2,a2,2

	// shift sample
	slli	a0,a0,1			// A0 <- shift sample 1 bit left

	// loop counter
	addi	a1,a1,-1
	bnez	a1,6b

	// restore stack
8:	add	sp,sp,16
	ret

	// cursor shape
9:	not	a0,a0			// invert font sample
	j	3b

	// cursor shape
10:	not	a0,a0			// invert font sample
	j	4b
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8
// ----------------------------------------------------------------------------
//                 tiles 4x4 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE4_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE4_8
DispHstxDviRender_TILE4_8:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	srli	a1,a6,RENDER_TILEW_BIT+2 // A1 big loop counter <- w >> (TILEW_B+2)
	beqz	a1,4f			// skip if loop counter is 0
	sh2add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with 4 tile indices
//  T0 = temporary

	// load sample with 4 tile indices -> A7
2:	lw	a7,0(a3)		// A7 <- load source data
	addi	a3,a3,4			// A3 <- increment source address

	// 1st tile
	zext.b	t0,a7			// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels

	// 2nd tile
	h3.bextmi t0,a7,8,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels

	// 3rd tile
	h3.bextmi t0,a7,16,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,2*4(a2)		// store pixels

	// 4th tile
	srli	t0,a7,24		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,3*4(a2)		// store pixels

	// shift destination address
	addi	a2,a2,4*4		// A2 <- shift destination address

	// loop
	bne	a3,a1,2b

// ---- check remaining pixels

4:	andi	a6,a6,4*RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	srli	a1,a6,RENDER_TILEW_BIT	// A1 <- number of tiles
	beqz	a1,6f			// no whole tiles left

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) remaining width of slot
//  A7 = (k) sample with 1 tile index
//  T0 = temporary

5:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// A3 <- increment source address
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	addi	a2,a2,1*4		// A2 <- shift destination address
	addi	a1,a1,-1		// loop counter
	bnez	a1,5b

// ---- small loop (by 1 pixel)

6:	andi	a6,a6,RENDER_TILEW-1	// A6 <- remaining pixels
	beqz	a6,8f			// skip small loop (if loop counter = 0)

	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
7:	lbu	t0,(a0)			// load pixel
	sb	t0,(a2)			// store pixel
	addi	a0,a0,1			// A0 <- shift source address
	addi	a2,a2,1			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8
// ----------------------------------------------------------------------------
//                 tiles 8x8 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE8_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE8_8
DispHstxDviRender_TILE8_8:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	srli	a1,a6,RENDER_TILEW_BIT+2 // A1 big loop counter <- w >> (TILEW_B+2)
	beqz	a1,4f			// skip if loop counter is 0
	sh2add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with 4 tile indices
//  T0 = temporary

	// load sample with 4 tile indices -> A7
2:	lw	a7,0(a3)		// A7 <- load source data
	addi	a3,a3,4			// A3 <- increment source address

	// 1st tile
	zext.b	t0,a7			// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels

	// 2nd tile
	h3.bextmi t0,a7,8,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,2*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,3*4(a2)		// store pixels

	// 3rd tile
	h3.bextmi t0,a7,16,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,4*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,5*4(a2)		// store pixels

	// 4th tile
	srli	t0,a7,24		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,6*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,7*4(a2)		// store pixels

	// shift destination address
	addi	a2,a2,8*4		// A2 <- shift destination address

	// loop
	bne	a3,a1,2b

// ---- check remaining pixels

4:	andi	a6,a6,4*RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	srli	a1,a6,RENDER_TILEW_BIT	// A1 <- number of tiles
	beqz	a1,6f			// no whole tiles left

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) remaining width of slot
//  A7 = (k) sample with 1 tile index
//  T0 = temporary

5:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// A3 <- increment source address
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels
	addi	a2,a2,2*4		// A2 <- shift destination address
	addi	a1,a1,-1		// loop counter
	bnez	a1,5b

// ---- small loop (by 1 pixel)

6:	andi	a6,a6,RENDER_TILEW-1	// A6 <- remaining pixels
	beqz	a6,8f			// skip small loop (if loop counter = 0)

	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
7:	lbu	t0,(a0)			// load pixel
	sb	t0,(a2)			// store pixel
	addi	a0,a0,1			// A0 <- shift source address
	addi	a2,a2,1			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8
// ----------------------------------------------------------------------------
//                 tiles 16x16 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE16_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE16_8
DispHstxDviRender_TILE16_8:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	srli	a1,a6,RENDER_TILEW_BIT+2 // A1 big loop counter <- w >> (TILEW_B+2)
	beqz	a1,4f			// skip if loop counter is 0
	sh2add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with 4 tile indices
//  T0 = temporary

	// load sample with 4 tile indices -> A7
2:	lw	a7,0(a3)		// A7 <- load source data
	addi	a3,a3,4			// A3 <- increment source address

	// 1st tile
	zext.b	t0,a7			// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,2*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,3*4(a2)		// store pixels

	// 2nd tile
	h3.bextmi t0,a7,8,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,4*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,5*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,6*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,7*4(a2)		// store pixels

	// 3rd tile
	h3.bextmi t0,a7,16,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,8*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,9*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,10*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,11*4(a2)		// store pixels

	// 4th tile
	srli	t0,a7,24		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,12*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,13*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,14*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,15*4(a2)		// store pixels

	// shift destination address
	addi	a2,a2,16*4		// A2 <- shift destination address

	// loop
	bne	a3,a1,2b

// ---- check remaining pixels

4:	andi	a6,a6,4*RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	srli	a1,a6,RENDER_TILEW_BIT	// A1 <- number of tiles
	beqz	a1,6f			// no whole tiles left

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) remaining width of slot
//  A7 = (k) sample with 1 tile index
//  T0 = temporary

5:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// A3 <- increment source address
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,2*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,3*4(a2)		// store pixels
	addi	a2,a2,4*4		// A2 <- shift destination address
	addi	a1,a1,-1		// loop counter
	bnez	a1,5b

// ---- small loop (by 1 pixel)

6:	andi	a6,a6,RENDER_TILEW-1	// A6 <- remaining pixels
	beqz	a6,8f			// skip small loop (if loop counter = 0)

	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
7:	lbu	t0,(a0)			// load pixel
	sb	t0,(a2)			// store pixel
	addi	a0,a0,1			// A0 <- shift source address
	addi	a2,a2,1			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8
// ----------------------------------------------------------------------------
//                 tiles 32x32 pixels, 8-bit format RGB332
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE32_8(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE32_8
DispHstxDviRender_TILE32_8:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

// ---- big loop (by 4 tiles)

	srli	a1,a6,RENDER_TILEW_BIT+2 // A1 big loop counter <- w >> (TILEW_B+2)
	beqz	a1,4f			// skip if loop counter is 0
	sh2add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with 4 tile indices
//  T0 = temporary

	// load sample with 4 tile indices -> A7
2:	lw	a7,0(a3)		// A7 <- load source data
	addi	a3,a3,4			// A3 <- increment source address

	// 1st tile
	zext.b	t0,a7			// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,2*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,3*4(a2)		// store pixels
	lw	t0,4*4(a0)		// load pixels
	sw	t0,4*4(a2)		// store pixels
	lw	t0,5*4(a0)		// load pixels
	sw	t0,5*4(a2)		// store pixels
	lw	t0,6*4(a0)		// load pixels
	sw	t0,6*4(a2)		// store pixels
	lw	t0,7*4(a0)		// load pixels
	sw	t0,7*4(a2)		// store pixels

	// 2nd tile
	h3.bextmi t0,a7,8,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,8*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,9*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,10*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,11*4(a2)		// store pixels
	lw	t0,4*4(a0)		// load pixels
	sw	t0,12*4(a2)		// store pixels
	lw	t0,5*4(a0)		// load pixels
	sw	t0,13*4(a2)		// store pixels
	lw	t0,6*4(a0)		// load pixels
	sw	t0,14*4(a2)		// store pixels
	lw	t0,7*4(a0)		// load pixels
	sw	t0,15*4(a2)		// store pixels

	// 3rd tile
	h3.bextmi t0,a7,16,8		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,16*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,17*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,18*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,19*4(a2)		// store pixels
	lw	t0,4*4(a0)		// load pixels
	sw	t0,20*4(a2)		// store pixels
	lw	t0,5*4(a0)		// load pixels
	sw	t0,21*4(a2)		// store pixels
	lw	t0,6*4(a0)		// load pixels
	sw	t0,22*4(a2)		// store pixels
	lw	t0,7*4(a0)		// load pixels
	sw	t0,23*4(a2)		// store pixels

	// 4th tile
	srli	t0,a7,24		// T0 <- tile index
	mul	t0,t0,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,24*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,25*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,26*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,27*4(a2)		// store pixels
	lw	t0,4*4(a0)		// load pixels
	sw	t0,28*4(a2)		// store pixels
	lw	t0,5*4(a0)		// load pixels
	sw	t0,29*4(a2)		// store pixels
	lw	t0,6*4(a0)		// load pixels
	sw	t0,30*4(a2)		// store pixels
	lw	t0,7*4(a0)		// load pixels
	sw	t0,31*4(a2)		// store pixels

	// shift destination address
	addi	a2,a2,32*4		// A2 <- shift destination address

	// loop
	bne	a3,a1,2b

// ---- check remaining pixels

4:	andi	a6,a6,4*RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

// ---- middle loop (by 1 tile)

	srli	a1,a6,RENDER_TILEW_BIT	// A1 <- number of tiles
	beqz	a1,6f			// no whole tiles left

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) remaining width of slot
//  A7 = (k) sample with 1 tile index
//  T0 = temporary

5:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// A3 <- increment source address
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
	lw	t0,0*4(a0)		// load pixels
	sw	t0,0*4(a2)		// store pixels
	lw	t0,1*4(a0)		// load pixels
	sw	t0,1*4(a2)		// store pixels
	lw	t0,2*4(a0)		// load pixels
	sw	t0,2*4(a2)		// store pixels
	lw	t0,3*4(a0)		// load pixels
	sw	t0,3*4(a2)		// store pixels
	lw	t0,4*4(a0)		// load pixels
	sw	t0,4*4(a2)		// store pixels
	lw	t0,5*4(a0)		// load pixels
	sw	t0,5*4(a2)		// store pixels
	lw	t0,6*4(a0)		// load pixels
	sw	t0,6*4(a2)		// store pixels
	lw	t0,7*4(a0)		// load pixels
	sw	t0,7*4(a2)		// store pixels
	addi	a2,a2,8*4		// A2 <- shift destination address
	addi	a1,a1,-1		// loop counter
	bnez	a1,5b

// ---- small loop (by 1 pixel)

6:	andi	a6,a6,RENDER_TILEW-1	// A6 <- remaining pixels
	beqz	a6,8f			// skip small loop (if loop counter = 0)

	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer
7:	lbu	t0,(a0)			// load pixel
	sb	t0,(a2)			// store pixel
	addi	a0,a0,1			// A0 <- shift source address
	addi	a2,a2,1			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE4_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 4x4 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE4_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	2
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE4_8_PAL
DispHstxDviRender_TILE4_8_PAL:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

	// prepare palette mapping -> T4 (map)
	lw	t4,DISPHSTXSLOT_PAL(a0)	// T4 <- pointer to palettes in RGB565 format

// ---- big loop (by 2 tiles)

	srli	a1,a6,RENDER_TILEW_BIT+2 // A1 big loop counter <- w >> TILEW_B+2
	beqz	a1,4f			// skip if loop counter is 0
	sh2add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with tile index, (kk) sample with 4 pixels
//  T0 = temporary 1
//  T1 = temporary 2
//  T4 = (map) palette mapping

// 1st tile

	// load sample with tile index -> A7
2:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,4			// shift source address

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,0*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,1*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,2*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	lbu	a7,-3(a3)		// A7 <- load tile index
	sh	t0,3*2(a2)		// store pixel

// 2nd tile

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,4*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,5*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,6*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	lbu	a7,-2(a3)		// A7 <- load tile index
	sh	t0,7*2(a2)		// store pixel

// 3rd tile

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,8*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,9*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,10*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	lbu	a7,-1(a3)		// A7 <- load tile index
	sh	t0,11*2(a2)		// store pixel

// 4th tile

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,12*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,13*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,14*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	addi	a2,a2,16*2		// A2 <- shift destination address
	sh	t0,15*2-16*2(a2)	// store pixel

	// loop
	bne	a3,a1,2b

// ---- check remaining pixels

4:	andi	a6,a6,4*RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

// ---- middle loop (1 tile)

5:	li	a0,RENDER_TILEW
	blt	a6,a0,6f		// no whole tile left

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) remaining width of slot
//  A7 = (k) sample with tile index, (kk) sample with 4 pixels
//  T0 = temporary 1
//  T1 = temporary 2
//  T4 = (map) palette mapping

	// load sample with tile index -> A7
	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// shift source address

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,0*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,1*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,2*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	addi	a2,a2,4*2		// A2 <- shift destination address
	sh	t0,3*2-4*2(a2)		// store pixel

	// counter
	addi	a6,a6,-RENDER_TILEW	// decrease width
	beqz	a6,8f			// no pixels left
	j	5b

// ---- small loop (by 1 pixel)

6:	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

7:	lbu	t0,(a0)			// load pixel
	addi	a0,a0,1			// A0 <- shift source address
	sh1add	t0,t0,t4		// T0 <- address in palette map
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,(a2)			// store pixel
	addi	a2,a2,2			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE8_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 8x8 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE8_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	3
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE8_8_PAL
DispHstxDviRender_TILE8_8_PAL:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

	// prepare palette mapping -> T4 (map)
	lw	t4,DISPHSTXSLOT_PAL(a0)	// T4 <- pointer to palettes in RGB565 format

// ---- big loop (by 2 tiles)

	srli	a1,a6,RENDER_TILEW_BIT+1 // A1 big loop counter <- w >> TILEW_B+1
	beqz	a1,4f			// skip if loop counter is 0
	sh1add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with tile index, (kk) sample with 4 pixels
//  T0 = temporary 1
//  T1 = temporary 2
//  T3 = next sample with 4 pixels
//  T4 = (map) palette mapping

// 1st tile

	// load sample with tile index -> A7
2:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,2			// shift source address

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels
	lw	t3,4(a0)		// T3 <- next pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,0*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,1*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,2*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,t3			// T1 <- pixel 4
	sh	t0,3*2(a2)		// store pixel

	// pixel 4
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,8,8		// T1 <- pixel 5
	sh	t0,4*2(a2)		// store pixel

	// pixel 5
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,16,8		// T1 <- pixel 6
	sh	t0,5*2(a2)		// store pixel

	// pixel 6
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,t3,24		// T1 <- pixel 7
	sh	t0,6*2(a2)		// store pixel

	// pixel 7
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel

// 2nd tile

	// load sample with tile index -> A7
	lbu	a7,-1(a3)		// A7 <- load tile index
	sh	t0,7*2(a2)		// store pixel

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels
	lw	t3,4(a0)		// T3 <- next pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,8*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,9*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,10*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,t3			// T1 <- pixel 4
	sh	t0,11*2(a2)		// store pixel

	// pixel 4
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,8,8		// T1 <- pixel 5
	sh	t0,12*2(a2)		// store pixel

	// pixel 5
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,16,8		// T1 <- pixel 6
	sh	t0,13*2(a2)		// store pixel

	// pixel 6
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,t3,24		// T1 <- pixel 7
	sh	t0,14*2(a2)		// store pixel

	// pixel 7
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	addi	a2,a2,16*2		// A2 <- shift destination address
	sh	t0,15*2-16*2(a2)	// store pixel

	// loop
	bne	a3,a1,2b

// ---- check remaining pixels

4:	andi	a6,a6,2*RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

// ---- middle loop (1 tile)

	li	a0,RENDER_TILEW
	blt	a6,a0,6f		// no whole tile left

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) remaining width of slot
//  A7 = (k) sample with tile index, (kk) sample with 4 pixels
//  T0 = temporary 1
//  T1 = temporary 2
//  T3 = next sample with 4 pixels
//  T4 = (map) palette mapping

	// load sample with tile index -> A7
	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// shift source address

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels
	lw	t3,4(a0)		// T3 <- next pixels

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,0*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,1*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,2*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,t3			// T1 <- pixel 4
	sh	t0,3*2(a2)		// store pixel

	// pixel 4
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,8,8		// T1 <- pixel 5
	sh	t0,4*2(a2)		// store pixel

	// pixel 5
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,16,8		// T1 <- pixel 6
	sh	t0,5*2(a2)		// store pixel

	// pixel 6
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,t3,24		// T1 <- pixel 7
	sh	t0,6*2(a2)		// store pixel

	// pixel 7
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	addi	a2,a2,8*2		// A2 <- shift destination address
	sh	t0,7*2-8*2(a2)		// store pixel

	// counter
	addi	a6,a6,-RENDER_TILEW	// decrease width
	beqz	a6,8f			// no pixels left

// ---- small loop (by 1 pixel)

6:	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

7:	lbu	t0,(a0)			// load pixel
	addi	a0,a0,1			// A0 <- shift source address
	sh1add	t0,t0,t4		// T0 <- address in palette map
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,(a2)			// store pixel
	addi	a2,a2,2			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE16_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 16x16 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE16_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	4
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE16_8_PAL
DispHstxDviRender_TILE16_8_PAL:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

	// prepare palette mapping -> T4 (map)
	lw	t4,DISPHSTXSLOT_PAL(a0)	// T4 <- pointer to palettes in RGB565 format

// ---- big loop (by 1 tile)

	srli	a1,a6,RENDER_TILEW_BIT	// A1 big loop counter <- w >> TILEW_B
	beqz	a1,4f			// skip if loop counter is 0
	add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with tile index, (kk) sample with 4 pixels
//  T0 = temporary 1
//  T1 = temporary 2
//  T3 = next sample with 4 pixels
//  T4 = (map) palette mapping

	// load sample with tile index -> A7
2:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// shift source address

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0(a0)		// A7 <- load pixels
	lw	t3,4(a0)		// T3 <- next pixels

// pixels 0-3

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,0*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,1*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,2*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,t3			// T1 <- pixel 4
	sh	t0,3*2(a2)		// store pixel

// pixels 4-7

	// pixel 4
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,8,8		// T1 <- pixel 5
	sh	t0,4*2(a2)		// store pixel

	// pixel 5
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,16,8		// T1 <- pixel 6
	sh	t0,5*2(a2)		// store pixel

	// pixel 6
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,t3,24		// T1 <- pixel 7
	sh	t0,6*2(a2)		// store pixel

	// pixel 7
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	lw	a7,8(a0)		// A7 <- load pixels
	sh	t0,7*2(a2)		// store pixel

// pixels 8-11

	// pixel 8
	zext.b	t1,a7			// T1 <- pixel 8
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 9
	sh	t0,8*2(a2)		// store pixel

	// pixel 9
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 10
	sh	t0,9*2(a2)		// store pixel

	// pixel 10
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 11
	sh	t0,10*2(a2)		// store pixel

	// pixel 11
	lw	a7,12(a0)		// A7 <- load pixels
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,a7			// T1 <- pixel 12
	sh	t0,11*2(a2)		// store pixel

// pixels 12-15

	// pixel 12
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 13
	sh	t0,12*2(a2)		// store pixel

	// pixel 13
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 14
	sh	t0,13*2(a2)		// store pixel

	// pixel 14
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 15
	sh	t0,14*2(a2)		// store pixel

	// pixel 15
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	addi	a2,a2,16*2		// A2 <- shift destination address
	sh	t0,15*2-16*2(a2)	// store pixel

	// loop
	bne	a3,a1,2b

// ---- small loop (by 1 pixel)

4:	andi	a6,a6,RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

7:	lbu	t0,(a0)			// load pixel
	addi	a0,a0,1			// A0 <- shift source address
	sh1add	t0,t0,t4		// T0 <- address in palette map
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,(a2)			// store pixel
	addi	a2,a2,2			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_TILE32_8_PAL
// ----------------------------------------------------------------------------
//                 tiles 32x32 pixels, 8-bit paletted colors
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_TILE32_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

#define RENDER_TILEW_BIT	5
#define RENDER_TILEW		(1<<RENDER_TILEW_BIT)
#define RENDER_TILEWB		(RENDER_TILEW*1)

// macro - store 4 pixels (T3 = 4 pixels)
//   dst ... first destination index 4, 8, 12, 16, 20, 24
//   next ... next source index 2, 3, 4, 5, 6, 7
.macro store16_4 dst next
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,8,8		// T1 <- pixel
	sh	t0,\dst*2+0*2(a2)	// store pixel

	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,16,8		// T1 <- pixel
	sh	t0,\dst*2+1*2(a2)	// store pixel

	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,t3,24		// T1 <- pixel
	sh	t0,\dst*2+2*2(a2)	// store pixel

	lw	t3,\next*4(a0)		// T3 <- load pixels
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,t3			// T1 <- pixel
	sh	t0,\dst*2+3*2(a2)	// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_TILE32_8_PAL
DispHstxDviRender_TILE32_8_PAL:

	// prepare source pointer -> A3 (src), get row index -> T4
	lhu	t2,DISPHSTXSLOT_PITCH(a0) // T2 pitch <- pitch of the line
	lw	t3,DISPHSTXSLOT_BUF(a0)	// T3 src <- source buffer base address
	srai	t4,a1,RENDER_TILEW_BIT	// T4 <- row <- line / tile_height
	mul	t2,t4,t2		// T2 <- row*pitch
	add	a3,t2,t3		// A3 src <- row*pitch + buf

	// destination buffer -> A2 (dst)
	lw	a2,(a2)			// A2 dst <- destination buffer

	// prepare pointer to tiles -> A4 (tiles), delta of tiles -> A5 (d)
	lh	t3,DISPHSTXSLOT_FONTH(a0) // T3 fonth <- tile pitch in horizontal row mode
	li	a5,RENDER_TILEWB	// A5 <- delta of tiles in horizontal row mode
	bgt	t3,a5,2f		// skip if tile pitch is not small
	mv	t3,a5			// T3 fonth <- tile pitch in vertical column mode
	li	a5,RENDER_TILEWB*RENDER_TILEW // A5 <- delta of tiles in vertical column mode
2:	lw	a4,DISPHSTXSLOT_FONT(a0) // A4 font <- pointer to tiles
	andi	a1,a1,RENDER_TILEW-1	// A1 <- subline
	mul	a1,a1,t3		// A1 <- subline*fonth
	add	a4,a4,a1		// A4 tiles <- subline*fonth + font

	// prepare width of the slot -> A6 (w)
	lhu	a6,DISPHSTXSLOT_W(a0)	// A6 w <- width of slot in graphics pixels

	// prepare palette mapping -> T4 (map)
	lw	t4,DISPHSTXSLOT_PAL(a0)	// T4 <- pointer to palettes in RGB565 format

// ---- big loop (by 1 tile)

	srli	a1,a6,RENDER_TILEW_BIT	// A1 big loop counter <- w >> TILEW_B
	beqz	a1,4f			// skip if loop counter is 0
	add	a1,a1,a3		// A1 <- end of source address

// Here is:
//  A0 = (s) temporary source
//  A1 = (i) loop counter, or end of source address
//  A2 = (dst) destination
//  A3 = (src) source
//  A4 = (tiles) pointer to tiles
//  A5 = (d) delta of tiles
//  A6 = (w) width of slot
//  A7 = (k) sample with tile index, (kk) sample with 4 pixels
//  T0 = temporary 1
//  T1 = temporary 2
//  T3 = next sample with 4 pixels
//  T4 = (map) palette mapping

	// load sample with tile index -> A7
2:	lbu	a7,0(a3)		// A7 <- load tile index
	addi	a3,a3,1			// shift source address

	// prepare pointer to tile graphics -> A0
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

	// load first 4 pixels -> A4
	lw	a7,0*4(a0)		// A7 <- load pixels
	lw	t3,1*4(a0)		// T3 <- next pixels

// pixels 0-3

	// pixel 0
	zext.b	t1,a7			// T1 <- pixel 0
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,8,8		// T1 <- pixel 1
	sh	t0,0*2(a2)		// store pixel

	// pixel 1
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,a7,16,8		// T1 <- pixel 2
	sh	t0,1*2(a2)		// store pixel

	// pixel 2
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,a7,24		// T1 <- pixel 3
	sh	t0,2*2(a2)		// store pixel

	// pixel 3
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	zext.b	t1,t3			// T1 <- pixel 4
	sh	t0,3*2(a2)		// store pixel

	// pixels 4-7
	store16_4 4,2

	// pixels 8-11
	store16_4 8,3

	// pixels 12-15
	store16_4 12,4

	// pixels 16-19
	store16_4 16,5

	// pixels 20-23
	store16_4 20,6

	// pixels 24-27
	store16_4 24,7

// pixels 28-31

	// pixel 28
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,8,8		// T1 <- pixel 29
	sh	t0,28*2(a2)		// store pixel

	// pixel 29
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	h3.bextmi t1,t3,16,8		// T1 <- pixel 30
	sh	t0,29*2(a2)		// store pixel

	// pixel 30
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	srli	t1,t3,24		// T1 <- pixel 31
	sh	t0,30*2(a2)		// store pixel

	// pixel 31
	sh1add	t1,t1,t4		// T1 <- offset in palette map
	lhu	t0,0(t1)		// T0 <- load pixel
	addi	a2,a2,32*2		// A2 <- shift destination address
	sh	t0,31*2-32*2(a2)	// store pixel

	// loop
	bne	a3,a1,2b

// ---- small loop (by 1 pixel)

4:	andi	a6,a6,RENDER_TILEW-1	// A6 small loop counter
	beqz	a6,8f			// skip loops (if loop counter = 0)

	lbu	a7,0(a3)		// A7 <- load tile index
	mul	t0,a7,a5		// T0 <- tile_index * delta
	add	a0,t0,a4		// A0 <- tile pointer

7:	lbu	t0,(a0)			// load pixel
	addi	a0,a0,1			// A0 <- shift source address
	sh1add	t0,t0,t4		// T0 <- address in palette map
	lhu	t0,(t0)			// T0 <- load pixel
	sh	t0,(a2)			// store pixel
	addi	a2,a2,2			// A2 <- shift destination address
	addi	a6,a6,-1		// loop counter
	bnez	a6,7b

8:	ret

#undef RENDER_TILEW_BIT
#undef RENDER_TILEW
#undef RENDER_TILEWB
#endif

#if DISPHSTX_USE_FORMAT_HSTX_15 || DISPHSTX_USE_FORMAT_HSTX_16
// ----------------------------------------------------------------------------
//                            HSTX RLE compression
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_HSTX(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_HSTX
DispHstxDviRender_HSTX:

	lw	a5,DISPHSTXSLOT_FONT(a0) // A5 (font) <- pointer to font
	lw	a3,DISPHSTXSLOT_BUF(a0)	// A3 (buf) <- source buffer base address
	sh2add	a1,a1,a5		// A1 <- font + line
	lw	a5,0(a1)		// A5 <- offset of current line
	lw	a4,4(a1)		// A4 <- offset of next line
	sub	a4,a4,a5		// A4 <- length of current line
	sw	a4,-4(a2)		// store length of current line
	sh1add	a5,a5,a3		// A5 <- address of current line
	sw	a5,0(a2)		// store address of current line
	ret
#endif

#if DISPHSTX_USE_FORMAT_PAT_8
// ----------------------------------------------------------------------------
//                       repeated pattern, 8-bit format
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Pat_8(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Pat_8
DispHstxDviRender_Pat_8:

	// line modulo pattern height
	lh	a5,DISPHSTXSLOT_FONTH(a0) // A5 <- fonth (modulo lines)
	rem	a1,a1,a5		// A1 <- line % fonth

	// prepare source address -> T4
	lhu	t3,DISPHSTXSLOT_PITCH(a0) // T3 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t3,a1		// T0 <- line*pitch = offset in buffer
	add	t4,t1,t0		// T4 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A7
	lhu	a7,DISPHSTXSLOT_W(a0)	// A7 <- width of slot in graphics pixels

// ---- start next pattern

DispHstxDviRender_Pat_8_2:

	mv	a1,t4			// A1 <- source
	mv	a4,t3			// A4 <- width of the pattern
	ble	a4,a7,1f		// check remaining width
	mv	a4,a7			// limit width
1:	sub	a7,a7,a4		// decrease total width

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	slli	a5,a5,4			// A5 <- counter * 16
	add	a5,a5,a1		// A5 <- end source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A4 = width of pattern to render
//  A5 = loop counter, end of loop
//  A6 = sample with 4 pixels
//  A7 = original width
//  T3 = original pitch (pattern width)
//  T4 = original source buffer

2:	lw	a0,0(a1)		// A6 <- load word
	sw	a0,0(a2)		// store word
	lw	a0,1*4(a1)		// A6 <- load word
	sw	a0,1*4(a2)		// store word
	lw	a0,2*4(a1)		// A6 <- load word
	sw	a0,2*4(a2)		// store word
	lw	a0,3*4(a1)		// A6 <- load word
	sw	a0,3*4(a2)		// store word
	addi	a1,a1,16		// A1 <- increment source address
	addi	a2,a2,16		// A2 <- increment destination address
	bne	a5,a1,2b		// next step

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> A5
5:	andi	a5,a4,0x0f		// A5 <- width low 4 bits
	beqz	a5,8f			// skip if counter is 0
	add	a5,a5,a1		// A5 <- end of source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A5 = loop counter, end of loop
//  A6 = sample with 4 pixels
//  A7 = original width
//  T3 = original pitch (pattern width)
//  T4 = original source buffer

	// process one pixel
6:	lbu	a0,0(a1)		// A0 <- load byte
	addi	a1,a1,1			// A1 <- increment source address
	sb	a0,0(a2)		// store pixel
	addi	a2,a2,1			// A2 <- shift destination address
	bne	a5,a1,6b		// next step

	// check if all is done
8:	bnez	a7,DispHstxDviRender_Pat_8_2 // check remaining width
	ret
#endif

#if DISPHSTX_USE_FORMAT_PAT_8_PAL
// ----------------------------------------------------------------------------
//                     repeated pattern, 8-bit format, paletted
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Pat_8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 8-bit pixel
//   dst ... destination index 0..31
//   bit ... source bit index 0..24
// A0 = temporary register
// A2 = destination buffer
// A3 = palette mapping address
// A6 = source sample with 4 pixels
.macro storepat8 dst bit
	h3.bextmi a0,a6,\bit,8		// A0 <- extract bits from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Pat_8_PAL
DispHstxDviRender_Pat_8_PAL:

	// line modulo pattern height
	lh	a5,DISPHSTXSLOT_FONTH(a0) // A5 <- fonth (modulo lines)
	rem	a1,a1,a5		// A1 <- line % fonth

	// prepare source address -> T4
	lhu	t3,DISPHSTXSLOT_PITCH(a0) // T3 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t3,a1		// T0 <- line*pitch = offset in buffer
	add	t4,t1,t0		// T4 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A7
	lhu	a7,DISPHSTXSLOT_W(a0)	// A7 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- start next pattern

DispHstxDviRender_Pat_8_PAL_2:

	mv	a1,t4			// A1 <- source
	mv	a4,t3			// A4 <- width of the pattern
	ble	a4,a7,1f		// check remaining width
	mv	a4,a7			// limit width
1:	sub	a7,a7,a4		// decrease total width

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	slli	a5,a5,4			// A5 <- counter * 16
	add	a5,a5,a1		// A5 <- end source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of pattern to render
//  A5 = loop counter, end of loop
//  A6 = sample with 4 pixels
//  A7 = original width
//  T3 = original pitch (pattern width)
//  T4 = original source buffer

// sample 0, destination index 0..3

	// load next source data -> A6
2:	lw	a6,0(a1)		// A6 <- load word
	addi	a1,a1,16		// A1 <- increment source address

	// process pixel 0, bits 0-7
	zext.b	a0,a6			// A0 <- extract bits 0-7 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0*2(a2)		// store pixel

	storepat8	1,8
	storepat8	2,16

	// process pixel 3, bits 24-31
	srli	a0,a6,24		// A0 <- extract bits 24-31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel

// sample 1, destination index 4..7

	// load next source data -> A6
	lw	a6,4-16(a1)		// A6 <- load word
	sh	a0,3*2(a2)		// store pixel

	// process pixel 4, bits 0-7
	zext.b	a0,a6			// A0 <- extract bits 0-7 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,4*2(a2)		// store pixel

	storepat8	5,8
	storepat8	6,16

	// process pixel 7, bits 24-31
	srli	a0,a6,24		// A0 <- extract bits 24-31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel

// sample 2, destination index 8..11

	// load next source data -> A6
	lw	a6,8-16(a1)		// A6 <- load word
	sh	a0,7*2(a2)		// store pixel

	// process pixel 8, bits 0-7
	zext.b	a0,a6			// A0 <- extract bits 0-7 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,8*2(a2)		// store pixel

	storepat8	9,8
	storepat8	10,16

	// process pixel 11, bits 24-31
	srli	a0,a6,24		// A0 <- extract bits 24-31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel

// sample 3, destination index 12..15

	// load next source data -> A6
	lw	a6,12-16(a1)		// A6 <- load word
	sh	a0,11*2(a2)		// store pixel

	// process pixel 12, bits 0-7
	zext.b	a0,a6			// A0 <- extract bits 0-7 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,12*2(a2)		// store pixel

	storepat8	13,8
	storepat8	14,16

	// process pixel 15, bits 24-31
	srli	a0,a6,24		// A0 <- extract bits 24-31 from the sample
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,15*2(a2)		// store pixel

	// shift destination address
	addi	a2,a2,16*2		// A2 <- increment destination address

	// loop counter
	bne	a5,a1,2b		// next step

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> A5
5:	andi	a5,a4,0x0f		// A5 <- width low 4 bits
	beqz	a5,8f			// skip if counter is 0
	add	a5,a5,a1		// A5 <- end of source address

// Here is:
//  A0 = temporary register
//  A1 = source buffer
//  A2 = destination buffer
//  A3 = palette mapping
//  A5 = loop counter, end of loop
//  A6 = sample with 4 pixels
//  A7 = original width
//  T3 = original pitch (pattern width)
//  T4 = original source buffer

	// process one pixel
6:	lbu	a0,0(a1)		// A0 <- load byte
	addi	a1,a1,1			// A1 <- increment source address
	sh1add	a0,a0,a3		// A0 <- bit*2 + map, offset in palette mapping
	lhu	a0,0(a0)		// A0 <- load pixel
	sh	a0,0(a2)		// store pixel

	// shift destination address
	addi	a2,a2,2			// A2 <- shift destination address

	// loop counter
	bne	a5,a1,6b		// next step

	// check if all is done
8:	bnez	a7,DispHstxDviRender_Pat_8_PAL_2 // check remaining width
	ret
#endif

#if DISPHSTX_USE_FORMAT_RLE8
// ----------------------------------------------------------------------------
//                            RLE8 compression
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Rle8(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Rle8
DispHstxDviRender_Rle8:

	// prepare source address -> A5
	lw	a5,DISPHSTXSLOT_FONT(a0) // A5 (font) <- pointer to font
	lw	a3,DISPHSTXSLOT_BUF(a0)	// A3 (buf) <- source buffer base address
	sh2add	a1,a1,a5		// A1 <- font + line
	lw	a5,0(a1)		// A5 <- offset of current line
	add	a5,a5,a3		// A5 <- address of current line

	// prepare destination address -> A3
	lw	a3,0(a2)		// A3 <- load data address from the command buffer

	// prepare width of the slot -> A7
	lhu	a7,DISPHSTXSLOT_W(a0)	// A7 <- width of slot in graphics pixels

	// prepare constants
	li	t3,3 - 128		// T3 <- constant 3 - 128
	li	t4,1			// T4 <- constant 1
	li	t5,2			// T5 <- constant 2
	li	t6,128			// T6 <- constant 128

// ---- main loop

// Here is:
//  A0 = temporary
//  A1 = color
//  A2 = (n) token, data length
//  A3 = (dst) destination buffer
//  A5 = (src) source buffer
//  A7 = (w) width of the slot
//  T2 = small loop counter
//  T3 = constant 3 - 128
//  T4 = constant 1
//  T5 = constant 2
//  T6 = constant 128

	// get token -> A2
2:	lbu	a2,(a5)			// A2 <- load token
	addi	a5,a5,1			// A5 <- shift source address

	// check repeat mode
	blt	a2,t6,6f		// skip if token < 128

// repeat mode

	// get data length -> A2
	add	a2,a2,t3		// A2 <- length = token - 128 + 3
	min	a2,a2,a7		// A2 <- limit length
	sub	a7,a7,a2		// A7 <- decrease width

	// get color -> A1
	lbu	a1,(a5)			// A1 <- color
	addi	a5,a5,1			// A5 <- shift source address

	// prepare big loop counter -> T2
	srli	t2,a2,2			// T2 <- data length / 4
	beqz	t2,4f			// skip if counter is zero
	sh2add	t2,t2,a3		// T2 <- end of loop

	// store color by 4 pixels
3:	sb	a1,0(a3)
	sb	a1,1(a3)
	sb	a1,2(a3)
	sb	a1,3(a3)
	addi	a3,a3,4			// shift destination address
	bne	t2,a3,3b		// loop

	// prepare small loop counter -> T2
4:	andi	t2,a2,3			// T2 <- data length % 4
	beqz	t2,9f			// continue main loop

	// store color by 1 pixel
	sb	a1,0(a3)
	beq	t2,t4,10f		// end of loop on 1
	sb	a1,1(a3)
	beq	t2,t5,10f		// end of loop on 2
	sb	a1,2(a3)
	j	10f

// Here is:
//  A0 = temporary
//  A1 = color
//  A2 = (n) token, data length
//  A3 = (dst) destination buffer
//  A5 = (src) source buffer
//  A7 = (w) width of the slot
//  T2 = small loop counter
//  T3 = constant 3 - 128
//  T4 = constant 1
//  T5 = constant 2
//  T6 = constant 128

// raw data

	// get data length -> A2
6:	addi	a2,a2,1			// A2 <- length = token + 1
	min	a2,a2,a7		// A2 <- limit length
	sub	a7,a7,a2		// A7 <- decrease width

	// prepare big loop counter -> T2
	srli	t2,a2,2			// T2 <- data length / 4
	beqz	t2,8f			// skip if counter is zero
	sh2add	t2,t2,a3		// T2 <- end of loop

	// transfer by 4 pixels
7:	lbu	a0,0(a5)
	sb	a0,0(a3)
	lbu	a0,1(a5)
	sb	a0,1(a3)
	lbu	a0,2(a5)
	sb	a0,2(a3)
	lbu	a0,3(a5)
	sb	a0,3(a3)
	addi	a5,a5,4
	addi	a3,a3,4
	bne	t2,a3,7b

	// prepare small loop counter -> T2
8:	andi	t2,a2,3			// T2 <- data length % 4
	beqz	t2,9f			// continue main loop

	// transfer by 1 pixel
	lbu	a0,0(a5)
	sb	a0,0(a3)
	beq	t2,t4,11f		// end of loop on 1
	lbu	a0,1(a5)
	sb	a0,1(a3)
	beq	t2,t5,11f		// end of loop on 2
	lbu	a0,2(a5)
	sb	a0,2(a3)

	// shift destination address A3 by loop counter T2
11:	add	a5,t2,a5		// A5 <- shift source address by loop counter
10:	add	a3,t2,a3		// A3 <- shift destination address by loop counter

	// main loop (check remaining width)
9:	bgtz	a7,2b

	ret
#endif

#if DISPHSTX_USE_FORMAT_RLE8_PAL
// ----------------------------------------------------------------------------
//                          RLE8 compression, paletted
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Rle8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Rle8_PAL
DispHstxDviRender_Rle8_PAL:

	// prepare source address -> A5
	lw	a5,DISPHSTXSLOT_FONT(a0) // A5 (font) <- pointer to font
	lw	a3,DISPHSTXSLOT_BUF(a0)	// A3 (buf) <- source buffer base address
	sh2add	a1,a1,a5		// A1 <- font + line
	lw	a5,0(a1)		// A5 <- offset of current line
	add	a5,a5,a3		// A5 <- address of current line

	// prepare destination address -> A3
	lw	a3,0(a2)		// A3 <- load data address from the command buffer

	// prepare width of the slot -> A7
	lhu	a7,DISPHSTXSLOT_W(a0)	// A7 <- width of slot in graphics pixels

	// prepare palette mapping address -> A4
	lw	a4,DISPHSTXSLOT_PAL(a0) // A4 <- pointer to palettes in RGB565 format

	// prepare constants
	li	t3,3 - 128		// T3 <- constant 3 - 128
	li	t4,1			// T4 <- constant 1
	li	t5,2			// T5 <- constant 2
	li	t6,128			// T6 <- constant 128

// ---- main loop

// Here is:
//  A0 = temporary
//  A1 = color
//  A2 = (n) token, data length
//  A3 = (dst) destination buffer
//  A4 = (map) palette mapping
//  A5 = (src) source buffer
//  A7 = (w) width of the slot
//  T2 = small loop counter
//  T3 = constant 3 - 128
//  T4 = constant 1
//  T5 = constant 2
//  T6 = constant 128

	// get token -> A2
2:	lbu	a2,(a5)			// A2 <- load token
	addi	a5,a5,1			// A5 <- shift source address

	// check repeat mode
	blt	a2,t6,6f		// skip if token < 128

// repeat mode

	// get data length -> A2
	add	a2,a2,t3		// A2 <- length = token - 128 + 3
	min	a2,a2,a7		// A2 <- limit length
	sub	a7,a7,a2		// A7 <- decrease width

	// get color -> A1
	lbu	a1,(a5)			// A1 <- color
	addi	a5,a5,1			// A5 <- shift source address
	sh1add	a1,a1,a4		// A1 <- pointer to mapping
	lhu	a1,(a1)			// A1 <- color

	// prepare big loop counter -> T2
	srli	t2,a2,2			// T2 <- data length / 4
	beqz	t2,4f			// skip if counter is zero
	sh3add	t2,t2,a3		// T2 <- end of loop

	// store color by 4 pixels
3:	sh	a1,0(a3)
	sh	a1,2(a3)
	sh	a1,4(a3)
	sh	a1,6(a3)
	addi	a3,a3,8			// shift destination address
	bne	t2,a3,3b		// loop

	// prepare small loop counter -> T2
4:	andi	t2,a2,3			// T2 <- data length % 4
	beqz	t2,9f			// continue main loop

	// store color by 1 pixel
	sh	a1,0(a3)
	beq	t2,t4,10f		// end of loop on 1
	sh	a1,2(a3)
	beq	t2,t5,10f		// end of loop on 2
	sh	a1,4(a3)
	j	10f

// Here is:
//  A0 = temporary
//  A1 = color
//  A2 = (n) token, data length
//  A3 = (dst) destination buffer
//  A5 = (src) source buffer
//  A7 = (w) width of the slot
//  T2 = small loop counter
//  T3 = constant 3 - 128
//  T4 = constant 1
//  T5 = constant 2
//  T6 = constant 128

// raw data

	// get data length -> A2
6:	addi	a2,a2,1			// A2 <- length = token + 1
	min	a2,a2,a7		// A2 <- limit length
	sub	a7,a7,a2		// A7 <- decrease width

	// prepare big loop counter -> T2
	srli	t2,a2,2			// T2 <- data length / 4
	beqz	t2,8f			// skip if counter is zero
	sh3add	t2,t2,a3		// T2 <- end of loop

	// transfer by 4 pixels
7:	lbu	a1,0(a5)
	lbu	a0,1(a5)
	sh1add	a1,a1,a4		// A1 <- pointer to mapping
	lhu	a1,(a1)			// A1 <- color
	sh	a1,0(a3)

	sh1add	a0,a0,a4		// A0 <- pointer to mapping
	lhu	a0,(a0)			// A0 <- color
	sh	a0,2(a3)

	lbu	a1,2(a5)
	lbu	a0,3(a5)
	sh1add	a1,a1,a4		// A1 <- pointer to mapping
	lhu	a1,(a1)			// A1 <- color
	sh	a1,4(a3)

	sh1add	a0,a0,a4		// A0 <- pointer to mapping
	lhu	a0,(a0)			// A0 <- color
	sh	a0,6(a3)

	addi	a5,a5,4
	addi	a3,a3,8
	bne	t2,a3,7b

	// prepare small loop counter -> T2
8:	andi	t2,a2,3			// T2 <- data length % 4
	beqz	t2,9f			// continue main loop

	// transfer by 1 pixel
	lbu	a0,0(a5)
	lbu	a1,1(a5)
	sh1add	a0,a0,a4		// A0 <- pointer to mapping
	lhu	a0,(a0)			// A0 <- color
	sh	a0,0(a3)
	beq	t2,t4,11f		// end of loop on 1

	sh1add	a1,a1,a4		// A1 <- pointer to mapping
	lhu	a1,(a1)			// A1 <- color
	sh	a1,2(a3)
	beq	t2,t5,11f		// end of loop on 2

	lbu	a0,2(a5)
	sh1add	a0,a0,a4		// A0 <- pointer to mapping
	lhu	a0,(a0)			// A0 <- color
	sh	a0,4(a3)

	// shift destination address A3 by loop counter T2
11:	add	a5,t2,a5		// A5 <- shift source address by loop counter
10:	sh1add	a3,t2,a3		// A3 <- shift destination address by loop counter

	// main loop (check remaining width)
9:	bgtz	a7,2b

	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR1_PAL
// ----------------------------------------------------------------------------
//   attribute compression 1, 4-bit paletted colors, cell 8x8, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr1_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 32 pixels
// SP[] = pixels
.macro attr1 dst bit
	bexti	a0,a6,\bit+1		// A0 <- extract bit from the sample
	bexti	a1,a6,\bit		// A1 <- extract bit from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	add	a1,a1,sp		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr1_PAL
DispHstxDviRender_Attr1_PAL:

	// reserve space in stack for local variables
	add	sp,sp,-4		// reserve space in stack for local variable: u8 map2[2]

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,3			// T0 <- line/8
	mul	a4,a4,t0		// A4 <- line/8 * fonth
	add	a7,a7,a4		// A7 <- font[line/8 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 32 pixels)

	// prepare counter of big loop (by 32 pixels) -> A5
	srli	a5,a4,5			// A5 <- width / 32
	beqz	a5,5f			// skip if width/32 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// increment source address

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	addi	a7,a7,4			// increment attribute address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

// byte 0, destination index 0..7, source bit 7..0

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 0-7, bit 7-0
	attr1	0,6
	attr1	2,4
	attr1	4,2
	attr1	6,0

// byte 1, destination index 8..15, source bit 15..8

	// prepare attributes
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 8-15, bit 15-8
	attr1	8,14
	attr1	10,12
	attr1	12,10
	attr1	14,8

// byte 2, destination index 16..23, source bit 23..16

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 16-23, bit 23-16
	attr1	16,22
	attr1	18,20
	attr1	20,18
	attr1	22,16

// byte 3, destination index 24..31, source bit 31..24

	// prepare attributes
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 24-31, bit 31-24
	attr1	24,30
	attr1	26,28
	attr1	28,26
	attr1	30,24

	// shift destination addres
	addi	a2,a2,32*2

	// loop
	bne	a5,t3,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x1f		// A4 <- width low 5 bits
	beqz	a4,8f			// skip if width & 0x1f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	
// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,3			// A5 <- width/8
	beqz	a5,6f

	// middle loop
1:	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	attr1	0,6
	attr1	2,4
	attr1	4,2
	attr1	6,0

	addi	a2,a2,8*2		// shift destination address
	srli	a6,a6,8			// shift sample >> 8
	srli	t2,t2,8			// shift attributes >> 8
	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,7
	beqz	a5,8f
	sh1add	a5,a5,a2		// A5 <- end destination address

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// process one pixel
7:	bexti	a0,a6,7			// A0 <- extract bit from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	addi	a2,a2,2			// increment destination address
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,1			// shift sample << 1
	sh	a0,-2(a2)		// store pixel
	bne	a5,a2,7b

	// restore stack
8:	add	sp,sp,4
	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR2_PAL
// ----------------------------------------------------------------------------
//   attribute compression 2, 4-bit paletted colors, cell 4x4, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr2_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 32 pixels
// SP[] = pixels
.macro attr2 dst bit
	bexti	a0,a6,\bit+1		// A0 <- extract bit from the sample
	bexti	a1,a6,\bit		// A1 <- extract bit from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	add	a1,a1,sp		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr2_PAL
DispHstxDviRender_Attr2_PAL:

	// reserve space in stack for local variables
	add	sp,sp,-4		// reserve space in stack for local variable: u8 map2[2]

	// odd line flag -> T2
	andi	t2,a1,1			// T2 <- odd line flag

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,2			// T0 <- line/4
	mul	a4,t0,a4		// A4 <- line/4 * fonth
	add	a7,a7,a4		// A7 <- font[line/4 * fonth], attribute row

	// prepare source address -> T3
	lhu	a4,DISPHSTXSLOT_PITCH(a0) // A4 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	srli	t0,a1,1			// T0 <- line/2
	mul	a4,t0,a4		// A4 <- line/2 * pitch = offset in buffer
	add	t3,t1,a4		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

	// check if line is even
	bnez	t2,DispHstxDviRender_Attr2_PAL_Odd // skip if line is odd

// ---- even line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width/16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	addi	a7,a7,4			// increment attribute address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 7..4

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 0-3, bit 7-4
	attr2	0,6
	attr2	2,4

// byte 1, destination index 4..7, source bit 15..12

	// prepare attributes
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 4-7, bit 15-12
	attr2	4,14
	attr2	6,12

// byte 2, destination index 8..11, source bit 23..20

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 8-11, bit 23-20
	attr2	8,22
	attr2	10,20

// byte 3, destination index 12..15, source bit 31..28

	// prepare attributes
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 12-15, bit 31-28
	attr2	12,30
	attr2	14,28

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- even line - small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x1f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	
// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,2			// A5 <- width/4
	beqz	a5,6f

	// middle loop
1:	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	attr2	0,6
	attr2	2,4

	addi	a2,a2,4*2		// shift destination address
	srli	a6,a6,8			// shift sample >> 8
	srli	t2,t2,8			// shift attributes >> 8
	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,3
	beqz	a5,8f
	sh1add	a5,a5,a2		// A5 <- end destination address

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// process one pixel
7:	bexti	a0,a6,7			// A0 <- extract bit from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,1			// shift sample << 1
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	bne	a5,a2,7b

	// restore stack
8:	add	sp,sp,4
	ret

DispHstxDviRender_Attr2_PAL_Odd:

// ---- even line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width/16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	addi	a7,a7,4			// increment attribute address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 3..0

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 0-3, bit 3-0
	attr2	0,2
	attr2	2,0

// byte 1, destination index 4..7, source bit 11..8

	// prepare attributes
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 4-7, bit 11-8
	attr2	4,10
	attr2	6,8

// byte 2, destination index 8..11, source bit 19..16

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 8-11, bit 19-16
	attr2	8,18
	attr2	10,16

// byte 3, destination index 12..15, source bit 27..24

	// prepare attributes
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// pixels 12-15, bit 27-24
	attr2	12,26
	attr2	14,24

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- even line - small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x1f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	
// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,2			// A5 <- width/4
	beqz	a5,6f

	// middle loop
1:	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	attr2	0,2
	attr2	2,0

	addi	a2,a2,4*2		// shift destination address
	srli	a6,a6,8			// shift sample >> 8
	srli	t2,t2,8			// shift attributes >> 8
	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,7
	beqz	a5,8f
	sh1add	a5,a5,a2		// A5 <- end destination address

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1

	// process one pixel
7:	bexti	a0,a6,3			// A0 <- extract bit from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,1			// shift sample << 1
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	bne	a5,a2,7b

	// restore stack
8:	add	sp,sp,4
	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR3_PAL
// ----------------------------------------------------------------------------
//   attribute compression 3, 4-bit paletted colors, cell 8x8, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr3_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..28
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 32 pixels
// SP[] = pixels
.macro attr3 dst bit
	h3.bextmi a0,a6,\bit+2,2	// A0 <- extract bits from the sample
	h3.bextmi a1,a6,\bit,2		// A1 <- extract bits from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	add	a1,a1,sp		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr3_PAL
DispHstxDviRender_Attr3_PAL:

	// reserve space in stack for local variables
	add	sp,sp,-4		// reserve space in stack for local variable: u8 map2[2]

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,3			// T0 <- line/8
	mul	a4,a4,t0		// A4 <- line/8 * fonth
	add	a7,a7,a4		// A7 <- font[line/8 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 32 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	addi	a7,a7,4			// increment attribute address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

// byte 0+1, destination index 0..7, source bit 15..0

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color1
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// pixels 0-3, bit 7-0
	attr3	0,4
	attr3	2,0

	// pixels 4-7, bit 15-8
	attr3	4,12
	attr3	6,8

// byte 2+3, destination index 8..15, source bit 31..16

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// pixels 8-11, bit 23-16
	attr3	8,20
	attr3	10,16

	// pixels 12-15, bit 31-24
	attr3	12,28
	attr3	14,24

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x0f is 0

	// load next attributes -> T2
	lw	t2,(a7)			// T2 <- load attributes
	
	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word
	rev8	a6,a6			// reverze byte order

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color1
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// check width 8 pixels
	li	a0,8			// 8 pixels
	blt	a4,a0,7f		// skip if < 8 pixels

	// pixels 0-7, bit 31-16
	attr3	0,28
	attr3	2,24
	attr3	4,20
	attr3	6,16

	addi	a2,a2,8*2		// A2 <- shift destination address
	slli	a6,a6,16		// A6 <- shift sample left by 16 bits
	addi	a4,a4,-8		// A4 <- decrease width
	beqz	a4,8f			// no data left

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// process one pixel
7:	srli	a0,a6,30		// A0 <- extract 2 bits from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,2			// shift sample << 2
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	addi	a4,a4,-1		// decrement loop counter
	bnez	a4,7b

	// restore stack
8:	add	sp,sp,4
	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR4_PAL
// ----------------------------------------------------------------------------
//   attribute compression 4, 4-bit paletted colors, cell 4x4, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr4_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..28
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 32 pixels
// SP[] = pixels
.macro attr4 dst bit
	h3.bextmi a0,a6,\bit+2,2	// A0 <- extract bits from the sample
	h3.bextmi a1,a6,\bit,2		// A1 <- extract bits from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	add	a1,a1,sp		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr4_PAL
DispHstxDviRender_Attr4_PAL:

	// reserve space in stack for local variables
	add	sp,sp,-4		// reserve space in stack for local variable: u8 map2[2]

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,2			// T0 <- line/4
	mul	a4,a4,t0		// A4 <- line/4 * fonth
	add	a7,a7,a4		// A7 <- font[line/4 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 32 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next attributes -> T2
2:	lw	t2,(a7)			// T2 <- load attributes

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 7..0

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color1
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// pixels 0-3, bit 7-0
	attr4	0,4
	attr4	2,0

// byte 1, destination index 4..7, source bit 15..8

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// load next attributes -> T2
	lw	t2,4(a7)		// T2 <- load attributes

	// pixels 4-7, bit 15-8
	attr4	4,12
	attr4	6,8

// byte 2, destination index 8..11, source bit 23..16

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color1
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// pixels 8-11, bit 23-16
	attr4	8,20
	attr4	10,16

// byte 3, destination index 12..15, source bit 31..24

	// prepare attributes
	h3.bextmi a0,t2,20,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	h3.bextmi a0,t2,16,4		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,28,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color0
	h3.bextmi a0,t2,24,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// pixels 12-15, bit 31-24
	attr4	12,28
	attr4	14,24

	// shift destination addres
	addi	a2,a2,16*2
	addi	a7,a7,8			// increment attribute address

	// loop
	bne	a5,t3,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x0f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word
	rev8	a6,a6			// reverze byte order

	// load next attributes -> T2
	lhu	t2,(a7)			// T2 <- load attributes
	addi	a7,a7,2			// increment attribute address
	
// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T2 = attributes
//  T3 = source buffer

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color1
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// prepare middle loop counter -> A5
	srli	a5,a4,2			// A5 <- width/4
	beqz	a5,6f

	// pixels 0-3, bit 31-24
1:	attr4	0,28
	attr4	2,24

	// load next attributes -> T2
	lhu	t2,(a7)			// T2 <- load attributes
	addi	a7,a7,2			// increment attribute address

	// prepare attributes
	h3.bextmi a0,t2,4,4		// A0 <- extract bits of color0
	sb	a0,0(sp)		// map2[0] <- color0
	andi	a0,t2,0x0f		// A0 <- extract bits of color1
	sb	a0,1(sp)		// map2[1] <- color1
	h3.bextmi a0,t2,12,4		// A0 <- extract bits of color2
	sb	a0,2(sp)		// map2[2] <- color1
	h3.bextmi a0,t2,8,4		// A0 <- extract bits of color3
	sb	a0,3(sp)		// map2[3] <- color1

	// shift destination addres
	addi	a2,a2,4*2

	// shift sample
	slli	a6,a6,8

	// loop
	addi	a5,a5,-1
	bnez	a5,1b

	// prepare small loop counter -> A5
6:	andi	a5,a4,3			// A5 <- width % 4
	beqz	a5,8f			// no data left

	// process one pixel
7:	srli	a0,a6,30		// A0 <- extract 2 bits from the sample
	add	a0,a0,sp		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,2			// shift sample << 2
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	addi	a5,a5,-1		// decrement loop counter
	bnez	a5,7b

	// restore stack
8:	add	sp,sp,4
	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR5_PAL
// ----------------------------------------------------------------------------
//   attribute compression 5, 8-bit paletted colors, cell 8x8, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr5_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 32 pixels
// A7 = pointer to attributes
.macro attr5 dst bit
	bexti	a0,a6,\bit+1		// A0 <- extract bit from the sample
	bexti	a1,a6,\bit		// A1 <- extract bit from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	add	a1,a1,a7		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr5_PAL
DispHstxDviRender_Attr5_PAL:

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,3			// T0 <- line/8
	mul	a4,a4,t0		// A4 <- line/8 * fonth
	add	a7,a7,a4		// A7 <- font[line/8 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 32 pixels)

	// prepare counter of big loop (by 32 pixels) -> A5
	srli	a5,a4,5			// A5 <- width / 32
	beqz	a5,5f			// skip if width/32 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

// byte 0, destination index 0..7, source bit 7..0

	attr5	0,6
	attr5	2,4
	attr5	4,2
	attr5	6,0
	addi	a7,a7,2			// A7 <- shift attributes

// byte 1, destination index 8..15, source bit 15..8

	attr5	8,14
	attr5	10,12
	attr5	12,10
	attr5	14,8
	addi	a7,a7,2			// A7 <- shift attributes

// byte 2, destination index 16..23, source bit 23..16

	attr5	16,22
	attr5	18,20
	attr5	20,18
	attr5	22,16
	addi	a7,a7,2			// A7 <- shift attributes

// byte 3, destination index 24..31, source bit 31..24

	attr5	24,30
	attr5	26,28
	attr5	28,26
	attr5	30,24
	addi	a7,a7,2			// A7 <- shift attributes

	// shift destination addres
	addi	a2,a2,32*2

	// loop
	bne	a5,t3,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x1f		// A4 <- width low 5 bits
	beqz	a4,8f			// skip if width & 0x1f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,3			// A5 <- width/8
	beqz	a5,6f

	// middle loop
1:	attr5	0,6
	attr5	2,4
	attr5	4,2
	attr5	6,0
	addi	a7,a7,2			// A7 <- shift attributes

	addi	a2,a2,8*2		// shift destination address
	srli	a6,a6,8			// shift sample >> 8
	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,7
	beqz	a5,8f
	sh1add	a5,a5,a2		// A5 <- end destination address

	// process one pixel
7:	bexti	a0,a6,7			// A0 <- extract bit from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,1			// shift sample << 1
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	bne	a5,a2,7b

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR6_PAL
// ----------------------------------------------------------------------------
//   attribute compression 6, 8-bit paletted colors, cell 4x4, 2 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr6_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..30
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 32 pixels
// A7 = pointer to attributes
.macro attr6 dst bit
	bexti	a0,a6,\bit+1		// A0 <- extract bit from the sample
	bexti	a1,a6,\bit		// A1 <- extract bit from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	add	a1,a1,a7		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr6_PAL
DispHstxDviRender_Attr6_PAL:

	// odd line flag -> T2
	andi	t2,a1,1			// T2 <- odd line flag

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,2			// T0 <- line/4
	mul	a4,a4,t0		// A4 <- line/4 * fonth
	add	a7,a7,a4		// A7 <- font[line/4 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	srli	a1,a1,1			// A1 <- line/2
	mul	t0,t0,a1		// T0 <- line/2*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

	// check if line is even
	bnez	t2,DispHstxDviRender_Attr6_PAL_Odd // skip if line is odd

// ---- even line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 7..4

	attr6	0,6
	attr6	2,4
	addi	a7,a7,2			// A7 <- shift attributes

// byte 1, destination index 4..7, source bit 15..12

	attr6	4,14
	attr6	6,12
	addi	a7,a7,2			// A7 <- shift attributes

// byte 2, destination index 8..11, source bit 23..20

	attr6	8,22
	attr6	10,20
	addi	a7,a7,2			// A7 <- shift attributes

// byte 3, destination index 12..15, source bit 31..28

	attr6	12,30
	attr6	14,28
	addi	a7,a7,2			// A7 <- shift attributes

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- even line - small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x0f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,2			// A5 <- width/4
	beqz	a5,6f

	// middle loop
1:	attr6	0,6
	attr6	2,4
	addi	a7,a7,2			// A7 <- shift attributes

	addi	a2,a2,4*2		// shift destination address
	srli	a6,a6,8			// shift sample >> 8
	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,3
	beqz	a5,8f
	sh1add	a5,a5,a2		// A5 <- end destination address

	// process one pixel
7:	bexti	a0,a6,7			// A0 <- extract bit from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,1			// shift sample << 1
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	bne	a5,a2,7b

8:	ret

DispHstxDviRender_Attr6_PAL_Odd:

// ---- odd line - big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 3..0

	attr6	0,2
	attr6	2,0
	addi	a7,a7,2			// A7 <- shift attributes

// byte 1, destination index 4..7, source bit 11..8

	attr6	4,10
	attr6	6,8
	addi	a7,a7,2			// A7 <- shift attributes

// byte 2, destination index 8..11, source bit 19..16

	attr6	8,18
	attr6	10,16
	addi	a7,a7,2			// A7 <- shift attributes

// byte 3, destination index 12..15, source bit 27..24

	attr6	12,26
	attr6	14,24
	addi	a7,a7,2			// A7 <- shift attributes

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- odd line - small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x0f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 32 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,2			// A5 <- width/4
	beqz	a5,6f

	// middle loop
1:	attr6	0,2
	attr6	2,0
	addi	a7,a7,2			// A7 <- shift attributes

	addi	a2,a2,4*2		// shift destination address
	srli	a6,a6,8			// shift sample >> 8
	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,3
	beqz	a5,8f
	sh1add	a5,a5,a2		// A5 <- end destination address

	// process one pixel
7:	bexti	a0,a6,3			// A0 <- extract bit from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,1			// shift sample << 1
	sh	a0,(a2)			// store pixel
	addi	a2,a2,2			// increment destination address
	bne	a5,a2,7b

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR7_PAL
// ----------------------------------------------------------------------------
//   attribute compression 7, 8-bit paletted colors, cell 8x8, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr7_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..28
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 16 pixels
// A7 = pointer to attributes
.macro attr7 dst bit
	h3.bextmi a0,a6,\bit+2,2	// A0 <- extract bits from the sample
	h3.bextmi a1,a6,\bit,2		// A1 <- extract bits from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	add	a1,a1,a7		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr7_PAL
DispHstxDviRender_Attr7_PAL:

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,3			// T0 <- line/8
	mul	a4,a4,t0		// A4 <- line/8 * fonth
	add	a7,a7,a4		// A7 <- font[line/8 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 7..0

	attr7	0,4
	attr7	2,0

// byte 1, destination index 4..7, source bit 15..8

	attr7	4,12
	attr7	6,8
	addi	a7,a7,4			// A7 <- shift attributes

// byte 2, destination index 8..11, source bit 23..16

	attr7	8,20
	attr7	10,16

// byte 3, destination index 12..15, source bit 31..24

	attr7	12,28
	attr7	14,24
	addi	a7,a7,4			// A7 <- shift attributes

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x0f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word
	rev8	a6,a6			// reverze byte order

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

	// check width 8 pixels
	li	a0,8			// 8 pixels
	blt	a4,a0,6f		// skip if < 8 pixels

	// middle loop
	attr7	0,28
	attr7	2,24
	attr7	4,20
	attr7	6,16

	addi	a7,a7,4			// A7 <- shift attributes
	addi	a2,a2,8*2		// shift destination address
	slli	a6,a6,16		// shift sample << 16

	addi	a4,a4,-8		// A4 <- decrease width
	beqz	a4,8f			// no data left
6:	sh1add	a5,a4,a2		// A5 <- end of loop

	// process one pixel
7:	srli	a0,a6,30		// A0 <- extract bits from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,2			// shift sample << 2
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	addi	a2,a2,2			// increment destination address
	sh	a0,-2(a2)		// store pixel
	bne	a5,a2,7b

8:	ret
#endif

#if DISPHSTX_USE_FORMAT_ATTR8_PAL
// ----------------------------------------------------------------------------
//   attribute compression 8, 8-bit paletted colors, cell 4x4, 4 attributes
// ----------------------------------------------------------------------------
// INPUT: A0 = pointer to slot descriptor
//	  A1 = current graphics line
//	  A2 = pointer to command buffer entry with pointer to render buffer
// ----------------------------------------------------------------------------
// void DispHstxDviRender_Attr8_PAL(sDispHstxVSlot* slot, int line, u32* cmd);

// macro - store 2x 1-bit pixel
//   dst ... destination index 0..30
//   bit ... source bit index 0..28
// A0 = temporary register
// A1 = temporary register 2
// A2 = destination buffer
// A3 = palette mapping
// A6 = source sample with 16 pixels
// A7 = pointer to attributes
.macro attr8 dst bit
	h3.bextmi a0,a6,\bit+2,2	// A0 <- extract bits from the sample
	h3.bextmi a1,a6,\bit,2		// A1 <- extract bits from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	add	a1,a1,a7		// A1 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	lbu	a1,(a1)			// A1 <- load pixel
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	sh1add	a1,a1,a3		// A1 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	lhu	a1,(a1)			// A1 <- load pixel
	sh	a0,\dst*2(a2)		// store pixel
	sh	a1,\dst*2+2(a2)		// store pixel
.endm

.align 2,,		// align address to 4-byte boundary
.global DispHstxDviRender_Attr8_PAL
DispHstxDviRender_Attr8_PAL:

	// prepare pixel mapping -> A7
	lh	a4,DISPHSTXSLOT_FONTH(a0) // A4 <- fonth (attribute pitch)
	lw	a7,DISPHSTXSLOT_FONT(a0) // A7 font <- pointer to font (attributes)
	srli	t0,a1,2			// T0 <- line/4
	mul	a4,a4,t0		// A4 <- line/4 * fonth
	add	a7,a7,a4		// A7 <- font[line/4 * fonth], attribute row

	// prepare source address -> T3
	lhu	t0,DISPHSTXSLOT_PITCH(a0) // T0 <- load pitch of the line
	lw	t1,DISPHSTXSLOT_BUF(a0)	// T1 <- load buffer base address
	mul	t0,t0,a1		// T0 <- line*pitch = offset in buffer
	add	t3,t1,t0		// T3 <- address in frame buffer

	// prepare destination address -> A2
	lw	a2,0(a2)		// A2 <- load data address from the command buffer

	// prepare width of the slot -> A4
	lhu	a4,DISPHSTXSLOT_W(a0)	// A4 <- width of slot in graphics pixels

	// prepare palette mapping address -> A3
	lw	a3,DISPHSTXSLOT_PAL(a0) // A3 <- pointer to palettes in RGB565 format

// ---- big loop (by 16 pixels)

	// prepare counter of big loop (by 16 pixels) -> A5
	srli	a5,a4,4			// A5 <- width / 16
	beqz	a5,5f			// skip if width/16 is 0
	sh2add	a5,a5,t3		// A5 <- end of source address

	// load next source data -> A6
2:	lw	a6,(t3)			// A6 <- load word
	addi	t3,t3,4			// T3 <- increment source address

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

// byte 0, destination index 0..3, source bit 7..0

	attr8	0,4
	attr8	2,0
	addi	a7,a7,4			// A7 <- shift attributes

// byte 1, destination index 4..7, source bit 15..8

	attr8	4,12
	attr8	6,8
	addi	a7,a7,4			// A7 <- shift attributes

// byte 2, destination index 8..11, source bit 23..16

	attr8	8,20
	attr8	10,16
	addi	a7,a7,4			// A7 <- shift attributes

// byte 3, destination index 12..15, source bit 31..24

	attr8	12,28
	attr8	14,24
	addi	a7,a7,4			// A7 <- shift attributes

	// shift destination addres
	addi	a2,a2,16*2

	// loop
	bne	a5,t3,2b

// ---- small loop (by 1 pixel)

	// prepare counter of small loop (by 1 pixel) -> R3
5:	andi	a4,a4,0x0f		// A4 <- width low 4 bits
	beqz	a4,8f			// skip if width & 0x0f is 0

	// load next source data -> A6
	lw	a6,(t3)			// A6 <- load word
	rev8	a6,a6			// reverze byte order

// Here is:
//  A0 = temporary register
//  A1 = temporary register 2
//  A2 = destination buffer
//  A3 = palette mapping
//  A4 = remaingin width of the slot
//  A5 = loop counter, end of loop
//  A6 = sample with 16 pixels
//  A7 = pointer to attributes
//  T3 = source buffer

	// prepare middle loop counter -> A5
	srli	a5,a4,2			// A5 <- width/4
	beqz	a5,6f

	// middle loop
1:	attr8	0,28
	attr8	2,24

	addi	a7,a7,4			// A7 <- shift attributes
	addi	a2,a2,4*2		// shift destination address
	slli	a6,a6,8			// shift sample << 8

	addi	a5,a5,-1		// loop counter
	bnez	a5,1b

	// prepare small loop counter -> R4
6:	andi	a5,a4,3
	beqz	a5,8f

	// process one pixel
7:	srli	a0,a6,30		// A0 <- extract bits from the sample
	add	a0,a0,a7		// A0 <- pixel addres
	lbu	a0,(a0)			// A0 <- load pixel
	slli	a6,a6,2			// shift sample << 2
	sh1add	a0,a0,a3		// A0 <- pixel*2 + map, offset in palette mapping
	lhu	a0,(a0)			// A0 <- load pixel
	addi	a2,a2,2			// increment destination address
	sh	a0,-2(a2)		// store pixel
	addi	a5,a5,-1
	bnez	a5,7b

8:	ret
#endif

#endif // DISPHSTX_RISCV_ASM && RISCV && DISPHSTX_USE_DVI

#endif // USE_DISPHSTX
