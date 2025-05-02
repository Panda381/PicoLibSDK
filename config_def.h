
// ****************************************************************************
//                                 
//                           Default Library Configuration
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

/*
Default system resources
------------------------
DMA 10 and 11 are reserved for temporary operations (DMA_MemCopy, CrcxxDMA),
	but they can be used temporary by user program, too. Search: UARTSAMPLE_TXDMA,
	UARTSAMPLE_RXDMA, DMA_TEMP_CHAN.
DMA channels 8 and 9 are used by miniVGA library.
DMA channels 2 to 7 are used by DVI (HDMI) library.

PIO 1 is used by miniVGA and DVI (HDMI) library.

Hardware interpolators are used when drawing to canvas (DRAW_HWINTER_INX) and
by VGA driver rendering service.

*/

#ifndef _CONFIG_DEF_H
#define _CONFIG_DEF_H

/* Options from command line:

RP2040		// 1=use MCU RP2040
RP2350		// 1=use MCU RP235xx
RP2350A		// 1=use MCU RP235xA (60 pin)
RP2350B		// 1=use MCU RP235xB (80 pin)

FLASHSIZE	// Flash size in bytes (2 MB, 4 MB, 8 MB, 16 MB)
RAMSIZE		// RAM base size in bytes (256 KB or 512 KB)

*/

// ----------------------------------------------------------------------------
//                      Device custom configutation
// ----------------------------------------------------------------------------

#if USE_DEMOVGA			// use DemoVGA device configuration
#include "_devices/demovga/_config.h"
#endif

#if USE_PICOTRON		// use Picotron device configuration
#include "_devices/picotron/_config.h"
#endif

#if USE_PICOINO			// use Picoino device configuration
#include "_devices/picoino/_config.h"
#endif

#if USE_PICOPAD			// use PicoPad device configuration
#include "_devices/picopad/_config.h"
#endif

#if USE_PICO			// use Pico device configuration
#include "_devices/pico/_config.h"
#endif

#if USE_PC
#define ASM64		1		// 1 = use assembler x64 optimization, if available
#endif

#ifndef USE_SCREENSHOT			// use screen shots
#define USE_SCREENSHOT	0		// use screen shots
#endif

// ----------------------------------------------------------------------------
//                        Spinlock resources
// ----------------------------------------------------------------------------

// Timer spinlock
#ifndef PICO_SPINLOCK_ID_TIMER
#define PICO_SPINLOCK_ID_TIMER 14	// timer spinlock
#endif

// SpinLock for printf function
#ifndef PRINTF_SPIN
#define PRINTF_SPIN	15		// printf spinlock
#endif

// Spinlocks 16 to 21 can be allocated by the SpinNextStriped() function
// - striped spinlocks (indexed with round robin)
#ifndef PICO_SPINLOCK_ID_STRIPED_FIRST
#define PICO_SPINLOCK_ID_STRIPED_FIRST 16	// first striped spinlock
#endif

#ifndef PICO_SPINLOCK_ID_STRIPED_LAST
#define PICO_SPINLOCK_ID_STRIPED_LAST 21	// last striped spinlock
#endif

// Spinlocks 22 to 27 can be allocated by the SpinClaimFree() function (auto-claimed spinlocks)
#ifndef PICO_SPINLOCK_ID_CLAIM_FREE_FIRST
#define PICO_SPINLOCK_ID_CLAIM_FREE_FIRST 22	// first free spinlock
#endif

#ifndef PICO_SPINLOCK_ID_CLAIM_FREE_LAST
#define PICO_SPINLOCK_ID_CLAIM_FREE_LAST 27	// last free spinlock
#endif

// Spinlocks 28 and 29 are used by UART test sample and by USART stdio. Search:
// UARTSAMPLE_TXSPIN, UARTSAMPLE_RXSPIN, UART_STDIO_TXSPIN, UART_STDIO_RXSPIN.
#ifndef UART_STDIO_TXSPIN
#define UART_STDIO_TXSPIN	28	// transmitter spinlock 0..31 (-1 = not used)
#endif

#ifndef UARTSAMPLE_TXSPIN
#define UARTSAMPLE_TXSPIN	28	// transmitter spinlock 0..31 (-1 = not used)
#endif

#ifndef UART_STDIO_RXSPIN
#define UART_STDIO_RXSPIN	29	// receiver spinlock 0..31 (-1 = not used)
#endif

#ifndef UARTSAMPLE_RXSPIN
#define UARTSAMPLE_RXSPIN	29	// receiver spinlock 0..31 (-1 = not used)
#endif

// Spinlock 30 is used by USB driver. Search USB_SPIN.
#ifndef USB_SPIN
#define USB_SPIN	30		// USB spinlock, -1=do not use (ring buffer, event buffer, claim endpoints)
#endif

// Spinlock 31 is used by system memory allocator, safe integer etc. Search SYS_SPIN.
#ifndef SYS_SPIN
#define SYS_SPIN	31		// system spinlock (memory allocator, safe integer etc.; -1 = not used)
#endif

// ----------------------------------------------------------------------------
//              SDK modules, folder _sdk (1=use, 0=do not use)
// ----------------------------------------------------------------------------

#ifndef USE_ORIGSDK
#define USE_ORIGSDK	1		// include interface of original-SDK
#endif

#ifndef USE_ADC
#define USE_ADC		1		// use ADC controller (sdk_adc.c, sdk_adc.h)
#endif

#ifndef USE_DMA
#define USE_DMA		1		// use DMA controller (sdk_dma.c, sdk_dma.h)
#endif

#ifndef USE_DOUBLE
#define USE_DOUBLE	1		// use Double-floating point 1=in RAM, 2=in Flash (sdk_double.c, sdk_double_asm.S, sdk_double.h)
#endif

