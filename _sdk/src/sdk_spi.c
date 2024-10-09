
// ****************************************************************************
//
//                                       SPI
//                           Serial Peripheral Interface
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

#if USE_SPI	// use SPI interface (sdk_spi.c, sdk_spi.h)

#include "../inc/sdk_spi.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_reset.h"
#include "../inc/sdk_dma.h"
#include "../inc/sdk_gpio.h"
#include "../sdk_dreq.h"

// SPI reset (spi = 0 or 1)
void SPI_Reset(int spi)
{
	u8 peri = (spi == 0) ? RESET_SPI0 : RESET_SPI1;
	ResetPeriphery(peri);
}

void SPI_Reset_hw(spi_hw_t* hw)
{
	u8 peri = (hw == spi0_hw) ? RESET_SPI0 : RESET_SPI1;
	ResetPeriphery(peri);
}

// SPI initialize to Motorola 8-bit Master mode (polarity 0, phase 0)
void SPI_Init_hw(spi_hw_t* hw, u32 baudrate)
{
	// SPI reset
	SPI_Reset_hw(hw);

	// set SPI baudrate
	SPI_Baudrate_hw(hw, baudrate);

	// set format: 8 data bits, polarity 0=steady state LOW, phase 0=first clock edge
	SPI_DataSize_hw(hw, 8);
	SPI_Pol_hw(hw, 0);
	SPI_Phase_hw(hw, 0);

	// enable DMA transfers - harmless if DMA is not listening
	SPI_TxDmaEnable_hw(hw);
	SPI_RxDmaEnable_hw(hw);

	// enable SPI
	SPI_Enable_hw(hw);
}

// SPI terminate
void SPI_Term_hw(spi_hw_t* hw)
{
	// disable SPI
	SPI_Disable_hw(hw);

	// disable DMA transfers
	SPI_TxDmaDisable_hw(hw);
	SPI_RxDmaDisable_hw(hw);

	// SPI reset
	SPI_Reset_hw(hw);
}

// set SPI baudrate (check real baudrate with SPI_GetBaudrate)
void SPI_Baudrate_hw(spi_hw_t* hw, u32 baudrate)
{
	u32 freq, prescale, postdiv;
	int k1, k2;

	// disable SPI
	u32 en = hw->cr1 & B1;
	SPI_Disable_hw(hw);

	// get current peripheral clock
	freq = CurrentFreq[CLK_PERI];
	if (baudrate > (freq+1)/2) baudrate = (freq+1)/2; // baudrate is too high

// baud = clk_peri / (prescale * postdiv)

	// find smallest prescale value which puts output frequency in
	// range of post-divide. Prescale is an even number from 2 to 254.
	for (prescale = 2; prescale < 254; prescale += 2)
	{
		if (freq < (prescale+2)*256*(u64)baudrate) break;
	}

	// find largest post-divide which makes output <= baudrate.
	// Post-divide is an integer from 1 to 256.
	for (postdiv = 256; postdiv > 1; postdiv--)
	{
		if ((u32)(freq / (prescale * (postdiv - 1))) > baudrate) break;
	}

	// rounding to nearest value
	if (postdiv > 1)
	{
		k1 = freq / (prescale * postdiv) - baudrate;
		if (k1 < 0) k1 = -k1;
		k2 = freq / (prescale * (postdiv-1)) - baudrate;
		if (k2 < 0) k2 = -k2;
		if (k2 < k1) postdiv--;
	}

	// set prescaler
	hw->cpsr = prescale;

	// set post-divider
	RegMask(&hw->cr0, (postdiv - 1) << 8, 0xff00);

	// re-enable SPI
	RegSet(&hw->cr1, en);
}

// get current SPI baudrate
u32 SPI_GetBaudrate_hw(const spi_hw_t* hw)
{
	// baud = clk_peri / (prescale * (postdiv + 1))
	u32 freq = CurrentFreq[CLK_PERI]; // peripheral clock, typically 48000000
	u32 prescale = hw->cpsr & 0xff; // clock prescale divisor, 2..254 even number
	u32 postdiv = ((hw->cr0 >> 8) & 0xff) + 1; // post-divider, 1..256
	return freq / (prescale * postdiv);
}

// send and receive 8-bit data simultaneously
void SPI_Send8Recv_hw(spi_hw_t* hw, const u8* src, u8* dst, int len)
{
	int rx_len = len;
	int tx_len = len;

	// flush false received data
	SPI_RxFlush_hw(hw);

	while ((rx_len > 0) || (tx_len > 0))
	{
		// send data
		if ((tx_len > 0) && !SPI_TxIsFull_hw(hw))
		{
			SPI_Write_hw(hw, *src++);
			tx_len--;
		}

		// receive data
		if ((rx_len > 0) && !SPI_RxIsEmpty_hw(hw))
		{
			*dst++ = (u8)SPI_Read_hw(hw);
			rx_len--;
		}
	}
}

