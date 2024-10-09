
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
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_bootrom.h"

/* Start of ROM

0x0000: .word _stacktop // MSP (initial boot stack pointer - invalid unused entry)
0x0004: .word _start    // Reset (pointer to ARM reset handler ... ARM entry point)
0x0008: .word _nmi      // NMI (pointer to ARM NMI handler)
0x000C: .word _dead     // HardFault (pointer to ARM hard fault handler)

0x0010: .byte 'M', 'u'	// magic
0x0012: .byte 1		// bootrom ROM major version (1=RP2040, 2=RP2350)
0x0013: .byte 2		// bootrom ROM minor version (RP2040: 1..3, RP2350: old 1 with 32-bit pointers, now used 2 with 16-bit pointers)

0x0014: .hword function_table		// pointer to entry table BOOTROM_ROMTABLE_START
0x0016: .hword data_table		// pointer to ARM helper function arm8_table_lookup_val+1
0x0018: .hword table_lookup + 1		// pointer to ARM helper function arm8_table_lookup_entry+1

0x7DF6: .hword 				// pointer to entry table BOOTROM_ROMTABLE_START
0x7DF8: .hword 				// pointer to RISC-V helper function riscv_table_lookup_val
0x7DFA: .hword 				// pointer to RISC-V helper function riscv_table_lookup_entry
0x7DFC: .word 				// RISC-V entry point riscv_entry_point
*/

#if RP2040
// RP2040 ROM function pointers
rom_popcount32_fn		popcount;
rom_reverse32_fn		reverse;
rom_clz32_fn			clz;
rom_ctz32_fn			ctz;
rom_memset_fn			MemSet;
rom_memset4_fn			MemSet32;
rom_memcpy_fn			MemCopy;
rom_memcpy44_fn			MemCopy32;
rom_reset_usb_boot_fn		RomResetUsb;
rom_wait_for_vector		WaitForVector;
#endif // RP2040

// common ROM function pointers
rom_connect_internal_flash_fn	FlashInternal;			// restore flash interface to default (connect_internal_flash)
rom_flash_exit_xip_fn		FlashExitXip;			// exit flash XIP mode (flash_exit_xip)
rom_flash_range_erase_fn	RomFlashErase;			// erase flash (flash_range_erase)
rom_flash_range_program_fn	RomFlashProgram;		// program flash (flash_range_program)
rom_flash_flush_cache_fn	FlashFlush;			// flush flash cache (flash_flush_cache)
rom_flash_enter_cmd_xip_fn	FlashEnterXip;			// enter flash standard XIP mode, clkdiv=12 (flash_enter_cmd_xip)

#if RP2350
// RP2350 ROM function pointers
rom_reboot_fn				RomReboot;		// reboot (reboot)
rom_bootrom_state_reset_fn		RomStateReset;		// resets internal bootrom state (bootrom_state_reset)
rom_flash_reset_address_trans_fn	RomFlashResetTrans;	// reset flash address translation (flash_reset_address_trans)
rom_flash_select_xip_read_mode_fn	RomFlashSelectMode;	// select XIP read mode (flash_select_xip_read_mode)
rom_get_sys_info_fn			RomGetSysInfo;		// get system info (get_sys_info)
rom_get_partition_table_info_fn		RomGetPartInfo;		// get partition table info (get_partition_table_info)
rom_explicit_buy_fn			RomExplicitBuy;		// perform "explicit" buy of executable (explicit_buy)
rom_validate_ns_buffer_fn		RomValidateNsBuf;	// validate buffer from non-secure code (validate_ns_buffer)
rom_set_rom_callback_fn			RomSetCallback;		// set callback for RomSecureCall (set_rom_callback)
rom_chain_image_fn			RomChainImage;		// searches memory region and run (chain_image)
rom_load_partition_table_fn		RomLoadPartTab;		// load current partition table (load_partition_table)
rom_pick_ab_partition_fn		RomPickABPart;		// pick A or B partition (pick_ab_partition)
rom_get_b_partition_fn			RomGetBPart;		// get index of B partition (get_b_partition)
rom_get_uf2_target_partition_fn		RomGetUF2Part;		// get UF2 partition (get_uf2_target_partition)
rom_func_otp_access_fn			RomOtpAccess;		// write/read data into OTP (otp_access)
rom_flash_runtime_to_storage_addr_fn	RomFlashTrans;		// perform address translation (flash_runtime_to_storage_addr)
rom_flash_op_fn				RomFlashOp;		// perform flash operation (flash_op)
#if RISCV
rom_set_bootrom_stack_fn		RomSetStack;		// set bootrom stack (set_bootrom_stack)
#else // RISCV
rom_set_ns_api_permission_fn		RomSetNsApi;		// allow/disallow specific NS API (set_ns_api_permission)
rom_func_secure_call			RomSecureCall;		// call secure method from non-secure code (secure_call)
#endif // RISCV
#endif // RP2350