#ifndef USE_FATALERROR
#define USE_FATALERROR	0		// use fatal error message 0=no, 1=display LCD message (sdk_fatal.c, sdk_fatal.h)
#endif

#ifndef USE_FIFO
#define USE_FIFO	1		// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)
#endif

#ifndef USE_FLASH
#define USE_FLASH	1		// use Flash memory programming (sdk_flash.c, sdk_flash.h)
#endif

#ifndef USE_FLOAT
#define USE_FLOAT	1		// use Single-floating point 1=in RAM, 2=in Flash (sdk_float.c, sdk_float_asm.S, sdk_float.h)
#endif

#ifndef USE_GPIOCOPROC
#if RISCV || RP2040
#define	USE_GPIOCOPROC	0		// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
#else
#define	USE_GPIOCOPROC	1		// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
#endif
#endif

#ifndef USE_HSTX
#define USE_HSTX	1		// use HSTX (sdk_hstx.c, sdk_hstx.h)
#endif

#ifndef USE_I2C
#define USE_I2C		1		// use I2C interface (sdk_i2c.c, sdk_i2c.h)
#endif

#ifndef USE_INTERP
#define USE_INTERP	1		// use interpolator (sdk_interp.c, sdk_interp.h)
#endif

#ifndef USE_IRQ
#define USE_IRQ		1		// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
#endif

#ifndef USE_MULTICORE
#define USE_MULTICORE	1		// use Multicore (sdk_multicore.c, sdk_multicore.h)
#endif

#ifndef USE_PIO
#define USE_PIO		1		// use PIO (sdk_pio.c, sdk_pio.h)
#endif

#ifndef USE_PLL
#define USE_PLL		1		// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)
#endif

#ifndef USE_PWM
#define USE_PWM		1		// use PWM (sdk_pwm.c, sdk_pwm.h)
#endif

#ifndef USE_ROSC
#define USE_ROSC	1		// use ROSC ring oscillator (sdk_rosc.c, sdk_rosc.h)
#endif

#ifndef USE_RTC
#define USE_RTC		1		// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)
#endif

#ifndef USE_SHA256
#define USE_SHA256	1		// use SHA-256 accelerator (sdk_sha256.c, sdk_sha256.h)
#endif

#ifndef USE_SPI
#define USE_SPI		1		// use SPI interface (sdk_spi.c, sdk_spi.h)
#endif

#ifndef USE_SPINLOCK
#define USE_SPINLOCK	1		// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)
#endif

#ifndef USE_SYSTICK
#define USE_SYSTICK	1		// use SysTick system timer (sdk_systick.c, sdk_systick.h)
#endif

#ifndef USE_TIMER
#define USE_TIMER	1		// use Timer with alarm (sdk_timer.c, sdk_timer.h)
#endif

#ifndef USE_TMDS
#define USE_TMDS	1		// use TMDS encoder (sdk_tmds.h)
#endif

#ifndef USE_TRNG
#define USE_TRNG	1		// use TRNG true random number generator (sdk_trng.h)
#endif

#ifndef USE_UART
#define USE_UART	1		// use UART serial port (sdk_uart.c, sdk_uart.h)
#endif

#ifndef USE_USBPAD
#define USE_USBPAD	0		// simulate keypad with USB keyboard
#endif

#ifndef USE_USBPORT
#define USE_USBPORT	0		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
#endif

#ifndef USE_USB_STDIO
#define USE_USB_STDIO	0		// use USB stdio (UsbPrint function)
#endif

#ifndef USE_UART_STDIO
#define USE_UART_STDIO	0		// use UART stdio (UartPrint function)
#endif

#ifndef USE_DRAW_STDIO
#define USE_DRAW_STDIO	0		// use DRAW stdio (DrawPrint function)
#endif

#ifndef USE_USB
#define USE_USB		0		// use USB (sdk_usb_*.c, sdk_usb_*.h) (only if not using TinyUSB library)
#endif

#ifndef USE_USB_DEV
#define USE_USB_DEV	0		// use USB Device Class Driver (sdk_usb_dev.c, sdk_usb_dev.h)
#endif

#ifndef USE_USB_DEV_AUDIO
#define USE_USB_DEV_AUDIO 0		// use USB AUDIO Audio device, value = number of interfaces (device)
#endif

#ifndef USE_USB_DEV_BTH
#define USE_USB_DEV_BTH	0		// use USB BTH Bluetooth (device)
#endif

#ifndef USE_USB_DEV_CDC
#define USE_USB_DEV_CDC	0		// use USB CDC Communication Device Class, value = number of interfaces (device)
#endif

#ifndef USE_USB_DEV_DFU
#define USE_USB_DEV_DFU	0		// use USB DFU-RT Device Firmware Upgrade (device)
#endif

#ifndef USE_USB_DEV_HID
#define USE_USB_DEV_HID	0		// use USB HID Human Interface Device, value = number of interfaces (device)
#endif

#ifndef USE_USB_DEV_MIDI
#define USE_USB_DEV_MIDI	0		// use USB MIDI Midi device, value = number of interfaces (device)
#endif

#ifndef USE_USB_DEV_MSC
#define USE_USB_DEV_MSC	0		// use USB MSC Mass Storage Class (device)
#endif

#ifndef USE_USB_DEV_NET
#define USE_USB_DEV_NET	0		// use USB NET Network adapter (device)
#endif

#ifndef USE_USB_DEV_TMC
#define USE_USB_DEV_TMC	0		// use USB TMC Test and Measurement Class (device)
#endif

