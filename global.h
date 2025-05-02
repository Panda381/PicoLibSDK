
// ****************************************************************************
//                                 
//                         Global common C definitions
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

#ifndef _GLOBAL_H
#define _GLOBAL_H

#define SDK_VER		208	// SDK library version in hundredths

// ----------------------------------------------------------------------------
//                               Attributes
// ----------------------------------------------------------------------------

// count of array entries
#define count_of(a) (sizeof(a)/sizeof((a)[0]))

// I/O port prefix
#define __IO	volatile

// request to use inline
#define INLINE __attribute__((always_inline)) inline

// avoid to use inline
#define NOINLINE __attribute__((noinline))

// weak function
#define WEAK __attribute__((weak))

// align array to 4-bytes
#define ALIGNED __attribute__((aligned(4)))

// packed structure, to allow access to unaligned entries
#define PACKED __attribute__ ((packed))

// place time critical function into RAM
#define NOFLASH(fnc) NOINLINE __attribute__((section(".time_critical." #fnc))) fnc

// fast function optimization
#define FASTCODE __attribute__ ((optimize("-Ofast")))

// small function optimization
#define SMALLCODE __attribute__ ((optimize("-Os")))

// wrapper function (to replace standard library function)
#define WRAPPER_FUNC(x) __wrap_ ## x

// change endian of u16 (little endian LSB MSB = Intel, big endian MSB LSB = Motorola)
#define ENDIAN16(k) ( (((k)>>8)&0xff) | (((k)&0xff)<<8) )

// clamp float value to valid range 0 to 1
#define CLAMP01(val) ( ((val)<=0) ? 0 : ( ((val)>=1) ? 1 : (val) ) )

// clamp value to u8 range
#define CLAMPU8(val) ( ((val)<=0) ? 0 : ( ((val)>=255) ? 255 : (val) ) )

// absolute value
#define ABS(val) ( ((val)<0) ? -(val) : (val) )

// maximal value
#define MAX(val1,val2) ( ((val1)<(val2)) ? (val2) : (val1) )

// minimal value
#define MIN(val1,val2) ( ((val1)<(val2)) ? (val1) : (val2) )

// default LED pin on Raspberry Pico
#ifndef LED_PIN
#define LED_PIN 25
#endif

// compile-time check
#ifdef __cplusplus
#define STATIC_ASSERT(c, msg) static_assert((c), msg)
#else
#define STATIC_ASSERT(c, msg) _Static_assert((c), msg)
#endif

// ----------------------------------------------------------------------------
//                              Base data types
// ----------------------------------------------------------------------------

/* Basic data types and their sizes (in bytes) depending on the system:

		8-bit	16-bit	32-bit	64-bit
char		1	1	1	1
short		1	1	2	2
int		2	2	4	4
long		4	4	4	4,8
long long	8	8	8	8
*/

// Note: 'char' can be signed or unsigned

// base types
typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;		// on 8-bit system use "signed int"
typedef unsigned short u16;		// on 8-bit system use "unsigned int"
typedef signed long int s32;		// on 64-bit system use "signed int"
typedef unsigned long int u32;		// on 64-bit system use "unsigned int"
typedef signed long long int s64;
typedef unsigned long long int u64;

typedef unsigned int uint;

// original-SDK base types
#ifndef _INT8_T_DECLARED
typedef s8 int8_t;
#define _INT8_T_DECLARED
#endif

#ifndef _UINT8_T_DECLARED
typedef u8 uint8_t;
#define	_UINT8_T_DECLARED
#endif

#ifndef _INT16_T_DECLARED
typedef s16 int16_t;
#define	_INT16_T_DECLARED
#endif

#ifndef _UINT16_T_DECLARED
typedef u16 uint16_t;
#define	_UINT16_T_DECLARED
#endif

#ifndef _INT32_T_DECLARED
typedef s32 int32_t;
#define	_INT32_T_DECLARED
#endif

