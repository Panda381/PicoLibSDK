
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

#if USE_SPI	// use SPI interface (sdk_spi.c, sdk_spi.h)

#ifndef _SDK_SPI_H
#define _SDK_SPI_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "../sdk_dreq.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_spi.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_spi.h"	// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#define SPI_NUM			2		// number of SPI controllers

// SPI hardware registers
#define SPI_BASE(spi) (SPI0_BASE+(spi)*(SPI1_BASE - SPI0_BASE))		// SPI base address (spi = 0 or 1)

#define SPI_CR0(spi) ((volatile u32*)(SPI_BASE(spi)+0x00))	// control register 0 (spi = 0 or 1)
#define SPI_CR1(spi) ((volatile u32*)(SPI_BASE(spi)+0x04))	// control register 1 (spi = 0 or 1)
#define SPI_DR(spi) ((volatile u32*)(SPI_BASE(spi)+0x08))	// data register (spi = 0 or 1)
#define SPI_SR(spi) ((volatile u32*)(SPI_BASE(spi)+0x0C))	// status register (spi = 0 or 1; read only)
#define SPI_CPSR(spi) ((volatile u32*)(SPI_BASE(spi)+0x10))	// clock prescaler divisor 2..254 (spi = 0 or 1)
#define SPI_IMSC(spi) ((volatile u32*)(SPI_BASE(spi)+0x14))	// interrupt mask
#define SPI_RIS(spi) ((volatile u32*)(SPI_BASE(spi)+0x18))	// raw interrupt status
#define SPI_MIS(spi) ((volatile u32*)(SPI_BASE(spi)+0x1C))	// masked interrupt status
#define SPI_ICR(spi) ((volatile u32*)(SPI_BASE(spi)+0x20))	// clear interrupt status
#define SPI_DMACR(spi) ((volatile u32*)(SPI_BASE(spi)+0x24))	// DMA control register

// SPI hardware interface
typedef struct {
	io32	cr0;	// 0x00: Control register 0
	io32	cr1;	// 0x04: Control register 1
	io32	dr;	// 0x08: Data register
	io32	sr;	// 0x0C: Status register
	io32	cpsr;	// 0x10: Clock prescale register
	io32	imsc;	// 0x14: Interrupt mask set or clear register
	io32	ris;	// 0x18: Raw interrupt status register
	io32	mis;	// 0x1C: Masked interrupt status register
	io32	icr;	// 0x20: Interrupt clear register
	io32	dmacr;	// 0x24: DMA control register
} spi_hw_t;

#define spi0_hw ((spi_hw_t*)SPI0_BASE)
#define spi1_hw ((spi_hw_t*)SPI1_BASE)

#define spi0	spi0_hw
#define spi1	spi1_hw

STATIC_ASSERT(sizeof(spi_hw_t) == 0x28, "Incorrect spi_hw_t!");

// frame format
#define SPI_FORMAT_SPI	0	// Motorola SPI
#define SPI_FORMAT_SS	1	// TI synchronous serial
#define SPI_FORMAT_MW	2	// National Microwire

// get SPI hardware interface from SPI index
INLINE spi_hw_t* SPI_GetHw(int spi) { return (spi == 0) ? spi0_hw : spi1_hw; }

// get SPI index from SPI hardware interface
INLINE u8 SPI_GetInx(const spi_hw_t* hw) { return (hw == spi0_hw) ? 0 : 1; }

// get SPI DREQ
//  spi ... SPI number 0 or 1
//  tx ... True for sending data to SPI, False for receiving data from SPI
INLINE u8 SPI_GetDreq(int spi, Bool tx) { return DREQ_SPI0_TX + spi*2 + (tx ? 0 : 1); }
INLINE u8 SPI_GetDreq_hw(const spi_hw_t* hw, Bool tx)
	{ return ((hw == spi0_hw) ? DREQ_SPI0_TX : DREQ_SPI1_TX) + (tx ? 0 : 1); }

// === SPI setup

// SPI reset (spi = 0 or 1)
void SPI_Reset(int spi);
void SPI_Reset_hw(spi_hw_t* hw);