#ifndef USE_USB_DEV_VENDOR
#define USE_USB_DEV_VENDOR	0		// use USB VENDOR Vendor specific device, value = number of interfaces (device)
#endif

#ifndef USE_USB_DEV_VIDEO
#define USE_USB_DEV_VIDEO	0		// use USB VIDEO Video device, value = number of interfaces (device)
#endif

#ifndef USE_USB_HOST
#define USE_USB_HOST	0		// use USB Host Class Driver (sdk_usb_host.c, sdk_usb_host.h; value = number of devices without HUB devices)
#endif

#ifndef USE_USB_HOST_CDC
#define USE_USB_HOST_CDC 0		// use USB CDC Communication Device Class, value = number of interfaces (host)
#endif

#ifndef USE_USB_HOST_HID
#define USE_USB_HOST_HID	0		// use USB HID Human Interface Device, value = number of interfaces (host)
#endif

#ifndef USE_USB_HOST_HUB
#define USE_USB_HOST_HUB	0		// use USB HUB (host; value = number of HUBs)
#endif

#ifndef USE_USB_HOST_MSC
#define USE_USB_HOST_MSC	0		// use USB MSC Mass Storage Class (host)
#endif

#ifndef USE_USB_HOST_VENDOR
#define USE_USB_HOST_VENDOR	0		// use USB VENDOR Vendor specific device (host)
#endif

#ifndef USE_VREG_LOCKED
#define USE_VREG_LOCKED		0		// 1=enable vreg locked values > 1.30V from function GetVoltageBySysClock() of RP2350
#endif

#ifndef USE_WATCHDOG
#define USE_WATCHDOG	1		// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)
#endif

#ifndef USE_XOSC
#define USE_XOSC	1		// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)
#endif

// ----------------------------------------------------------------------------
//              Library modules, folder _lib (1=use, 0=do not use)
// ----------------------------------------------------------------------------

#ifndef USE_ALARM
#define USE_ALARM	1		// use SysTick alarm (lib_alarm.c, lib_alarm.h)
#endif

#ifndef USE_CALENDAR
#define USE_CALENDAR	1		// use 32-bit calendar (lib_calendar.c, lib_calendar.h)
#endif

#ifndef USE_CALENDAR64
#define USE_CALENDAR64	1		// use 64-bit calendar (lib_calendar64.c, lib_calendar64.h)
#endif

#ifndef USE_CANVAS
#define USE_CANVAS	1		// use Canvas (lib_canvas.c, lib_canvas.h)
#endif

#ifndef USE_COLOR
#define USE_COLOR	1		// use color vector (lib_color.c, lib_color.h)
#endif

#ifndef USE_CONFIG
#define USE_CONFIG	1		// use device configuration (lib_config.c, lib_config.h)
#endif

#ifndef USE_CRC
#define USE_CRC		1		// use CRC Check Sum (lib_crc.c, lib_crc.h)
#endif

#ifndef USE_DECNUM
#define USE_DECNUM	1		// use DecNum (decnum.c, decnum.h)
#endif

#ifndef USE_DISPHSTX
#define USE_DISPHSTX	0		// 1=use HSTX Display driver
#endif

#ifndef USE_DRAW
#define USE_DRAW	0		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#endif

#ifndef USE_DRAWCAN
#define USE_DRAWCAN	0		// 1=use drawing canvas library (lib_drawcan*.c, lib_drawcan*.h)
#endif

#ifndef USE_DRAWCAN0
#define USE_DRAWCAN0	1		// 1=use DrawCan common functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN1
#define USE_DRAWCAN1	1		// 1=use DrawCan1 1-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN2
#define USE_DRAWCAN2	1		// 1=use DrawCan2 2-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN3
#define USE_DRAWCAN3	1		// 1=use DrawCan3 3-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN4
#define USE_DRAWCAN4	1		// 1=use DrawCan4 4-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN6
#define USE_DRAWCAN6	1		// 1=use DrawCan6 6-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN8
#define USE_DRAWCAN8	1		// 1=use DrawCan8 8-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN12
#define USE_DRAWCAN12	1		// 1=use DrawCan12 12-bit functions, if use drawing canvas
#endif

#ifndef USE_DRAWCAN16
#define USE_DRAWCAN16	1		// 1=use DrawCan15/16 15/16-bit functions, if use drawing canvas
#endif

#ifndef USE_ESCPKT
#define USE_ESCPKT	1		// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)
#endif

#ifndef USE_EVENT
#define USE_EVENT	1		// use Event Ring buffer (lib_event.c, lib_event.h)
#endif

#ifndef USE_FAT
#define USE_FAT		0		// use FAT file system (lib_fat.c, lib_fat.h)
#endif

#ifndef USE_FILESEL
#define USE_FILESEL	0		// use file selection (lib_filesel.c, lib_filesel.h)
#endif

#ifndef USE_FRAMEBUF
#define USE_FRAMEBUF	1		// use default display frame buffer
#endif

#ifndef USE_LIST
#define USE_LIST	1		// use Doubly Linked List (lib_list.c, lib_list.h)
#endif

#ifndef USE_MAT2D
#define USE_MAT2D	1		// use 2D transformation matrix (lib_mat2d.c, lib_mat2d.h)
#endif

#ifndef USE_MALLOC
#define USE_MALLOC	1		// use Memory Allocator (lib_malloc.c, lib_malloc.h)
#endif

#ifndef USE_MD5
#define USE_MD5		1		// use MD5 hash - may need to be disabled when colliding with BTStack
#endif

#ifndef USE_MINIRING
#define USE_MINIRING	1		// use Mini-Ring buffer (lib_miniring.c, lib_miniring.h)
#endif