#ifndef _UINT32_T_DECLARED
typedef u32 uint32_t;
#define	_UINT32_T_DECLARED
#endif

#ifndef _INT64_T_DECLARED
typedef s64 int64_t;
#define	_INT64_T_DECLARED
#endif

#ifndef _UINT64_T_DECLARED
typedef u64 uint64_t;
#define	_UINT64_T_DECLARED
#endif

#ifndef _UINTPTR_T_DECLARED
typedef unsigned long uintptr_t;
#define _UINTPTR_T_DECLARED
#endif

#ifndef _INTPTR_T_DECLARED
typedef long intptr_t;
#define _INTPTR_T_DECLARED
#endif

// optimised Bool
//  Note: Why is a Bool type of boolean variable used? Two types of boolean variables
//  are commonly used. In Windows notation, a boolean variable is called BOOL with
//  the values TRUE and FALSE. It is based on the C language, where the boolean value
//  FALSE is considered to be the zero value of an integer number, and TRUE is
//  considered to be a non-zero value. This type of logical variable requires a variable
//  of size int, that is, typically 32 bits. In C++, the bool logical variable was
//  introduced with the values true and false. It is a bit variable with a typical size
//  of 1 byte and values of 0 and 1. The bool variable is more memory efficient than the
//  BOOL type, but it is more demanding for the processor - it requires permanent
//  conversion of the result of operations to the value 0 or 1. The Bool type variable
//  is an optimal compromise between the two methods. It uses a variable of size 1 byte,
//  treats a value of False as a zero value and a non-zero value as True. This method is
//  memory-efficient while being fast and efficient for the processor.
typedef unsigned char Bool;
#define True 1
#define False 0

// original-SDK bool (C++ internal bool)
#ifndef __cplusplus
typedef unsigned char bool;
#endif
#define true 1
#define false 0

// original-SDK macro
#define __aligned(x) __attribute__((aligned(x)))
#define __noinline __attribute__((noinline))
#define __not_in_flash(group) __attribute__((section(".time_critical." #group)))
#define __not_in_flash_func(func_name) __not_in_flash(__STRING(func_name)) func_name
#define __no_inline_not_in_flash_func(func_name) __noinline __not_in_flash_func(func_name)
#define __time_critical_func(func_name) __not_in_flash_func(func_name)
#define __uninitialized_ram(group) __attribute__((section(".uninitialized_data." #group))) group
#define __scratch_x(group) __attribute__((section(".time_critical." #group)))
#define __scratch_y(group) __attribute__((section(".time_critical." #group)))

#ifndef assert
#define assert(x)
#endif

// hw registers
typedef volatile u32 io32;
typedef volatile u16 io16;
typedef volatile u8 io8;

// original-SDK types
typedef volatile uint32_t io_rw_32;
typedef const volatile uint32_t io_ro_32;
typedef volatile uint32_t io_wo_32;
typedef volatile uint16_t io_rw_16;
typedef const volatile uint16_t io_ro_16;
typedef volatile uint16_t io_wo_16;
typedef volatile uint8_t io_rw_8;
typedef const volatile uint8_t io_ro_8;
typedef volatile uint8_t io_wo_8;
typedef volatile uint8_t *const ioptr;
typedef ioptr const const_ioptr;

// NULL
#ifndef NULL
#ifdef __cplusplus
#define NULL 0
#else
#define NULL ((void*)0)
#endif
#endif

// original-SDK define unsigned number
#ifndef _u
#ifdef __ASSEMBLER__
#define _u(x) x
#else
#define _u(x) x ## u
#endif
#endif

// original-SDK absolute time entry in [us]
typedef u64 absolute_time_t;

// original-SDK datetime; set field to -1 for RTC alarm to not match
typedef struct {
	s16	year;	// 0..4095
	s8	month;	// 1..12, 1 is January
	s8	day;	// 1..28,29,30,31 depending on month
	s8	dotw;	// 0..6, 0 is Sunday
	s8	hour;	// 0..23
	s8	min;	// 0..59
	s8	sec;	// 0..59
} datetime_t;

