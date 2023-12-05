
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

// If core 1 is running, it must be stopped by the lockout procedure while
// the flash memory is being programmed (see sdk_sync.h). Or reset core 1.

#if USE_FLASH	// use Flash memory programming (sdk_flash.c, sdk_flash.h)

#ifndef _SDK_FLASH_H
#define _SDK_FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_PAGE_SIZE (1u << 8)	// Flash page size 256 B (to program)
#define FLASH_SECTOR_SIZE (1u << 12)	// Flash sector size 4 KB (to erase)
#define FLASH_PAGES_PER_SECTOR (FLASH_SECTOR_SIZE/FLASH_PAGE_SIZE) // number of flash pages per flash sector (= 16)
#define FLASH_64KBLOCK_SIZE (1u << 16)	// Flash block size 64 KB (to fast erase)
#define FLASH_32KBLOCK_SIZE (1u << 15)	// Flash block size 32 KB (to fast erase)

#define FLASH_ID_SIZE	8		// size of flash ID
#define FLASH_UNIQUE_ID_SIZE_BYTES FLASH_ID_SIZE // size of flash ID (original-SDK flag)
// from config: PICO_FLASH_SIZE_BYTES or FLASH_SIZE, size of the flash in bytes at compile time

// flash info
extern u8 FlashID[FLASH_ID_SIZE];	// unique flash ID (from highest bytes to lowest)
extern u64 FlashID64;			// unique flash ID as 64-bit number (the lowest bits are the most variable if you need to use fewer bits)
extern char FlashIDText[FLASH_ID_SIZE*2+1]; // unique flash ID as HEX text
extern u8 FlashManufact;		// flash manufacturer (0xef = Winbond)
extern u8 FlashType;			// flash type (0x40 = W25Q16JV-IQ/JQ, 0x70 = W25Q16JV-IM/JM)
extern u32 FlashSize;			// flash size in bytes

// boot 2 loader
#define BOOT2_SIZE_BYTES	256	// boot 2 size in bytes (including CRC)
extern const u32 __boot2_start__[BOOT2_SIZE_BYTES/4];
#define Boot2	__boot2_start__

#if !NO_FLASH
// check boot2 crc code, if it is valid
Bool NOINLINE Boot2Check();
#endif

// erase flash memory
//  addr = start address to erase (offset from start of flash XIP_BASE; must be aligned to 4 KB FLASH_SECTOR_SIZE)
//  count = number of bytes to erase (must be multiple of 4 KB FLASH_SECTOR_SIZE)
// If core 1 is running, lockout it or reset it!
void NOFLASH(FlashErase)(u32 addr, u32 count);

// program flash memory
//  addr = start address to program (offset from start of flash XIP_BASE; must be aligned to 256 B FLASH_PAGE_SIZE)
//  data = pointer to source data to program (must be in RAM)
//  count = number of bytes to program (must be multiple of 256 B FLASH_PAGE_SIZE)
// If core 1 is running, lockout it or reset it!
void NOFLASH(FlashProgram)(u32 addr, const u8* data, u32 count);

// execute flash command
//  txbuf = pointer to byte buffer which will be transmitted to the flash
//  rxbuf = pointer to byte buffer where data receive from the flash
//  count = length in bytes of txbuf and of rxbuf
// If core 1 is running, lockout it or reset it!
void NOFLASH(FlashCmd)(const u8* txbuf, u8* rxbuf, u32 count);

// load flash info (called during system startup)
// If core 1 is running, lockout it or reset it!
void FlashLoadInfo();

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Erase areas of flash
//   flash_offs ... Offset into flash, in bytes, to start the erase. Must be aligned to a 4096-byte flash sector.
//   count ... Number of bytes to be erased. Must be a multiple of 4096 bytes (one sector).
INLINE void flash_range_erase(u32 flash_offs, u32 count) { FlashErase(flash_offs, count); }

// Program flash
//   flash_offs ... Flash address of the first byte to be programmed. Must be aligned to a 256-byte flash page.
//   data ... Pointer to the data to program into flash
//   count ... Number of bytes to program. Must be a multiple of 256 bytes (one page).
INLINE void flash_range_program(u32 flash_offs, const u8* data, u32 count) { FlashProgram(flash_offs, data, count); }

// Get flash unique 64 bit identifier
INLINE void flash_get_unique_id(u8* id_out) { memcpy(id_out, FlashID, FLASH_ID_SIZE); }

// Execute bidirectional flash command
INLINE void flash_do_cmd(const u8 *txbuf, u8 *rxbuf, u32 count) { FlashCmd(txbuf, rxbuf, count); }

// Unique board identifier
#define PICO_UNIQUE_BOARD_ID_SIZE_BYTES 8
typedef struct
{
	u8 id[PICO_UNIQUE_BOARD_ID_SIZE_BYTES];
} pico_unique_board_id_t;

extern pico_unique_board_id_t retrieved_id;

INLINE void pico_get_unique_board_id(pico_unique_board_id_t *id_out)
{
	flash_get_unique_id(retrieved_id.id);
	*id_out = retrieved_id;
}

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_FLASH_H

#endif // USE_FLASH	// use Flash memory programming (sdk_flash.c, sdk_flash.h)
