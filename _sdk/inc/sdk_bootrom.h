
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

// Boot ROM tables
#if RP2040

#define BOOTROM_FUNC_TABLE	0x14	// function_table: u16 2-char symbol, u16 pointer to function
#define BOOTROM_DATA_TABLE	0x16	// data_table: u16 2-char symbol, u16 pointer to data
#define BOOTROM_LOOKUP		0x18	// table_lookup: R0 = pointer to table, R1 = 2-char symbol, returns pointer from table or NULL

#else // RP2350
#if RISCV // RP2350 RISC-V

#define BOOTROM_ENTRY		0x7DFC	// Boot ROM entry vector
#define BOOTROM_ROMTABLE	(BOOTROM_ENTRY-6) // ROM table BOOTROM_ROMTABLE_START: u16 2-char symbol, u16 flags, u16 data
#define BOOTROM_LOOKUP		(BOOTROM_ENTRY-4) // arm8_table_lookup_val: R0 = 2-char symbol, R1 = flag query, returns data from table or NULL
#define BOOTROM_LOOKUP_ENTRY	(BOOTROM_ENTRY-2) // arm8_table_lookup_entry: R0 = 2-char symbol, R1 = flag query, returns pointer to entry or NULL

#else // RP2350 ARM

#define BOOTROM_ROMTABLE	0x14	// ROM table BOOTROM_ROMTABLE_START: u16 2-char symbol, u16 flags, u16 data
#define BOOTROM_LOOKUP		0x16	// arm8_table_lookup_val: R0 = 2-char symbol, R1 = flag query, returns data from table or NULL
#define BOOTROM_LOOKUP_ENTRY	0x18	// arm8_table_lookup_entry: R0 = 2-char symbol, R1 = flag query, returns pointer to entry or NULL

#endif
#endif

// prepare bootrom lookup code based on two ASCII characters (RP2040 datasheet page 156)
#define ROM_TABLE_CODE(ch1, ch2) ((u8)(ch1) | ((u16)(u8)(ch2) << 8))

#if RP2040
// RP2040 ROM functions
#define ROM_FUNC_POPCOUNT32			ROM_TABLE_CODE('P', '3')	// counts '1' bits
#define ROM_FUNC_REVERSE32			ROM_TABLE_CODE('R', '3')	// reverse order of bits
#define ROM_FUNC_CLZ32				ROM_TABLE_CODE('L', '3')	// count leading zeros
#define ROM_FUNC_CTZ32				ROM_TABLE_CODE('T', '3')	// count trailing zeros
#define ROM_FUNC_MEMSET				ROM_TABLE_CODE('M', 'S')	// fill memory
#define ROM_FUNC_MEMSET4			ROM_TABLE_CODE('S', '4')	// fill memory aligned to u32
#define ROM_FUNC_MEMCPY				ROM_TABLE_CODE('M', 'C')	// copy memory
#define ROM_FUNC_MEMCPY44			ROM_TABLE_CODE('C', '4')	// copy memory aligned to u32
#define ROM_FUNC_RESET_USB_BOOT			ROM_TABLE_CODE('U', 'B')	// reset CPU to BOOTSEL mode
#define ROM_FUNC_WAIT_FOR_VECTOR		ROM_TABLE_CODE('W', 'V')	// wait core 1 for launch

#define ROM_DATA_COPYRIGHT			ROM_TABLE_CODE('C', 'R')
#endif // RP2040

// common ROM functions
#define ROM_FUNC_CONNECT_INTERNAL_FLASH		ROM_TABLE_CODE('I', 'F')	// restore flash interface to default
#define ROM_FUNC_FLASH_EXIT_XIP			ROM_TABLE_CODE('E', 'X')	// exit flash XIP mode
#define ROM_FUNC_FLASH_RANGE_ERASE		ROM_TABLE_CODE('R', 'E')	// erase flash
#define ROM_FUNC_FLASH_RANGE_PROGRAM		ROM_TABLE_CODE('R', 'P')	// program flash
#define ROM_FUNC_FLASH_FLUSH_CACHE		ROM_TABLE_CODE('F', 'C')	// flush flash cache
#define ROM_FUNC_FLASH_ENTER_CMD_XIP		ROM_TABLE_CODE('C', 'X')	// enter flash XIP mode

#define ROM_DATA_SOFTWARE_GIT_REVISION		ROM_TABLE_CODE('G', 'R')	// bootrom git revision

