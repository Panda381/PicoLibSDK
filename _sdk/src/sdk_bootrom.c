
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

#include "../../global.h"	// globals
#include "../inc/sdk_bootrom.h"

/* Start of ROM
__vectors:
0x0000: .word _stacktop // MSP
0x0004: .word _start    // Reset
0x0008: .word _nmi      // NMI
0x000C: .word _dead     // HardFault

_magic:
0x0010: .byte 'M', 'u'
# compatibility version
0x0012: .byte 1		// bootrom compatibility
# ROM version
0x0013: .byte 2		// bootrom version (1..3)

_well_known:
0x0014: .hword function_table
0x0016: .hword data_table
0x0018: .hword table_lookup + 1
*/

// ROM function pointers (60 bytes)
pPopCount	popcount;
pReverse	reverse;
pClz		clz;
pCtz		ctz;
pMemSet		MemSet;
pMemSet32	MemSet32;
pMemCopy	MemCopy;
pMemCopy32	MemCopy32;
pResetUsb	ResetUsb;
pFlashInternal	FlashInternal;
pFlashExitXip	FlashExitXip;
pFlashErase	RomFlashErase;
pFlashProgram	RomFlashProgram;
pFlashFlush	FlashFlush;
pFlashEnterXip	FlashEnterXip;

// find ROM function given by the code (returns NULL if not found)
void* RomFunc(u16 code)
{
	// prepare pointer to lookup function (.hword table_lookup + 1)
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(0x18);

	// prepare pointer to table with functions (.hword function_table)
	u16* tab = (u16*)ROM_HWORD_PTR(0x14);

	// search function
	return lookup(tab, code);
}

// find ROM data given by the code (returns NULL if not found)
void* RomData(u16 code)
{
	// prepare pointer to lookup function (.hword table_lookup + 1)
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(0x18);

	// prepare pointer to table with data (.hword data_table)
	u16* tab = (u16*)ROM_HWORD_PTR(0x16);

	// search function
	return lookup(tab, code);
}

// initialize ROM functions
void RomFncInit()
{
	popcount = (pPopCount)(u32)RomFunc(ROM_FUNC_POPCOUNT32);
	reverse = (pReverse)(u32)RomFunc(ROM_FUNC_REVERSE32);
	clz = (pClz)(u32)RomFunc(ROM_FUNC_CLZ32);
	ctz = (pCtz)(u32)RomFunc(ROM_FUNC_CTZ32);
	MemSet = (pMemSet)(u32)RomFunc(ROM_FUNC_MEMSET);
	MemSet32 = (pMemSet32)(u32)RomFunc(ROM_FUNC_MEMSET4);
	MemCopy = (pMemCopy)(u32)RomFunc(ROM_FUNC_MEMCPY);
	MemCopy32 = (pMemCopy32)(u32)RomFunc(ROM_FUNC_MEMCPY44);
	ResetUsb = (pResetUsb)(u32)RomFunc(ROM_FUNC_RESET_USB_BOOT);
	FlashInternal = (pFlashInternal)(u32)RomFunc(ROM_FUNC_CONNECT_INTERNAL_FLASH);
	FlashExitXip = (pFlashExitXip)(u32)RomFunc(ROM_FUNC_FLASH_EXIT_XIP);
	RomFlashErase = (pFlashErase)(u32)RomFunc(ROM_FUNC_FLASH_RANGE_ERASE);
	RomFlashProgram = (pFlashProgram)(u32)RomFunc(ROM_FUNC_FLASH_RANGE_PROGRAM);
	FlashFlush = (pFlashFlush)(u32)RomFunc(ROM_FUNC_FLASH_FLUSH_CACHE);
	FlashEnterXip = (pFlashEnterXip)(u32)RomFunc(ROM_FUNC_FLASH_ENTER_CMD_XIP);
}

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Helper function to lookup the addresses of multiple bootrom functions
// This method looks up the 'codes' in the table, and convert each table entry to the looked up
// function pointer, if there is a function for that code in the bootrom.
//   table ... an IN/OUT array, elements are codes on input, function pointers on success.
//   count ... the number of elements in the table
// return true if all the codes were found, and converted to function pointers, false otherwise
bool rom_funcs_lookup(u32* table, uint count)
{
	uint i;
	bool ok = true;
	for (i = 0; i < count; i++)
	{
		table[i] = (u32)rom_func_lookup(table[i]);
		if (table[i] == 0) ok = false;
	}
	return ok;
}

#endif // USE_ORIGSDK
