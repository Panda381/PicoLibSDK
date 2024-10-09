
// ****************************************************************************
//
//                                Flash Memory
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

#if USE_FLASH	// use Flash memory programming (sdk_flash.c, sdk_flash.h)

#include "../inc/sdk_flash.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_qspi.h"
#include "../inc/sdk_ssi.h"
#include "../inc/sdk_qmi.h"
#include "../inc/sdk_bootrom.h"
#include "../../_lib/inc/lib_crc.h"
#include "../../_lib/inc/lib_decnum.h"

#if USE_ORIGSDK		// include interface of original SDK
pico_unique_board_id_t retrieved_id;
#endif // USE_ORIGSDK

#define FLASH_64KBLOCK_ERASE_CMD 0xd8	// flash erase command of 64KB block
#define FLASH_32KBLOCK_ERASE_CMD 0x52	// flash erase command of 32KB block

// RUID instruction from Winbond memory: 4Bh command prefix, 32 dummy bits, 64 data bits.
#define FLASH_RUID_CMD 0x4b
#define FLASH_RUID_DUMMY_BYTES 4
#define FLASH_RUID_DATA_BYTES 8
#define FLASH_RUID_TOTAL_BYTES (1 + FLASH_RUID_DUMMY_BYTES + FLASH_RUID_DATA_BYTES) // = 13, size of buffer

// Standard get size instruction: 0x9F command prefix, 1 byte manufacturer ID (Winbond 0xef, Micron 0x20), 1 byte memory type, 1 byte capacity order (0x15 = 2 MB)
#define FLASH_SIZE_CMD 0x9f
#define FLASH_SIZE_TOTAL_BYTES 4
#define FLASH_SIZE_SIZE_OFF 3

#define FLASH_BUFF_SIZE		16	// flash cmd buffer size

// flash info
u8 FlashID[FLASH_ID_SIZE];	// unique flash ID (from highest bytes to lowest)
u64 FlashID64 = 0;		// unique flash ID as 64-bit number (the lowest bits are the most variable if you need to use fewer bits)
char FlashIDText[FLASH_ID_SIZE*2+1]; // unique flash ID as HEX text
u8 FlashManufact = 0;		// flash manufacturer (0xef = Winbond)
u8 FlashType = 0;		// flash type (0x40 = W25Q16JV-IQ/JQ, 0x70 = W25Q16JV-IM/JM)
u32 FlashSize = 0;		// flash size in bytes

typedef void (*pBoot2)(void); // Boot2 function

#if !NO_FLASH
// check boot2 crc code, if it is valid
Bool NOINLINE Boot2Check()
{
	u32 crc = Crc32ASlow(Boot2, BOOT2_SIZE_BYTES-4);
	return crc == Boot2[BOOT2_SIZE_BYTES/4-1];
}
#endif // !NO_FLASH

// erase flash memory
//  addr = start address to erase (offset from start of flash XIP_BASE; must be aligned to 4 KB FLASH_SECTOR_SIZE)
//  count = number of bytes to erase (must be multiple of 4 KB FLASH_SECTOR_SIZE)
// If core 1 is running, lockout it or reset it!
void NOFLASH(FlashErase)(u32 addr, u32 count)
{
#if !NO_FLASH

	// check boot 2 loader (this function can be run from Flash memory)
	Bool ok = Boot2Check();

	// copy boot 2 loader into temporary buffer - it is located in the stack, but it will fit OK
	u32 boot2[BOOT2_SIZE_BYTES/4-1];
	if (ok) memcpy(boot2, Boot2, BOOT2_SIZE_BYTES-4);


#endif // !NO_FLASH

	// compiler barrier
	cb();

	// disable interrupts
	IRQ_LOCK;

	// restore QSPI to default and connect SSI to QSPI pads
	FlashInternal();

	// exit XIP mode to SSI
	FlashExitXip();

	// erase flash memory (first erase 32KB block)
	if (((addr & 0xffff) == FLASH_32KBLOCK_SIZE) && (count >= FLASH_32KBLOCK_SIZE))
	{
		RomFlashErase(addr, FLASH_32KBLOCK_SIZE, FLASH_32KBLOCK_SIZE, FLASH_32KBLOCK_ERASE_CMD);
		addr += FLASH_32KBLOCK_SIZE;
		count -= FLASH_32KBLOCK_SIZE;
	}
	if (count > 0) RomFlashErase(addr, count, FLASH_64KBLOCK_SIZE, FLASH_64KBLOCK_ERASE_CMD);

	// flush flash cache (and remove CSn IO force)
	FlashFlush();

#if NO_FLASH

	// set flash to fast QSPI mode
	FlashQspi(FLASHQSPI_CLKDIV_DEF);

#else // NO_FLASH

	// check if boot 2 loader is valid
	if (ok && (boot2[0] != 0) && (boot2[0] != (u32)-1))
	{
		// run boot 2 loader to restore XIP flash settings
		pBoot2 p = (pBoot2)((u32)boot2 + 1);
		p();
	}
	else
		// enter XIP mode
		FlashEnterXip();

#endif // NO_FLASH

	// enable interrupts
	IRQ_UNLOCK;
}

