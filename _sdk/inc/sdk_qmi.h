
// ****************************************************************************
//
//                      QMI QSPI memory interface (only RP2350)
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

#if RP2350

#ifndef _SDK_QMI_H
#define _SDK_QMI_H

#include "../sdk_addressmap.h"		// Register address offsets

#if USE_ORIGSDK		// include interface of original SDK
#include "orig_rp2350/orig_qmi.h"	// constants of original SDK
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// === QMI control registers

typedef struct {
	io32		timing;		// 0x00: timing configuration for memory address window
					//	B0..B7: (8 bits) CLKDIV clock divisor (odd or even) 1..256, default 4
					//	B8..B10: (3 bits) RXDELAY delay read data sample timing
					//	B12..B16: (5 bits) MIN_DESELECT delay for chip is deasserted
					//	B17..B22: (6 bts) MAX_SELECT max. assertion diration
					//	B23..B24: (2 bits) SELECT_HOLD hold before deassertion
					//	B25: SELECT_SETUP additional system clock cycle before first rising SCK
					//	B28..B29: (2 bits) PAGEBREAK auto deassert when crossing address boundary
					//		0=no boundary, 1=256B boundary, 2=1KB boundary, 3=4KB boundary
					//	B30..B31: (2 bits) COOLDOWN delay before deassert to save power
	io32		rfmt;		// 0x04: read transfer format for memory address window
					//	B0..B1: (2 bits) PREFIX_WIDTH width of command prefix
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B2..B3: (2 bits) ADDR_WIDTH width of address (address is 24 bits)
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B4..B5: (2 bits) SUFFIX_WIDTH width of post-address command suffix
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B6..B7: (2 bits) DUMMY_WIDTH width of dummy phase
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B8..B9: (2 bits) DATA_SIDTH data width
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B12: PREFIX_LEN length of command prefix 0=no, 1=8 bits
					//	B14..B15: (2 bits) SUFFIX_LEN length of post-address command suffix 0=no, 2=8 bits
					//	B16..B18: (3 bits) DUMMY_LEN 0..7 = 0..28 dummy bits (in multiply of 4) between command and data
					//	B28: DTR enable double transfer rate
	io32		rcmd;		// 0x08: command constans used for reads from memory address window
					//	B0..B7: (8 bits) PREFIX command prefix (default 0x03)
					//	B8..B15: (8 bits) SUFFIX command suffix (default 0xA0)
	io32		wfmt;		// 0x0C: write transfer format for memory address window
					//	B0..B1: (2 bits) PREFIX_WIDTH width of command prefix
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B2..B3: (2 bits) ADDR_WIDTH width of address (address is 24 bits)
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B4..B5: (2 bits) SUFFIX_WIDTH width of post-address command suffix
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B6..B7: (2 bits) DUMMY_WIDTH width of dummy phase
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B8..B9: (2 bits) DATA_SIDTH data width
					//		0=SPI, 1=DSPI, 2=QSPI
					//	B12: PREFIX_LEN length of command prefix 0=no, 1=8 bits
					//	B14..B15: (2 bits) SUFFIX_LEN length of post-address command suffix 0=no, 2=8 bits
					//	B16..B18: (3 bits) DUMMY_LEN 0..7 = 0..28 dummy bits (in multiply of 4) between command and data
					//	B28: DTR enable double transfer rate
	io32		wcmd;		// 0x10: command constans used for writes to memory address window
					//	B0..B7: (8 bits) PREFIX command prefix (default 0x02)
					//	B8..B15: (8 bits) SUFFIX command suffix (defalt 0xA0)
} qmi_mem_hw_t;
STATIC_ASSERT(sizeof(qmi_mem_hw_t) == 0x14, "Incorrect qmi_mem_hw_t!");

typedef struct {
	io32		direct_csr;	// 0x00: control and status for direct serial mode
					//	- only SPI mode 0 (CPOL=0, CPHA=0) is supported
					//	B0: EN enable direct mode
					//	B1: BUSY direct mode busy
					//	B2: ASSERT_CS0N assert CS0n chip select line
					//	B3: ASSERT_CS1N assert CS1n chip select line
					//	B6: AUTO_CS0N automatically assert CS0n chip select line
					//	B7: AUTO_CS1N automatically assert CS1n chip select line
					//	B10: TXFULL direct TX FIFO is full
					//	B11: TXEMPTY direct_TX FIFO is empty
					//	B12..B14: (3 bits) TXLEVEL currect level of direct TX FIFO
					//	B16: RXEMPTY direct RX FIFO is empty
					//	B17: RXFULL direct RX FIFO is full
					//	B18..B20: (3 bits) RXLEVEL current level of direct RX FIFO
					//	B22..B29: (8 bits) CLKDIV clock divisor for direct serial mode 1..256
					//	B30..B31: RXDELAY delay read data sample timing
	io32		direct_tx;	// 0x04: transmit FIFO for direct mode
					//	B0..B15: (16 bits) DATA data to transmit, LSB transmitted first
					//	B16..B17: (2 bits) IWIDTH interface width 0=SPI, 1=DSPI, 2=QSPI
					//	B18: DWIDTH data width 0=8 bit, 1=16 bit
					//	B19: OE output enable (active-high) (SPI: SD0 always output, SD1 always input)
					//	B20: NOPUSH inhibit RX FIFO push
	io32		direct_rx;	// 0x08: receive FIFO for direct mode
					//	B0..B15: (16 bits) DATA received data
	qmi_mem_hw_t	m[2];		// 0x0C: (size 2*0x14=0x28) memory address window 0 and 1
	io32		atrans[8];	// 0x34: (size 0x20) configure translation of virtual address n*0x400000 (4MB window, 2 memories of 16 MB)
					//	B0..B11: (12 bits) BASE physical address base in units of 4 KB
					//		- physical_address to Flash is 24 bits (16 MB)
					//		- virtual address to CPU is 24 bits (16 MB)
					//		physical_address = (virtual_address & 0x3FFFFF) + (BASE << 12), wrap 16 MB
					//	B16..B26: (11 bits) aperture size in 4 KB units
} qmi_hw_t;