// SPI initialize to Motorola 8-bit Master mode (polarity 0, phase 0)
void SPI_Init_hw(spi_hw_t* hw, u32 baudrate);
INLINE void SPI_Init(int spi, u32 baudrate) { SPI_Init_hw(SPI_GetHw(spi), baudrate); }

// SPI terminate
void SPI_Term_hw(spi_hw_t* hw);
INLINE void SPI_Term(int spi)  { SPI_Term_hw(SPI_GetHw(spi)); }

// set SPI baudrate (check real baudrate with SPI_GetBaudrate)
void SPI_Baudrate_hw(spi_hw_t* hw, u32 baudrate);
INLINE void SPI_Baudrate(int spi, u32 baudrate) { SPI_Baudrate_hw(SPI_GetHw(spi), baudrate); }

// get current SPI baudrate
u32 SPI_GetBaudrate_hw(const spi_hw_t* hw);
INLINE u32 SPI_GetBaudrate(int spi) { return SPI_GetBaudrate_hw(SPI_GetHw(spi)); }

// set SPI data size to 4..16 bits (spi = 0 or 1)
INLINE void SPI_DataSize(int spi, int size) { RegMask(SPI_CR0(spi), (u32)(size-1), 0x0f); }
INLINE void SPI_DataSize_hw(spi_hw_t* hw, int size) { RegMask(&hw->cr0, (u32)(size-1), 0x0f); }

// set SPI frame format to SPI_FORM_* (spi = 0 or 1)
INLINE void SPI_Format(int spi, int form) { RegMask(SPI_CR0(spi), (u32)form << 4, B4|B5); }
INLINE void SPI_Format_hw(spi_hw_t* hw, int form) { RegMask(&hw->cr0, (u32)form << 4, B4|B5); }

// set SPI clock polarity to steady state 0=LOW or 1=HIGH (only Motorola SPI frame format; spi = 0 or 1)
INLINE void SPI_Pol(int spi, int pol) { RegMask(SPI_CR0(spi), (pol << 6), B6); }
INLINE void SPI_Pol_hw(spi_hw_t* hw, int pol) { RegMask(&hw->cr0, (pol << 6), B6); }

// set SPI clock phase to data captured on 0=first or 1=second clock edge (only Motorola SPI frame format; spi = 0 or 1)
INLINE void SPI_Phase(int spi, u8 phase) { RegMask(SPI_CR0(spi), (phase << 7), B7); }
INLINE void SPI_Phase_hw(spi_hw_t* hw, u8 phase) { RegMask(&hw->cr0, (phase << 7), B7); }

// set loop back mode ON (spi = 0 or 1)
INLINE void SPI_LoopBackOn(int spi) { RegSet(SPI_CR1(spi), B0); }
INLINE void SPI_LoopBackOn_hw(spi_hw_t* hw) { RegSet(&hw->cr1, B0); }

// set loop back mode OFF (default state; spi = 0 or 1)
INLINE void SPI_LoopBackOff(int spi) { RegClr(SPI_CR1(spi), B0); }
INLINE void SPI_LoopBackOff_hw(spi_hw_t* hw) { RegClr(&hw->cr1, B0); }

// enable SPI (spi = 0 or 1)
INLINE void SPI_Enable(int spi) { RegSet(SPI_CR1(spi), B1); }
INLINE void SPI_Enable_hw(spi_hw_t* hw) { RegSet(&hw->cr1, B1); }

// disable SPI (default state; spi = 0 or 1)
INLINE void SPI_Disable(int spi) { RegClr(SPI_CR1(spi), B1); }
INLINE void SPI_Disable_hw(spi_hw_t* hw) { RegClr(&hw->cr1, B1); }

// set SPI master mode (default state; spi = 0 or 1)
INLINE void SPI_Master(int spi) { RegClr(SPI_CR1(spi), B2); }
INLINE void SPI_Master_hw(spi_hw_t* hw) { RegClr(&hw->cr1, B2); }

// set SPI slave mode (spi = 0 or 1)
INLINE void SPI_Slave(int spi) { RegSet(SPI_CR1(spi), B2); }
INLINE void SPI_Slave_hw(spi_hw_t* hw) { RegSet(&hw->cr1, B2); }