STATIC_ASSERT(sizeof(datetime_t) == 8, "Incorrect datetime_t!");

// BMP file header (size 70 bytes)
#pragma pack(push,1)
typedef struct {
	// BMP file header (size 14 bytes)
	u16	bfType;			// 0x00: magic, 'B' 'M' = 0x4D42
	u32	bfSize;			// 0x02: file size, aligned to DWORD = 70 + 320*240*2 + 2 = 153672 = 0x25848
	u16	bfReserved1;		// 0x06: = 0
	u16	bfReserved2;		// 0x08: = 0
	u32	bfOffBits;		// 0x0A: offset of data bits after file header = 70 (0x46)
	// BMP info header (size 40 bytes)
	u32	biSize;			// 0x0E: size of this info header = 56 (0x38)
	s32	biWidth;		// 0x12: width = 320 (0x140)
	s32	biHeight;		// 0x16: height, negate if flip row order = -240 (0xFFFFFF10)
	u16	biPlanes;		// 0x1A: planes = 1
	u16	biBitCount;		// 0x1C: number of bits per pixel = 16
	u32	biCompression;		// 0x1E: compression = 3 (BI_BITFIELDS)
	u32	biSizeImage;		// 0x22: size of data of image + aligned file = 320*240*2 + 2 = 153602 (0x25802)
	s32	biXPelsPerMeter;	// 0x26: X pels per meter = 2834 (= 0xB12)
	s32	biYPelsPerMeter;	// 0x2A: Y pels per meter = 2834 (= 0xB12)
	u32	biClrUsed;		// 0x2E: number of user colors (0 = all)
	u32	biClrImportant;		// 0x32: number of important colors (0 = all)
	// BMP color bit mask (size 16 bytes)
	u32	biRedMask;		// 0x36: red mask = 0x0000F800
	u32	biGreenMask;		// 0x3A: green mask = 0x000007E0
	u32	biBlueMask;		// 0x3E: blue mask = 0x0000001F
	u32	biAlphaMask;		// 0x42: alpha mask = 0x00000000
					// 0x46
} sBmp;
#pragma pack(pop)

STATIC_ASSERT(sizeof(sBmp) == 0x46, "Incorrect sBmp!");

// original-SDK error codes
enum pico_error_codes {
	PICO_OK = 0,
	PICO_ERROR_NONE = 0,
	PICO_ERROR_TIMEOUT = -1,
	PICO_ERROR_GENERIC = -2,
	PICO_ERROR_NO_DATA = -3,
	PICO_ERROR_NOT_PERMITTED = -4,
	PICO_ERROR_INVALID_ARG = -5,
	PICO_ERROR_IO = -6,
	PICO_ERROR_BADAUTH = -7,
	PICO_ERROR_CONNECT_FAILED = -8,
};

// compile-time check 
STATIC_ASSERT(sizeof(s8) == 1, "Incorrect typedef s8!");
STATIC_ASSERT(sizeof(u8) == 1, "Incorrect typedef u8!");
STATIC_ASSERT(sizeof(s16) == 2, "Incorrect typedef s16!");
STATIC_ASSERT(sizeof(u16) == 2, "Incorrect typedef u16!");
STATIC_ASSERT(sizeof(s32) == 4, "Incorrect typedef s32!");
STATIC_ASSERT(sizeof(u32) == 4, "Incorrect typedef u32!");
STATIC_ASSERT(sizeof(s64) == 8, "Incorrect typedef s64!");
STATIC_ASSERT(sizeof(u64) == 8, "Incorrect typedef u64!");

extern const u8* __etext;	// end of .text section
extern const u8* __data_start__; // start of initialize .data section
extern const u8* __data_end__; // end of initialize .data section

// ----------------------------------------------------------------------------
//                               Constants
// ----------------------------------------------------------------------------

