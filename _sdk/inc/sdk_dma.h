
// ****************************************************************************
//
//                                      DMA
//                             Direct Memory Access
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

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)

#ifndef _SDK_DMA_H
#define _SDK_DMA_H

//#include "resource.h"		// Resources
#include "../sdk_addressmap.h"		// Register address offsets
#include "../sdk_dreq.h"
#include "sdk_cpu.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_dma.h"		// constants of original SDK
#else
#include "orig_rp2350/orig_dma.h"		// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#if RP2040
#define DMA_CHANNELS 		12		// number of DMA channels
#define DMA_IRQ_NUM		2		// number of IRQs
#else
#define DMA_CHANNELS 		16		// number of DMA channels
#define DMA_IRQ_NUM		4		// number of IRQs
#endif

#define DMA_TIMERS		4		// number of DMA timers

// Trigger: write nonzero value to reload counter and start transfer
// Read address - auto shifted during transfer, can be read to get next address
// Write address - auto shifted during transfer, can be read to get next address
// Transfer count - can be read to get number of remaining transfers; auto reloaded from last set value on the trigger

// DMA channel hardware registers
// DMA channel register index - alias 0
#define DMA_CH_READ		0	// DMA channel read address
#define DMA_CH_WRITE		1	// DMA channel write address
#define DMA_CH_COUNT		2	// DMA channel transfer count (auto reloaded on trigger)
#define DMA_CH_CTRL_TRIG	3	// DMA channel control and status + trigger

// DMA channel register index - alias 1
#define DMA_CH_AL1_CTRL		4	// DMA channel control and status
#define DMA_CH_AL1_READ		5	// DMA channel read address
#define DMA_CH_AL1_WRITE	6	// DMA channel write address
#define DMA_CH_AL1_COUNT_TRIG	7	// DMA channel transfer count + trigger (auto reloaded on trigger)

// DMA channel register index - alias 2
#define DMA_CH_AL2_CTRL		8	// DMA channel control and status
#define DMA_CH_AL2_COUNT	9	// DMA channel transfer count (auto reloaded on trigger)
#define DMA_CH_AL2_READ		10	// DMA channel read address
#define DMA_CH_AL2_WRITE_TRIG	11	// DMA channel write address + trigger

// DMA channel register index - alias 3
#define DMA_CH_AL3_CTRL		12	// DMA channel control and status
#define DMA_CH_AL3_WRITE	13	// DMA channel write address
#define DMA_CH_AL3_COUNT	14	// DMA channel transfer count (auto reloaded on trigger)
#define DMA_CH_AL3_READ_TRIG	15	// DMA channel read address + trigger

// DMA channel hardware interface
typedef struct {
	// alias 0
	io32	read_addr;		// 0x00: Read Address pointer
	io32	write_addr;		// 0x04: Write Address pointer
	io32	transfer_count;		// 0x08: Transfer Count
	io32	ctrl_trig;		// 0x0C: Control and Status
	// alias 1
	io32	al1_ctrl;		// 0x10: Alias 1 CTRL register
	io32	al1_read_addr;		// 0x14: Alias 1 READ_ADDR register
	io32	al1_write_addr;		// 0x18: Alias 1 WRITE_ADDR register
	io32	al1_transfer_count_trig; // 0x1C: Alias 1 TRANS_COUNT register
	// alias 2
	io32	al2_ctrl;		// 0x20: Alias 2 CTRL register
	io32	al2_transfer_count;	// 0x24: Alias 2 TRANS_COUNT register
	io32	al2_read_addr;		// 0x28: Alias 2 READ_ADDR register
	io32	al2_write_addr_trig;	// 0x2C: Alias 2 WRITE_ADDR register
	// alias 3
	io32	al3_ctrl;		// 0x30: Alias 3 CTRL register
	io32	al3_write_addr;		// 0x34: Alias 3 WRITE_ADDR register
	io32	al3_transfer_count;	// 0x38: Alias 3 TRANS_COUNT register
	io32	al3_read_addr_trig;	// 0x3C: Alias 3 READ_ADDR register
} dma_channel_hw_t;

STATIC_ASSERT(sizeof(dma_channel_hw_t) == 0x40, "Incorrect dma_channel_hw_t!");

// DMA hardware registers
#define DMA_CHAN(chan)	((volatile u32*)(DMA_BASE+(chan)*0x40)) // DMA channel base address (chan = 0..11 or 0..15)

#define DMA_READ(chan)		(DMA_CHAN(chan)+DMA_CH_READ)	// DMA channel read register
#define DMA_READ_TRIG(chan)	(DMA_CHAN(chan)+DMA_CH_AL3_READ_TRIG) // DMA channel read register + trigger
#define DMA_WRITE(chan)		(DMA_CHAN(chan)+DMA_CH_WRITE)	// DMA channel write register
#define DMA_WRITE_TRIG(chan)	(DMA_CHAN(chan)+DMA_CH_AL2_WRITE_TRIG) // DMA channel write register + trigger
#define DMA_COUNT(chan)		(DMA_CHAN(chan)+DMA_CH_COUNT)	// DMA channel count register
#define DMA_COUNT_TRIG(chan)	(DMA_CHAN(chan)+DMA_CH_AL1_COUNT_TRIG) // DMA channel count register + trigger
#define DMA_CTRL(chan)		(DMA_CHAN(chan)+DMA_CH_AL1_CTRL)	// DMA channel control register
#define DMA_CTRL_TRIG(chan)	(DMA_CHAN(chan)+DMA_CH_CTRL_TRIG) // DMA channel control register + trigger

#define DMA_INTR	((volatile u32*)(DMA_BASE+0x400))	// raw interrupt status (write 1 to bit 0..15 to clear interrupt request)
#define DMA_INTE0	((volatile u32*)(DMA_BASE+0x404))	// interrupt enables for IRQ 0 (1 on bit 0..15 = pass interrupt to IRQ_DMA_0)
#define DMA_INTF0	((volatile u32*)(DMA_BASE+0x408))	// force interrupts for IRQ 0 (1 on bit 0..15 = force interrupt to IRQ_DMA_0, must be cleared)
#define DMA_INTS0	((volatile u32*)(DMA_BASE+0x40c))	// interrupt status for IRQ 0 (1 on bit 0..15 if request is pending on IRQ_DMA_0, write 1 to clear)
#define DMA_INTE1	((volatile u32*)(DMA_BASE+0x414))	// interrupt enables for IRQ 1 (1 on bit 0..15 = pass interrupt to IRQ_DMA_1)
#define DMA_INTF1	((volatile u32*)(DMA_BASE+0x418))	// force interrupts for IRQ 1 (1 on bit 0..15 = force interrupt to IRQ_DMA_1, must be cleared)
#define DMA_INTS1	((volatile u32*)(DMA_BASE+0x41c))	// interrupt status for IRQ 1 (1 on bit 0..15 if request is pending on IRQ_DMA_1, write 1 to clear)

#if RP2040

#define DMA_TIMER(inx)	((volatile u32*)(DMA_BASE+0x420+(inx)*4)) // base address of fractional timer 0..3, trigger on rate X/Y*sys_clk, bit 0..15=Y, bit 16..31=X, 0=off
#define DMA_MULTRIG	((volatile u32*)(DMA_BASE+0x430))	// multiply trigger, bits 0..15
#define DMA_SNIFF_CTRL	((volatile u32*)(DMA_BASE+0x434))	// sniffer control
#define DMA_SNIFF_DATA	((volatile u32*)(DMA_BASE+0x438))	// sniffer data (write initial seed value, read result)
#define DMA_FIFO_LEVELS	((volatile u32*)(DMA_BASE+0x440))	// debug levels (bit 0..7 = transfer data FIFO, bit 8..15 = write address FIFO, bit 16..23 = read address FIFO)
#define DMA_ABORT	((volatile u32*)(DMA_BASE+0x444))	// abort transfer (write 1 to bit 0..15 to abort transfer, read to wait until abort completed)

#else // RP2040