// enable SPI output, if in slave mode (default state; spi = 0 or 1)
INLINE void SPI_OutEnable(int spi) { RegClr(SPI_CR1(spi), B3); }
INLINE void SPI_OutEnable_hw(spi_hw_t* hw) { RegClr(&hw->cr1, B3); }

// disable SPI output, if in slave mode (spi = 0 or 1)
INLINE void SPI_OutDisable(int spi) { RegSet(SPI_CR1(spi), B3); }
INLINE void SPI_OutDisable_hw(spi_hw_t* hw) { RegSet(&hw->cr1, B3); }

// === SPI status

// check if transmit FIFO is empty
INLINE Bool SPI_TxIsEmpty(int spi) { return (*SPI_SR(spi) & B0) != 0; }
INLINE Bool SPI_TxIsEmpty_hw(const spi_hw_t* hw) { return (hw->sr & B0) != 0; }

// check if transmit FIFO is full
INLINE Bool SPI_TxIsFull(int spi) { return (*SPI_SR(spi) & B1) == 0; }
INLINE Bool SPI_TxIsFull_hw(const spi_hw_t* hw) { return (hw->sr & B1) == 0; }

// check if receive FIFO is empty
INLINE Bool SPI_RxIsEmpty(int spi) { return (*SPI_SR(spi) & B2) == 0; }
INLINE Bool SPI_RxIsEmpty_hw(const spi_hw_t* hw) { return (hw->sr & B2) == 0; }

// check if receive FIFO is full
INLINE Bool SPI_RxIsFull(int spi) { return (*SPI_SR(spi) & B3) != 0; }
INLINE Bool SPI_RxIsFull_hw(const spi_hw_t* hw) { return (hw->sr & B3) != 0; }

// check if SPI is busy (currently transmitting/receiving frame)
INLINE Bool SPI_IsBusy(int spi) { return (*SPI_SR(spi) & B4) != 0; }
INLINE Bool SPI_IsBusy_hw(const spi_hw_t* hw) { return (hw->sr & B4) != 0; }

// === SPI interrupt

// enable interrupt on receive overrun
INLINE void SPI_RxOverEnable(int spi) { RegSet(SPI_IMSC(spi), B0); }
INLINE void SPI_RxOverEnable_hw(spi_hw_t* hw) { RegSet(&hw->imsc, B0); }

// disable interrupt on receive overrun
INLINE void SPI_RxOverDisable(int spi) { RegClr(SPI_IMSC(spi), B0); }
INLINE void SPI_RxOverDisable_hw(spi_hw_t* hw) { RegClr(&hw->imsc, B0); }

// clear interrupt on receive overrun status
INLINE void SPI_RxOverClear(int spi) { *SPI_ICR(spi) = B0; }
INLINE void SPI_RxOverClear_hw(spi_hw_t* hw) { hw->icr = B0; }

// get raw status of interrupt on receive overrun (0 or 1)
INLINE u8 SPI_GetRxOverRaw(int spi) { return (u8)(*SPI_RIS(spi) & 1); }
INLINE u8 SPI_GetRxOverRaw_hw(const spi_hw_t* hw) { return (u8)(hw->ris & 1); }

// get masked status of interrupt on receive overrun (0 or 1)
INLINE u8 SPI_GetRxOver(int spi) { return (u8)(*SPI_MIS(spi) & 1); }
INLINE u8 SPI_GetRxOver_hw(const spi_hw_t* hw) { return (u8)(hw->mis & 1); }

// enable interrupt on receive timeout
INLINE void SPI_RxToutEnable(int spi) { RegSet(SPI_IMSC(spi), B1); }
INLINE void SPI_RxToutEnable_hw(spi_hw_t* hw) { RegSet(&hw->imsc, B1); }

// disable interrupt on receive timeout
INLINE void SPI_RxToutDisable(int spi) { RegClr(SPI_IMSC(spi), B1); }
INLINE void SPI_RxToutDisable_hw(spi_hw_t* hw) { RegClr(&hw->imsc, B1); }