#ifndef USE_MP3
#define USE_MP3		0		// use MP3 decoder (lib_mp3*.c, lib_mp3*.h)
#endif

#ifndef USE_PRINT
#define USE_PRINT	1		// use Formatted print (lib_print.c, lib_print.h)
#endif

#ifndef USE_PWMSND
#define USE_PWMSND	0		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#endif

#ifndef USE_RAND
#define USE_RAND	1		// use Random number generator (lib_rand.c, lib_rand.h)
#endif

#ifndef USE_RECT
#define USE_RECT	1		// use Rectangle (lib_rect.c, lib_rect.h)
#endif

#ifndef USE_RING
#define USE_RING	1		// use Ring buffer (lib_ring.c, lib_ring.h)
#endif

#ifndef USE_RINGRX
#define USE_RINGRX	0		// use Ring buffer with DMA receiver (lib_ringrx.c, lib_ringrx.h)
#endif

#ifndef USE_RINGTX
#define USE_RINGTX	0		// use Ring buffer with DMA transmitter (lib_ringtx.c, lib_ringtx.h)
#endif

#ifndef USE_SD
#define USE_SD		0		// use SD card (lib_sd.c, lib_sd.h)
#endif

#ifndef USE_STREAM
#define USE_STREAM	1		// use Data stream (lib_stream.c, lib_stream.h)
#endif

#ifndef USE_TEXT
#define USE_TEXT	1		// use Text strings, except StrLen and StrComp (lib_text.c, lib_textnum.c, lib_text.h)
#endif

#ifndef USE_TEXTLIST
#define USE_TEXTLIST	1		// use List of text strings (lib_textlist.c, lib_textlist.h)
#endif

#ifndef USE_TPRINT
#define USE_TPRINT	1		// use Text Print (lib_tprint.c, lib_tprint.h)
#endif

#ifndef USE_TREE
#define USE_TREE	1		// use Tree list (lib_tree.c, lib_tree.h)
#endif

#ifndef USE_VIDEO
#define USE_VIDEO	0		// use video player (lib_video.c, lib_video.h)
#endif

#ifndef FONT
#define FONT		FontBold8x16	// default system font
#endif

#ifndef FONTW
#define FONTW		8		// width of system font
#endif

#ifndef FONTH
#define FONTH		16		// height of system font
#endif

// ----------------------------------------------------------------------------
//                     Fast float functions
// ----------------------------------------------------------------------------

#ifndef USE_M33_VFP
#define USE_M33_VFP	1		// 1=enable VFP floating point instructions on RP2350-ARM, 0=emulate with DCP coprocessor
#endif

// float
#ifndef USE_M33_SQRTF
#define USE_M33_SQRTF	1		// RP2350-ARM sqrtf() function: 1=fast (recommended), 2=precise
#endif

#ifndef USE_M33_FDIV
#define USE_M33_FDIV	1		// RP2350-ARM fdiv() function: 1=fast (recommended), 2=precise
#endif

#ifndef USE_RISCV_SQRTF
#define USE_RISCV_SQRTF	1		// RISC-V sqrtf() function: 0=libc, 1=fast (recommended)
#endif

#ifndef USE_RISCV_EXPF
#define USE_RISCV_EXPF	1		// RISC-V expf(), exp2f() and exp10f() function: 0=libc, 1=Chebyshev (recommended), 2=Taylor, 3=Cordic
#endif

#ifndef USE_RISCV_LOGF
#define USE_RISCV_LOGF	1		// RISC-V logf(), log2f() and log10f() function: 0=libc, 1=Chebyshev (recommended), 2=Mercator, 3=Cordic
#endif

#ifndef USE_RISCV_SINF
#define USE_RISCV_SINF	2		// RISC-V sinf(), cosf(), sincosf(), tanf(), cotanf() functions: 0=libc, 1=Chebyshev, 2=Taylor (recommended), 3=Cordic
#endif

#ifndef USE_RISCV_ASINF
#define USE_RISCV_ASINF	2		// RISC-V asinf(), acosf() functions: 0=libc, 2=Taylor (recommended), 3=Cordic
#endif

#ifndef USE_RISCV_ATANF
#define USE_RISCV_ATANF	1		// RISC-V atanf(), acotanf(), atan2f() functions: 0=libc, 1=Chebyshev (recommended), 3=Cordic
#endif

// double
#ifndef USE_M33_SQRT
#define USE_M33_SQRT	1		// RP2350-ARM sqrt() function: 1=fast (recommended), 2=precise
#endif

#ifndef USE_M33_DDIV
#define USE_M33_DDIV	1		// RP2350-ARM ddiv() function: 1=fast (recommended), 2=precise
#endif

// ----------------------------------------------------------------------------
//               Big integers, folder _bigint (1=use, 0=do not use)
// ----------------------------------------------------------------------------

#ifndef USE_BIGINT
#define USE_BIGINT	0 //1		// use Big Integers (bigint.c, bigint.h)
#endif

#ifndef BIGINT_BERN_NUM
#define BIGINT_BERN_NUM	1024		// number of table Bernoulli numbers - use number 0, 256, 512, 768 or 1024
#endif

// ----------------------------------------------------------------------------
//                   REAL numbers  (1=use, 0=do not use)
// ----------------------------------------------------------------------------
// To generate constants with GenConst generator, enable all number types
// to allow more accurate generation of Chebyshev constants.

#ifndef USE_REAL
#define USE_REAL	0		// use real numbers
#endif

#ifndef USE_REAL16
#define USE_REAL16	0		// 1 = use real16 numbers (3 digits, exp +-4)
#endif

#ifndef USE_REAL32
#define USE_REAL32	0		// 1 = use real32 numbers (float, 7 digits, exp +-38)
#endif