#define DMA_INTE2	((volatile u32*)(DMA_BASE+0x424))	// interrupt enables for IRQ 2 (1 on bit 0..15 = pass interrupt to IRQ_DMA_2)
#define DMA_INTF2	((volatile u32*)(DMA_BASE+0x428))	// force interrupts for IRQ 2 (1 on bit 0..15 = force interrupt to IRQ_DMA_2, must be cleared)
#define DMA_INTS2	((volatile u32*)(DMA_BASE+0x42c))	// interrupt status for IRQ 2 (1 on bit 0..15 if request is pending on IRQ_DMA_2, write 1 to clear)
#define DMA_INTE3	((volatile u32*)(DMA_BASE+0x434))	// interrupt enables for IRQ 3 (1 on bit 0..15 = pass interrupt to IRQ_DMA_3)
#define DMA_INTF3	((volatile u32*)(DMA_BASE+0x438))	// force interrupts for IRQ 3 (1 on bit 0..15 = force interrupt to IRQ_DMA_3, must be cleared)
#define DMA_INTS3	((volatile u32*)(DMA_BASE+0x43c))	// interrupt status for IRQ 3 (1 on bit 0..15 if request is pending on IRQ_DMA_3, write 1 to clear)

#define DMA_TIMER(inx)	((volatile u32*)(DMA_BASE+0x440+(inx)*4)) // base address of fractional timer 0..3, trigger on rate X/Y*sys_clk, bit 0..15=Y, bit 16..31=X, 0=off
#define DMA_MULTRIG	((volatile u32*)(DMA_BASE+0x450))	// multiply trigger, bits 0..15
#define DMA_SNIFF_CTRL	((volatile u32*)(DMA_BASE+0x454))	// sniffer control
#define DMA_SNIFF_DATA	((volatile u32*)(DMA_BASE+0x458))	// sniffer data (write initial seed value, read result)
#define DMA_FIFO_LEVELS	((volatile u32*)(DMA_BASE+0x460))	// debug levels (bit 0..7 = transfer data FIFO, bit 8..15 = write address FIFO, bit 16..23 = read address FIFO)
#define DMA_ABORT	((volatile u32*)(DMA_BASE+0x464))	// abort transfer (write 1 to bit 0..15 to abort transfer, read to wait until abort completed)

#endif // RP2040

#define DMA_NEXT(chan)	((volatile u32*)(DMA_BASE+0x804+(chan)*0x40)) // next transfer count of channel 0..11 (auto loaded into count register on trigger)

#if !RP2040
// DMA MPU region
typedef struct {
	io32	bar;		// 0x00: base address
	io32	lar;		// 0x04: limit address
} dma_mpu_region_hw_t;

STATIC_ASSERT(sizeof(dma_mpu_region_hw_t) == 0x08, "Incorrect dma_mpu_region_hw_t!");
#endif

// DMA IRQ interupt
typedef struct {
	io32	intr;			// 0x00: Raw Interrupt Status
	io32	inte;			// 0x04: Interrupt Enable
	io32	intf;			// 0x08: Force Interrupt
	io32	ints;			// 0x0C: Interrupt Status
} dma_irq_ctrl_hw_t;

STATIC_ASSERT(sizeof(dma_irq_ctrl_hw_t) == 0x10, "Incorrect dma_irq_ctrl_hw_t!");

// DMA hardware interface
typedef struct {
	dma_channel_hw_t ch[DMA_CHANNELS]; // 0x000: DMA channels
#if RP2040
	io32	_pad0[64];		// 0x300:
	union {
		struct {
			io32	intr;			// 0x400: Raw Interrupt Status
			io32	inte0;			// 0x404: Interrupt Enables for IRQ 0
			io32	intf0;			// 0x408: Force Interrupts for IRQ 0
			io32	ints0;			// 0x40C: Interrupt Status for IRQ 0

			io32	__pad0;			// 0x410:
			io32	inte1;			// 0x414: Interrupt Enables for IRQ 1
			io32	intf1;			// 0x418: Force Interrupts for IRQ 1
			io32	ints1;			// 0x41C: Interrupt Status for IRQ 1
		};
		dma_irq_ctrl_hw_t	irq_ctrl[2];	// 0x400:
	};
#else // RP2040
	union {
		struct {
			io32	intr;			// 0x400: Raw Interrupt Status
			io32	inte0;			// 0x404: Interrupt Enables for IRQ 0
			io32	intf0;			// 0x408: Force Interrupts for IRQ 0
			io32	ints0;			// 0x40C: Interrupt Status for IRQ 0

			io32	__pad0;			// 0x410:
			io32	inte1;			// 0x414: Interrupt Enables for IRQ 1
			io32	intf1;			// 0x418: Force Interrupts for IRQ 1
			io32	ints1;			// 0x41C: Interrupt Status for IRQ 1

			io32	__pad1;			// 0x420:
			io32	inte2;			// 0x424: Interrupt Enables for IRQ 2
			io32	intf2;			// 0x428: Force Interrupts for IRQ 2
			io32	ints2;			// 0x42C: Interrupt Status for IRQ 2

			io32	__pad2;			// 0x430:
			io32	inte3;			// 0x434: Interrupt Enables for IRQ 3
			io32	intf3;			// 0x438: Force Interrupts for IRQ 3
			io32	ints3;			// 0x43C: Interrupt Status for IRQ 3
		};
		dma_irq_ctrl_hw_t	irq_ctrl[4];	// 0x400:
	};
#endif // RP2040

	io32	timer[DMA_TIMERS];	// 0x420 (0x440): 4x Pacing (X/Y) Fractional Timer
	io32	multi_channel_trigger;	// 0x430 (0x450): Trigger one or more channels simultaneously
	io32	sniff_ctrl;		// 0x434 (0x454): Sniffer Control
	io32	sniff_data;		// 0x438 (0x458): Data accumulator for sniff hardware
	io32	_pad1;			// 0x43C (0x45C):
	io32	fifo_levels;		// 0x440 (0x460): Debug RAF, WAF, TDF levels
	io32	abort;			// 0x444 (0x464): Abort an in-progress transfer sequence on one or more channels
#if !RP2040
	io32	n_channels;		// 0x468: R/O Number of DMA channels (12 or 16)
	io32	_pad2[5];		// 0x46C:
	io32	seccfg_h[DMA_CHANNELS];	// 0x480: Security level configuration for channels
	io32	seccfg_irq[4];		// 0x4C0: Security configuration for IRQs
	io32	seccfg_misc;		// 0x4D0: Miscellaneous security configuration
	io32	_pad3[11];		// 0x4D4:
	io32	mpu_ctrl;		// 0x500:
	dma_mpu_region_hw_t mpu_region[8]; // 0x504: Control register for DMA MPU
#endif
} dma_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(dma_hw_t) == 0x448, "Incorrect dma_hw_t!");
#else
STATIC_ASSERT(sizeof(dma_hw_t) == 0x544, "Incorrect dma_hw_t!");
#endif

#define dma_hw ((dma_hw_t*)DMA_BASE)

// DMA debug interface
typedef struct {
	io32	ctrdeq;		// 0x00: get channel DREQ counter
	io32	tcr;		// 0x04: get channel TRANS_COUNT reload value
	io32	pad[14];
} dma_debug_channel_hw_t;

STATIC_ASSERT(sizeof(dma_debug_channel_hw_t) == 0x40, "Incorrect dma_debug_channel_hw_t!");

typedef struct {
	dma_debug_channel_hw_t ch[DMA_CHANNELS];
} dma_debug_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(dma_debug_hw_t) == 0x300, "Incorrect dma_debug_hw_t!");
#else
STATIC_ASSERT(sizeof(dma_debug_hw_t) == 0x400, "Incorrect dma_debug_hw_t!");
#endif

#define dma_debug_hw ((dma_debug_hw_t*)(DMA_BASE + 0x800))

// DMA transfer size
#define DMA_SIZE_8	0	// 1 byte (8 bits)
#define DMA_SIZE_16	1	// 2 bytes (16 bits)
#define DMA_SIZE_32	2	// 4 bytes (32 bits)

// flags of DMA control and status register CTRL
#define DMA_CTRL_ERROR		B31	// 1 = read or write error, channel halts on error and raises IRQ (read only)
#define DMA_CTRL_READ_ERROR	B30	// 1 = read error (write 1 to clear)
#define DMA_CTRL_WRITE_ERROR	B29	// 1 = write error (write 1 to clear)

#if RP2040

#define DMA_CTRL_TREQ_LSB	15
#define DMA_CTRL_CHAIN_LSB	11
#define DMA_CTRL_RING_LSB	6

