
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

#include "../../global.h"	// globals

#if USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)

#include "../inc/sdk_cpu.h"
#include "../inc/sdk_dma.h"

// claimed DMA channels
u16 DmaClaimed = 0;		// claimed DMA channels
u8 DmaTimerClaimed = 0;		// claimed DMA timers

// claim free unused DMA channel (returns index of channel. of -1 on error)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
s8 DMA_ClaimFree(void)
{
	int inx, mask;
	mask = 1;
	for (inx = 0; inx < DMA_CHANNELS; inx++)
	{
		// check if DMA channel is already claimed
		if ((DmaClaimed & mask) == 0)
		{
			// claim this DMA channel
			DmaClaimed |= mask;
			return inx;
		}

		// shift to next DMA channel
		mask <<= 1;
	}

	// no free DMA channel	
	return -1;
}

// claim free unused DMA timer (returns index of timer, -1 on error)
//  This function is not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)
s8 DMA_TimerClaimFree(void)
{
	int inx, mask;
	mask = 1;
	for (inx = 0; inx < DMA_TIMERS; inx++)
	{
		// check if DMA timer is already claimed
		if ((DmaTimerClaimed & mask) == 0)
		{
			// claim this DMA timer
			DmaTimerClaimed |= mask;
			return inx;
		}

		// shift to next DMA timer
		mask <<= 1;
	}

	// no free DMA timer	
	return -1;
}

// set DMA config, without trigger
//  dma = channel 0..11 (or 0..15)
//  src = source address
//  dst = destination address
//  count = number of transfers (can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS)
//  ctrl = control word (can be prepared using DMA_Cfg* functions)
void DMA_Config(int dma, const volatile void* src, volatile void* dst, u32 count, u32 ctrl)
{
	DMA_SetRead(dma, src);
	DMA_SetWrite(dma, dst);
	DMA_SetCount(dma, count);
	DMA_SetCtrl(dma, ctrl);
}

void DMA_Config_hw(dma_channel_hw_t* hw, const volatile void* src, volatile void* dst, u32 count, u32 ctrl)
{
	DMA_SetRead_hw(hw, src);
	DMA_SetWrite_hw(hw, dst);
	DMA_SetCount_hw(hw, count);
	DMA_SetCtrl_hw(hw, ctrl);
}

// set DMA config, with trigger
//  dma = channel 0..11 (or 0..15)
//  src = source address
//  dst = destination address
//  count = number of transfers (can be combined with flags DMA_COUNT_TRIGGER or DMA_COUNT_ENDLESS)
//  ctrl = control word (can be prepared using DMA_Cfg* functions)
void DMA_ConfigTrig(int dma, const volatile void* src, volatile void* dst, u32 count, u32 ctrl)
{
	DMA_SetRead(dma, src);
	DMA_SetWrite(dma, dst);
	DMA_SetCount(dma, count);
	cb(); // compiler barrier
	DMA_SetCtrlTrig(dma, ctrl);
}

void DMA_ConfigTrig_hw(dma_channel_hw_t* hw, const volatile void* src, volatile void* dst, u32 count, u32 ctrl)
{
	DMA_SetRead_hw(hw, src);
	DMA_SetWrite_hw(hw, dst);
	DMA_SetCount_hw(hw, count);
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, ctrl);
}

// abort DMA transfer
void DMA_Abort(int dma)
{
	int i;

	// prepare address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// disable channel
	Bool chan_en = DMA_IsEnabled_hw(hw);
	DMA_Disable_hw(hw);

	// cancel chain
	int chain = DMA_Chain_hw(hw);
	DMA_SetChain_hw(hw, dma);

	// disable interrupts
	Bool irq_en[DMA_IRQ_NUM];
	for (i = 0; i < DMA_IRQ_NUM; i++)
	{
		irq_en[i] = DMA_IRQIsEnabled(i, dma);
		DMA_IRQDisable(i, dma);
	}

	// lock interrupt
	IRQ_LOCK;
	dmb();

// Interrupts must be disabled - an interrupt could come from the DMA after aborting
// the transfer and the waiting loop would not finish.

	// abort channel
	dma_hw->abort = BIT(dma); // request to abort transfers

	// wait to finish
	DMA_Wait_hw(hw); // wait to finish all transfers from FIFO

	// clear interrupt status
	for (i = 0; i < DMA_IRQ_NUM; i++) DMA_IRQClear(i, dma);

	// clear errors
	DMA_ClearError_hw(hw);

	// unlock interrupt
	dmb();
	IRQ_UNLOCK;

	// re-enable interrupts
	for (i = 0; i < DMA_IRQ_NUM; i++)
	{
		if (irq_en[i]) DMA_IRQEnable(i, dma);
	}

	// restore chain
	DMA_SetChain_hw(hw, chain);

	// re-enable channel
	if (chan_en) DMA_Enable_hw(hw);
}