// clear interrupt on receive timeout status
INLINE void SPI_RxToutClear(int spi) { *SPI_ICR(spi) = B1; }
INLINE void SPI_RxToutClear_hw(spi_hw_t* hw) { hw->icr = B1; }

// get raw status of interrupt on receive timeout (0 or 1)
INLINE u8 SPI_GetRxToutRaw(int spi) { return (u8)((*SPI_RIS(spi) >> 1) & 1); }
INLINE u8 SPI_GetRxToutRaw_hw(const spi_hw_t* hw) { return (u8)((hw->ris >> 1) & 1); }

// get masked status of interrupt on receive timeout (0 or 1)
INLINE u8 SPI_GetRxTout(int spi) { return (u8)((*SPI_MIS(spi) >> 1) & 1); }
INLINE u8 SPI_GetRxTout_hw(const spi_hw_t* hw) { return (u8)((hw->mis >> 1) & 1); }

// enable interrupt on receive half FIFO
INLINE void SPI_RxFifoEnable(int spi) { RegSet(SPI_IMSC(spi), B2); }
INLINE void SPI_RxFifoEnable_hw(spi_hw_t* hw) { RegSet(&hw->imsc, B2); }

// disable interrupt on receive half FIFO
INLINE void SPI_RxFifoDisable(int spi) { RegClr(SPI_IMSC(spi), B2); }
INLINE void SPI_RxFifoDisable_hw(spi_hw_t* hw) { RegClr(&hw->imsc, B2); }

// get raw status of interrupt on receive half FIFO (0 or 1)
INLINE u8 SPI_GetRxFifoRaw(int spi) { return (u8)((*SPI_RIS(spi) >> 2) & 1); }
INLINE u8 SPI_GetRxFifoRaw_hw(const spi_hw_t* hw) { return (u8)((hw->ris >> 2) & 1); }

// get masked status of interrupt on receive half FIFO (0 or 1)
INLINE u8 SPI_GetRxFifo(int spi) { return (u8)((*SPI_MIS(spi) >> 2) & 1); }
INLINE u8 SPI_GetRxFifo_hw(const spi_hw_t* hw) { return (u8)((hw->mis >> 2) & 1); }

// enable interrupt on send half FIFO
INLINE void SPI_TxFifoEnable(int spi) { RegSet(SPI_IMSC(spi), B3); }
INLINE void SPI_TxFifoEnable_hw(spi_hw_t* hw) { RegSet(&hw->imsc, B3); }

// disable interrupt on send half FIFO
INLINE void SPI_TxFifoDisable(int spi) { RegClr(SPI_IMSC(spi), B3); }
INLINE void SPI_TxFifoDisable_hw(spi_hw_t* hw) { RegClr(&hw->imsc, B3); }

// get raw status of interrupt on send half FIFO (0 or 1)
INLINE u8 SPI_GetTxFifoRaw(int spi) { return (u8)((*SPI_RIS(spi) >> 3) & 1); }
INLINE u8 SPI_GetTxFifoRaw_hw(const spi_hw_t* hw) { return (u8)((hw->ris >> 3) & 1); }

// get masked status of interrupt on send half FIFO (0 or 1)
INLINE u8 SPI_GetTxFifo(int spi) { return (u8)((*SPI_MIS(spi) >> 3) & 1); }
INLINE u8 SPI_GetTxFifo_hw(const spi_hw_t* hw) { return (u8)((hw->mis >> 3) & 1); }

// === SPI DMA

// enable transmit DMA
INLINE void SPI_TxDmaEnable(int spi) { RegSet(SPI_DMACR(spi), B1); }
INLINE void SPI_TxDmaEnable_hw(spi_hw_t* hw) { RegSet(&hw->dmacr, B1); }

// disable transmit DMA
INLINE void SPI_TxDmaDisable(int spi) { RegClr(SPI_DMACR(spi), B1); }
INLINE void SPI_TxDmaDisable_hw(spi_hw_t* hw) { RegClr(&hw->dmacr, B1); }

