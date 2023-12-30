
// ****************************************************************************
//
//                                Boot ROM
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

#ifndef _SDK_BOOTROM_H
#define _SDK_BOOTROM_H

#ifdef __cplusplus
extern "C" {
#endif

// prepare bootrom lookup code based on two ASCII characters (RP2040 datasheet page 156)
#define ROM_TABLE_CODE(ch1, ch2) ((ch1) | ((ch2) << 8))

// ROM functions
#define ROM_FUNC_POPCOUNT32		ROM_TABLE_CODE('P', '3')	// counts '1' bits
#define ROM_FUNC_REVERSE32		ROM_TABLE_CODE('R', '3')	// reverse order of bits
#define ROM_FUNC_CLZ32			ROM_TABLE_CODE('L', '3')	// count leading zeros
#define ROM_FUNC_CTZ32			ROM_TABLE_CODE('T', '3')	// count trailing zeros
#define ROM_FUNC_MEMSET			ROM_TABLE_CODE('M', 'S')	// fill memory
#define ROM_FUNC_MEMSET4		ROM_TABLE_CODE('S', '4')	// fill memory aligned to u32
#define ROM_FUNC_MEMCPY			ROM_TABLE_CODE('M', 'C')	// copy memory
#define ROM_FUNC_MEMCPY44		ROM_TABLE_CODE('C', '4')	// copy memory aligned to u32
#define ROM_FUNC_RESET_USB_BOOT		ROM_TABLE_CODE('U', 'B')	// reset CPU to BOOTSEL mode
#define ROM_FUNC_DEBUG_TRAMPOLINE	ROM_TABLE_CODE('D', 'T')	// debug trampoline
#define ROM_FUNC_DEBUG_TRAMPOLINE_END	ROM_TABLE_CODE('D', 'E')	// debug trampoline end
#define ROM_FUNC_WAIT_FOR_VECTOR	ROM_TABLE_CODE('W', 'V')	// wait core 1 for launch
#define ROM_FUNC_CONNECT_INTERNAL_FLASH ROM_TABLE_CODE('I', 'F')	// restore QSPI to default and connect SSI to QSPI pads
#define ROM_FUNC_FLASH_EXIT_XIP		ROM_TABLE_CODE('E', 'X')	// exit XIP mode to SSI
#define ROM_FUNC_FLASH_RANGE_ERASE	ROM_TABLE_CODE('R', 'E')	// erase flash
#define ROM_FUNC_FLASH_RANGE_PROGRAM	ROM_TABLE_CODE('R', 'P')	// program flash
#define ROM_FUNC_FLASH_FLUSH_CACHE	ROM_TABLE_CODE('F', 'C')	// flush flash cache
#define ROM_FUNC_FLASH_ENTER_CMD_XIP	ROM_TABLE_CODE('C', 'X')	// enter XIP mode
#define ROM_FUNC_ENABLE_CLOCKS		ROM_TABLE_CODE('E', 'C')	// enable clocks after warm boot

// Convert 16-bit pointer stored at the given rom address into 32-bit pointer
#define ROM_HWORD_PTR(addr) (void*)(u32)(*(u16*)(u32)(addr))

// ROM function prototypes
typedef u8 (*pPopCount)(u32);				// counts '1' bits
typedef u32 (*pReverse)(u32);				// reverse order of bits
typedef u8 (*pClz)(u32);				// count leading zeros
typedef u8 (*pCtz)(u32);				// count trailing zeros
typedef u8* (*pMemSet)(void*, u8, u32);			// fill memory
typedef u32* (*pMemSet32)(u32*, u8, u32);		// fill memory aligned to u32
typedef u8* (*pMemCopy)(void*, const void*, u32);	// copy memory
typedef u32* (*pMemCopy32)(u32*, const u32*, u32);	// copy memory aligned to u32
typedef void __attribute__((noreturn)) (*pResetUsb)(u32, u32); // reset CPU to BOOTSEL mode
typedef void __attribute__((noreturn)) (*pWaitForVector)(); // wait core 1 for launch
typedef void (*pFlashInternal)(void);			// restore QSPI to default and connect SSI to QSPI pads
typedef void (*pFlashExitXip)(void);			// exit XIP mode to SSI
typedef void (*pFlashErase)(u32, u32, u32, u8);		// erase flash
typedef void (*pFlashProgram)(u32, const u8*, u32);
typedef void (*pFlashFlush)(void);			// flush flash cache
typedef void (*pFlashEnterXip)(void);			// enter XIP mode

// ROM function pointers (60 bytes)
extern pPopCount popcount;
extern pReverse reverse;
extern pClz clz;
extern pCtz ctz;
extern pMemSet MemSet;
extern pMemSet32 MemSet32;
extern pMemCopy MemCopy;
extern pMemCopy32 MemCopy32;
extern pResetUsb ResetUsb;
extern pFlashInternal FlashInternal;
extern pFlashExitXip FlashExitXip;
extern pFlashErase RomFlashErase;
extern pFlashProgram RomFlashProgram;
extern pFlashFlush FlashFlush;
extern pFlashEnterXip FlashEnterXip;

// lookup function (returns 32-bit pointer into ROM or NULL if not found)
typedef void* (*pTableLookup)(u16* table, u16 code);

// find ROM function given by the code (returns NULL if not found)
void* RomFunc(u16 code);

// find ROM data given by the code (returns NULL if not found)
void* RomData(u16 code);

// get boot rom version (1 for RP2040-B0, 2 for RP2040-B1, 3 for RP2040-B2)
//    Boot rom version 1 does not contain double library and some float functions.
INLINE u8 RomGetVersion(void) { return *(u8*)0x13; }

// initialize ROM functions
void RomFncInit(void);

// === ROM functions initialized by RomFncInit()

// counts '1' bits
// u8 popcount(u32 val);

// reverse 32-bit value
// u32 reverse(u32 val);

// count leading zeros (takes 170 ns)
// u8 clz(u32 val);

// count trailing zeros
// u8 ctz(u32 val);

// fill memory (returns dst; takes 4 us per 1 KB)
//  For large data use DMA_MemFill with speed 2 us per 1 KB.
// u8* MemSet(u8* dst, u8 data, u32 len);

// fill memory aligned to u32 (returns dst; takes 4 us per 1 KB)
//  For large data use DMA_MemFill with speed 2 us per 1 KB.
// u32* MemSet32(u32* dst, u8 data, u32 len);

// copy memory (returns dst; takes 7 us per 1 KB for aligned data, 57 us for unaligned data)
//  For large data use DMA_MemCopy with speed 2 us per 1 KB for aligned data, 8 us for unaligned data.
//  - do not use on overlapped memory areas
// u8* MemCopy(u8* dst, const u8* src, u32 len);

// copy memory aligned to u32 (returns dst; takes 7 us per 1 KB)
//  For large data use DMA_MemCopy with speed 2 us per 1 KB for aligned data, 8 us for unaligned data.
//  - do not use on overlapped memory areas
// u32* MemCopy32(u32* dst, const u32* src, u32 len);

// reset CPU to BOOTSEL mode
//  gpio = mask of pins used as indicating LED during mass storage activity
//  interface = 0 both interfaces (as cold boot), 1 only USB PICOBOOT, 2 only USB Mass Storage
// void __attribute__((noreturn)) ResetUsb(u32 gpio, u32 interface);

// restore QSPI to default and connect SSI to QSPI pads
// void FlashInternal();

// exit XIP mode to SSI
// void FlashExitXip();

// erase flash (ROM internal, use function FlashErase instead)
//  addr = start address to erase (offset from start of flash XIP_BASE; must be aligned to 4 KB FLASH_SECTOR_SIZE)
//  count = number of bytes to erase (must be multiple of 4 KB FLASH_SECTOR_SIZE)
//  block = block size to erase larger block, use default 64 KB FLASH_BLOCK_SIZE
//  cmd = block erase command, to erase large block, use default 0xD8 FLASH_BLOCK_ERASE_CMD
// void RomFlashErase(u32 addr, u32 count, u32 block, u8 cmd);

// program flash (ROM internal, use function FlashProgram instead)
//  addr = start address to program (offset from start of flash XIP_BASE; must be aligned to 256 B FLASH_PAGE_SIZE)
//  data = data to program
//  count = number of bytes to program (must be multiple of 256 B FLASH_PAGE_SIZE)
// void RomFlashProgram(u32 addr, const u8* data, u32 count);

// flush flash cache
// void FlashFlush();

// enter XIP mode
// void FlashEnterXip();

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// prepare bootrom lookup code based on two ASCII characters (RP2040 datasheet page 156)
INLINE u32 rom_table_code(u8 ch1, u8 ch2) { return ROM_TABLE_CODE((u32)ch1, (u32)ch2); }

// find ROM function given by the code (returns NULL if not found)
INLINE void* rom_func_lookup(u32 code) { return RomFunc((u16)code); }

// find ROM data given by the code (returns NULL if not found)
INLINE void* rom_data_lookup(u32 code) { return RomData((u16)code); }

// Helper function to lookup the addresses of multiple bootrom functions
// This method looks up the 'codes' in the table, and convert each table entry to the looked up
// function pointer, if there is a function for that code in the bootrom.
//   table ... an IN/OUT array, elements are codes on input, function pointers on success.
//   count ... the number of elements in the table
// return true if all the codes were found, and converted to function pointers, false otherwise
bool rom_funcs_lookup(u32* table, uint count);

// lookup function (returns 32-bit pointer into ROM or NULL if not found)
typedef void* (*rom_table_lookup_fn)(u16* table, u32 code);

// Convert a 16 bit pointer stored at the given rom address into a 32 bit pointer
#define rom_hword_as_ptr(addr) ROM_HWORD_PTR(addr)

// find ROM function given by the code INLINE (returns NULL if not found)
INLINE void* rom_func_lookup_inline(u32 code)
{
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(0x18);
	u16* tab = (u16*)ROM_HWORD_PTR(0x14);
	return lookup(tab, code);
}

// reset CPU to BOOTSEL mode
//  gpio = mask of pins used as indicating LED during mass storage activity
//  interface = 0 both interfaces (as cold boot), 1 only USB PICOBOOT, 2 only USB Mass Storage
INLINE void __attribute__((noreturn)) reset_usb_boot(u32 gpio, u32 interface) { ResetUsb(gpio, interface); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_BOOTROM_H