// find ROM function given by the code (returns NULL if not found)
void* RomFunc(u32 code)
{
#if RP2040

	// prepare pointer to lookup function
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(BOOTROM_LOOKUP);

	// prepare pointer to table with functions
	u16* tab = (u16*)ROM_HWORD_PTR(BOOTROM_FUNC_TABLE);

	// search function
	return lookup(tab, code);

#else // RP2350

#if RISCV

	// prepare pointer to lookup function
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(BOOTROM_LOOKUP_ENTRY);

	// search function (on RISC-V the code (jmp) is stored directly in the table)
	return lookup(code, 0x01); // 0x01 = search RISC-V function

#else // RP2350 ARM

	// prepare pointer to lookup function
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(BOOTROM_LOOKUP);

	// search function (on ARM the function poiner is stored in the table)
	if (NonSecure()) // is CPU in non-secure state?
		return lookup(code, 0x10); // 0x10 = search ARM non-secure function
	else
		return lookup(code, 0x04); // 0x04 = search ARM secure function
#endif
#endif
}

// find ROM data given by the code (returns NULL if not found)
void* RomData(u32 code)
{
#if RP2040

	// prepare pointer to lookup function
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(BOOTROM_LOOKUP);

	// prepare pointer to table with data
	u16* tab = (u16*)ROM_HWORD_PTR(BOOTROM_DATA_TABLE);

	// search data
	return lookup(tab, code);

#else // RP2350

	// prepare pointer to lookup function
	pTableLookup lookup = (pTableLookup)(u32)ROM_HWORD_PTR(BOOTROM_LOOKUP);

	// search data
	return lookup(code, 0x40); // 0x40 = search data

#endif
}