#define DMA_CTRL_BUSY		B24	// 1 = DMA transfer is busy, transfer was triggered but not finished yet (read only)
#define DMA_CTRL_SNIFF		B23	// 1 = transfer is visible to sniff CRC hardware, 0 = normal DMA transfer
#define DMA_CTRL_BSWAP		B22	// 1 = swap order of bytes
#define DMA_CTRL_QUIET		B21	// 1 = generate IRQ when NULL is written to a trigger, 0 = generate IRQ at end of every transfer block
#define DMA_CTRL_TREQ(dreq)	((u32)(dreq)<<15) // 0..0x3f select transfer request signal 0..0x3a = DREQ_*, 0x3b..0x3e = Timer0..Timer3, 0x3f = permanent request
#define DMA_CTRL_TREQ_FORCE	DMA_CTRL_TREQ(DREQ_FORCE) // permanent request
#define DMA_CTRL_CHAIN(chan)	((chan)<<11) // trigger channel 0..11 when completed, disable by setting to this channel (=default state)
#define DMA_CTRL_RING_WRITE	B10	//  1 = write addresses are wrapped, 0 = read addresses are wrapped
#define DMA_CTRL_RING_SIZE(order) ((order)<<6) // size 1..15 of address wrap region in bytes '1 << order' (2..32768 bytes), 0 = don't wrap
					//     Ring buffer must be aligned to wrap size (only lowest 'order' bits are changed)
#define DMA_CTRL_INC_WRITE	B5	// 1 = increment write address with each transfer (memory), 0 = do not increment (port)

#else // RP2040

#define DMA_CTRL_TREQ_LSB	17
#define DMA_CTRL_CHAIN_LSB	13
#define DMA_CTRL_RING_LSB	8

#define DMA_CTRL_BUSY		B26	// 1 = DMA transfer is busy, transfer was triggered but not finished yet (read only)
#define DMA_CTRL_SNIFF		B25	// 1 = transfer is visible to sniff CRC hardware, 0 = normal DMA transfer
#define DMA_CTRL_BSWAP		B24	// 1 = swap order of bytes
#define DMA_CTRL_QUIET		B23	// 1 = generate IRQ when NULL is written to a trigger, 0 = generate IRQ at end of every transfer block
#define DMA_CTRL_TREQ(dreq)	((u32)(dreq)<<17) // 0..0x3f select transfer request signal 0..0x3a = DREQ_*, 0x3b..0x3e = Timer0..Timer3, 0x3f = permanent request
#define DMA_CTRL_TREQ_FORCE	DMA_CTRL_TREQ(DREQ_FORCE) // permanent request
#define DMA_CTRL_CHAIN(chan)	((chan)<<13) // trigger channel 0..15 when completed, disable by setting to this channel (=default state)
#define DMA_CTRL_RING_WRITE	B12	//  1 = write addresses are wrapped, 0 = read addresses are wrapped
#define DMA_CTRL_RING_SIZE(order) ((order)<<8) // size 1..15 of address wrap region in bytes '1 << order' (2..32768 bytes), 0 = don't wrap
					//     Ring buffer must be aligned to wrap size (only lowest 'order' bits are changed)
#define DMA_CTRL_INC_WRITE_REV	B7	// write address is 1 = decremented, 0 = incremented
#define DMA_CTRL_INC_WRITE	B6	// 1 = increment/decrement write address with each transfer (memory), 0 = do not change (port)
#define DMA_CTRL_INC_READ_REV	B5	// read address is 1 = decremented, 0 = incremented

#endif

#define DMA_CTRL_INC_READ	B4	// 1 = increment read address with each transfer (memory), 0 = do not increment (port)
#define DMA_CTRL_SIZE(size)	((size)<<2) // data transfer size DMA_SIZE_* (0=8 bits, 1=16 bits, 2=32 bits)
#define DMA_CTRL_HIGH_PRIORITY	B1	// 1 = channel is preferred during DMA scheduling round, 0 = normal priority
#define DMA_CTRL_EN		B0	// 1 = channel is enabled and responds to trigger, 0 = channel is paused (BUSY will remain unchanged)

// DMA CRC method
#define DMA_CRC_CRC32		0	// CRC-32 (IEEE802.3)
#define DMA_CRC_CRC32REV	1	// CRC-32 (IEEE802.3) bit reversed data
#define DMA_CRC_CRC16		2	// CRC-16-CCITT
#define DMA_CRC_CRC16REV	3	// CRC-16-CCITT bit reversed data
#define DMA_CRC_XOR		14	// XOR reduction over all data (1 = population is odd)
#define DMA_CRC_SUM		15	// simple 32-bit sum

#define DMA_CRC_MASK		0x0f	// mask of base CRC method (used internally in DMA_CRC function)
#define DMA_CRC_INV		0x10	// invert result (used only as parameter of DMA_CRC function)
#define DMA_CRC_REV		0x20	// bit-reverse result (used only as parameter of DMA_CRC function)

// flags of DMA sniffer control register
#define DMA_SNIFF_INV		B11	// 1 = inverted result on read (bitwise complement)
#define DMA_SNIFF_REV		B10	// 1 = bit-reverse result on read
#define DMA_SNIFF_BSWAP		B9	// 1 = byte reverse sniffed data (cancel effect if BSWAP of DMA is set)
#define DMA_SNIFF_CRC(crc)	((crc)<<5) // CRC method DMA_CRC_* (0=CRC32, 1=CRC32 reversed, 2=CRC16-CCITT, 3=CCR16_CCITT reversed, 14=XOR, 15=sum)
#define DMA_SNIFF_CHAN(chan)	((chan)<<1) // used DMA channel for sniffer 0..11 or 0..15
#define DMA_SNIFF_EN		B0	// 1 = enable sniffer

// address increment direction
#define DMA_DIR_NONE		0	// direction none (address not changed)
#define DMA_DIR_INC		1	// increment address
#define DMA_DIR_DEC		3	// decrement address (only RP2350)

// counter mode
#define DMA_COUNT_NORMAL	0x00000000UL	// mode normal: decrement until 0, then trigger next chained channel
#define DMA_COUNT_TRIGGER	0x10000000UL	// mode trigger self: decrement until 0, then trigger selft and next chained channel (only RP2350)
#define DMA_COUNT_ENDLESS	0xf0000000UL	// mode endless: transfer counter does not decrement (counter should not be 0) (only RP2350)

// claimed DMA channels
extern u16 DmaClaimed;		// claimed DMA channels
extern u8 DmaTimerClaimed;	// claimed DMA timers

// get DMA channel hardware interface from DMA channel index
INLINE dma_channel_hw_t* DMA_GetHw(int dma) { return &dma_hw->ch[dma]; }

// get DMA channel index from DMA channel hardware interface
INLINE u8 DMA_GetInx(const dma_channel_hw_t* hw) { return (u8)(hw - dma_hw->ch); }

// get DMA base address of alias 0..3 of channel 0..11
INLINE volatile u32* DMA_Alias(int dma, int alias) { return DMA_CHAN(dma) + alias*4; }

// === claim DMA channel
// Functions are not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)

// claim DMA channel (mark it as used)
INLINE void DMA_Claim(int dma) { DmaClaimed |= (u16)BIT(dma); }

// claim DMA channels with mask (mark them as used)
INLINE void DMA_ClaimMask(int mask) { DmaClaimed |= (u16)mask; }

// unclaim DMA channel (mark it as not used)
INLINE void DMA_Unclaim(int dma) { DmaClaimed &= (u16)~BIT(dma); }

// unclaim DMA channels with mask (mark them as not used)
INLINE void DMA_UnclaimMask(int mask) { DmaClaimed &= (u16)~mask; }

// check if DMA channel is claimed
INLINE Bool DMA_IsClaimed(int dma) { return (DmaClaimed & (u16)BIT(dma)) != 0; }

// claim free unused DMA channel (returns index of channel. of -1 on error)
s8 DMA_ClaimFree(void);

// === claim DMA timer
// Functions are not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)

// claim DMA timer (mark it as used)
INLINE void DMA_TimerClaim(int timer) { DmaTimerClaimed |= (u8)BIT(timer); }

// unclaim DMA timer (mark it as not used)
INLINE void DMA_TimerUnclaim(int timer) { DmaTimerClaimed &= (u8)~BIT(timer); }

// check if DMA timer is claimed
INLINE Bool DMA_TimerIsClaimed(int timer) { return (DmaTimerClaimed & (u8)BIT(timer)) != 0; }

// claim free unused DMA timer (returns index of timer, -1 on error)
s8 DMA_TimerClaimFree(void);

// === DMA configuration structure (u32 control word)
// After setup, write configuration word using DMA_SetCtrl() or DMA_SetCtrlTrig() function.