#define	B0 (1<<0)
#define	B1 (1<<1)
#define	B2 (1<<2)
#define	B3 (1<<3)
#define	B4 (1<<4)
#define	B5 (1<<5)
#define	B6 (1<<6)
#define	B7 (1<<7)
#define	B8 (1U<<8)
#define	B9 (1U<<9)
#define	B10 (1U<<10)
#define	B11 (1U<<11)
#define	B12 (1U<<12)
#define	B13 (1U<<13)
#define	B14 (1U<<14)
#define	B15 (1U<<15)
#define B16 (1UL<<16)
#define B17 (1UL<<17)
#define B18 (1UL<<18)
#define	B19 (1UL<<19)
#define B20 (1UL<<20)
#define B21 (1UL<<21)
#define B22 (1UL<<22)
#define B23 (1UL<<23)
#define B24 (1UL<<24)
#define B25 (1UL<<25)
#define B26 (1UL<<26)
#define B27 (1UL<<27)
#define B28 (1UL<<28)
#define B29 (1UL<<29)
#define B30 (1UL<<30)
#define B31 (1UL<<31)

#define BIT(pos) (1UL<<(pos))
#define BIT64(pos) (1ULL<<(pos))

#define	BIGINT	0x40000000 // big int value

#define PI 3.14159265358979324
#define PI2 (3.14159265358979324*2)

#define KHZ 1000	// kHz multiply
#define MHZ 1000000	// MHz multiply

// ----------------------------------------------------------------------------
//                               Constants
// ----------------------------------------------------------------------------

// Control characters (ASCII characters)
#define CH_NUL		0x00	// '\0'  NUL null ^@			... no character, end of text
#define CH_ALL		0x01	// '\1'  SOH start of heading ^A	... select [A]ll
#define CH_BLOCK	0x02	// '\2'  STX start of text ^B		... mark [B]lock
#define CH_STX		CH_BLOCK
#define CH_COPY		0x03	// '\3'  ETX end of text ^C		... [C]opy block, copy file
#define CH_ETX		CH_COPY
#define CH_END		0x04	// '\4'  EOT end of transmission ^D	... en[D] of row, end of files
#define CH_MOVE		0x05	// '\5'  ENQ enquiry ^E			... rename files, mov[E] block
#define CH_FIND		0x06	// '\6'  ACK acknowledge ^F		... [F]ind
#define CH_NEXT		0x07	// '\a'  BEL bell (alert) ^G		... [G]o next, repeat find
#define CH_BS		0x08	// '\b'  BS backspace ^H		... backspace
#define CH_TAB		0x09	// '\t'  HT horizontal tabulator ^I	... tabulator
#define CH_LF		0x0A	// '\n'  LF line feed ^J		... line feed
#define CH_PGUP		0x0B	// '\v'  VT vertical tabulator ^K	... page up
#define CH_PGDN		0x0C	// '\f'  FF form feed ^L		... page down
#define CH_FF		CH_PGDN
#define CH_CR		0x0D	// '\r'  CR carriage return ^M		... enter, next row, run file
#define CH_NEW		0x0E	// '\16' SO shift-out ^N		... [N]ew file
#define CH_OPEN		0x0F	// '\17' SI shift-in ^O			... [O]pen file, edit file
#define CH_PRINT	0x10	// '\20' DLE data link escape ^P	... [P]rint file, send file
#define CH_QUERY	0x11	// '\21' DC1 device control 1 ^Q	... [Q]uery, display help
#define CH_REPLACE	0x12	// '\22' DC2 device control 2 ^R	... find and [R]eplace
#define CH_SAVE		0x13	// '\23' DC3 device control 3 ^S	... [S]ave file
#define CH_INS		0x14	// '\24' DC4 device control 4 ^T	... [T]oggle Insert switch, mark file
#define CH_HOME		0x15	// '\25' NAK negative acknowledge ^U	... Home, begin of row, begin of files
#define CH_PASTE	0x16	// '\26' SYN synchronous idle ^V	... paste from clipboard
#define CH_SYN		CH_PASTE
#define CH_CLOSE	0x17	// '\27' ETB end of transmission block ^W ... close file
#define CH_CUT		0x18	// '\30' CAN cancel ^X			... cut selected text
#define CH_REDO		0x19	// '\31' EM end of medium ^Y		... redo previously undo action
#define CH_UNDO		0x1A	// '\32' SUB subtitle character ^Z	... undo action
#define CH_ESC		0x1B	// '\e' or '\33' ESC escape ^[		... Esc, break, menu
#define CH_RIGHT	0x1C	// '\34' FS file separator "^\"		... Right, Shift: End, Ctrl: Word right
#define CH_UP		0x1D	// '\35' GS group separator ^]		... Up, Shift: PageUp, Ctrl: Text start
#define CH_LEFT		0x1E	// '\36' RS record separator ^^		... Left, Shift: Home, Ctrl: Word left
#define CH_DOWN		0x1F	// '\37' US unit separator ^_		... Down, Shift: PageDn, Ctrl: Text end
#define CH_SPC		0x20	// SPC space