// perform 32-bit DMA transfer from memory to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u32 (must be aligned to u32)
//   src = pointer to first source u32 (must be aligned to u32)
//   count = number of u32 elements to transfer
// Transfer speed: 2 us per 1 KB
void DMA_MemCopy32(int dma, u32* dst, const u32* src, int count)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, src); // set source address
	DMA_SetWrite_hw(hw, dst); // set destination address
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw,  // set control and trigger transfer
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
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		DMA_CTRL_INC_READ |	// increment read
		DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// perform 16-bit DMA transfer from memory to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u16 (must be aligned to u16)
//   src = pointer to first source u16 (must be aligned to u16)
//   count = number of u16 elements to transfer
// Transfer speed: 4 us per 1 KB
void DMA_MemCopy16(int dma, u16* dst, const u16* src, int count)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, src); // set source address
	DMA_SetWrite_hw(hw, dst); // set destination address
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, // set control and trigger transfer
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
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		DMA_CTRL_INC_READ |	// increment read
		DMA_CTRL_SIZE(DMA_SIZE_16) | // data size 16 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// perform 8-bit DMA transfer from memory to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u8
//   src = pointer to first source u8
//   count = number of u8 elements to transfer
// Transfer speed: 8 us per 1 KB
void DMA_MemCopy8(int dma, u8* dst, const u8* src, int count)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, src); // set source address
	DMA_SetWrite_hw(hw, dst); // set destination address
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, // set control and trigger transfer
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
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		DMA_CTRL_INC_READ |	// increment read
		DMA_CTRL_SIZE(DMA_SIZE_8) | // data size 8 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// perform optimised DMA transfer from memory to memory, waiting for completion
//   dst = pointer to destination
//   src = pointer to source
//   num = number of bytes
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 32-bit aligned 2 us per 1 KB, 16-bit aligned 4 us per 1 KB,
//  unaligned 8 us per 1 KB (software loop 11 us, memcpy 32-bit aligned 7 us, memcpy unaligned 55 us)
void DMA_MemCopy(volatile void* dst, const volatile void* src, int num)
{
	int num2;
	u8* dst8 = (u8*)dst;
	const u8* src8 = (const u8*)src;
	int dma = DMA_TEMP_CHAN(); // DMA temporary channel

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// copy small amount of data
	if (num < 64)
	{
		memcpy(dst8, src8, num);
	}

	// fast copy 32-bit DMA, addresses must be dword aligned
	else if (((u32)dst & 3) == ((u32)src & 3))
	{
		// copy first unaligned bytes ('num' is big enough here)
		while (((u32)dst8 & 3) != 0)
		{
			*dst8++ = *src8++;
			num--;
		}

		// copy 32-bit DMA
		num2 = num & 3; // rest in last dword
		num >>= 2; // number of dwords
		DMA_MemCopy32(dma, (u32*)dst8, (const u32*)src8, num);

		// wait DMA for completion
		DMA_Wait_hw(hw);

		// disable DMA channel
		DMA_Disable_hw(hw);

		// copy rest of data
		num <<= 2; // convert dwords back to bytes
		dst8 += num; // shift destination address
		src8 += num; // shift source address
		while (num2 > 0)
		{
			*dst8++ = *src8++;
			num2--;
		}
	}
	
	// copy 16-bit DMA, addresses must be word aligned
	else if (((u32)dst & 1) == ((u32)src & 1))
	{
		// copy first unaligned bytes ('num' is big enough here)
		if (((u32)dst8 & 1) != 0)
		{
			*dst8++ = *src8++;
			num--;
		}

		// copy 16-bit DMA
		num2 = num & 1; // rest in last word
		num >>= 1; // number of words
		DMA_MemCopy16(dma, (u16*)dst8, (const u16*)src8, num);

		// wait DMA for completion
		DMA_Wait_hw(hw);

		// disable DMA channel
		DMA_Disable_hw(hw);

		// copy rest of data
		num <<= 1; // convert words back to bytes
		dst8 += num; // shift destination address
		src8 += num; // shift source address
		if (num2 > 0) *dst8++ = *src8++;
	}

	// copy 8-bit DMA
	else
	{
		// copy 8-bit DMA
		DMA_MemCopy8(dma, dst8, src8, num);

		// wait DMA for completion
		DMA_Wait_hw(hw);

		// disable DMA channel
		DMA_Disable_hw(hw);
	}
}