// send 8-bit data, discard any data received back
void SPI_Send8_hw(spi_hw_t* hw, const u8* src, int len)
{
	// send data
	for (; len > 0; len--)
	{
		// send data
		while (SPI_TxIsFull_hw(hw)) {}
		SPI_Write_hw(hw, *src++);

		// flush received data
		SPI_RxFlush_hw(hw);
	}

	// waiting for transmission to complete
	while (SPI_IsBusy_hw(hw)) SPI_RxFlush_hw(hw);

	// flush rest of received data
	SPI_RxFlush_hw(hw);

	// clear interrupt on receive overrun status
	SPI_RxOverClear_hw(hw);
}

// send 8-bit data using DMA
void SPI_Send8DMA_hw(spi_hw_t* hw, int dma, const u8* src, int len)
{
	// get DMA channel hardware interface
	dma_channel_hw_t* hwdma = DMA_GetHw(dma);

	// configure DMA
	DMA_Abort(dma); // abort current transfer
	DMA_ClearError_hw(hwdma); // clear errors
	DMA_SetRead_hw(hwdma, src); // set source address
	DMA_SetWrite_hw(hwdma, &hw->dr); // set destination address
	DMA_SetCount_hw(hwdma, len); // set count of elements
	cb(); // compiler barrier
	DMA_SetCtrlTrig_hw(hwdma,
		DMA_CTRL_TREQ(SPI_GetDreq_hw(hw, True)) |
		DMA_CTRL_CHAIN(dma) |
		//DMA_CTRL_INC_WRITE |
		DMA_CTRL_INC_READ |
		DMA_CTRL_SIZE(DMA_SIZE_8)
		| DMA_CTRL_EN); // set control and trigger transfer

	// wait DMA for completion
	DMA_Wait_hw(hwdma);

	// disable DMA channel
	DMA_Disable_hw(hwdma);

	// waiting for transmission to complete
	while (SPI_IsBusy_hw(hw)) SPI_RxFlush_hw(hw);

	// flush rest of received data
	SPI_RxFlush_hw(hw);

	// clear interrupt on receive overrun status
	SPI_RxOverClear_hw(hw);
}

// receive 8-bit data, send repeated byte (usually 0)
void SPI_Recv8_hw(spi_hw_t* hw, u8 repeated_tx, u8* dst, int len)
{
	int rx_len = len;
	int tx_len = len;

	// flush false received data
	SPI_RxFlush_hw(hw);

	while ((rx_len > 0) || (tx_len > 0))
	{
		// send repeated data
		if ((tx_len > 0) && !SPI_TxIsFull_hw(hw))
		{
			SPI_Write_hw(hw, repeated_tx);
			tx_len--;
		}

		// receive data
		if ((rx_len > 0) && !SPI_RxIsEmpty_hw(hw))
		{
			*dst++ = (u8)SPI_Read_hw(hw);
			rx_len--;
		}
	}
}

// send and receive 16-bit data simultaneously
void SPI_Send16Recv_hw(spi_hw_t* hw, const u16* src, u16* dst, int len)
{
	int rx_len = len;
	int tx_len = len;

	// flush false received data
	SPI_RxFlush_hw(hw);

	while ((rx_len > 0) || (tx_len > 0))
	{
		// send data
		if ((tx_len > 0) && !SPI_TxIsFull_hw(hw))
		{
			SPI_Write_hw(hw, *src++);
			tx_len--;
		}

		// receive data
		if ((rx_len > 0) && !SPI_RxIsEmpty_hw(hw))
		{
			*dst++ = (u16)SPI_Read_hw(hw);
			rx_len--;
		}
	}
}

// send 16-bit data, discard any data received back
void SPI_Send16_hw(spi_hw_t* hw, const u16* src, int len)
{
	// send data
	for (; len > 0; len--)
	{
		// send data
		while (SPI_TxIsFull_hw(hw)) {}
		SPI_Write_hw(hw, *src++);

		// flush received data
		SPI_RxFlush_hw(hw);
	}

	// waiting for transmission to complete
	while (SPI_IsBusy_hw(hw)) SPI_RxFlush_hw(hw);

	// flush rest of received data
	SPI_RxFlush_hw(hw);

	// clear interrupt on receive overrun status
	SPI_RxOverClear_hw(hw);
}

// receive 16-bit data, send repeated word (usually 0)
void SPI_Recv16_hw(spi_hw_t* hw, u16 repeated_tx, u16* dst, int len)
{
	int rx_len = len;
	int tx_len = len;

	// flush false received data
	SPI_RxFlush_hw(hw);

	while ((rx_len > 0) || (tx_len > 0))
	{
		// send repeated data
		if ((tx_len > 0) && !SPI_TxIsFull_hw(hw))
		{
			SPI_Write_hw(hw, repeated_tx);
			tx_len--;
		}

		// receive data
		if ((rx_len > 0) && !SPI_RxIsEmpty_hw(hw))
		{
			*dst++ = (u16)SPI_Read_hw(hw);
			rx_len--;
		}
	}
}

#endif // USE_SPI	// use SPI interface (sdk_spi.c, sdk_spi.h)
