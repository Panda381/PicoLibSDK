
// ****************************************************************************
//
//                  SSI synchronous serial interface (only RP2040)
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

#if RP2040

#include "../inc/sdk_ssi.h"
#include "../inc/sdk_qspi.h"
#include "../inc/sdk_cpu.h"

#define CMD_READ		0xeb	// Read data fast quad IO
#define MODE_CONTINUOUS_READ	0xa0	// mode bits sent after address bits in CMD_READ
#define CMD_WRITE_ENABLE	0x06
#define CMD_READ_STATUS		0x05
#define CMD_READ_STATUS2	0x35
#define CMD_WRITE_STATUS	0x01
#define SREG_DATA		0x02	// Enable quad-SPI mode

// wait SSI to send
void NOFLASH(SSI_Wait)(ssi_hw_t* ssi)
{
	u32 sr;

	// wait while transmit FIFO is not empty or SSI is busy
	do sr = ssi->sr; while (((sr & B2) == 0) || ((sr & B0) != 0));
}

// read flash status register
u32 NOFLASH(SSI_ReadSreg)(ssi_hw_t* ssi, u32 cmd)
{
	u32 res;

	// send byte to first data register (and dummy byte)
	ssi->dr0 = cmd;
	ssi->dr0 = cmd;

	// wait SSI to send
	SSI_Wait(ssi);

	// get response
	res = ssi->dr0;
	res = ssi->dr0;
	return res;
}

// set flash speed
void NOFLASH(SSI_SetFlashClkDiv)(int clkdiv)
{
	// disable interrupts
	IRQ_LOCK;

	// disable SSI to allow setup
	ssi_hw_t* ssi = ssi_hw;
	ssi->ssienr = 0;

	// set baud rate divider (must be even numer)
	// serial flash interface will run on CLK_SYS/BAUDR
	ssi->baudr = clkdiv;

	// enable SSI
	ssi->ssienr = B0;

	// enable interrupts
	IRQ_UNLOCK;
}

// set flash to fast QSPI mode (clkdiv = clock divider, must be even number, FLASHQSPI_CLKDIV_DEF=4 is default)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ
void NOFLASH(SSI_FlashQspi)(int clkdiv)
{
	u32 res;

	// disable interrupts
	IRQ_LOCK;

	// initialize QSPI pins to connect Flash memory
	QSPI_InitFlash();

	// disable SSI to allow setup
	ssi_hw_t* ssi = ssi_hw;
	ssi->ssienr = 0;

	// set baud rate divider (must be even numer)
	// serial flash interface will run on CLK_SYS/BAUDR
	ssi->baudr = clkdiv;

	// set 1-cycle sample delay
	ssi->rx_sample_dly = 1;

	// set 8 bits per data frame in 32-bit transfer mode, both transmit and receive
	ssi->ctrlr0 = (7 << 16);

	// enable SSI
	ssi->ssienr = B0;

	// check whether SR needs updating (check if QSPI mode is set)
	res = SSI_ReadSreg(ssi, CMD_READ_STATUS2);
	if (res != SREG_DATA)
	{
		// send write enable command (discard response)
		ssi->dr0 = CMD_WRITE_ENABLE;
		SSI_Wait(ssi);
		res = ssi->dr0;

		// send status write enable command followed by data bytes - enable QSPI mode
		ssi->dr0 = CMD_WRITE_STATUS;
		ssi->dr0 = 0;
		ssi->dr0 = SREG_DATA;
		SSI_Wait(ssi);
		res = ssi->dr0;
		res = ssi->dr0;
		res = ssi->dr0;

		// poll status register for write completion
		do res = SSI_ReadSreg(ssi, CMD_READ_STATUS); while ((res & B0) != 0);
	}

	// disable SSI again, to allow reconfiguration
	ssi->ssienr = 0;

	// set quad-SPI frame format, 32 bits per data frame
	ssi->ctrlr0 =	(2 << 21) |	// quad-SPI frame format
			(31 << 16) |	// 32 bits per data frame
			(3 << 8);	// EEPROM read mode

	// number of data frames = 0 (single 32-bit read)
	ssi->ctrlr1 = 0;

	// setup SPI control
	ssi->spi_ctrlr0 = ((32/4) << 2) | 	// number of address + mode bits, divided by 4 (= 4 bytes)
			(4 << 11) |	// number of clocks of Hi-Z after mode bits
			(2 << 8) |	// 8-bit instruction
			1;		// transfer type: command SPI, address QSPI

	// enable SSI
	ssi->ssienr = B0;

	// start continuous reading
	ssi->dr0 = CMD_READ;
	ssi->dr0 = MODE_CONTINUOUS_READ;
	SSI_Wait(ssi);

	// disable SSI again, to allow configuration of XIP bus access translation
	ssi->ssienr = 0;

	// setup SPI control
	//  to send to flash: no instruction, 32-bit word of ADDR (23 HIGH bits) and XIP_CMD (8 LOW bits)
	ssi->spi_ctrlr0 = (MODE_CONTINUOUS_READ << 24) |  // SPI command to send
			((32/4) << 2) | // number of address + mode bits, divided by 4 (= 4 bytes)
			(4 << 11) |	// number of clocks of Hi-Z after mode bits
			(0 << 8) |	// no instruction, only XIP_CMD as mode bits after address
			2;		// transfer type: command and address both QSPI

	// enable SSI
	ssi->ssienr = B0;

	// enable interrupts
	IRQ_UNLOCK;
}

// initialize Flash interface (clkdiv = clock divider, must be even number, FLASHQSPI_CLKDIV_DEF=4 is default)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ
void NOFLASH(SSI_InitFlash)(int clkdiv)
{
	// disable interrupts
	IRQ_LOCK;

	// restore QSPI to default and connect SSI to QSPI pads
	FlashInternal();

	// exit XIP mode to SSI
	FlashExitXip();

	// flush flash cache (and remove CSn IO force)
	FlashFlush();

	// enter XIP mode from bootrom
	FlashEnterXip();

	// set flash to fast QSPI mode
	SSI_FlashQspi(clkdiv);

	// enable interrupts
	IRQ_UNLOCK;
}

#endif // RP2040