// fill memory with 32-bit sample using 32-bit DMA, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u32 (must be aligned to u32)
//   data = pointer to 32-bit sample to fill (must be aligned to u32)
//   count = number of u32 elements to fill
// Transfer speed: 2 us per 1 KB
void DMA_MemFill32(int dma, u32* dst, const volatile u32* data, int count)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, data); // set source address
	DMA_SetWrite_hw(hw, dst); // set destination address
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, // set control and trigger transfer
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
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		// DMA_CTRL_INC_READ |	// NOT increment read
		DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// fill memory with 16-bit sample using 16-bit DMA, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u16 (must be aligned to u16)
//   data = pointer to 16-bit sample to fill (must be aligned to u16)
//   count = number of u16 elements to fill
// Transfer speed: 4 us per 1 KB
void DMA_MemFill16(int dma, u16* dst, const volatile u16* data, int count)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, data); // set source address
	DMA_SetWrite_hw(hw, dst); // set destination address
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, // set control and trigger transfer
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
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		// DMA_CTRL_INC_READ |	// NOT increment read
		DMA_CTRL_SIZE(DMA_SIZE_16) | // data size 16 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// fill memory with 8-bit sample using 8-bit DMA, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u8
//   data = pointer to 8-bit sample to fill
//   count = number of u8 elements to fill
// Transfer speed: 8 us per 1 KB
void DMA_MemFill8(int dma, u8* dst, const volatile u8* data, int count)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, data); // set source address
	DMA_SetWrite_hw(hw, dst); // set destination address
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, // set control and trigger transfer
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
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		// DMA_CTRL_INC_READ |	// NOT increment read
		DMA_CTRL_SIZE(DMA_SIZE_8) | // data size 8 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// fill memory with 32-bit double word sample using optimised DMA transfer, waiting for completion
//   dst = pointer to destination
//   data = 32-bit sample to fill
//   num = number of bytes (last sample may be cropped)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 2 us per 1 KB
void DMA_MemFillDW(volatile void* dst, u32 data, int num)
{
	int num2;
	u8* dst8 = (u8*)dst;
	int dma;

	// fill first unaligned bytes
	while (((u32)dst8 & 3) != 0)
	{
		if (num <= 0) return;
		*dst8++ = (u8)data;
		data = (data >> 8) | (data << 24);
		num--;
	}

	// get address of DMA channel registers
	dma = DMA_TEMP_CHAN(); // DMA temporary channel
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// fill 32-bit DMA
	if (num <= 0) return;
	num2 = num & 3; // rest in last dword
	num >>= 2; // number of dwords
	cb(); // compiler barrier (data must stay valid at this point)
	DMA_MemFill32(dma, (u32*)dst8, &data, num);

	// wait DMA for completion
	DMA_Wait_hw(hw);

	// compiler barrier (data must stay valid at this point)
	cb();

	// disable DMA channel
	DMA_Disable_hw(hw);

	// copy rest of data
	num <<= 2; // convert dwords back to bytes
	dst8 += num; // shift destination address
	while (num2 > 0)
	{
		*dst8++ = (u8)data;
		data = (data >> 8) | (data << 24);
		num2--;
	}
}

// fill memory with 16-bit word sample using optimised DMA transfer, waiting for completion
//   dst = pointer to destination
//   data = 16-bit sample to fill
//   num = number of bytes (last sample may be cropped)
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 2 us per 1 KB
void DMA_MemFillW(volatile void* dst, u16 data, int num)
{
	DMA_MemFillDW(dst, data | ((u32)data << 16), num);
}

// fill memory with 8-bit byte sample using optimised DMA transfer, waiting for completion
//   dst = pointer to destination
//   data = 8-bit sample to fill
//   num = number of bytes
// Can be used simultaneously in both CPUs, but not simultaneously in an interrupt.
// Transfer speed: 2 us per 1 KB
void DMA_MemFill(volatile void* dst, u8 data, int num)
{
	if (num < 32)
		// fill small amount of data
		memset((void*)dst, data, num);
	else
		// fast fill 32-bit DMA
		DMA_MemFillW(dst, data | ((u16)data << 8), num);
}