#if RP2350
// RP2350 ROM functions
#define ROM_FUNC_REBOOT                         ROM_TABLE_CODE('R', 'B')	// reboot
#define ROM_FUNC_BOOTROM_STATE_RESET            ROM_TABLE_CODE('S', 'R')
#define ROM_FUNC_FLASH_RESET_ADDRESS_TRANS      ROM_TABLE_CODE('R', 'A')
#define ROM_FUNC_FLASH_SELECT_XIP_READ_MODE     ROM_TABLE_CODE('X', 'M')
#define ROM_FUNC_GET_SYS_INFO                   ROM_TABLE_CODE('G', 'S')
#define ROM_FUNC_GET_PARTITION_TABLE_INFO	ROM_TABLE_CODE('G', 'P')
#define ROM_FUNC_EXPLICIT_BUY			ROM_TABLE_CODE('E', 'B')
#define ROM_FUNC_VALIDATE_NS_BUFFER             ROM_TABLE_CODE('V', 'B')
#define ROM_FUNC_SET_ROM_CALLBACK               ROM_TABLE_CODE('R', 'C')
#define ROM_FUNC_CHAIN_IMAGE			ROM_TABLE_CODE('C', 'I')
#define ROM_FUNC_LOAD_PARTITION_TABLE           ROM_TABLE_CODE('L', 'P')
#define ROM_FUNC_PICK_AB_PARTITION		ROM_TABLE_CODE('A', 'B')
#define ROM_FUNC_GET_B_PARTITION		ROM_TABLE_CODE('G', 'B')
#define ROM_FUNC_GET_UF2_TARGET_PARTITION       ROM_TABLE_CODE('G', 'U')
#define ROM_FUNC_OTP_ACCESS                     ROM_TABLE_CODE('O', 'A')
#define ROM_FUNC_FLASH_RUNTIME_TO_STORAGE_ADDR	ROM_TABLE_CODE('F', 'A')
#define ROM_FUNC_FLASH_OP			ROM_TABLE_CODE('F', 'O')
#define ROM_FUNC_SET_BOOTROM_STACK              ROM_TABLE_CODE('S', 'S')
#define ROM_FUNC_SET_NS_API_PERMISSION          ROM_TABLE_CODE('S', 'P')
#define ROM_FUNC_SECURE_CALL                    ROM_TABLE_CODE('S', 'C')

#define ROM_DATA_FLASH_DEVINFO16_PTR		ROM_TABLE_CODE('F', 'D')	// pointer to flash device info
#define ROM_DATA_PARTITION_TABLE_PTR            ROM_TABLE_CODE('P', 'T')	// pointer to partition table info
#define ROM_DATA_SAVED_XIP_SETUP_FUNC_PTR       ROM_TABLE_CODE('X', 'F')	// pointer xip_setup_func_ptr
#endif // RP2350

// Convert 16-bit pointer stored at the given rom address into 32-bit pointer
#define ROM_HWORD_PTR(addr) (void*)(u32)(*(u16*)(u32)(addr))

// Bootrom XIP mode
#define BOOTROM_XIP_MODE_03H_SERIAL	0
#define BOOTROM_XIP_MODE_0BH_SERIAL	1
#define BOOTROM_XIP_MODE_BBH_DUAL	2
#define BOOTROM_XIP_MODE_EBH_QUAD	3
#define BOOTROM_XIP_MODE_N_MODES	4

#if RP2040
// RP2040 ROM function prototypes
typedef u32 (*rom_popcount32_fn)(u32);				// counts '1' bits
typedef u32 (*rom_reverse32_fn)(u32);				// reverse order of bits
typedef u32 (*rom_clz32_fn)(u32);				// count leading zeros
typedef u32 (*rom_ctz32_fn)(u32);				// count trailing zeros
typedef void* (*rom_memset_fn)(void*, u8, u32);			// fill memory
typedef u32* (*rom_memset4_fn)(u32*, u8, u32);			// fill memory aligned to u32
typedef void* (*rom_memcpy_fn)(void*, const void*, u32);		// copy memory
typedef u32* (*rom_memcpy44_fn)(u32*, const u32*, u32);		// copy memory aligned to u32
typedef /*__attribute__((noreturn))*/ void (*rom_reset_usb_boot_fn)(u32, u32); // reset CPU to BOOTSEL mode
typedef void (*rom_wait_for_vector)();				// wait core 1 for launch
#endif // RP2040