#define CH_DEL		0x7F	// '\x7F' delete			... delete character on cursor, Ctrl: delete block, delete file

#define NOCHAR		0	// no character
#define NOKEY		0	// no key from keyboard

// ----------------------------------------------------------------------------
//                           Real numbers control
// ----------------------------------------------------------------------------

// angle unit
#define UNIT_DEG	0	// degrees
#define UNIT_RAD	1	// radians
#define UNIT_GRAD	2	// grads

// radix numeric base
#define BASE_DEC	0	// decimal
#define BASE_BIN	1	// binary
#define BASE_OCT	2	// octal
#define BASE_HEX	3	// hexadecimal

//extern unsigned char Base;		// current numeric radix base BASE_*

// exponent mode
#define EXP_AUTO	0	// auto mode
#define EXP_FIX		1	// fixed mode (no exponent on small numbers)
#define EXP_EE		2	// exponent mode
#define EXP_ENG		3	// engineering technical mode (exponent multiply of 3)

// rounding
#define FIX_OFF		-1	// fix decimals are off

#define EDITBUF_MAX	3000	// size of edit buffer (without terminating 0)
						// - should be big enough to decode BIN format, use MAX_DIG*4

// ----------------------------------------------------------------------------
//                           Base Includes
// ----------------------------------------------------------------------------

// project configuration
#include "config.h"		// project configuration
#include <string.h>		// memcpy
#include <stdarg.h>		// va_list
#include <math.h>		// HUGE_VAL

// ----------------------------------------------------------------------------
//                 Colors (must be used after "config.h")
// ----------------------------------------------------------------------------

#if !USE_DRAWCAN		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)

// 4-bit color YRGB1111
#if COLBITS == 4

// - base colors
#define COL_BLACK	COLOR4(0,0,0,0)
#define COL_BLUE	COLOR4(0,0,0,1)
#define COL_GREEN	COLOR4(0,0,1,0)
#define COL_CYAN	COLOR4(0,0,1,1)
#define COL_RED		COLOR4(0,1,0,0)
#define COL_MAGENTA	COLOR4(0,1,0,1)
#define COL_YELLOW	COLOR4(0,1,1,0)
#define COL_GRAY	COLOR4(0,1,1,1)
#define COL_WHITE	COLOR4(1,1,1,1)
// - dark colors
#define COL_DKBLUE	COLOR4(0,0,0,1)
#define COL_DKGREEN	COLOR4(0,0,1,0)
#define COL_DKCYAN	COLOR4(0,0,1,1)
#define COL_DKRED	COLOR4(0,1,0,0)
#define COL_DKMAGENTA	COLOR4(0,1,0,1)
#define COL_DKYELLOW	COLOR4(0,1,1,0)
#define COL_DKWHITE	COLOR4(0,1,1,1)
#define COL_DKGRAY	COLOR4(1,0,0,0)
// - light colors
#define COL_LTBLUE	COLOR4(1,0,0,1)
#define COL_LTGREEN	COLOR4(1,0,1,0)
#define COL_LTCYAN	COLOR4(1,0,1,1)
#define COL_LTRED	COLOR4(1,1,0,0)
#define COL_LTMAGENTA	COLOR4(1,1,0,1)
#define COL_LTYELLOW	COLOR4(1,1,1,0)
#define COL_LTGRAY	COLOR4(1,1,1,1)