#ifndef USE_REAL48
#define USE_REAL48	0		// 1 = use real48 numbers (11 digits, exp +-153)
#endif

#ifndef USE_REAL64
#define USE_REAL64	0		// 1 = use real64 numbers (double, 16 digits, exp +-308)
#endif

#ifndef USE_REAL80
#define USE_REAL80	0		// 1 = use real80 numbers (19 digits, exp +-4932)
#endif

#ifndef USE_REAL96
#define USE_REAL96	0		// 1 = use real96 numbers (25 digits, exp +-1233)
#endif

#ifndef USE_REAL128
#define USE_REAL128	0		// 1 = use real128 numbers (34 digits, exp +-4932)
#endif

#ifndef USE_REAL160
#define USE_REAL160	0		// 1 = use real160 numbers (43 digits, exp +-9864)
#endif

#ifndef USE_REAL192
#define USE_REAL192	0		// 1 = use real192 numbers (52 digits, exp +-19728)
#endif

#ifndef USE_REAL256
#define USE_REAL256	0		// 1 = use real256 numbers (71 digits, exp +-78913)
#endif

#ifndef USE_REAL384
#define USE_REAL384	0		// 1 = use real384 numbers (109 digits, exp +-315652)
#endif

#ifndef USE_REAL512
#define USE_REAL512	0		// 1 = use real512 numbers (147 digits, exp +-1262611)
#endif

#ifndef USE_REAL768
#define USE_REAL768	0		// 1 = use real768 numbers (224 digits, exp +-5050445)
#endif

#ifndef USE_REAL1024
#define USE_REAL1024	0		// 1 = use real1024 numbers (300 digits, exp +-20201781)
#endif

#ifndef USE_REAL1536
#define USE_REAL1536	0		// 1 = use real1536 numbers (453 digits, exp +-161614248)
#endif

#ifndef USE_REAL2048
#define USE_REAL2048	0		// 1 = use real2048 numbers (607 digits, exp +-161614248)
#endif

#ifndef USE_REAL3072
#define USE_REAL3072	0		// 1 = use real3072 numbers (915 digits, exp +-161614248)
#endif

#ifndef USE_REAL4096
#define USE_REAL4096	0		// 1 = use real4096 numbers (1224 digits, exp +-161614248)
#endif

#ifndef USE_REAL6144
#define USE_REAL6144	0		// 1 = use real6144 numbers ( digits, exp +-161614248)
#endif

#ifndef USE_REAL8192
#define USE_REAL8192	0		// 1 = use real8192 numbers ( digits, exp +-161614248)
#endif

#ifndef USE_REAL12288
#define USE_REAL12288	0		// 1 = use real12288 numbers ( digits, exp +-161614248)
#endif

// Limiting the usage of large Chebyshev tables
//  Chebyshev approximations can speed up calculations of mathematical functions.
//  However, the use of Chebyshev approximations for long numbers takes a lot of
//  Flash memory and can be slow due to slow Flash memory access. If needed, set
//  a limit here on the bit length of the numbers for which Chebyshev approximations
//  will be used. Calculations using Chebyshev approximations are not recommended for
//  use in this library. The tables are prepared up to a maximum number size of 1536 bits.
#ifndef MAXCHEB_LN
#define MAXCHEB_LN	0 //1536		// max. REAL number supporting Chebyshev approximations of Ln()
#endif

#ifndef MAXCHEB_EXP
#define MAXCHEB_EXP	0 //1536		// max. REAL number supporting Chebyshev approximations of Exp()
#endif

#ifndef MAXCHEB_SIN
#define MAXCHEB_SIN	0 //1536		// max. REAL number supporting Chebyshev approximations of Sin()
#endif

#ifndef MAXCHEB_ASIN
#define MAXCHEB_ASIN	0 //1536		// max. REAL number supporting Chebyshev approximations of ASin()
#endif

#ifndef MAXCHEB_ATAN
#define MAXCHEB_ATAN	0 //1536		// max. REAL number supporting Chebyshev approximations of ATan()
#endif

#ifndef MAXCHEB_SQRT
#define MAXCHEB_SQRT	0 //1536		// max. REAL number supporting Chebyshev approximations of Sqrt()
#endif

// Limiting the usage of large Cordic tables (see note of Chebyshev tables)
//   Note: Calculations using Cordic are not complete in this library because
//   they have lower precision and they are not recommended for use in this library.
#ifndef MAXCORD_ATAN
#define MAXCORD_ATAN	0 //8192	// max. REAL number supporting Cordic atan table (to calculate sin, cos, tan, asin, acos, atan)
#endif

// Limiting the usage of linear factorials
//  Calculating the linear factorial requires tables that take up much Flash memory.
//  If you need to limit the size of the tables, set a limit here of the bit length of
//  the numbers that will support the linear factorial. The integer factorial
//  can still be used, but it is slow for large numbers.
#ifndef MAXFACT_COEFF
#define MAXFACT_COEFF	8192	// max. REAL number supporting linear factorial
#endif

// ----------------------------------------------------------------------------
//                            Miscellaneous Settings
// ----------------------------------------------------------------------------

#ifndef USE_MEMLOCK
#define USE_MEMLOCK	1		// lock of Memory Allocator 0=no, 1=multicore, 2=multicore+IRQ (lib_malloc.c, lib_malloc.h)
#endif

#ifndef DRAW_HWINTER
#define DRAW_HWINTER	1		// 1=use hardware interpolator to draw images on Canvas
#endif

#ifndef DRAW_HWINTER_INX
#define DRAW_HWINTER_INX 0		// index of hardware interpolator to draw on canvas
#endif