// get default configuration word (to setup, write configuration word to DMA control register)
INLINE u32 DMA_CfgDef(int dma) { return
			// DMA_CTRL_SNIFF |	// NOT sniff
			// DMA_CTRL_BSWAP |	// NOT byte swap
			// DMA_CTRL_QUIET |	// NOT quiet
			DMA_CTRL_TREQ_FORCE |	// permanent request
			DMA_CTRL_CHAIN(dma) |	// chain disabled
			// DMA_CTRL_RING_WRITE | // NOT wrap ring on write
			DMA_CTRL_RING_SIZE(0) |	// NOT ring
#if !RP2040
			// DMA_CTRL_INC_WRITE_REV | // NOT decrement write
#endif
			// DMA_CTRL_INC_WRITE |	// NOT increment write
#if !RP2040
			// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
			DMA_CTRL_INC_READ |	// increment read
			DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
			// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
			DMA_CTRL_EN;		// enable DMA
}

// config read increment - use DMA_DIR_* (default DMA_DIR_INC) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgReadInc(u32* cfg, int read_inc)
{
	if (read_inc) *cfg |= DMA_CTRL_INC_READ; else *cfg &= ~DMA_CTRL_INC_READ;
#if !RP2040
	if (read_inc & B1) *cfg |= DMA_CTRL_INC_READ_REV; else *cfg &= ~DMA_CTRL_INC_READ_REV;
#endif
}

// config write increment - use DMA_DIR_* (default DMA_DIR_INC) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgWriteInc(u32* cfg, int write_inc)
{
	if (write_inc) *cfg |= DMA_CTRL_INC_WRITE; else *cfg &= ~DMA_CTRL_INC_WRITE;
#if !RP2040
	if (write_inc & B1) *cfg |= DMA_CTRL_INC_WRITE_REV; else *cfg &= ~DMA_CTRL_INC_WRITE_REV;
#endif
}

// config set data request DREQ_* (default DREQ_FORCE = permanent) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgDreq(u32* cfg, int dreq)
	{ *cfg = (*cfg & ~(0x3f << DMA_CTRL_TREQ_LSB)) | (dreq << DMA_CTRL_TREQ_LSB); }

// config set chain (to disable, set to the same channel number = default) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgChain(u32* cfg, int dma)
	{ *cfg = (*cfg & ~(0x0f << DMA_CTRL_CHAIN_LSB)) | (dma << DMA_CTRL_CHAIN_LSB); }

// config set transfer size DMA_SIZE_* (default DMA_SIZE_32) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgSize(u32* cfg, int size) { *cfg = (*cfg & ~(B2|B3)) | (size << 2); }

// config set ring (default 0 = disabled; apply on read) (to setup, write configuration word to DMA control register)
//  write ... True to apply ring to write address, False to apply ring to read address
//  size ... order of ring size 1..15 (0=disabled, 1=2 byte, 2=4 bytes, 3=8 bytes ... 15=32 KB)
INLINE void DMA_CfgRing(u32* cfg, Bool write, int size)
	{ *cfg = (*cfg & ~(0x1f << DMA_CTRL_RING_LSB)) | (size << DMA_CTRL_RING_LSB) | (write ? DMA_CTRL_RING_WRITE : 0); }

// config set byte swap (default False) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgBSwap(u32* cfg, Bool bswap)
	{ if (bswap) *cfg |= DMA_CTRL_BSWAP; else *cfg &= ~DMA_CTRL_BSWAP; }

// config set quiet (default False) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgQuiet(u32* cfg, Bool quiet)
	{ if (quiet) *cfg |= DMA_CTRL_QUIET; else *cfg &= ~DMA_CTRL_QUIET; }

// config set high priority (default False) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgHighPrior(u32* cfg, Bool high)
	{ if (high) *cfg |= DMA_CTRL_HIGH_PRIORITY; else *cfg &= ~DMA_CTRL_HIGH_PRIORITY; }

// config set enable (default True) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgEnable(u32* cfg, Bool enable)
	{ if (enable) *cfg |= DMA_CTRL_EN; else *cfg &= ~DMA_CTRL_EN; }

// config set sniff enable (default False) (to setup, write configuration word to DMA control register)
INLINE void DMA_CfgSniff(u32* cfg, Bool sniff)
	{ if (sniff) *cfg |= DMA_CTRL_SNIFF; else *cfg &= ~DMA_CTRL_SNIFF; }

// === DMA status

// check DMA read or write error
INLINE Bool DMA_IsError(int dma) { return (*DMA_CTRL(dma) & DMA_CTRL_ERROR) != 0; }
INLINE Bool DMA_IsError_hw(dma_channel_hw_t* hw) { return (hw->al1_ctrl & DMA_CTRL_ERROR) != 0; }

// check DMA read error
INLINE Bool DMA_IsReadError(int dma) { return (*DMA_CTRL(dma) & DMA_CTRL_READ_ERROR) != 0; }
INLINE Bool DMA_IsReadError_hw(dma_channel_hw_t* hw) { return (hw->al1_ctrl & DMA_CTRL_READ_ERROR) != 0; }

// check DMA write error
INLINE Bool DMA_IsWriteError(int dma) { return (*DMA_CTRL(dma) & DMA_CTRL_WRITE_ERROR) != 0; }
INLINE Bool DMA_IsWriteError_hw(dma_channel_hw_t* hw) { return (hw->al1_ctrl & DMA_CTRL_WRITE_ERROR) != 0; }

// clear DMA error flags
INLINE void DMA_ClearError(int dma) { *DMA_CTRL(dma) = *DMA_CTRL(dma); cb(); }
INLINE void DMA_ClearError_hw(dma_channel_hw_t* hw) { hw->al1_ctrl = hw->al1_ctrl; cb(); }

// check if DMA is busy (can stay busy if paused using enable flag)
INLINE Bool DMA_IsBusy(int dma) { return (*DMA_CTRL(dma) & DMA_CTRL_BUSY) != 0; }
INLINE Bool DMA_IsBusy_hw(const dma_channel_hw_t* hw) { return (hw->al1_ctrl & DMA_CTRL_BUSY) != 0; }

// === DMA registers

// get DMA read address
INLINE void* DMA_GetRead(int dma) { return (void*)*DMA_READ(dma); }
INLINE void* DMA_GetRead_hw(const dma_channel_hw_t* hw) { return (void*)hw->read_addr; }

// set DMA read address, without trigger
INLINE void DMA_SetRead(int dma, const volatile void* addr) { *DMA_READ(dma) = (u32)addr; }
INLINE void DMA_SetRead_hw(dma_channel_hw_t* hw, const volatile void* addr) { hw->read_addr = (u32)addr; }

// set DMA read address, with trigger
INLINE void DMA_SetReadTrig(int dma, const volatile void* addr) { cb(); *DMA_READ_TRIG(dma) = (u32)addr; }
INLINE void DMA_SetReadTrig_hw(dma_channel_hw_t* hw, const volatile void* addr) { cb(); hw->al3_read_addr_trig = (u32)addr; }

// get DMA write address
INLINE void* DMA_GetWrite(int dma) { return (void*)*DMA_WRITE(dma); }
INLINE void* DMA_GetWrite_hw(const dma_channel_hw_t* hw) { return (void*)hw->write_addr; }

// set DMA write address, without trigger
INLINE void DMA_SetWrite(int dma, volatile void* addr) { *DMA_WRITE(dma) = (u32)addr; }
INLINE void DMA_SetWrite_hw(dma_channel_hw_t* hw, volatile void* addr) { hw->write_addr = (u32)addr; }

// set DMA write address, with trigger
INLINE void DMA_SetWriteTrig(int dma, volatile void* addr) { cb(); *DMA_WRITE_TRIG(dma) = (u32)addr; }
INLINE void DMA_SetWriteTrig_hw(dma_channel_hw_t* hw, volatile void* addr) { cb(); hw->al2_write_addr_trig = (u32)addr; }

// get DMA transfer remaining count (returns 0..0x0fffffff)
//  Count is number of transfers, not number of bytes.
INLINE u32 DMA_GetCount(int dma) { return (u32)*DMA_COUNT(dma) & 0x0fffffff; }
INLINE u32 DMA_GetCount_hw(const dma_channel_hw_t* hw) { return (u32)hw->transfer_count & 0x0fffffff; }

// set DMA transfer count, without trigger
//  Count can be max. 0x0fffffff (268'435'455).
//  Count can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS.
//  Count is number of transfers, not number of bytes.
INLINE void DMA_SetCount(int dma, u32 count) { *DMA_COUNT(dma) = count; }
INLINE void DMA_SetCount_hw(dma_channel_hw_t* hw, u32 count) { hw->transfer_count = count; }