// program flash memory
//  addr = start address to program (offset from start of flash XIP_BASE; must be aligned to 256 B FLASH_PAGE_SIZE)
//  data = pointer to source data to program (must be in RAM)
//  count = number of bytes to program (must be multiple of 256 B FLASH_PAGE_SIZE)
// If core 1 is running, lockout it or reset it!
void NOFLASH(FlashProgram)(u32 addr, const u8* data, u32 count)
{
#if !NO_FLASH

	// check boot 2 loader (this function can be run from Flash memory)
	Bool ok = Boot2Check();

	// copy boot 2 loader into temporary buffer - it is located in the stack, but it will fit OK
	u32 boot2[BOOT2_SIZE_BYTES/4-1];
	if (ok) memcpy(boot2, Boot2, BOOT2_SIZE_BYTES-4);

#endif // !NO_FLASH

	// compiler barrier
	cb();

	// disable interrupts
	IRQ_LOCK;

	// restore QSPI to default and connect SSI to QSPI pads
	FlashInternal();

	// exit XIP mode to SSI
	FlashExitXip();

	// erase flash memory
	RomFlashProgram(addr, data, count);

	// flush flash cache (and remove CSn IO force)
	FlashFlush();

#if NO_FLASH

	// set flash to fast QSPI mode
	FlashQspi(FLASHQSPI_CLKDIV_DEF);

#else // NO_FLASH

	// check if boot 2 loader is valid
	if (ok && (boot2[0] != 0) && (boot2[0] != (u32)-1))
	{
		// run boot 2 loader to restore XIP flash settings
		pBoot2 p = (pBoot2)((u32)boot2 + 1);
		p();
	}
	else
		// enter XIP mode
		FlashEnterXip();

#endif // !NO_FLASH

	// enable interrupts
	IRQ_UNLOCK;
}