#define COL_AZURE	COLOR4(1,0,1,1)
#define COL_ORANGE	COLOR4(0,1,1,0)

// other colors
#else // COLBITS

// - base colors
#define COL_BLACK	COLOR(  0,  0,  0)
#define COL_BLUE	COLOR(  0,  0,255)
#define COL_GREEN	COLOR(  0,255,  0)
#define COL_CYAN	COLOR(  0,255,255)
#define COL_RED		COLOR(255,  0,  0)
#define COL_MAGENTA	COLOR(255,  0,255)
#define COL_YELLOW	COLOR(255,255,  0)
#define COL_WHITE	COLOR(255,255,255)
#define COL_GRAY	COLOR(128,128,128)
// - dark colors
#define COL_DKBLUE	COLOR(  0,  0,127)
#define COL_DKGREEN	COLOR(  0,127,  0)
#define COL_DKCYAN	COLOR(  0,127,127)
#define COL_DKRED	COLOR(127,  0,  0)
#define COL_DKMAGENTA	COLOR(127,  0,127)
#define COL_DKYELLOW	COLOR(127,127,  0)
#define COL_DKWHITE	COLOR(127,127,127)
#define COL_DKGRAY	COLOR( 64, 64, 64)
// - light colors
#define COL_LTBLUE	COLOR(127,127,255)
#define COL_LTGREEN	COLOR(127,255,127)
#define COL_LTCYAN	COLOR(127,255,255)
#define COL_LTRED	COLOR(255,127,127)
#define COL_LTMAGENTA	COLOR(255,127,255)
#define COL_LTYELLOW	COLOR(255,255,127)
#define COL_LTGRAY	COLOR(192,192,192)

#define COL_AZURE	COLOR(  0,127,255)
#define COL_ORANGE	COLOR(255,127,  0)

#endif // COLBITS

#endif // !USE_DRAWCAN

#define COL_PRINT_DEF	COL_GRAY		// default console print color

// convert 24-bit RGB to 16-bit color
#define RGBTO16(r,g,b)	( (((r)&0xf8)<<8) | (((g)&0xfc)<<3) | (((b)&0xf8)>>3) )

// convert 24-bit RGB to 15-bit color
#define RGBTO15(r,g,b)	( (((r)&0xf8)<<7) | (((g)&0xf8)<<2) | (((b)&0xf8)>>3) )

// blend 16-bit colors with 25%, 50% and 75% ratio
//  @TODO: Fast, but not precise (may be darker with missing lowest bit)
#define RGBBLEND25(a,b) ((((a)>>2) & ~(B3|B4|B9|B10|B14|B15)) + (((b)>>1) & ~(B4|B10|B15)) + (((b)>>2) & ~(B3|B4|B9|B10|B14|B15)))
#define RGBBLEND50(a,b) ((((a)>>1) & ~(B4|B10|B15)) + (((b)>>1) & ~(B4|B10|B15)))
#define RGBBLEND75(a,b) ((((a)>>1) & ~(B4|B10|B15)) + (((a)>>2) & ~(B3|B4|B9|B10|B14|B15)) + (((b)>>2) & ~(B3|B4|B9|B10|B14|B15)))

#define RGBBLEND4(a,b,c,d) ((((a)>>2) & ~(B3|B4|B9|B10|B14|B15)) + (((b)>>2) & ~(B3|B4|B9|B10|B14|B15)) + (((c)>>2) & ~(B3|B4|B9|B10|B14|B15)) + (((d)>>2) & ~(B3|B4|B9|B10|B14|B15)))

#endif // _GLOBAL_H