// enable receive DMA
INLINE void SPI_RxDmaEnable(int spi) { RegSet(SPI_DMACR(spi), B0); }
INLINE void SPI_RxDmaEnable_hw(spi_hw_t* hw) { RegSet(&hw->dmacr, B0); }

// disable receive DMA
INLINE void SPI_RxDmaDisable(int spi) { RegClr(SPI_DMACR(spi), B0); }
INLINE void SPI_RxDmaDisable_hw(spi_hw_t* hw) { RegClr(&hw->dmacr, B0); }

// === SPI data transfer

// send data to transmit FIFO (does not check status register; spi = 0 or 1)
INLINE void SPI_Write(int spi, u16 data) { *SPI_DR(spi) = data; }
INLINE void SPI_Write_hw(spi_hw_t* hw, u16 data) { hw->dr = data; }

// receive data from receive FIFO (does not check status register; spi = 0 or 1)
INLINE u16 SPI_Read(int spi) { return (u16)*SPI_DR(spi); }
INLINE u16 SPI_Read_hw(const spi_hw_t* hw) { return (u16)hw->dr; }

// flush receive FIFO
INLINE void SPI_RxFlush(int spi) { while (!SPI_RxIsEmpty(spi)) SPI_Read(spi); }
INLINE void SPI_RxFlush_hw(spi_hw_t* hw) { while (!SPI_RxIsEmpty_hw(hw)) SPI_Read_hw(hw); }

// send and receive 8-bit data simultaneously
void SPI_Send8Recv_hw(spi_hw_t* hw, const u8* src, u8* dst, int len);
INLINE void SPI_Send8Recv(int spi, const u8* src, u8* dst, int len)
	{ SPI_Send8Recv_hw(SPI_GetHw(spi), src, dst, len); }

// send 8-bit data, discard any data received back
void SPI_Send8_hw(spi_hw_t* hw, const u8* src, int len);
INLINE void SPI_Send8(int spi, const u8* src, int len)
	{ SPI_Send8_hw(SPI_GetHw(spi), src, len); }

// send 8-bit data using DMA
void SPI_Send8DMA_hw(spi_hw_t* hw, int dma, const u8* src, int len);
INLINE void SPI_Send8DMA(int spi, int dma, const u8* src, int len)
	{ SPI_Send8DMA_hw(SPI_GetHw(spi), dma, src, len); }

// receive 8-bit data, send repeated byte (usually 0)
void SPI_Recv8_hw(spi_hw_t* hw, u8 repeated_tx, u8* dst, int len);
INLINE void SPI_Recv8(int spi, u8 repeated_tx, u8* dst, int len)
	{ SPI_Recv8_hw(SPI_GetHw(spi), repeated_tx, dst, len); }

// send and receive 16-bit data simultaneously
void SPI_Send16Recv_hw(spi_hw_t* hw, const u16* src, u16* dst, int len);
INLINE void SPI_Send16Recv(int spi, const u16* src, u16* dst, int len)
	{ SPI_Send16Recv_hw(SPI_GetHw(spi), src, dst, len); }

// send 16-bit data, discard any data received back
void SPI_Send16_hw(spi_hw_t* hw, const u16* src, int len);
INLINE void SPI_Send16(int spi, const u16* src, int len)
	{ SPI_Send16_hw(SPI_GetHw(spi), src, len); }

// receive 16-bit data, send repeated word (usually 0)
void SPI_Recv16_hw(spi_hw_t* hw, u16 repeated_tx, u16* dst, int len);
INLINE void SPI_Recv16(int spi, u16 repeated_tx, u16* dst, int len)
	{ SPI_Recv16_hw(SPI_GetHw(spi), repeated_tx, dst, len); }

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

typedef spi_hw_t spi_inst_t;
#undef spi0
#undef spi1
#define spi0 ((spi_inst_t*)spi0_hw)
#define spi1 ((spi_inst_t*)spi1_hw)

// Enumeration of SPI CPHA (clock phase) values.
typedef enum {
	SPI_CPHA_0 = 0,
	SPI_CPHA_1 = 1
} spi_cpha_t;

// Enumeration of SPI CPOL (clock polarity) values.
typedef enum {
	SPI_CPOL_0 = 0,
	SPI_CPOL_1 = 1
} spi_cpol_t;