// DMA send data from memory to port, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   port = pointer to u32 port
//   src = pointer to first source u32 (must be aligned to u32)
//   count = number of u32 elements to transfer
//   dreq = data request channel of port DREQ_*
void DMA_ToPort(int dma, volatile u32* port, const u32* src, int count, int dreq)
{
	// abort current transfer
	DMA_Abort(dma);

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, src); // set source address (memory)
	DMA_SetWrite_hw(hw, port); // set destination address (port)
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, // set control and trigger transfer
		// DMA_CTRL_SNIFF |	// NOT sniff
		// DMA_CTRL_BSWAP |	// NOT byte swap
		// DMA_CTRL_QUIET |	// NOT quiet
		DMA_CTRL_TREQ(dreq) |	// data request
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
		DMA_CTRL_EN);		// enable DMA
}

// DMA receive data from port to memory, not waiting for completion (wait with DMA_Wait function)
//   dma = DMA channel 0..11 or 0..15
//   dst = pointer to first destination u32 (must be aligned to u32)
//   port = pointer to u32 port
//   count = number of u32 elements to transfer
//   dreq = data request channel of port DREQ_*
void DMA_FromPort(int dma, u32* dst, const volatile u32* port, int count, int dreq)
{
	// abort current transfer
	DMA_Abort(dma); // abort current transfer

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// setup DMA
//	DMA_ClearError_hw(hw); // clear errors
	DMA_SetRead_hw(hw, port); // set source address (port)
	DMA_SetWrite_hw(hw, dst); // set destination address (memory)
	DMA_SetCount_hw(hw, count); // set count of elements

	// start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw,
		// DMA_CTRL_SNIFF |	// NOT sniff
		// DMA_CTRL_BSWAP |	// NOT byte swap
		// DMA_CTRL_QUIET |	// NOT quiet
		DMA_CTRL_TREQ(dreq) |	// data request
		DMA_CTRL_CHAIN(dma) |	// chain disabled
		// DMA_CTRL_RING_WRITE | // NOT wrap ring on write
		DMA_CTRL_RING_SIZE(0) |	// NOT ring
#if !RP2040
		// DMA_CTRL_INC_WRITE_REV | // NOT decrement write
#endif
		DMA_CTRL_INC_WRITE |	// increment write
#if !RP2040
		// DMA_CTRL_INC_READ_REV | // NOT decrement read
#endif
		// DMA_CTRL_INC_READ |	// NOT increment read
		DMA_CTRL_SIZE(DMA_SIZE_32) | // data size 32 bits
		// DMA_CTRL_HIGH_PRIORITY | // NOT high priority
		DMA_CTRL_EN);		// enable DMA
}