// common ROM function prototypes
typedef void (*rom_connect_internal_flash_fn)(void);		// restore flash interface to default (connect_internal_flash)
typedef void (*rom_flash_exit_xip_fn)(void);			// exit flash XIP mode (flash_exit_xip)
typedef void (*rom_flash_range_erase_fn)(u32, u32, u32, u8);	// erase flash
typedef void (*rom_flash_range_program_fn)(u32, const u8*, u32); // program flash
typedef void (*rom_flash_flush_cache_fn)(void);			// flush flash cache (flash_flush_cache)
typedef void (*rom_flash_enter_cmd_xip_fn)(void);		// enter flash standard XIP mode, clkdiv=12 (flash_enter_cmd_xip)

#if RP2350
// RP2350 ROM function prototypes
typedef int (*bootrom_api_callback_generic_t)(u32 r0, u32 r1, u32 r2);
typedef struct {
	u32	permissions_and_location;
	u32	permissions_and_flags;
} resident_partition_t;
STATIC_ASSERT(sizeof(resident_partition_t) == 8, "Incorrect resident_partition_t!");

typedef int (*rom_reboot_fn)(u32 flags, u32 delay, u32 p0, u32 p1);
typedef void (*rom_bootrom_state_reset_fn)(u32 flags);					// resets internal bootrom state (bootrom_state_reset)
typedef void (*rom_flash_reset_address_trans_fn)(void);
typedef void (*rom_flash_select_xip_read_mode_fn)(u32 mode, u8 clkdiv); 		// mode = BOOTROM_XIP_MODE_*
typedef int (*rom_get_sys_info_fn)(u32* buf, u32 bufsize, u32 flags);
typedef int (*rom_get_partition_table_info_fn)(u32* buf, u32 bufsize, u32 flags);
typedef int (*rom_explicit_buy_fn)(u8* buf, u32 bufsize);				// perform "explicit" buy of executable (explicit_buy)
typedef void* (*rom_validate_ns_buffer_fn)(const void* addr, u32 size, u32 write, u32* ok);
typedef int* (*rom_set_rom_callback_fn)(u32 num, bootrom_api_callback_generic_t cb);
typedef int (*rom_chain_image_fn)(u8* wrk_base, u32 wrk_size, u32 win_base, u32 win_size); // searches memory region and run (chain_image)
typedef int (*rom_load_partition_table_fn)(u8* wrk_base, u32 wrk_size, Bool force);
typedef int (*rom_pick_ab_partition_fn)(u8* wrk_base, u32 wrk_size, uint part_num, u32 update);
typedef int (*rom_get_b_partition_fn)(uint pi_a);
typedef int (*rom_get_uf2_target_partition_fn)(u8* wrk_base, u32 wrk_size, u32 family, resident_partition_t* part);
typedef int (*rom_func_otp_access_fn)(u8* buf, u32 bufsize, u32 cmd);
typedef int* (*rom_flash_runtime_to_storage_addr_fn)(uint* addr);
typedef int (*rom_flash_op_fn)(int flags, uint* addr, u32 size, u8* buf);		// perform flash operation (flash_op)
#if RISCV
typedef struct {
	u32*	base;
	u32	size;
} bootrom_stack_t;
typedef int (*rom_set_bootrom_stack_fn)(bootrom_stack_t* stack);
#else // RISCV
typedef int (*rom_set_ns_api_permission_fn)(uint apinum, Bool allow);
typedef int (*rom_func_secure_call)(uint* a0, ...);
#endif // RISCV
#endif // RP2350

#if RP2040
// RP2040 ROM function pointers
extern rom_popcount32_fn		popcount;
extern rom_reverse32_fn			reverse;
extern rom_clz32_fn			clz;
extern rom_ctz32_fn			ctz;
extern rom_memset_fn			MemSet;
extern rom_memset4_fn			MemSet32;
extern rom_memcpy_fn			MemCopy;
extern rom_memcpy44_fn			MemCopy32;
extern rom_reset_usb_boot_fn		RomResetUsb;
extern rom_wait_for_vector		WaitForVector;			// reset core 1 (another alternative: Core1ExitBootrom())
#endif // RP2040