// Enumeration of SPI bit-order values.
typedef enum {
	SPI_LSB_FIRST = 0,
	SPI_MSB_FIRST = 1
} spi_order_t;

// === Setup

// Initialise SPI instances
INLINE uint spi_init(spi_inst_t *spi, uint baudrate)
	{ SPI_Init_hw(spi, baudrate); return SPI_GetBaudrate_hw(spi); }

// Deinitialise SPI instances
INLINE void spi_deinit(spi_inst_t *spi) { SPI_Term_hw(spi); }

// Set SPI baudrate
INLINE uint spi_set_baudrate(spi_inst_t *spi, uint baudrate)
	{ SPI_Baudrate_hw(spi, baudrate); return SPI_GetBaudrate_hw(spi); }

// Get SPI baudrate
INLINE uint spi_get_baudrate(const spi_inst_t *spi) { return SPI_GetBaudrate_hw(spi); }

// Convert SPI instance to hardware instance number
INLINE uint spi_get_index(const spi_inst_t *spi) { return SPI_GetInx(spi); }
INLINE spi_hw_t* spi_get_hw(spi_inst_t *spi) { return (spi_hw_t*)spi; }
INLINE const spi_hw_t* spi_get_const_hw(const spi_inst_t *spi) { return (const spi_hw_t*)spi; }

// Configure SPI
INLINE void spi_set_format(spi_inst_t *spi, uint data_bits, spi_cpol_t cpol, spi_cpha_t cpha, __unused spi_order_t order)
{
	SPI_DataSize_hw(spi, data_bits);
	SPI_Pol_hw(spi, cpol);
	SPI_Phase_hw(spi, cpha);
}

// Set SPI master/slave
INLINE void spi_set_slave(spi_inst_t *spi, bool slave)
	{ if (slave) SPI_Slave_hw(spi); else SPI_Master_hw(spi); }

// === Generic input/output

// Check whether a write can be done on SPI device
INLINE bool spi_is_writable(const spi_inst_t *spi) { return !SPI_TxIsFull_hw(spi); }

// Check whether a read can be done on SPI device
INLINE bool spi_is_readable(const spi_inst_t *spi) { return !SPI_RxIsEmpty_hw(spi); }

// Check whether SPI is busy
INLINE bool spi_is_busy(const spi_inst_t *spi) { return SPI_IsBusy_hw(spi); }

// Write/Read to/from an SPI device
INLINE int spi_write_read_blocking(spi_inst_t *spi, const u8* src, u8* dst, size_t len)
	{ SPI_Send8Recv_hw(spi, src, dst, len); return len; }

// Write to an SPI device, blocking
INLINE int spi_write_blocking(spi_inst_t *spi, const u8* src, size_t len)
	{ SPI_Send8_hw(spi, src, len); return len; }

// Read from an SPI device
INLINE int spi_read_blocking(spi_inst_t *spi, u8 repeated_tx_data, u8* dst, size_t len)
	{ SPI_Recv8_hw(spi, repeated_tx_data, dst, len); return len; }

// Write/Read half words to/from an SPI device
INLINE int spi_write16_read16_blocking(spi_inst_t *spi, const u16* src, u16* dst, size_t len)
	{ SPI_Send16Recv_hw(spi, src, dst, len); return len; }

// Write to an SPI device
INLINE int spi_write16_blocking(spi_inst_t *spi, const u16* src, size_t len)
	{ SPI_Send16_hw(spi, src, len); return len; }

// Read from an SPI device
INLINE int spi_read16_blocking(spi_inst_t *spi, u16 repeated_tx_data, u16* dst, size_t len)
	{ SPI_Recv16_hw(spi, repeated_tx_data, dst, len); return len; }

// Return the DREQ to use for pacing transfers to/from a particular SPI instance
INLINE uint spi_get_dreq(spi_inst_t *spi, bool is_tx) { return SPI_GetDreq_hw(spi, is_tx); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_SPI_H

#endif // USE_SPI	// use SPI interface (sdk_spi.c, sdk_spi.h)