// set DMA transfer count, with trigger
//  Count can be max. 0x0fffffff (268'435'455).
//  Count can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS.
//  Count is number of transfers, not number of bytes.
INLINE void DMA_SetCountTrig(int dma, u32 count) { cb(); *DMA_COUNT_TRIG(dma) = count; }
INLINE void DMA_SetCountTrig_hw(dma_channel_hw_t* hw, u32 count) { cb(); hw->al1_transfer_count_trig = count; }

// get DMA next transfer count (not updated during transfer, auto loaded to COUNT at trigger)
INLINE u32 DMA_Next(int dma) { return *DMA_NEXT(dma); }

// get DMA control register
INLINE u32 DMA_GetCtrl(int dma) { return *DMA_CTRL(dma); }
INLINE u32 DMA_GetCtrl_hw(const dma_channel_hw_t* hw) { return hw->al1_ctrl; }

// set DMA control register, without trigger
INLINE void DMA_SetCtrl(int dma, u32 ctrl) { *DMA_CTRL(dma) = ctrl; }
INLINE void DMA_SetCtrl_hw(dma_channel_hw_t* hw, u32 ctrl) { hw->al1_ctrl = ctrl; }

// set DMA control register, with trigger
INLINE void DMA_SetCtrlTrig(int dma, u32 ctrl) { cb(); *DMA_CTRL_TRIG(dma) = ctrl; }
INLINE void DMA_SetCtrlTrig_hw(dma_channel_hw_t* hw, u32 ctrl) { cb(); hw->ctrl_trig = ctrl; }

// === DMA setup

// set DMA config, without trigger
//  dma = channel 0..11 (or 0..15)
//  src = source address
//  dst = destination address
//  count = number of transfers (can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS)
//  ctrl = control word (can be prepared using DMA_Cfg* functions)
void DMA_Config(int dma, const volatile void* src, volatile void* dst, u32 count, u32 ctrl);
void DMA_Config_hw(dma_channel_hw_t* hw, const volatile void* src, volatile void* dst, u32 count, u32 ctrl);

// set DMA config, with trigger
//  dma = channel 0..11 (or 0..15)
//  src = source address
//  dst = destination address
//  count = number of transfers (can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS)
//  ctrl = control word (can be prepared using DMA_Cfg* functions)
void DMA_ConfigTrig(int dma, const volatile void* src, volatile void* dst, u32 count, u32 ctrl);
void DMA_ConfigTrig_hw(dma_channel_hw_t* hw, const volatile void* src, volatile void* dst, u32 count, u32 ctrl);

// disable DMA channel (pause - transfer will be paused but not aborted, busy will stay active)
INLINE void DMA_Disable(int dma) { RegClr(DMA_CTRL(dma), DMA_CTRL_EN); }
INLINE void DMA_Disable_hw(dma_channel_hw_t* hw) { RegClr(&hw->al1_ctrl, DMA_CTRL_EN); }

// enable DMA channel (unpause - continue transfers)
INLINE void DMA_Enable(int dma) { RegSet(DMA_CTRL(dma), DMA_CTRL_EN); }
INLINE void DMA_Enable_hw(dma_channel_hw_t* hw) { RegSet(&hw->al1_ctrl, DMA_CTRL_EN); }

// check if DMA is enabled (not paused)
INLINE Bool DMA_IsEnabled(int dma) { return (*DMA_CTRL(dma) & DMA_CTRL_EN) != 0; }
INLINE Bool DMA_IsEnabled_hw(const dma_channel_hw_t* hw) { return (hw->al1_ctrl & DMA_CTRL_EN) != 0; }

// set chain (set to itself to disable)
INLINE void DMA_SetChain(int dma, int chain) { RegMask(DMA_CTRL(dma), chain << DMA_CTRL_CHAIN_LSB, 0x0f << DMA_CTRL_CHAIN_LSB); }
INLINE void DMA_SetChain_hw(dma_channel_hw_t* hw, int chain) { RegMask(&hw->al1_ctrl, chain << DMA_CTRL_CHAIN_LSB, 0x0f << DMA_CTRL_CHAIN_LSB); }

// get chain
INLINE int DMA_Chain(int dma) { return (*DMA_CTRL(dma) >> DMA_CTRL_CHAIN_LSB) & 0x0f; }
INLINE int DMA_Chain_hw(dma_channel_hw_t* hw) { return (hw->al1_ctrl >> DMA_CTRL_CHAIN_LSB) & 0x0f; }

// get DMA sniff control register
INLINE u32 DMA_SniffCtrl(void) { return *DMA_SNIFF_CTRL; }

// set DMA sniff control register
INLINE void DMA_SetSniffCtrl(u32 ctrl) { *DMA_SNIFF_CTRL = ctrl; }

// get sniff control data
INLINE u32 DMA_SniffData(void) { return *DMA_SNIFF_DATA; }

// set sniff control data
INLINE void DMA_SetSniffData(u32 data) { *DMA_SNIFF_DATA = data; }

// start multiply DMA transfers with mask (bit 0..11 = channels set to 1)
INLINE void DMA_StartMask(u32 mask) { *DMA_MULTRIG = mask; }

// start DMA transfer of channel 0..11
INLINE void DMA_Start(int dma) { DMA_StartMask(BIT(dma)); }

// wait DMA for completion
INLINE void DMA_Wait(int dma) { cb(); while (DMA_IsBusy(dma)) {} cb(); }
INLINE void DMA_Wait_hw(const dma_channel_hw_t* hw) { cb(); while (DMA_IsBusy_hw(hw)) {} cb(); }

// abort DMA transfer
void DMA_Abort(int dma);

// === DMA interrupt

// enable interrupt from DMA channels for IRQ_DMA_0..IRQ_DMA_3 masked (set bits 0..15 to enable channel 0..15)
// To use DMA mask in range (first..last), use function RangeMask.
INLINE void DMA_IRQEnableMask(int irq, u32 mask) { RegSet(&dma_hw->irq_ctrl[irq].inte, mask); }
INLINE void DMA_IRQ0EnableMask(u32 mask) { RegSet(&dma_hw->inte0, mask); }
INLINE void DMA_IRQ1EnableMask(u32 mask) { RegSet(&dma_hw->inte1, mask); }
#if !RP2040
INLINE void DMA_IRQ2EnableMask(u32 mask) { RegSet(&dma_hw->inte2, mask); }
INLINE void DMA_IRQ3EnableMask(u32 mask) { RegSet(&dma_hw->inte3, mask); }
#endif

// enable interrupt from DMA channel for IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQEnable(int irq, int dma) { DMA_IRQEnableMask(irq, BIT(dma)); }
INLINE void DMA_IRQ0Enable(int dma) { DMA_IRQ0EnableMask(BIT(dma)); }
INLINE void DMA_IRQ1Enable(int dma) { DMA_IRQ1EnableMask(BIT(dma)); }
#if !RP2040
INLINE void DMA_IRQ2Enable(int dma) { DMA_IRQ2EnableMask(BIT(dma)); }
INLINE void DMA_IRQ3Enable(int dma) { DMA_IRQ3EnableMask(BIT(dma)); }
#endif

// disable interrupt from DMA channels for IRQ_DMA_0..IRQ_DMA_3 masked (set bits 0..15 to disable channel 0..15)
// To use DMA mask in range (first..last), use function RangeMask.
INLINE void DMA_IRQDisableMask(int irq, u32 mask) { RegClr(&dma_hw->irq_ctrl[irq].inte, mask); }
INLINE void DMA_IRQ0DisableMask(u32 mask) { RegClr(&dma_hw->inte0, mask); }
INLINE void DMA_IRQ1DisableMask(u32 mask) { RegClr(&dma_hw->inte1, mask); }
#if !RP2040
INLINE void DMA_IRQ2DisableMask(u32 mask) { RegClr(&dma_hw->inte2, mask); }
INLINE void DMA_IRQ3DisableMask(u32 mask) { RegClr(&dma_hw->inte3, mask); }
#endif

// disable interrupt from DMA channel for IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQDisable(int irq, int dma) { DMA_IRQDisableMask(irq, BIT(dma)); }
INLINE void DMA_IRQ0Disable(int dma) { DMA_IRQ0DisableMask(BIT(dma)); }
INLINE void DMA_IRQ1Disable(int dma) { DMA_IRQ1DisableMask(BIT(dma)); }
#if !RP2040
INLINE void DMA_IRQ2Disable(int dma) { DMA_IRQ2DisableMask(BIT(dma)); }
INLINE void DMA_IRQ3Disable(int dma) { DMA_IRQ3DisableMask(BIT(dma)); }
#endif