#ifndef TEMP_NOISE
#define TEMP_NOISE	20		// number of steps of noise reduction of temperature measurement, 0 = not used (requires array of u16)
#endif

#ifndef USE_STACKCHECK
#define USE_STACKCHECK	1		// use Stack check (sdk_cpu.c, sdk_cpu.h)
#endif

#ifndef ASM64
#define ASM64		0		// 1 = use assembler x64 optimization, if available
#endif

#if RP2040
#define BOOTLOADER_SIZE	0x8000		// size of boot loader RP2040
#else // RP2350
#define BOOTLOADER_SIZE	0x10000		// size of boot loader RP2350
#endif
#define BOOTLOADER_DATA		32	// size of boot loader resident data

#ifndef USB_DEV_CDC_RX_BUFSIZE
#define USB_DEV_CDC_RX_BUFSIZE	256	// USB CDC device RX buffer size
#endif

#ifndef USB_DEV_CDC_TX_BUFSIZE
#define USB_DEV_CDC_TX_BUFSIZE	256	// USB CDC device TX buffer size
#endif

#ifndef USB_HOST_CDC_RX_BUFSIZE
#define USB_HOST_CDC_RX_BUFSIZE	256	// USB CDC host RX buffer size
#endif

#ifndef USB_HOST_CDC_TX_BUFSIZE
#define USB_HOST_CDC_TX_BUFSIZE	256	// USB CDC host TX buffer size
#endif

#ifndef USB_DEV_HID_RX_BUFSIZE
#define USB_DEV_HID_RX_BUFSIZE	16	// USB HID device RX buffer size
#endif

#ifndef USB_DEV_HID_TX_BUFSIZE
#define USB_DEV_HID_TX_BUFSIZE	32	// USB HID device TX buffer size
#endif

#ifndef USB_HOST_HID_RX_BUFSIZE
#define USB_HOST_HID_RX_BUFSIZE	32	// USB HID host RX buffer size
#endif

#ifndef USB_HOST_HID_TX_BUFSIZE
#define USB_HOST_HID_TX_BUFSIZE	16	// USB HID host TX buffer size
#endif

#ifndef USB_HOST_HID_REPSIZE
#define USB_HOST_HID_REPSIZE	256	// USB HID host buffer to load report descriptor
#endif

#ifndef USB_HOST_HID_KEY_BUFSIZE
#define USB_HOST_HID_KEY_BUFSIZE 16	// USB HID host keyboard ring buffer size
#endif

#ifndef USB_HOST_HID_MOUSE_BUFSIZE
#define USB_HOST_HID_MOUSE_BUFSIZE 16	// USB HID host mouse ring buffer size
#endif

#ifndef KEY_REP_TIME1
#define KEY_REP_TIME1		400	// delta time of first press in [ms] (max 500)
#endif

#ifndef KEY_REP_TIME2
#define KEY_REP_TIME2		100	// delta time of repeat press in [ms] (max 500)
#endif

#ifndef SYSTICK_KEYSCAN
#define SYSTICK_KEYSCAN		0	// call KeyScan() function from SysTick system timer
#endif

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT		0	// UART stdio port 0 or 1
#endif

#ifndef UART_STDIO_TX
#define UART_STDIO_TX		0	// UART stdio TX GPIO pin (function mode UART or AUX is auto-selected)
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX		1	// UART stdio RX GPIO pin (function mode UART or AUX is auto-selected)
#endif

#ifndef UART_STDIO_TXBUF
#define UART_STDIO_TXBUF	128	// size of transmit ring buffer of UART stdio
#endif

#ifndef UART_STDIO_RXBUF
#define UART_STDIO_RXBUF	128	// size of receive ring buffer of UART stdio
#endif

#ifndef USE_FAST_PERI
#define USE_FAST_PERI		0	// use fast perifery - use system clock instead of USB clock
					// Warning: If you change system clock, you must also repair
					//          peripheral clock and re-initialize peripherals.
#endif

#ifndef USE_DISP_DMA
#define USE_DISP_DMA		1	// use DMA output do LCD display
#endif

#ifndef DISP_ROT
#define DISP_ROT	1		// display rotation of LCD: 0 Portrait, 1 Landscape, 2 Inverted Portrait, 3 Inverted Landscape
#endif

#ifndef FLASH_CLKDIV			// current divider of flash SPI
#ifdef PRESET_FLASH_CLKDIV
#define FLASH_CLKDIV PRESET_FLASH_CLKDIV // preset other divider
#else
#define FLASH_CLKDIV 4
#endif
#endif

#ifndef FLASH_SIZE
#define FLASH_SIZE (2*1024*1024)	// compiled flash size
#endif

#define RAM_SIZE (264*1024)		// RAM size

#define PICO_FLASH_SPI_CLKDIV FLASH_CLKDIV	// original-SDK flag
#define PICO_FLASH_SIZE_BYTES FLASH_SIZE	// original-SDK flag

#ifndef LOAD_FLASH_INFO
#define LOAD_FLASH_INFO		1	// load flash info on startup
#endif

#ifndef PICO_DEFAULT_LED_PIN
#define PICO_DEFAULT_LED_PIN 25
#endif

// ----------------------------------------------------------------------------
//              UART test sample (files sdk_uart.c, sdk_uart.h)
// ----------------------------------------------------------------------------

#ifndef UARTSAMPLE
#define UARTSAMPLE		0	// flag in sdk_uart.*, compile UART test sample: 1 = compile, 0 = not
#endif

#ifndef UARTSAMPLE_UART
#define UARTSAMPLE_UART		0	// UART 0 or 1
#endif

