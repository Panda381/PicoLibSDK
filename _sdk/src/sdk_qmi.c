
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

#include "../../global.h"	// globals

#if RP2350

#include "../inc/sdk_cpu.h"
#include "../inc/sdk_qmi.h"
#include "../inc/sdk_qspi.h"

#define CMD_READ		0xeb	// Read data fast quad IO
#define MODE_CONTINUOUS_READ	0xa0	// mode bits sent after address bits in CMD_READ
#define CMD_WRITE_ENABLE	0x06
#define CMD_READ_STATUS		0x05
#define CMD_READ_STATUS2	0x35
#define CMD_WRITE_STATUS	0x01
#define SREG_DATA		0x02	// Enable quad-SPI mode

// read flash status register
static u8 NOFLASH(QMI_ReadFlashSreg)(u8 cmd)
{
	// write command and dummy byte
	QMI_Send(cmd);
	QMI_Send(cmd);

	// wait transfer
	QMI_Wait();

	// discard dirst dummy byte, return second
	u8 d = QMI_Recv();
	d = QMI_Recv();
	return d;
}

// set flash to fast QSPI mode (clkdiv = clock divider, FLASHQSPI_CLKDIV_DEF=2 or 4 is default) (must be run from RAM)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ, Raspberry Pico 2 cointains W25Q32RVXHJQ
void NOFLASH(QMI_FlashQspi)(int clkdiv)
{
	u32 res;

	// disable interrupts
	IRQ_LOCK;

	// initialize QSPI pins to connect Flash memory
	QSPI_InitFlash();

	// activate direct mode: enable direct mode, enable auto assert CS0n, clock divisor = 30, RX delay = 1
	qmi_hw_t* qmi = qmi_hw;
	u32 oldcsr = qmi->direct_csr; // save old setup
	qmi->direct_csr = B0 | B6 | (30 << 22) | B30;

	// wait if last transfer is busy
	QMI_Wait();

	// check whether SR needs updating (QSPI mode must be enabled)
	u8 d = QMI_ReadFlashSreg(CMD_READ_STATUS2);
	if (d != SREG_DATA)
	{
		// send write enable command, discard response
		QMI_Send(CMD_WRITE_ENABLE);
		QMI_Wait();
		d = QMI_Recv();

		// send status write enable command followed by data bytes - enable QSPI mode
		QMI_Send(CMD_WRITE_STATUS);
		QMI_Send(0);
		QMI_Send(SREG_DATA);

		// wait transfer
		QMI_Wait();

		// discard response
		d = QMI_Recv();
		d = QMI_Recv();
		d = QMI_Recv();

		// poll status register for write completion
		do {
			d = QMI_ReadFlashSreg(CMD_READ_STATUS);
		} while ((d & B0) != 0);
	}

	// deactivate direct mode: disable direct mode, enable auto assert CS0n, clock divisor = 6
	qmi->direct_csr = B6 | (6 << 22) | (oldcsr & (B7+B30+B31)); // restore old state of auto_CS1n and RXdelay

	// setup timing: clkdiv = 2, rxdelay = 2, cooldown = 1
	qmi->m[0].timing = clkdiv /*FLASHQSPI_CLKDIV_DEF*/ | (2 << 8) | (1 << 30);

	// setup read command: prefix = 0xEB, suffix = 0xA0
        qmi->m[0].rcmd = CMD_READ | (MODE_CONTINUOUS_READ << 8);

	// setup read format: prefix S, addr Q, suffix Q, dummy Q, data Q, prefix 8bit, suffix 8bit, 16 dummy bits
	qmi->m[0].rfmt = 0 | (2<<2) | (2<<4) | (2<<6) | (2<<8) | B12 | (2<<14) | (4<<16);

	// dummy transfer
	dmb();
	d = *(volatile u8*)(u32)XIP_NOCACHE_NOALLOC_BASE;
	dmb();

	// Set prefix length to 0, as flash no longer expects to see commands
	RegClr(&qmi->m[0].rfmt, B12);

	// enable interrupts
	IRQ_UNLOCK;
}

// initialize Flash interface (clkdiv = clock divider, must be even number, FLASHQSPI_CLKDIV_DEF=2 or 4 is default) (must be run from RAM)
//   Supported devices: Winbond W25Q080, W25Q16JV, AT25SF081, S25FL132K0
//   Raspberry Pico cointains W25Q16JVUXIQ, Raspberry Pico 2 cointains W25Q32RVXHJQ
void NOFLASH(QMI_InitFlash)(int clkdiv)
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
	QMI_FlashQspi(clkdiv);

	// enable interrupts
	IRQ_UNLOCK;
}

#endif // RP2350