#define qmi_hw ((qmi_hw_t*)XIP_QMI_BASE)
STATIC_ASSERT(sizeof(qmi_hw_t) == 0x54, "Incorrect qmi_hw_t!");

// Usual devices: 0 = QSPI Flash, 1 = PSRAM

// enable direct mode (= controlled by software)
INLINE void QMI_DirectEnable(void) { RegSet(&qmi_hw->direct_csr, B0); }

// disable direct mode, set auto mode (= controlled by hardware)
INLINE void QMI_DirectDisable(void) { RegClr(&qmi_hw->direct_csr, B0); }

// check if transfer is busy
INLINE Bool QMI_Busy(void) { return (qmi_hw->direct_csr & B1) != 0; }

// wait if direct is busy
INLINE void QMI_Wait(void) { while (QMI_Busy()) {} }

// set QMI CSn level ON = LOW (dev = device 0 or 1)
INLINE void QMI_CsLow(int dev) { RegSet(&qmi_hw->direct_csr, (B2 << dev)); }

// set QMI CSn level OFF = HIGH (dev = device 0 or 1)
INLINE void QMI_CsHigh(int dev) { RegClr(&qmi_hw->direct_csr, (B2 << dev)); }

// Enable auto assert CSn on busy
INLINE void QMI_AutoCsEnable(int dev) { RegSet(&qmi_hw->direct_csr, (B6 << dev)); }

// Disable auto assert CSn on busy
INLINE void QMI_AutoCsDisable(int dev) { RegClr(&qmi_hw->direct_csr, (B6 << dev)); }

// check if direct TX FIFO is full
INLINE Bool QMI_TxIsFull(void) { return (qmi_hw->direct_csr & B10) != 0; }

// check if direct TX FIFO is empty
INLINE Bool QMI_TxIsEmpty(void) { return (qmi_hw->direct_csr & B11) != 0; }

// get current direct TX FIFO level (returns 0..7)
INLINE int QMI_TxLevel(void) { return (qmi_hw->direct_csr >> 12) & 7; }

// check if direct RX FIFO is empty
INLINE Bool QMI_RxIsEmpty(void) { return (qmi_hw->direct_csr & B16) != 0; }

// check if direct RX FIFO is full
INLINE Bool QMI_RxIsFull(void) { return (qmi_hw->direct_csr & B17) != 0; }

// get current direct RX FIFO level (returns 0..7)
INLINE int QMI_RxLevel(void) { return (qmi_hw->direct_csr >> 18) & 7; }

// set clock divisor for direct mode 1..256 (default value 6)
INLINE void QMI_SetDirClkDiv(int clkdiv) { RegMask(&qmi_hw->direct_csr, (clkdiv << 22), (0xff << 22)); }

// get clock divisor for direct mode (returns 1..256, default 6)
INLINE int QMI_DirClkDiv(void) { return ((u32)((qmi_hw->direct_csr >> 22) - 1) & 0xff) + 1; }

// set delay read data sample timing in half of sys_clk for direct mode (0..3, default 0)
INLINE void QMI_SetRxDelay(int delay) { RegMask(&qmi_hw->direct_csr, (delay << 30), (3 << 30)); }

// get delay read data sample timing in half of sys_clk for direct mode (returns 0..3, default 0)
INLINE int QMI_RxDelay(void) { return (qmi_hw->direct_csr >> 30) & 3; }

// push data in direct mode (width = SPI, data width = 8 bits)
INLINE void QMI_Send(u8 data) { qmi_hw->direct_tx = data; }

// read data in direct mode (data width = 8 bits)
INLINE u8 QMI_Recv(void) { return (u8)qmi_hw->direct_rx; }

// get QMI speed of auto mode (dev = device 0 or 1; returns 1..256, default 4)
INLINE int QMI_ClkDiv(int dev) { return ((u32)(qmi_hw->m[dev].timing - 1) & 0xff) + 1; }

// set QMI speed 1..256 of auto mode (dev = device 0 or 1; default value 4)
INLINE void QMI_SetClkDiv(int dev, int clkdiv) { RegMask(&qmi_hw->m[dev].timing, clkdiv, 0xff); }

// set flash to fast QSPI mode (clkdiv = clock divider, FLASHQSPI_CLKDIV_DEF=2 is default) (must be run from RAM)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ, Raspberry Pico 2 cointains W25Q32RVXHJQ
#define FLASHQSPI_CLKDIV_DEF	2
void NOFLASH(QMI_FlashQspi)(int clkdiv);

// initialize Flash interface (clkdiv = clock divider, must be even number, FLASHQSPI_CLKDIV_DEF=2 is default) (must be run from RAM)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ, Raspberry Pico 2 cointains W25Q32RVXHJQ
void NOFLASH(QMI_InitFlash)(int clkdiv);

#ifdef __cplusplus
}
#endif

#endif // _SDK_QMI_H

#endif // RP2350