// common ROM function pointers
extern rom_connect_internal_flash_fn	FlashInternal;			// restore flash interface to default (connect_internal_flash)
extern rom_flash_exit_xip_fn		FlashExitXip;			// exit flash XIP mode (flash_exit_xip)
extern rom_flash_range_erase_fn		RomFlashErase;			// erase flash (flash_range_erase)
extern rom_flash_range_program_fn	RomFlashProgram;		// program flash (flash_range_program)
extern rom_flash_flush_cache_fn		FlashFlush;			// flush flash cache (flash_flush_cache)
extern rom_flash_enter_cmd_xip_fn	FlashEnterXip;			// enter flash standard XIP mode, clkdiv=12 (flash_enter_cmd_xip)

#if RP2350
// RP2350 ROM function pointers
extern rom_reboot_fn				RomReboot;		// reboot (reboot)
extern rom_bootrom_state_reset_fn		RomStateReset;		// resets internal bootrom state (bootrom_state_reset)
extern rom_flash_reset_address_trans_fn		RomFlashResetTrans;	// reset flash address translation (flash_reset_address_trans)
extern rom_flash_select_xip_read_mode_fn	RomFlashSelectMode;	// select XIP read mode (flash_select_xip_read_mode)
extern rom_get_sys_info_fn			RomGetSysInfo;		// get system info (get_sys_info)
extern rom_get_partition_table_info_fn		RomGetPartInfo;		// get partition table info (get_partition_table_info)
extern rom_explicit_buy_fn			RomExplicitBuy;		// perform "explicit" buy of executable (explicit_buy)
extern rom_validate_ns_buffer_fn		RomValidateNsBuf;	// validate buffer from non-secure code (validate_ns_buffer)
extern rom_set_rom_callback_fn			RomSetCallback;		// set callback for RomSecureCall (set_rom_callback)
extern rom_chain_image_fn			RomChainImage;		// searches memory region and run (chain_image)
extern rom_load_partition_table_fn		RomLoadPartTab;		// load current partition table (load_partition_table)
extern rom_pick_ab_partition_fn			RomPickABPart;		// pick A or B partition (pick_ab_partition)
extern rom_get_b_partition_fn			RomGetBPart;		// get index of B partition (get_b_partition)
extern rom_get_uf2_target_partition_fn		RomGetUF2Part;		// get UF2 partition (get_uf2_target_partition)
extern rom_func_otp_access_fn			RomOtpAccess;		// write/read data into OTP (otp_access)
extern rom_flash_runtime_to_storage_addr_fn	RomFlashTrans;		// perform address translation (flash_runtime_to_storage_addr)
extern rom_flash_op_fn				RomFlashOp;		// perform flash operation (flash_op)
#if RISCV
extern rom_set_bootrom_stack_fn			RomSetStack;		// set bootrom stack (set_bootrom_stack)
#else // RISCV
extern rom_set_ns_api_permission_fn		RomSetNsApi;		// allow/disallow specific NS API (set_ns_api_permission)
extern rom_func_secure_call			RomSecureCall;		// call secure method from non-secure code (secure_call)
#endif // RISCV

// Fill memory (returns destination address)
INLINE void* MemSet(void* dst, int val, u32 size) { return memset(dst, val, size); }
INLINE void* MemSet32(void* dst, int val, u32 size) { return memset(dst, val, size); }

// Copy memory in upward direction (returns destination address)
INLINE void* MemCopy(void* dst, const void* src, u32 size) { return memcpy(dst, src, size); }
INLINE void* MemCopy32(void* dst, const void* src, u32 size) { return memcpy(dst, src, size); }

#endif // RP2350

// lookup function (returns 32-bit pointer into ROM or NULL if not found)
#if RP2040
typedef void* (*pTableLookup)(u16* table, u32 code);
#else // RP2350
typedef void* (*pTableLookup)(u32 code, u32 mask);
#endif

// find ROM function given by the code (returns NULL if not found)
void* RomFunc(u32 code);

// find ROM data given by the code (returns NULL if not found)
void* RomData(u32 code);