// calculate optimised CRC checksum using DMA (wait for completion)
//   mode = CRC mode DMA_CRC_* (can include OR-ed flags DMA_CRC_INV and DMA_CRC_REV)
//   init = init data
//   dma = DMA channel 0..11 or 0..15
//   data = pointer to data
//   num = number of bytes
// Calculation speed: 2 us per 1 KB
u32 DMA_CRC(int mode, u32 init, int dma, const void* data, int num)
{
	int num2;

	// temporary destination
	u32 k = 0;

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// abort previous transfer
	DMA_Abort(dma); // abort current transfer
//	DMA_ClearError_hw(hw); // clear errors
	cb(); // compiler barrier

	// prepare sniff registers
	u32 sniff = DMA_SNIFF_EN | DMA_SNIFF_CHAN(dma) | DMA_SNIFF_CRC(mode & DMA_CRC_MASK);
	if ((mode & DMA_CRC_INV) != 0) sniff |= DMA_SNIFF_INV;
	if ((mode & DMA_CRC_REV) != 0) sniff |= DMA_SNIFF_REV;
	DMA_SetSniffCtrl(sniff); // set control word
	DMA_SetSniffData(init); // set init data

	// prepare DMA transfer
	DMA_SetRead_hw(hw, data); // set source address
	DMA_SetWrite_hw(hw, &k); // set destination address

	// prepare control word, without data size
	u32 ctrl = DMA_CTRL_SNIFF | DMA_CTRL_TREQ_FORCE | DMA_CTRL_CHAIN(dma) | DMA_CTRL_INC_READ | DMA_CTRL_EN;

	// accumulate first unaligned bytes
	if ((((u32)data & 3) != 0) && (num > 0))
	{
		// prepare number of bytes - rest up to 32-bit boundary
		num2 = 4 - ((u32)data & 3);
		if (num2 > num) num2 = num;
		num -= num2;

		// set count of bytes
		DMA_SetCount_hw(hw, num2);

		// set DMA control word and start transfer
		cb(); // compiler barrier
		DMA_SetCtrlTrig_hw(hw, ctrl | DMA_CTRL_SIZE(DMA_SIZE_8)); // set control and trigger transfer

		// wait DMA for completion
		DMA_Wait_hw(hw);
	}

	// accumulate aligned data
	num2 = num >> 2; // convert count to dwords
	if (num2 > 0)
	{
		// set count of dwords
		DMA_SetCount_hw(hw, num2);

		// set DMA control word and start transfer
		cb(); // compiler barrier
		DMA_SetCtrlTrig_hw(hw, ctrl | DMA_CTRL_SIZE(DMA_SIZE_32)); // set control and trigger transfer

		// wait DMA for completion
		DMA_Wait_hw(hw);
	}

	// accumulate rest of unaligned bytes
	num -= num2 << 2;
	if (num > 0)
	{
		// set count of bytes
		DMA_SetCount_hw(hw, num);

		// set DMA control word and start transfer
		cb(); // compiler barrier
		DMA_SetCtrlTrig_hw(hw, ctrl | DMA_CTRL_SIZE(DMA_SIZE_8)); // set control and trigger transfer

		// wait DMA for completion
		DMA_Wait_hw(hw);
	}

	// disable DMA channel
	DMA_Disable_hw(hw);

	// get result and deactivate sniff
	u32 res = DMA_SniffData();
	DMA_SetSniffCtrl(0); // set control word, disable sniff
	return res;
}

// calculate checksum using DMA, on aligned data (wait for completion)
//   mode = CRC mode DMA_CRC_SUM or other (can include OR-ed flags DMA_CRC_INV and DMA_CRC_REV)
//   init = init data
//   dma = DMA channel 0..11
//   data = pointer to data (must be aligned to the u8/u16/u32 entry)
//   num = number of u8/u16/u32 entries
//   size = size of one entry DMA_SIZE_* (u8/u16/u32)
// Calculation speed: 32-bit 2 us per 1 KB, 16-bit 4 us per 1 KB, 8-bit 8 us per 1 KB
u32 DMA_SUM(int mode, u32 init, int dma, const void* data, int num, int size)
{
	// temporary destination
	u32 k = 0;

	// get address of DMA channel registers
	dma_channel_hw_t* hw = DMA_GetHw(dma);

	// abort previous transfer
	DMA_Abort(dma); // abort current transfer
//	DMA_ClearError_hw(hw); // clear errors
	cb(); // compiler barrier

	// prepare sniff registers
	u32 sniff = DMA_SNIFF_EN | DMA_SNIFF_CHAN(dma) | DMA_SNIFF_CRC(mode & DMA_CRC_MASK);
	if ((mode & DMA_CRC_INV) != 0) sniff |= DMA_SNIFF_INV;
	if ((mode & DMA_CRC_REV) != 0) sniff |= DMA_SNIFF_REV;
	DMA_SetSniffCtrl(sniff); // set control word
	DMA_SetSniffData(init); // set init data

	// prepare DMA transfer
	DMA_SetRead_hw(hw, data); // set source address
	DMA_SetWrite_hw(hw, &k); // set destination address
	DMA_SetCount_hw(hw, num); // set count of bytes

	// set DMA control word and start transfer
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hw, DMA_CTRL_SNIFF | DMA_CTRL_TREQ_FORCE | DMA_CTRL_CHAIN(dma)
		| DMA_CTRL_INC_READ | DMA_CTRL_EN | DMA_CTRL_SIZE(size)); // set control and trigger transfer

	// wait DMA for completion
	DMA_Wait_hw(hw);

	// disable DMA channel
	DMA_Disable_hw(hw);

	// get result and deactivate sniff
	u32 res = DMA_SniffData();
	DMA_SetSniffCtrl(0); // set control word, disable sniff
	return res;
}

#endif // USE_DMA	// use DMA controller (sdk_dma.c, sdk_dma.h)