#ifndef UARTSAMPLE_TXORDER
#define UARTSAMPLE_TXORDER	7	// order of size of transmitter ring buffers (size in bytes = 1 << order)
#endif

#ifndef UARTSAMPLE_RXORDER
#define UARTSAMPLE_RXORDER	7	// order of size of receiver ring buffers (size in bytes = 1 << order)
#endif

#ifndef UARTSAMPLE_TXMODE
#define UARTSAMPLE_TXMODE	3	// transmit mode: 0=polling, 1=interrupt, 2=DMA stream, 3=DMA single
#endif

#ifndef UARTSAMPLE_RXMODE
#define UARTSAMPLE_RXMODE	2	// receive mode: 0=polling, 1=interrupt, 2=DMA stream
#endif

#ifndef UARTSAMPLE_TXDMA
#define UARTSAMPLE_TXDMA	10	// transmitter DMA channel 0..11 (used in DMA mode)
#endif

#ifndef UARTSAMPLE_RXDMA
#define UARTSAMPLE_RXDMA	11	// receiver DMA channel 0..11 (used in DMA mode)
#endif

// ----------------------------------------------------------------------------
//                            System resources
// ----------------------------------------------------------------------------

// DMA temporary channel to calculate CRC and memory copy/fill.
//  CPU0 uses DMA channel 11, CPU1 uses DMA channel 10, so functions can be used
//  simultaneously, without collision. However, they cannot be used simultaneously in an interrupt.
// DMA channels 14 and 15 use DispHSTX controller.
#ifndef DMA_TEMP_CHAN
#define DMA_TEMP_CHAN()	(11 - CpuID())
#endif

#ifndef ROSC_MHZ
#if RP2040
#define ROSC_MHZ	6		// ring oscillator frequency in MHz (default RP2040: 6 MHz)
#else // RP2350
#define ROSC_MHZ	11		// ring oscillator frequency in MHz (default RP2350: 11 MHz)
#endif
#endif

#ifndef XOSC_MHZ
#define XOSC_MHZ	12		// crystal oscillator frequency in MHz (must be in range 1..15 MHz, default 12 MHz)
#endif

#ifndef PLL_KHZ
#if RP2040
#define PLL_KHZ		125000		// PLL system frequency in kHz (default 125000 kHz)
#else // RP2350
#define PLL_KHZ		150000		// PLL system frequency in kHz (default 150000 kHz)
#endif
#endif

#ifndef BATTERY_FULL
#define BATTERY_FULL	4.2f		// voltage of full battery (default value, use Config.bat_full)
#endif

#ifndef BATTERY_FULL_INT
#define BATTERY_FULL_INT 4200		// voltage of full battery (default value, use Config.bat_full)
#endif

#ifndef BATTERY_EMPTY
#define BATTERY_EMPTY	3.1f		// voltage of empty battery (default value, use Config.bat_empty)
#endif

#ifndef BATTERY_EMPTY_INT
#define BATTERY_EMPTY_INT 3100		// voltage of empty battery (default value, use Config.bat_empty)
#endif

#ifndef BAT_DIODE_FV
#define BAT_DIODE_FV 	0.311f		// voltage drop in mV on diode (default value, use Config.bat_diode)
#endif

#ifndef BAT_DIODE_FV_INT
#define BAT_DIODE_FV_INT 311		// voltage drop in mV on diode (default value, use Config.bat_diode)
#endif

#ifndef ADC_UREF
#define ADC_UREF	3.3f		// ADC reference voltage (default value, use Config.adc_ref)
#endif

#ifndef ADC_UREF_INT
#define ADC_UREF_INT	3300		// ADC reference voltage (default value, use Config.adc_ref)
#endif

#ifndef TEMP_BASE
#define TEMP_BASE	0.706f		// temperature base voltage at 27°C (default value, use Config.temp_base)
#endif

#ifndef TEMP_SLOPE
#define	TEMP_SLOPE	0.001721f	// temperature slope - voltage per 1 degree (default value, use Config.temp_slope)
#endif

// ----------------------------------------------------------------------------
//                             Auto-dependencies
// ----------------------------------------------------------------------------

#if !USE_ORIGSDK		// include interface of original SDK
#define PICO_NO_HARDWARE 1	// this switch is not use in other places of the SDK,
				//  it is used only in PIO program to cut-out some unwanted declarations
#endif

#if USE_VIDEO
#undef USE_FRAMEBUF
#define USE_FRAMEBUF	0
#endif

#if USE_MEMLOCK
#undef USE_SPINLOCK
#define USE_SPINLOCK 1
#endif

#if USE_MINIVGA
#undef USE_PIO
#define USE_PIO 1
#undef USE_DMA
#define USE_DMA 1
#undef USE_IRQ
#define USE_IRQ 1
#undef USE_PLL
#define USE_PLL 1
#undef USE_MULTICORE
#define USE_MULTICORE 1
#undef USE_FIFO
#define USE_FIFO 1
#endif

#if USE_SCREENSHOT
#undef USE_FAT
#define USE_FAT 1
#undef USE_SD
#define USE_SD 1
#endif

#if USE_FAT
#undef USE_SD
#define USE_SD 1
#endif

#if LOAD_FLASH_INFO
#undef USE_FLASH
#define USE_FLASH 1
#endif

#if USE_FLASH && !NO_FLASH
#undef USE_CRC
#define USE_CRC 1
#endif

#if USE_FLASH
#undef USE_DECNUM
#define USE_DECNUM 1
#endif

#if USE_SPINLOCK
#undef USE_TIMER
#define USE_TIMER 1
#endif