// initialize ROM functions
void RomFncInit()
{
#if RP2040
	// RP2040 ROM function pointers
	popcount	= (rom_popcount32_fn	)(u32)RomFunc(ROM_FUNC_POPCOUNT32	);	// counts '1' bits           
	reverse		= (rom_reverse32_fn	)(u32)RomFunc(ROM_FUNC_REVERSE32	);	// reverse order of bits     
	clz		= (rom_clz32_fn		)(u32)RomFunc(ROM_FUNC_CLZ32		);	// count leading zeros       
	ctz		= (rom_ctz32_fn		)(u32)RomFunc(ROM_FUNC_CTZ32		);	// count trailing zeros      
	MemSet		= (rom_memset_fn	)(u32)RomFunc(ROM_FUNC_MEMSET		);	// fill memory               
	MemSet32	= (rom_memset4_fn	)(u32)RomFunc(ROM_FUNC_MEMSET4		);	// fill memory aligned to u32
	MemCopy		= (rom_memcpy_fn	)(u32)RomFunc(ROM_FUNC_MEMCPY		);	// copy memory               
	MemCopy32	= (rom_memcpy44_fn	)(u32)RomFunc(ROM_FUNC_MEMCPY44		);	// copy memory aligned to u32
	RomResetUsb	= (rom_reset_usb_boot_fn)(u32)RomFunc(ROM_FUNC_RESET_USB_BOOT	);	// reset CPU to BOOTSEL mode 
	WaitForVector	= (rom_wait_for_vector  )(u32)RomFunc(ROM_FUNC_WAIT_FOR_VECTOR  );
#endif // RP2040

	// common ROM function pointers
	FlashInternal		= (rom_connect_internal_flash_fn)(u32)RomFunc(ROM_FUNC_CONNECT_INTERNAL_FLASH	);	// restore flash interface to default (connect_internal_flash)
	FlashExitXip		= (rom_flash_exit_xip_fn	)(u32)RomFunc(ROM_FUNC_FLASH_EXIT_XIP		);     	// exit flash XIP mode (flash_exit_xip)
	RomFlashErase		= (rom_flash_range_erase_fn	)(u32)RomFunc(ROM_FUNC_FLASH_RANGE_ERASE	);     	// erase flash (flash_range_erase)
	RomFlashProgram		= (rom_flash_range_program_fn	)(u32)RomFunc(ROM_FUNC_FLASH_RANGE_PROGRAM	);   	// program flash (flash_range_program)
	FlashFlush		= (rom_flash_flush_cache_fn	)(u32)RomFunc(ROM_FUNC_FLASH_FLUSH_CACHE	);     	// flush flash cache (flash_flush_cache)
	FlashEnterXip		= (rom_flash_enter_cmd_xip_fn	)(u32)RomFunc(ROM_FUNC_FLASH_ENTER_CMD_XIP	);   	// enter flash standard XIP mode, clkdiv=12 (flash_enter_cmd_xip)

#if RP2350
	// RP2350 ROM function pointers
	RomReboot		= (rom_reboot_fn			)(u32)RomFunc(ROM_FUNC_REBOOT				);	// reboot (reboot)
	RomStateReset		= (rom_bootrom_state_reset_fn		)(u32)RomFunc(ROM_FUNC_BOOTROM_STATE_RESET		);	// resets internal bootrom state (bootrom_state_reset)
	RomFlashResetTrans	= (rom_flash_reset_address_trans_fn	)(u32)RomFunc(ROM_FUNC_FLASH_RESET_ADDRESS_TRANS	);	// reset flash address translation (flash_reset_address_trans)
	RomFlashSelectMode	= (rom_flash_select_xip_read_mode_fn	)(u32)RomFunc(ROM_FUNC_FLASH_SELECT_XIP_READ_MODE	);	// select XIP read mode (flash_select_xip_read_mode)
	RomGetSysInfo		= (rom_get_sys_info_fn			)(u32)RomFunc(ROM_FUNC_GET_SYS_INFO			);	// get system info (get_sys_info)
	RomGetPartInfo		= (rom_get_partition_table_info_fn	)(u32)RomFunc(ROM_FUNC_GET_PARTITION_TABLE_INFO		);	// get partition table info (get_partition_table_info)
	RomExplicitBuy		= (rom_explicit_buy_fn			)(u32)RomFunc(ROM_FUNC_EXPLICIT_BUY			);	// perform "explicit" buy of executable (explicit_buy)
	RomValidateNsBuf	= (rom_validate_ns_buffer_fn		)(u32)RomFunc(ROM_FUNC_VALIDATE_NS_BUFFER		);	// validate buffer from non-secure code (validate_ns_buffer)
	RomSetCallback		= (rom_set_rom_callback_fn		)(u32)RomFunc(ROM_FUNC_SET_ROM_CALLBACK			);	// set callback for RomSecureCall (set_rom_callback)
	RomChainImage		= (rom_chain_image_fn			)(u32)RomFunc(ROM_FUNC_CHAIN_IMAGE			);	// searches memory region and run (chain_image)
	RomLoadPartTab		= (rom_load_partition_table_fn		)(u32)RomFunc(ROM_FUNC_LOAD_PARTITION_TABLE		);	// load current partition table (load_partition_table)
	RomPickABPart		= (rom_pick_ab_partition_fn		)(u32)RomFunc(ROM_FUNC_PICK_AB_PARTITION		);	// pick A or B partition (pick_ab_partition)
	RomGetBPart		= (rom_get_b_partition_fn		)(u32)RomFunc(ROM_FUNC_GET_B_PARTITION			);	// get index of B partition (get_b_partition)
	RomGetUF2Part		= (rom_get_uf2_target_partition_fn	)(u32)RomFunc(ROM_FUNC_GET_UF2_TARGET_PARTITION		);	// get UF2 partition (get_uf2_target_partition)
	RomOtpAccess		= (rom_func_otp_access_fn		)(u32)RomFunc(ROM_FUNC_OTP_ACCESS			);	// write/read data into OTP (otp_access)
	RomFlashTrans		= (rom_flash_runtime_to_storage_addr_fn	)(u32)RomFunc(ROM_FUNC_FLASH_RUNTIME_TO_STORAGE_ADDR	);	// perform address translation (flash_runtime_to_storage_addr)
	RomFlashOp		= (rom_flash_op_fn			)(u32)RomFunc(ROM_FUNC_FLASH_OP				);	// perform flash operation (flash_op)
#if RISCV
	RomSetStack		= (rom_set_bootrom_stack_fn		)(u32)RomFunc(ROM_FUNC_SET_BOOTROM_STACK		);	// set bootrom stack (set_bootrom_stack)
#else // RISCV
	RomSetNsApi		= (rom_set_ns_api_permission_fn		)(u32)RomFunc(ROM_FUNC_SET_NS_API_PERMISSION		);	// allow/disallow specific NS API (set_ns_api_permission)
	RomSecureCall		= (rom_func_secure_call			)(u32)RomFunc(ROM_FUNC_SECURE_CALL			);	// call secure method from non-secure code (secure_call)
#endif // RISCV
#endif // RP2350
}

// reset CPU to BOOTSEL mode
//  gpio = mask of pins used as indicating LED during mass storage activity
//  interface = 0 both interfaces (as cold boot), 1 only USB PICOBOOT, 2 only USB Mass Storage
#if RP2350
void ResetUsb(u32 gpio, u32 interface)
{
	if (gpio != 0) // LED is used
	{
		interface |= BOOTSEL_FLAG_GPIO_PIN_SPECIFIED;
		gpio = __builtin_ctz(gpio); // get index of the LED
	}
	RomReboot(REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL | REBOOT2_FLAG_NO_RETURN_ON_SUCCESS, 10, interface, gpio);
}
#endif

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