// get boot rom full version (low byte = compatibility version, high byte = major version)
#define BOOTROM_FULLVER_RP2040FPGA	0x0100		// RP2040 running on FPGA 48 MHz (unpublished)
#define BOOTROM_FULLVER_RP2040B0	0x0101		// RP2040-B0, no double library in Boot ROM
#define BOOTROM_FULLVER_RP2040B1	0x0102		// RP2040-B1, add double library to Boot ROM
#define BOOTROM_FULLVER_RP2040B2	0x0103		// RP2040-B2, better power regulator, new fsincos
#define BOOTROM_FULLVER_RP2350FPGA	0x0200		// RP2350 running on FPGA 48 MHz (unpublished)
#define BOOTROM_FULLVER_RP2350A1	0x0201		// RP2350-A1, with 32-pointers in Boot ROM (unpublished)
#define BOOTROM_FULLVER_RP2350A2	0x0202		// RP2350-A2, with 16-pointers in Boot ROM
INLINE u16 RomGetFullVersion(void) { return *(u8*)0x13 | ((u16)(*(u8*)0x12)<<8); }

// get boot rom compatibility version
#define BOOTROM_VER_RP2040FPGA		0x00		// RP2040 running on FPGA 48 MHz
#define BOOTROM_VER_RP2040B0		0x01		// RP2040-B0, no double library in Boot ROM
#define BOOTROM_VER_RP2040B1		0x02		// RP2040-B1, add double library to Boot ROM
#define BOOTROM_VER_RP2040B2		0x03		// RP2040-B2, better power regulator, new fsincos
#define BOOTROM_VER_RP2350FPGA		0x00		// RP2350 running on FPGA 48 MHz (unpublished)
#define BOOTROM_VER_RP2350A1		0x01		// RP2350-A1, with 32-pointers in Boot ROM (unpublished)
#define BOOTROM_VER_RP2350A2		0x02		// RP2350-A2, with 16-pointers in Boot ROM
INLINE u8 RomGetVersion(void) { return *(u8*)0x13; }

// initialize ROM functions
void RomFncInit(void);

// Flags of RomReboot function
#if RP2350
#define REBOOT2_FLAG_REBOOT_TYPE_NORMAL       0x0 // param0 = diagnostic partition
#define REBOOT2_FLAG_REBOOT_TYPE_BOOTSEL      0x2 // param0 = bootsel_flags, param1 = gpio_config
#define REBOOT2_FLAG_REBOOT_TYPE_RAM_IMAGE    0x3 // param0 = image_base, param1 = image_end
#define REBOOT2_FLAG_REBOOT_TYPE_FLASH_UPDATE 0x4 // param0 = update_base
#define REBOOT2_FLAG_REBOOT_TYPE_PC_SP        0xd
#define REBOOT2_FLAG_REBOOT_TO_ARM            0x10
#define REBOOT2_FLAG_REBOOT_TO_RISCV          0x20
#define REBOOT2_FLAG_NO_RETURN_ON_SUCCESS     0x100

#define BOOTSEL_FLAG_DISABLE_MSD_INTERFACE      0x01
#define BOOTSEL_FLAG_DISABLE_PICOBOOT_INTERFACE 0x02
#define BOOTSEL_FLAG_GPIO_PIN_ACTIVE_LOW        0x10
#define BOOTSEL_FLAG_GPIO_PIN_SPECIFIED         0x20
#endif

// reset CPU to BOOTSEL mode
//  gpio = mask of pins used as indicating LED during mass storage activity
//  interface = 0 both interfaces (as cold boot), 1 only USB PICOBOOT, 2 only USB Mass Storage
#if RP2040
INLINE void ResetUsb(u32 gpio, u32 interface) { RomResetUsb(gpio, interface); }
#else
void ResetUsb(u32 gpio, u32 interface);
#endif

// === ROM functions initialized by RomFncInit()

// Only RP2040
// -----------
// counts '1' bits
// u32 popcount(u32 val);

// reverse 32-bit value
// u32 reverse(u32 val);

// count leading zeros
// u32 clz(u32 val);

// count trailing zeros
// u32 ctz(u32 val);

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

// reset CPU to BOOTSEL mode (call common function ResetUsb instead)
//  gpio = mask of pins used as indicating LED during mass storage activity
//  interface = 0 both interfaces (as cold boot), 1 only USB PICOBOOT, 2 only USB Mass Storage
// void /*__attribute__((noreturn))*/ RomResetUsb(u32 gpio, u32 interface);

// exit core 1 to BOOTROM (another alternative: Core1ExitBootrom())
// void __attribute__((noreturn)) WaitForVector();

// Common functions to RP2040 and RP2350
// -------------------------------------
// restore flash interface to default
// void FlashInternal();

// exit flash XIP mode
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

// enter flash standard XIP mode
// void FlashEnterXip();