// execute flash command
//  txbuf = pointer to byte buffer which will be transmitted to the flash
//  rxbuf = pointer to byte buffer where data receive from the flash
//  count = length in bytes of txbuf and of rxbuf
// If core 1 is running, lockout it or reset it!
void NOFLASH(FlashCmd)(const u8* txbuf, u8* rxbuf, u32 count)
{
#if !NO_FLASH

	// check boot 2 loader (this function can be run from Flash memory)
	Bool ok = Boot2Check();

	// copy boot 2 loader into temporary buffer - it is located in the stack, but it will fit OK
	u32 boot2[BOOT2_SIZE_BYTES/4-1];
	if (ok) memcpy(boot2, Boot2, BOOT2_SIZE_BYTES-4);

#endif // !NO_FLASH

	// compiler barrier
	cb();

	// disable interrupts
	IRQ_LOCK;

	// restore QSPI to default and connect SSI to QSPI pads
	FlashInternal();

	// exit XIP mode to SSI
	FlashExitXip();

	// set CS to LOW (using IO overrides, like bootrom does)
	FlashCsLow();

	// prepare counters of remaining data
	u32 tx_count = count;
	u32 rx_count = count;

#if RP2040
	// process data
	while ((tx_count > 0) || (rx_count > 0)) // while there are some data to send or receive
	{
		// get SSI status register
		u32 flags = ssi_hw->sr;
		Bool can_tx = ((flags & B1) != 0); // check if transmit FIFO is not full (can transmit)
		Bool can_rx = ((flags & B3) != 0); // check if receive FIFO is not empty (can receive)

		// send data (leave a small reserve to avoid overflowing the FIFO)
		if (can_tx && (tx_count > 0) && (rx_count - tx_count < 14))
		{
			ssi_hw->dr0 = *txbuf++;	// send byte
			tx_count--;
		}

		// receive data
		if (can_rx && (rx_count > 0))
		{
			*rxbuf++ = (u8)ssi_hw->dr0; // receive byte
			rx_count--;
		}
  	}
#else // RP2350

	// QMI
	qmi_hw_t* qmi = qmi_hw;

	// enable QMI direct mode
	QMI_DirectEnable();

	// process data
	while ((tx_count > 0) || (rx_count > 0)) // while there are some data to send or receive
	{
		// get QMI status register
		u32 flags = qmi->direct_csr;
		Bool can_tx = ((flags & B10) == 0); // check if transmit FIFO is not full (can transmit)
		Bool can_rx = ((flags & B16) == 0); // check if receive FIFO is not empty (can receive)

		// send data (leave a small reserve to avoid overflowing the FIFO)
		if (can_tx && (tx_count > 0))
		{
			qmi->direct_tx = *txbuf++;	// send byte
			tx_count--;
		}

		// receive data
		if (can_rx && (rx_count > 0))
		{
			*rxbuf++ = (u8)qmi->direct_rx; // receive byte
			rx_count--;
		}
	}

	// disable QMI direct mode
	QMI_DirectDisable();

#endif

	// set CS to HIGH (using IO overrides, like bootom does)
	FlashCsHigh();

	// flush flash cache (and remove CSn IO force)
	FlashFlush();

#if NO_FLASH

	// set flash to fast QSPI mode
	FlashQspi(FLASHQSPI_CLKDIV_DEF);

#else // NO_FLASH

	// check if boot 2 loader is valid
	if (ok && (boot2[0] != 0) && (boot2[0] != (u32)-1))
	{
		// run boot 2 loader to restore XIP flash settings
		pBoot2 p = (pBoot2)((u32)boot2 + 1);
		p();
	}
	else
		// enter XIP mode
		FlashEnterXip();

#endif // NO_FLASH

	// enable interrupts
	IRQ_UNLOCK;
}

// load flash info (called during system startup)
// If core 1 is running, lockout it or reset it!
void FlashLoadInfo()
{
	int i;
	u8 txbuf[FLASH_BUFF_SIZE];
	u8 rxbuf[FLASH_BUFF_SIZE];

	// prepare buffers to load ID
	memset(txbuf, 0, FLASH_RUID_TOTAL_BYTES);
	memset(rxbuf, 0, FLASH_RUID_TOTAL_BYTES);
	txbuf[0] = FLASH_RUID_CMD;

	// send command and receive response
	FlashCmd(txbuf, rxbuf, FLASH_RUID_TOTAL_BYTES);

	// copy ID to the buffer
	u8 b;
	u64 id64 = 0;
	for (i = 0; i < FLASH_ID_SIZE; i++)
	{
		b = rxbuf[i + 1 + FLASH_RUID_DUMMY_BYTES];
		FlashID[i] = b;
		FlashIDText[2*i] = DecHexDig(b>>4);
		FlashIDText[2*i+1] = DecHexDig(b);
		id64 <<= 8;
		id64 |= b;
	}
	FlashID64 = id64;
	FlashIDText[2*FLASH_ID_SIZE] = 0;

	// prepare buffers to load info
	memset(txbuf, 0, FLASH_SIZE_TOTAL_BYTES);
	memset(rxbuf, 0, FLASH_SIZE_TOTAL_BYTES);
	txbuf[0] = FLASH_SIZE_CMD;

	// send command and receive response
	FlashCmd(txbuf, rxbuf, FLASH_SIZE_TOTAL_BYTES);

	// process info
	FlashManufact = rxbuf[1];
	FlashType = rxbuf[2];
	FlashSize = BIT(rxbuf[3]);
}

#endif // USE_FLASH	// use Flash memory programming (sdk_flash.c, sdk_flash.h)