/*
// Note: The following auto-configurations are disabled because they are enabled by default,
//       so that the user will be warned if he wants to disable them on purpose.

#if USE_SD
#undef USE_SPI
#define USE_SPI 1
#undef USE_TIMER
#define USE_TIMER 1
#endif

#if SYSTICK_KEYSCAN
#undef USE_SYSTICK
#define USE_SYSTICK 1
#endif

#if USE_ALARM
#undef USE_SYSTICK
#define USE_SYSTICK 1
#undef USE_LIST
#define USE_LIST 1
#endif

#if USE_CALENDAR64
#undef USE_CALENDAR
#define USE_CALENDAR 1
#endif

#if USE_RTC
#undef USE_CALENDAR
#define USE_CALENDAR 1
#undef USE_IRQ
#define USE_IRQ 1
#endif

#if USE_TEXTLIST
#undef USE_TEXT
#define USE_TEXT 1
#endif

#if USE_TEXT
#undef USE_MALLOC
#define USE_MALLOC 1
#undef USE_CALENDAR
#define USE_CALENDAR 1
#undef USE_PRINT
#define USE_PRINT 1
#endif

#if USE_MALLOC
#undef USE_LIST
#define USE_LIST 1
#endif

#if USE_UART
#undef USE_PRINT
#define USE_PRINT 1
#endif

#if USE_RINGRX
#undef USE_DMA
#define USE_DMA 1
#endif

#if USE_RINGTX
#undef USE_DMA
#define USE_DMA 1
#undef USE_PRINT
#define USE_PRINT 1
#endif

#if USE_MULTICORE
#undef USE_FIFO
#define USE_FIFO 1
#undef USE_IRQ
#define USE_IRQ 1
#endif

#if USE_PLL
#undef USE_XOSC
#define USE_XOSC 1
#endif

#if USE_FIFO
#undef USE_TIMER
#define USE_TIMER 1
#endif

#if USE_TIMER
#undef USE_IRQ
#define USE_IRQ 1
#endif

#if USE_FLASH
#undef USE_CRC
#define USE_CRC 1
#endif

#if USE_CRC
#undef USE_DMA
#define USE_DMA 1
#endif
*/

#if USE_USBPAD
#if !USE_USB_HOST_HID
#undef USE_USB_HOST_HID
#define USE_USB_HOST_HID 4
#endif
#endif

#if USE_USB_DEV_AUDIO || USE_USB_DEV_BTH || USE_USB_DEV_CDC || USE_USB_DEV_DFU || USE_USB_DEV_HID || \
     USE_USB_DEV_MIDI || USE_USB_DEV_MSC || USE_USB_DEV_NET || USE_USB_DEV_TMC || USE_USB_DEV_VENDOR
#undef USE_USB_DEV
#define USE_USB_DEV 1
#endif

#if USE_USB_HOST_CDC || USE_USB_HOST_HID || USE_USB_HOST_HUB || USE_USB_HOST_MSC || USE_USB_HOST_VENDOR
#if USE_USB_HOST == 0
#undef USE_USB_HOST
#define USE_USB_HOST 4
#endif
#endif

#if USE_USB_DEV || USE_USB_HOST
#undef USE_USB
#define USE_USB 1
#endif

#if USE_USB_STDIO && USE_USB
#error If you enable USE_USB_STDIO, do not use USB functions!
#endif

#if USE_USB_STDIO
#ifndef USE_DRAW_STDIO
#define USE_DRAW_STDIO 0
#endif
#undef USE_USB
#define USE_USB 1
#undef USE_USB_DEV
#define USE_USB_DEV 1
#undef USE_USB_HOST
#define USE_USB_HOST 0
#undef USE_USB_DEV_CDC
#define USE_USB_DEV_CDC 1
#undef USE_PRINT
#define USE_PRINT 1
#endif

#if USE_DRAW_STDIO
#if USE_PICOPAD || USE_PICOINO || USE_PICOTRON || USE_DEMOVGA // if we have TFT or VGA display
#undef USE_DRAW
#define USE_DRAW 1
//#else
//#undef USE_DRAW_STDIO
//#define USE_DRAW_STDIO 0
#endif
#endif

/*
// Note: The following auto-configurations are disabled because they are enabled by default,
//       so that the user will be warned if he wants to disable them on purpose.

#if USE_USB_HOST
#undef USE_SYSTICK
#define USE_SYSTICK 1
#endif

#if USE_USB_HOST_HID
#undef USE_EVENT
#define USE_EVENT 1
#endif

#if USE_USB
#undef USE_RING
#define USE_RING 1
#endif

#if USE_RING
#undef USE_PRINT
#define USE_PRINT 1
#endif

#if USE_SYSTICK
#undef USE_IRQ
#define USE_IRQ 1
#endif

#if USE_PRINT
#undef USE_STREAM
#define USE_STREAM 1
#endif
*/

#if USE_REAL16 || USE_REAL32 || USE_REAL48 || USE_REAL64 || USE_REAL80 || USE_REAL96 || USE_REAL128 \
	|| USE_REAL160 || USE_REAL192 || USE_REAL256 || USE_REAL384 || USE_REAL512 || USE_REAL768 \
	|| USE_REAL1024 || USE_REAL1536 || USE_REAL2048 || USE_REAL3072 || USE_REAL4096 \
	|| USE_REAL6144 || USE_REAL8192 || USE_REAL12288
#undef USE_REAL
#define USE_REAL 1
#endif

#if USE_REAL
#undef USE_BIGINT
#define USE_BIGINT 1
#endif

#if !USE_BIGINT
#undef BIGINT_BERN_NUM
#define BIGINT_BERN_NUM 0
#endif

#if USE_USB && USE_USBPORT
#error Cannot use USB and USB Mini-Port together!
#endif

#endif // _CONFIG_DEF_H