// check if DMA interrupt is enabled for IRQ_DMA_0..IRQ_DMA_3
INLINE Bool DMA_IRQIsEnabled(int irq, int dma) { return (dma_hw->irq_ctrl[irq].inte & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ0IsEnabled(int dma) { return (dma_hw->inte0 & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ1IsEnabled(int dma) { return (dma_hw->inte1 & BIT(dma)) != 0; }
#if !RP2040
INLINE Bool DMA_IRQ2IsEnabled(int dma) { return (dma_hw->inte2 & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ3IsEnabled(int dma) { return (dma_hw->inte3 & BIT(dma)) != 0; }
#endif

// check if DMA RAW interrupt request is pending (without enable and force)
INLINE Bool DMA_IRQRawIsPending(int dma) { return (dma_hw->intr & BIT(dma)) != 0; }

// check if DMA interrupt request IRQ_DMA_0..IRQ_DMA_3 is pending (after enable and force)
INLINE Bool DMA_IRQIsPending(int irq, int dma) { return (dma_hw->irq_ctrl[irq].ints & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ0IsPending(int dma) { return (dma_hw->ints0 & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ1IsPending(int dma) { return (dma_hw->ints1 & BIT(dma)) != 0; }
#if !RP2040
INLINE Bool DMA_IRQ2IsPending(int dma) { return (dma_hw->ints2 & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ3IsPending(int dma) { return (dma_hw->ints3 & BIT(dma)) != 0; }
#endif

// clear DMA interrupt request IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQClear(int irq, int dma) { dma_hw->irq_ctrl[irq].ints = BIT(dma); cb(); }
INLINE void DMA_IRQ0Clear(int dma) { dma_hw->ints0 = BIT(dma); cb(); }
INLINE void DMA_IRQ1Clear(int dma) { dma_hw->ints1 = BIT(dma); cb(); }
#if !RP2040
INLINE void DMA_IRQ2Clear(int dma) { dma_hw->ints2 = BIT(dma); cb(); }
INLINE void DMA_IRQ3Clear(int dma) { dma_hw->ints3 = BIT(dma); cb(); }
#endif

// force interrupt from DMA channel for IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQForce(int irq, int dma) { RegSet(&dma_hw->irq_ctrl[irq].intf, BIT(dma)); }
INLINE void DMA_IRQ0Force(int dma) { RegSet(&dma_hw->intf0, BIT(dma)); }
INLINE void DMA_IRQ1Force(int dma) { RegSet(&dma_hw->intf1, BIT(dma)); }
#if !RP2040
INLINE void DMA_IRQ2Force(int dma) { RegSet(&dma_hw->intf2, BIT(dma)); }
INLINE void DMA_IRQ3Force(int dma) { RegSet(&dma_hw->intf3, BIT(dma)); }
#endif

// unforce interrupt from DMA channel for IRQ_DMA_0..IRQ_DMA_3
INLINE void DMA_IRQUnforce(int irq, int dma) { RegClr(&dma_hw->irq_ctrl[irq].intf, BIT(dma)); }
INLINE void DMA_IRQ0Unforce(int dma) { RegClr(&dma_hw->intf0, BIT(dma)); }
INLINE void DMA_IRQ1Unforce(int dma) { RegClr(&dma_hw->intf1, BIT(dma)); }
#if !RP2040
INLINE void DMA_IRQ2Unforce(int dma) { RegClr(&dma_hw->intf2, BIT(dma)); }
INLINE void DMA_IRQ3Unforce(int dma) { RegClr(&dma_hw->intf3, BIT(dma)); }
#endif

// check if DMA interrupt is forced for IRQ_DMA_0..IRQ_DMA_3
INLINE Bool DMA_IRQIsForced(int irq, int dma) { return (dma_hw->irq_ctrl[irq].intf & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ0IsForced(int dma) { return (dma_hw->intf0 & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ1IsForced(int dma) { return (dma_hw->intf1 & BIT(dma)) != 0; }
#if !RP2040
INLINE Bool DMA_IRQ2IsForced(int dma) { return (dma_hw->intf2 & BIT(dma)) != 0; }
INLINE Bool DMA_IRQ3IsForced(int dma) { return (dma_hw->intf3 & BIT(dma)) != 0; }
#endif

// === DMA transfers

// perform 32-bit DMA transfer from memory to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u32
//   src = pointer to first source u32
//   count = number of u32 elements to transfer
// Transfer speed: 2 us per 1 KB
void DMA_MemCopy32(int dma, u32* dst, const u32* src, int count);

// perform 16-bit DMA transfer from memory to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u16
//   src = pointer to first source u16
//   count = number of u16 elements to transfer
// Transfer speed: 4 us per 1 KB
void DMA_MemCopy16(int dma, u16* dst, const u16* src, int count);

// perform 8-bit DMA transfer from memory to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u8
//   src = pointer to first source u8
//   count = number of u8 elements to transfer
// Transfer speed: 8 us per 1 KB
void DMA_MemCopy8(int dma, u8* dst, const u8* src, int count);

// perform optimised DMA transfer from memory to memory, waiting for completion
//   dst = pointer to destination
//   src = pointer to source
//   num = number of bytes
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 32-bit aligned 2 us per 1 KB, 16-bit aligned 4 us per 1 KB,
//  unaligned 8 us per 1 KB (software loop 11 us, memcpy 32-bit aligned 7 us, memcpy unaligned 55 us)
void DMA_MemCopy(volatile void* dst, const volatile void* src, int num);

// fill memory with 32-bit sample using 32-bit DMA, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u32 (must be aligned to u32)
//   data = pointer to 32-bit sample to fill (must be aligned to u32)
//   count = number of u32 elements to fill
// Transfer speed: 2 us per 1 KB
void DMA_MemFill32(int dma, u32* dst, const volatile u32* data, int count);

// fill memory with 16-bit sample using 16-bit DMA, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u16 (must be aligned to u16)
//   data = pointer to 16-bit sample to fill (must be aligned to u16)
//   count = number of u16 elements to fill
// Transfer speed: 4 us per 1 KB
void DMA_MemFill16(int dma, u16* dst, const volatile u16* data, int count);

// fill memory with 8-bit sample using 8-bit DMA, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u8
//   data = pointer to 8-bit sample to fill
//   count = number of u8 elements to fill
// Transfer speed: 8 us per 1 KB
void DMA_MemFill8(int dma, u8* dst, const volatile u8* data, int count);

// fill memory with 32-bit double word sample using optimised DMA transfer, waiting for completion
//   dst = pointer to destination
//   data = 32-bit sample to fill
//   num = number of bytes (last sample may be cropped)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 2 us per 1 KB
void DMA_MemFillDW(volatile void* dst, u32 data, int num);

// fill memory with 16-bit word sample using optimised DMA transfer, waiting for completion
//   dst = pointer to destination
//   data = 16-bit sample to fill
//   num = number of bytes (last sample may be cropped)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 2 us per 1 KB
void DMA_MemFillW(volatile void* dst, u16 data, int num);

// fill memory with 8-bit byte sample using optimised DMA transfer, waiting for completion
//   dst = pointer to destination
//   data = 8-bit sample to fill
//   num = number of bytes
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 2 us per 1 KB
void DMA_MemFill(volatile void* dst, u8 data, int num);

// DMA send data from memory to port, not waiting for completion (wait with DMA_Wait function)
//   chan = DMA channel 0..11 or 0..15
//   port = pointer to u32 port
//   src = pointer to first source u32
//   count = number of u32 elements to transfer
//   dreq = data request channel of port DREQ_*
void DMA_ToPort(int dma, volatile u32* port, const u32* src, int count, int dreq);

// DMA receive data from port to memory, not waiting for completion (wait with DMA_Wait function)
//   chan = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u32
//   port = pointer to u32 port
//   count = number of u32 elements to transfer
//   dreq = data request channel of port DREQ_*
void DMA_FromPort(int dma, u32* dst, const volatile u32* port, int count, int dreq);

// calculate optimised CRC checksum using DMA (wait for completion)
//   mode = CRC mode DMA_CRC_* (can include OR-ed flags DMA_CRC_INV and DMA_CRC_REV)
//   init = init data
//   dma = DMA channel 0..11  or 0..15
//   data = pointer to data
//   num = number of bytes
// Calculation speed: 2 us per 1 KB
u32 DMA_CRC(int mode, u32 init, int dma, const void* data, int num);

// calculate checksum using DMA, on aligned data (wait for completion)
//   mode = CRC mode DMA_CRC_SUM or other (can include OR-ed flags DMA_CRC_INV and DMA_CRC_REV)
//   init = init data
//   dma = DMA channel 0..11 or 0..15
//   data = pointer to data (must be aligned to the u8/u16/u32 entry)
//   num = number of u8/u16/u32 entries
//   size = size of one entry DMA_SIZE_* (u8/u16/u32)
// Calculation speed: 32-bit 2 us per 1 KB, 16-bit 4 us per 1 KB, 8-bit 8 us per 1 KB
u32 DMA_SUM(int mode, u32 init, int dma, const void* data, int num, int size);

// set DMA fractional timer (trigger on rate X/Y*sys_clk)
//   timer = index of timer 0..3
//   x = numerator (0 = timer is OFF)
//   y = denominator
INLINE void DMA_SetTimer(int timer, int x, int y) { *DMA_TIMER(timer) = ((u32)x << 16) | y; }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

#define NUM_DMA_CHANNELS	DMA_CHANNELS	// number of DMA channels
#define NUM_DMA_TIMERS		DMA_TIMERS	// number of DMA timers

#define DREQ_DMA_TIMER0 DMA_CH0_CTRL_TRIG_TREQ_SEL_VALUE_TIMER0
#define DREQ_DMA_TIMER1 DMA_CH0_CTRL_TRIG_TREQ_SEL_VALUE_TIMER1
#define DREQ_DMA_TIMER2 DMA_CH0_CTRL_TRIG_TREQ_SEL_VALUE_TIMER2
#define DREQ_DMA_TIMER3 DMA_CH0_CTRL_TRIG_TREQ_SEL_VALUE_TIMER3

// DMA channel config of control register
typedef struct {
	u32	ctrl;
} dma_channel_config;

STATIC_ASSERT(sizeof(dma_channel_config) == 4, "Incorrect dma_channel_config!");

// get address of DMA channel registers
INLINE dma_channel_hw_t *dma_channel_hw_addr(uint chan) { return DMA_GetHw(chan); }

// Mark a dma channel as used
INLINE void dma_channel_claim(uint chan) { DMA_Claim(chan); }

// Mark multiple dma channels as used
INLINE void dma_claim_mask(u32 mask) { DMA_ClaimMask(mask); }

//Mark a dma channel as no longer used
INLINE void dma_channel_unclaim(uint chan) { DMA_Unclaim(chan); }

// Mark multiple dma channels as no longer used
INLINE void dma_unclaim_mask(u32 mask) { DMA_UnclaimMask(mask); }

// Claim a free dma channel (returns -1 on error)
INLINE int dma_claim_unused_channel(bool req) { return DMA_ClaimFree(); }

// Determine if a dma channel is claimed
INLINE bool dma_channel_is_claimed(uint chan) { return (bool)DMA_IsClaimed(chan); }

// Set DMA channel read increment in a channel configuration object
INLINE void channel_config_set_read_increment(dma_channel_config *c, bool incr) { DMA_CfgReadInc(&c->ctrl, incr); }

// Set DMA channel write increment in a channel configuration object
INLINE void channel_config_set_write_increment(dma_channel_config *c, bool incr) { DMA_CfgWriteInc(&c->ctrl, incr); }

// Select a transfer request signal in a channel configuration object DREQ_* (DREQ_FORCE = permanent)
INLINE void channel_config_set_dreq(dma_channel_config *c, uint dreq) { DMA_CfgDreq(&c->ctrl, dreq); }

// Set DMA channel chain_to channel in a channel configuration object (to disable, set to the same channel number = default)
INLINE void channel_config_set_chain_to(dma_channel_config *c, uint chain_to) { DMA_CfgChain(&c->ctrl, chain_to); }

// Set the size of each DMA bus transfer in a channel configuration object DMA_SIZE_*
INLINE void channel_config_set_transfer_data_size(dma_channel_config *c, uint size) { DMA_CfgSize(&c->ctrl, size); }

// Set address wrapping parameters in a channel configuration object
//  write ... True to apply ring to write address, False to apply ring to read address
//  size ... order of ring size 1..15 (0=disabled, 1=2 byte, 2=4 bytes, 3=8 bytes ... 15=32 KB)
INLINE void channel_config_set_ring(dma_channel_config *c, bool write, uint size) { DMA_CfgRing(&c->ctrl, write, size); }

// Set DMA byte swapping config in a channel configuration object
INLINE void channel_config_set_bswap(dma_channel_config *c, bool bswap) { DMA_CfgBSwap(&c->ctrl, bswap); }

// Set IRQ quiet mode in a channel configuration object
INLINE void channel_config_set_irq_quiet(dma_channel_config *c, bool quiet) { DMA_CfgQuiet(&c->ctrl, quiet); }

// Set the channel priority in a channel configuration object
INLINE void channel_config_set_high_priority(dma_channel_config *c, bool high) { DMA_CfgHighPrior(&c->ctrl, high); }

// Enable/Disable the DMA channel in a channel configuration object
INLINE void channel_config_set_enable(dma_channel_config *c, bool enable) { DMA_CfgEnable(&c->ctrl, enable); }

// Enable access to channel by sniff hardware in a channel configuration object
INLINE void channel_config_set_sniff_enable(dma_channel_config *c, bool sniff) { DMA_CfgSniff(&c->ctrl, sniff); }

// Get the default channel configuration for a given channel
/* Setting		Default
 * ----------------------------
 * Read Increment	true
 * Write Increment	false
 * DReq			DREQ_FORCE
 * Chain to		self
 * Data size		DMA_SIZE_32
 * Ring			write=false, size=0 (i.e. off)
 * Byte Swap		false
 * Quiet IRQs		false
 * High Priority	false
 * Channel Enable	true
 * Sniff Enable		false
*/
INLINE dma_channel_config dma_channel_get_default_config(uint chan)
	{ dma_channel_config c = { DMA_CfgDef(chan) }; return c; }

// Get the current configuration for the specified channel.
INLINE dma_channel_config dma_get_channel_config(uint chan)
	{ dma_channel_config c = { DMA_GetCtrl(chan) }; return c; }

// Get the raw configuration register from a channel configuration
INLINE u32 channel_config_get_ctrl_value(const dma_channel_config *config) { return config->ctrl; }

// Set a channel configuration
INLINE void dma_channel_set_config(uint chan, const dma_channel_config *config, bool trigger)
	{ if (trigger) DMA_SetCtrlTrig(chan, config->ctrl); else DMA_SetCtrl(chan, config->ctrl); }

// Set the DMA initial read address.
INLINE void dma_channel_set_read_addr(uint chan, const volatile void *addr, bool trigger)
	{ if (trigger) DMA_SetReadTrig(chan, addr); else DMA_SetRead(chan, addr); }

// Set the DMA initial write address
INLINE void dma_channel_set_write_addr(uint chan, volatile void *addr, bool trigger)
	{ if (trigger) DMA_SetWriteTrig(chan, addr); else DMA_SetWrite(chan, addr); }

// Set the number of bus transfers the channel will do
INLINE void dma_channel_set_trans_count(uint chan, uint32_t count, bool trigger)
	{ if (trigger) DMA_SetCountTrig(chan, count); else DMA_SetCount(chan, count); }

// Configure all DMA parameters and optionally start transfer
//   chan ... DMA channel
//   config ... Pointer to DMA config structure
//   write_addr ... Initial write address
//   read_addr ... Initial read address
//   count ... Number of transfers to perform
//   trigger ... True to start the transfer immediately
INLINE void dma_channel_configure(uint chan, const dma_channel_config *config,
	volatile void *write_addr, const volatile void *read_addr, uint count, bool trigger)
{
	dma_channel_set_read_addr(chan, read_addr, false);
	dma_channel_set_write_addr(chan, write_addr, false);
	dma_channel_set_trans_count(chan, count, false);
	dma_channel_set_config(chan, config, trigger);
}

// Start a DMA transfer from a buffer immediately
//   channel ... DMA channel
//   read_addr ... Sets the initial read address
//   transfer_count ... Number of transfers to make. Not bytes, but the number of transfers of channel_config_set_transfer_data_size() to be sent.
INLINE void __attribute__((always_inline)) dma_channel_transfer_from_buffer_now(uint channel,
	const volatile void *read_addr, u32 transfer_count)
{
	dma_channel_hw_t *hw = dma_channel_hw_addr(channel);
	hw->read_addr = (u32)read_addr;
	hw->al1_transfer_count_trig = transfer_count;
}

// Start a DMA transfer to a buffer immediately
//   channel ... DMA channel
//   write_addr ... Sets the initial write address
//   transfer_count ... Number of transfers to make. Not bytes, but the number of transfers of channel_config_set_transfer_data_size() to be sent.
INLINE void dma_channel_transfer_to_buffer_now(uint channel, volatile void *write_addr, u32 transfer_count)
{
	dma_channel_hw_t *hw = dma_channel_hw_addr(channel);
	hw->write_addr = (u32)write_addr;
	hw->al1_transfer_count_trig = transfer_count;
}

// Start one or more channels simultaneously
INLINE void dma_start_channel_mask(u32 mask) { DMA_StartMask(mask); }

// Start a single DMA channel
INLINE void dma_channel_start(uint chan) { DMA_Start(chan); }

// Stop a DMA transfer
//  The calling code should be sure to ignore a completion IRQ as a result of this method.
//
//  // disable the channel on IRQ0
//  dma_channel_set_irq0_enabled(channel, false);
//  // abort the channel
//  dma_channel_abort(channel);
//  // clear the spurious IRQ (if there was one)
//  dma_channel_acknowledge_irq0(channel);
//  // re-enable the channel on IRQ0
//  dma_channel_set_irq0_enabled(channel, true);
INLINE void dma_channel_abort(uint chan) { DMA_Abort(chan); }

// Enable single DMA channel's interrupt via DMA_IRQ_0
INLINE void dma_channel_set_irq0_enabled(uint chan, bool enabled)
	{ if (enabled) DMA_IRQ0Enable(chan); else DMA_IRQ0Disable(chan); }

// Enable multiple DMA channels' interrupts via DMA_IRQ_0
INLINE void dma_set_irq0_channel_mask_enabled(u32 mask, bool enabled)
	{ if (enabled) DMA_IRQ0EnableMask(mask); else DMA_IRQ0DisableMask(mask); }

// Enable single DMA channel's interrupt via DMA_IRQ_1
INLINE void dma_channel_set_irq1_enabled(uint chan, bool enabled)
	{ if (enabled) DMA_IRQ1Enable(chan); else DMA_IRQ1Disable(chan); }

// Enable multiple DMA channels' interrupts via DMA_IRQ_1
INLINE void dma_set_irq1_channel_mask_enabled(u32 mask, bool enabled)
	{ if (enabled) DMA_IRQ1EnableMask(mask); else DMA_IRQ1DisableMask(mask); }

// Enable single DMA channel interrupt on either DMA_IRQ_0 or DMA_IRQ_1
INLINE void dma_irqn_set_channel_enabled(uint irq_index, uint chan, bool enabled)
{
	if (irq_index)
		dma_channel_set_irq1_enabled(chan, enabled);
	else
	        dma_channel_set_irq0_enabled(chan, enabled);
}

// Enable multiple DMA channels' interrupt via either DMA_IRQ_0 or DMA_IRQ_1
INLINE void dma_irqn_set_channel_mask_enabled(uint irq_index, u32 channel_mask, bool enabled)
{
	if (irq_index)
		dma_set_irq1_channel_mask_enabled(channel_mask, enabled);
	else
		dma_set_irq0_channel_mask_enabled(channel_mask, enabled);
}

// Determine if a particular channel is a cause of DMA_IRQ_0
INLINE bool dma_channel_get_irq0_status(uint chan) { return dma_hw->ints0 & BIT(chan); }

// Determine if a particular channel is a cause of DMA_IRQ_1
INLINE bool dma_channel_get_irq1_status(uint chan) { return dma_hw->ints1 & BIT(chan); }

// Determine if a particular channel is a cause of DMA_IRQ_N
INLINE bool dma_irqn_get_channel_status(uint irq_index, uint chan)
	{ return (irq_index ? dma_hw->ints1 : dma_hw->ints0) & BIT(chan); }

// Acknowledge a channel IRQ, resetting it as the cause of DMA_IRQ_0
INLINE void dma_channel_acknowledge_irq0(uint chan) { DMA_IRQ0Clear(chan); }

// Acknowledge a channel IRQ, resetting it as the cause of DMA_IRQ_1
INLINE void dma_channel_acknowledge_irq1(uint chan) { DMA_IRQ1Clear(chan); }

// Acknowledge a channel IRQ, resetting it as the cause of DMA_IRQ_N
INLINE void dma_irqn_acknowledge_channel(uint irq_index, uint chan)
	{ if (irq_index) DMA_IRQ1Clear(chan); else DMA_IRQ0Clear(chan); }

// Check if DMA channel is busy
INLINE bool dma_channel_is_busy(uint chan) { return DMA_IsBusy(chan); }

// Wait for a DMA channel transfer to complete
INLINE void dma_channel_wait_for_finish_blocking(uint chan) { DMA_Wait(chan); }

// Enable the DMA sniffing targeting the specified channel
//   channel ... channel
//   mode ... 0=CRC32, 1=CRC32rev, 2=CRC16, 3=CRC16rev, 14=XOR, 15=SUM
//   force ... Set true to also turn on sniffing in the channel configuration
INLINE void dma_sniffer_enable(uint channel, uint mode, bool force)
{
	if (force) hw_set_bits(&dma_hw->ch[channel].al1_ctrl, DMA_CH0_CTRL_TRIG_SNIFF_EN_BITS);

	hw_write_masked(&dma_hw->sniff_ctrl,
		(((channel << DMA_SNIFF_CTRL_DMACH_LSB) & DMA_SNIFF_CTRL_DMACH_BITS) |
		((mode << DMA_SNIFF_CTRL_CALC_LSB) & DMA_SNIFF_CTRL_CALC_BITS) |
		DMA_SNIFF_CTRL_EN_BITS),
		(DMA_SNIFF_CTRL_DMACH_BITS |
		DMA_SNIFF_CTRL_CALC_BITS |
		DMA_SNIFF_CTRL_EN_BITS));
}

// Enable the Sniffer byte swap function
INLINE void dma_sniffer_set_byte_swap_enabled(bool swap)
{
	if (swap)
		hw_set_bits(&dma_hw->sniff_ctrl, DMA_SNIFF_CTRL_BSWAP_BITS);
	else
		hw_clear_bits(&dma_hw->sniff_ctrl, DMA_SNIFF_CTRL_BSWAP_BITS);
}

// Enable the Sniffer output invert function
INLINE void dma_sniffer_set_output_invert_enabled(bool invert)
{
	if (invert)
		hw_set_bits(&dma_hw->sniff_ctrl, DMA_SNIFF_CTRL_OUT_INV_BITS);
	else
		hw_clear_bits(&dma_hw->sniff_ctrl, DMA_SNIFF_CTRL_OUT_INV_BITS);
}

// Enable the Sniffer output bit reversal function
INLINE void dma_sniffer_set_output_reverse_enabled(bool reverse)
{
	if (reverse)
		hw_set_bits(&dma_hw->sniff_ctrl, DMA_SNIFF_CTRL_OUT_REV_BITS);
	else
		hw_clear_bits(&dma_hw->sniff_ctrl, DMA_SNIFF_CTRL_OUT_REV_BITS);
}

// Disable the DMA sniffer
INLINE void dma_sniffer_disable(void) { dma_hw->sniff_ctrl = 0; }

// Set the sniffer's data accumulator with initial value (normally 0xFFFF or 0xFFFFFFFF)
INLINE void dma_sniffer_set_data_accumulator(uint32_t seed_value) { dma_hw->sniff_data = seed_value; }

// Get the sniffer's data accumulator value
INLINE u32 dma_sniffer_get_data_accumulator(void) { return dma_hw->sniff_data; }

// Mark a dma timer as used
INLINE void dma_timer_claim(uint timer) { DMA_TimerClaim(timer); }

// Mark a dma timer as no longer used
INLINE void dma_timer_unclaim(uint timer) { DMA_TimerUnclaim(timer); }

// Claim a free dma timer (returns -1 on error)
INLINE int dma_claim_unused_timer(bool required) { return DMA_TimerClaimFree(); }

// Determine if a dma timer is claimed
INLINE bool dma_timer_is_claimed(uint timer) { return DMA_TimerIsClaimed(timer); }

// Set the divider for the given DMA timer (trigger on rate numerator/denominator*sys_clk)
INLINE void dma_timer_set_fraction(uint timer, u16 numerator, u16 denominator) { DMA_SetTimer(timer, numerator, denominator); }

// Return the DREQ number for a given DMA timer
INLINE uint dma_get_timer_dreq(uint timer) { return DREQ_DMA_TIMER0 + timer; }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_DMA_H

#endif // USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