// Only RP2350
// -----------
// Reboot (see 'reboot' function in datasheet for details) (call common function ResetUsb instead)
//  flags = REBOOT2_FLAG_*
//  p0 = BOOTSEL_FLAG_*
// int RomReboot(u32 flags, u32 delay, u32 p0, u32 p1);

// Reset internal bootrom state (see 'bootrom_state_reset' function in datasheet for details)
// void RomStateReset(u32 flags);

// Reset flash address translation (see 'flash_reset_address_trans' function in datasheet for details)
// void RomFlashResetTrans();

// Select XIP read mode (see 'flash_select_xip_read_mode' function in datasheet for details)
// void RomFlashSelectMode(u32 mode, u8 clkdiv);

// Set system info (see 'get_sys_info' function in datasheet for details)
// int RomGetSysInfo(u32* buf, u32 bufsize, u32 flags);

// Get partition table info (see 'get_partition_table_info' function in datasheet for details)
// int RomGetPartInfo(u32* buf, u32 bufsize, u32 flags);

// Perform "explicit" buy of executable (see 'explicit_buy' function in datasheet for details)
// int RomExplicitBuy(u8* buf, u32 bufsize);

// Validate buffer from non-secure code (see 'validate_ns_buffer' function in datasheet for details)
// void* RomValidateNsBuf(const void* addr, u32 size, u32 write, u32* ok);

// Set callback for RomSecureCall (see 'set_rom_callback' function in datasheet for details)
// int* RomSetCallback(u32 num, bootrom_api_callback_generic_t cb);

// Searches memory region and run (see 'chain_image' function in datasheet for details)
// int RomChainImage(u8* wrk_base, u32 wrk_size, u32 win_base, u32 win_size);

// Load current partition table (see 'load_partition_table' function in datasheet for details)
// int RomLoadPartTab(u8* wrk_base, u32 wrk_size, Bool force);

// Pick A or B partition (see 'pick_ab_partition' function in datasheet for details)
// int RomPickABPart(u8* wrk_base, u32 wrk_size, uint part_num, u32 update);

// Get index of B partition (see 'get_b_partition' function in datasheet for details)
// int RomGetBPart(uint pi_a);

// Get UF2 partition (see 'get_uf2_target_partition' function in datasheet for details)
// int RomGetUF2Part(u8* wrk_base, u32 wrk_size, u32 family, resident_partition_t* part);

// Write/read data into OTP (see 'otp_access' function in datasheet for details)
// int RomOtpAccess(u8* buf, u32 bufsize, u32 cmd);

// Perform address translation (see 'flash_runtime_to_storage_addr' function in datasheet for details)
// int* RomFlashTrans(uint* addr);

// Perform flash operation (see 'flash_op' function in datasheet for details)
// int RomFlashOp(int flags, uint* addr, u32 size, u8* buf);

// Only RP2350 RISC-V
// ------------------
// Set bootrom stack (see 'set_bootrom_stack' function in datasheet for details)
// int RomSetStack(bootrom_stack_t* stack);

// Only RP2350 ARM
// ---------------
// Allow/disallow specific NS API (see 'set_ns_api_permission' function in datasheet for details)
// int RomSetNsApi(uint apinum, Bool allow);

// Call secure method from non-secure code (see 'secure_call' function in datasheet for details)
// int RomSecureCall(uint* a0, ...);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// prepare bootrom lookup code based on two ASCII characters (RP2040 datasheet page 156)
INLINE u32 rom_table_code(u8 ch1, u8 ch2) { return ROM_TABLE_CODE((u32)ch1, (u32)ch2); }

// find ROM function given by the code (returns NULL if not found)
INLINE void* rom_func_lookup(u32 code) { return RomFunc(code); }

// find ROM data given by the code (returns NULL if not found)
INLINE void* rom_data_lookup(u32 code) { return RomData(code); }

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
INLINE void* rom_func_lookup_inline(u32 code) { return RomFunc(code); }

// reset CPU to BOOTSEL mode
//  gpio = mask of pins used as indicating LED during mass storage activity
//  interface = 0 both interfaces (as cold boot), 1 only USB PICOBOOT, 2 only USB Mass Storage
INLINE void /*__attribute__((noreturn))*/ reset_usb_boot(u32 gpio, u32 interface) { ResetUsb(gpio, interface); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_BOOTROM_H
