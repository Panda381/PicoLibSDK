
// ****************************************************************************
//
//                                GPIO pins
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

// QFN-60 (RP2040, RP2350A): 30 GPIO pins, index 0..29
// QFN-80 (RP2350B): 48 GPIO pins, index 0..47

/* GPIO functions, RP2040
 * ======================
 * | GPIO   | F1       | F2        | F3       | F4     | F5  | F6   | F7   | F8            | F9            |
 * |--------|----------|-----------|----------|--------|-----|------|------|---------------|---------------|
 * | 0      | SPI0 RX  | UART0 TX  | I2C0 SDA | PWM0 A | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 1      | SPI0 CSn | UART0 RX  | I2C0 SCL | PWM0 B | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 2      | SPI0 SCK | UART0 CTS | I2C1 SDA | PWM1 A | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 3      | SPI0 TX  | UART0 RTS | I2C1 SCL | PWM1 B | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 4      | SPI0 RX  | UART1 TX  | I2C0 SDA | PWM2 A | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 5      | SPI0 CSn | UART1 RX  | I2C0 SCL | PWM2 B | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 6      | SPI0 SCK | UART1 CTS | I2C1 SDA | PWM3 A | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 7      | SPI0 TX  | UART1 RTS | I2C1 SCL | PWM3 B | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 8      | SPI1 RX  | UART1 TX  | I2C0 SDA | PWM4 A | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 9      | SPI1 CSn | UART1 RX  | I2C0 SCL | PWM4 B | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 10     | SPI1 SCK | UART1 CTS | I2C1 SDA | PWM5 A | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 11     | SPI1 TX  | UART1 RTS | I2C1 SCL | PWM5 B | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 12     | SPI1 RX  | UART0 TX  | I2C0 SDA | PWM6 A | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 13     | SPI1 CSn | UART0 RX  | I2C0 SCL | PWM6 B | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 14     | SPI1 SCK | UART0 CTS | I2C1 SDA | PWM7 A | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 15     | SPI1 TX  | UART0 RTS | I2C1 SCL | PWM7 B | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 16     | SPI0 RX  | UART0 TX  | I2C0 SDA | PWM0 A | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 17     | SPI0 CSn | UART0 RX  | I2C0 SCL | PWM0 B | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 18     | SPI0 SCK | UART0 CTS | I2C1 SDA | PWM1 A | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 19     | SPI0 TX  | UART0 RTS | I2C1 SCL | PWM1 B | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 20     | SPI0 RX  | UART1 TX  | I2C0 SDA | PWM2 A | SIO | PIO0 | PIO1 | CLOCK GPIN0   | USB VBUS EN   |
 * | 21     | SPI0 CSn | UART1 RX  | I2C0 SCL | PWM2 B | SIO | PIO0 | PIO1 | CLOCK GPOUT0  | USB OVCUR DET |
 * | 22     | SPI0 SCK | UART1 CTS | I2C1 SDA | PWM3 A | SIO | PIO0 | PIO1 | CLOCK GPIN1   | USB VBUS DET  |
 * | 23     | SPI0 TX  | UART1 RTS | I2C1 SCL | PWM3 B | SIO | PIO0 | PIO1 | CLOCK GPOUT1  | USB VBUS EN   |
 * | 24     | SPI1 RX  | UART1 TX  | I2C0 SDA | PWM4 A | SIO | PIO0 | PIO1 | CLOCK GPOUT2  | USB OVCUR DET |
 * | 25     | SPI1 CSn | UART1 RX  | I2C0 SCL | PWM4 B | SIO | PIO0 | PIO1 | CLOCK GPOUT3  | USB VBUS DET  |
 * | 26     | SPI1 SCK | UART1 CTS | I2C1 SDA | PWM5 A | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
 * | 27     | SPI1 TX  | UART1 RTS | I2C1 SCL | PWM5 B | SIO | PIO0 | PIO1 |               | USB OVCUR DET |
 * | 28     | SPI1 RX  | UART0 TX  | I2C0 SDA | PWM6 A | SIO | PIO0 | PIO1 |               | USB VBUS DET  |
 * | 29     | SPI1 CSn | UART0 RX  | I2C0 SCL | PWM6 B | SIO | PIO0 | PIO1 |               | USB VBUS EN   |
*/

/* GPIO functions, RP2350
 * ======================
 * | GPIO  | F0   | F1       | F2        | F3       | F4     | F5  | F6   | F7   | F8   | F9           | F10           | F11      |
 * |-------|------|----------|-----------|----------|--------|-----|------|------|------|--------------|---------------|----------|
 * | 0     |      | SPI0 RX  | UART0 TX  | I2C0 SDA | PWM0 A | SIO | PIO0 | PIO1 | PIO2 | XIP_CS1n     | USB OVCUR DET |          |
 * | 1     |      | SPI0 CSn | UART0 RX  | I2C0 SCL | PWM0 B | SIO | PIO0 | PIO1 | PIO2 | TRACECLK     | USB VBUS DET  |          |
 * | 2     |      | SPI0 SCK | UART0 CTS | I2C1 SDA | PWM1 A | SIO | PIO0 | PIO1 | PIO2 | TRACEDATA0   | USB VBUS EN   | UART0 TX |
 * | 3     |      | SPI0 TX  | UART0 RTS | I2C1 SCL | PWM1 B | SIO | PIO0 | PIO1 | PIO2 | TRACEDATA1   | USB OVCUR DET | UART0 RX |
 * | 4     |      | SPI0 RX  | UART1 TX  | I2C0 SDA | PWM2 A | SIO | PIO0 | PIO1 | PIO2 | TRACEDATA2   | USB VBUS DET  |          |
 * | 5     |      | SPI0 CSn | UART1 RX  | I2C0 SCL | PWM2 B | SIO | PIO0 | PIO1 | PIO2 | TRACEDATA3   | USB VBUS EN   |          |
 * | 6     |      | SPI0 SCK | UART1 CTS | I2C1 SDA | PWM3 A | SIO | PIO0 | PIO1 | PIO2 |              | USB OVCUR DET | UART1 TX |
 * | 7     |      | SPI0 TX  | UART1 RTS | I2C1 SCL | PWM3 B | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS DET  | UART1 RX |
 * | 8     |      | SPI1 RX  | UART1 TX  | I2C0 SDA | PWM4 A | SIO | PIO0 | PIO1 | PIO2 | XIP_CS1n     | USB VBUS EN   |          |
 * | 9     |      | SPI1 CSn | UART1 RX  | I2C0 SCL | PWM4 B | SIO | PIO0 | PIO1 | PIO2 |              | USB OVCUR DET |          |
 * | 10    |      | SPI1 SCK | UART1 CTS | I2C1 SDA | PWM5 A | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS DET  | UART1 TX |
 * | 11    |      | SPI1 TX  | UART1 RTS | I2C1 SCL | PWM5 B | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS EN   | UART1 RX |
 * | 12    | HSTX | SPI1 RX  | UART0 TX  | I2C0 SDA | PWM6 A | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPIN0  | USB OVCUR DET |          |
 * | 13    | HSTX | SPI1 CSn | UART0 RX  | I2C0 SCL | PWM6 B | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPOUT0 | USB VBUS DET  |          |
 * | 14    | HSTX | SPI1 SCK | UART0 CTS | I2C1 SDA | PWM7 A | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPIN1  | USB VBUS EN   | UART0 TX |
 * | 15    | HSTX | SPI1 TX  | UART0 RTS | I2C1 SCL | PWM7 B | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPOUT1 | USB OVCUR DET | UART0 RX |
 * | 16    | HSTX | SPI0 RX  | UART0 TX  | I2C0 SDA | PWM0 A | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS DET  |          |
 * | 17    | HSTX | SPI0 CSn | UART0 RX  | I2C0 SCL | PWM0 B | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS EN   |          |
 * | 18    | HSTX | SPI0 SCK | UART0 CTS | I2C1 SDA | PWM1 A | SIO | PIO0 | PIO1 | PIO2 |              | USB OVCUR DET | UART0 TX |
 * | 19    | HSTX | SPI0 TX  | UART0 RTS | I2C1 SCL | PWM1 B | SIO | PIO0 | PIO1 | PIO2 | XIP_CS1n     | USB VBUS DET  | UART0 RX |
 * | 20    |      | SPI0 RX  | UART1 TX  | I2C0 SDA | PWM2 A | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPIN0  | USB VBUS EN   |          |
 * | 21    |      | SPI0 CSn | UART1 RX  | I2C0 SCL | PWM2 B | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPOUT0 | USB OVCUR DET |          |
 * | 22    |      | SPI0 SCK | UART1 CTS | I2C1 SDA | PWM3 A | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPIN1  | USB VBUS DET  | UART1 TX |
 * | 23    |      | SPI0 TX  | UART1 RTS | I2C1 SCL | PWM3 B | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPOUT1 | USB VBUS EN   | UART1 RX |
 * | 24    |      | SPI1 RX  | UART1 TX  | I2C0 SDA | PWM4 A | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPOUT2 | USB OVCUR DET |          |
 * | 25    |      | SPI1 CSn | UART1 RX  | I2C0 SCL | PWM4 B | SIO | PIO0 | PIO1 | PIO2 | CLOCK GPOUT3 | USB VBUS DET  |          |
 * | 26    |      | SPI1 SCK | UART1 CTS | I2C1 SDA | PWM5 A | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS EN   | UART1 TX |
 * | 27    |      | SPI1 TX  | UART1 RTS | I2C1 SCL | PWM5 B | SIO | PIO0 | PIO1 | PIO2 |              | USB OVCUR DET | UART1 RX |
 * | 28    |      | SPI1 RX  | UART0 TX  | I2C0 SDA | PWM6 A | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS DET  |          |
 * | 29    |      | SPI1 CSn | UART0 RX  | I2C0 SCL | PWM6 B | SIO | PIO0 | PIO1 | PIO2 |              | USB VBUS EN   |          |
 *
 * GPIOs 30 through 47 are QFN-80 only (RP2350B):
 *
 * | GPIO | F0 | F1       | F2       | F3        | F4      | F5  | F6   | F7   | F8   | F9       | F10           | F11      |
 * |------|----|----------|----------|-----------|---------|-----|------|------|------|----------|---------------|----------|
 * | 30   |    | SPI1 SCK | UART0 CTS | I2C1 SDA | PWM7 A  | SIO | PIO0 | PIO1 | PIO2 |          | USB OVCUR DET | UART0 TX |
 * | 31   |    | SPI1 TX  | UART0 RTS | I2C1 SCL | PWM7 B  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS DET  | UART0 RX |
 * | 32   |    | SPI0 RX  | UART0 TX  | I2C0 SDA | PWM8 A  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS EN   |          |
 * | 33   |    | SPI0 CSn | UART0 RX  | I2C0 SCL | PWM8 B  | SIO | PIO0 | PIO1 | PIO2 |          | USB OVCUR DET |          |
 * | 34   |    | SPI0 SCK | UART0 CTS | I2C1 SDA | PWM9 A  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS DET  | UART0 TX |
 * | 35   |    | SPI0 TX  | UART0 RTS | I2C1 SCL | PWM9 B  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS EN   | UART0 RX |
 * | 36   |    | SPI0 RX  | UART1 TX  | I2C0 SDA | PWM10 A | SIO | PIO0 | PIO1 | PIO2 |          | USB OVCUR DET |          |
 * | 37   |    | SPI0 CSn | UART1 RX  | I2C0 SCL | PWM10 B | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS DET  |          |
 * | 38   |    | SPI0 SCK | UART1 CTS | I2C1 SDA | PWM11 A | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS EN   | UART1 TX |
 * | 39   |    | SPI0 TX  | UART1 RTS | I2C1 SCL | PWM11 B | SIO | PIO0 | PIO1 | PIO2 |          | USB OVCUR DET | UART1 RX |
 * | 40   |    | SPI1 RX  | UART1 TX  | I2C0 SDA | PWM8 A  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS DET  |          |
 * | 41   |    | SPI1 CSn | UART1 RX  | I2C0 SCL | PWM8 B  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS EN   |          |
 * | 42   |    | SPI1 SCK | UART1 CTS | I2C1 SDA | PWM9 A  | SIO | PIO0 | PIO1 | PIO2 |          | USB OVCUR DET | UART1 TX |
 * | 43   |    | SPI1 TX  | UART1 RTS | I2C1 SCL | PWM9 B  | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS DET  | UART1 RX |
 * | 44   |    | SPI1 RX  | UART0 TX  | I2C0 SDA | PWM10 A | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS EN   |          |
 * | 45   |    | SPI1 CSn | UART0 RX  | I2C0 SCL | PWM10 B | SIO | PIO0 | PIO1 | PIO2 |          | USB OVCUR DET |          |
 * | 46   |    | SPI1 SCK | UART0 CTS | I2C1 SDA | PWM11 A | SIO | PIO0 | PIO1 | PIO2 |          | USB VBUS DET  | UART0 TX |
 * | 47   |    | SPI1 TX  | UART0 RTS | I2C1 SCL | PWM11 B | SIO | PIO0 | PIO1 | PIO2 | XIP_CS1n | USB VBUS EN   | UART0 RX |
*/

#ifndef _SDK_GPIO_H
#define _SDK_GPIO_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_cpu.h"
#include "sdk_irq.h"
#include "sdk_sio.h"
#include "sdk_gpio_coproc.h"		// GPIO coprocessor

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_io_bank0.h"		// constants of original SDK
#include "orig_rp2040/orig_pads_bank0.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_io_bank0.h"		// constants of original SDK
#include "orig_rp2350/orig_pads_bank0.h"	// constants of original SDK
#endif
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

// number of pins
#if RP2040

#define GPIO_PIN_NUM		30	// number of GPIO pin registers
#define GPIO_INT_REGNUM		4	// number of GPIO interrupt registers
#define GPIO_PIN_SWCLK		30	// SWCLK pin (only pad functions)
#define GPIO_PIN_SWD		31	// SWD pin (only pad functions)
typedef u32 gpio_mask_t;		// 32-bit pin mask
INLINE u32 PinRangeMask(int first, int last) { return RangeMask(first, last); } // mask of range of pins

#else // RP2350

#define GPIO_PIN_NUM		48	// number of GPIO pin registers
#define GPIO_INT_REGNUM		6	// number of GPIO interrupt registers
#define GPIO_PIN_SWCLK		48	// SWCLK pin (only pad functions)
#define GPIO_PIN_SWD		49	// SWD pin (only pad functions)
#define GPIO_PIN_USB_DP		56	// USB D+ pin (only pin data functions)
#define GPIO_PIN_USB_DM		57	// USB D- pin (only pin data functions)
#define GPIO_PIN_QSPI_SCK	58	// QSPI SCK pin (only pin data functions)
#define GPIO_PIN_QSPI_CSN	59	// QSPI CSn pin (only pin data functions)
#define GPIO_PIN_QSPI_SD0	60	// QSPI SD0 MOSI pin (only pin data functions)
#define GPIO_PIN_QSPI_SD1	61	// QSPI SD1 MISO pin (only pin data functions)
#define GPIO_PIN_QSPI_SD2	62	// QSPI SD2 pin (only pin data functions)
#define GPIO_PIN_QSPI_SD3	63	// QSPI SD3 pin (only pin data functions)
typedef u64 gpio_mask_t;		// 64-bit pin mask
INLINE u64 PinRangeMask(int first, int last) { return RangeMask64(first, last); } // mask of range of pins

#endif

// === GPIO pad control (pin = 0..31/RP2040 or 0..49/RP2350, including SWCLK and SWD)

// pad control hardware registers
#define GPIO_VOLT	((volatile u32*)(PADS_BANK0_BASE+0))	// voltage select
#define GPIO_PAD(pin) ((volatile u32*)(PADS_BANK0_BASE+(pin)*4+4)) // GPIO pad control register, pin = 0..31 or 0..49 (including SWCLK and SWD)

// pad control hardware interface
typedef struct {
	io32	voltage_select;		// 0x00: voltage select
	io32	io[GPIO_PIN_NUM];	// 0x04: pad control register
	io32	swclk;			// 0x7C: SWCLK pin pad control
	io32	swd;			// 0x80: SWD pin pad control
} pads_bank0_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(pads_bank0_hw_t) == 0x84, "Incorrect pads_bank0_hw_t!");
#else
STATIC_ASSERT(sizeof(pads_bank0_hw_t) == 0xcc, "Incorrect pads_bank0_hw_t!");
#endif

#define pads_bank0_hw ((pads_bank0_hw_t*)PADS_BANK0_BASE)
#define padsbank0_hw_t pads_bank0_hw_t
#define padsbank0_hw pads_bank0_hw

// === GPIO pin control (pin = 0..29/RP2040 or 0..47/RP2350)

// pin control hardware registers
#define GPIO_STATUS(pin) ((volatile u32*)(IO_BANK0_BASE+(pin)*8)) // GPIO status (read only), pin = 0..29 or 0..47
#define GPIO_CTRL(pin) ((volatile u32*)(IO_BANK0_BASE+(pin)*8+4)) // GPIO pin control, pin = 0..29 or 0..47

// pin control hardware interface
typedef struct {
	io32	status;		// 0x00: GPIO status
	io32	ctrl;		// 0x04: GPIO control including function select and overrides
} io_bank0_status_ctrl_hw_t;

STATIC_ASSERT(sizeof(io_bank0_status_ctrl_hw_t) == 0x08, "Incorrect io_bank0_status_ctrl_hw_t!");

#define iobank0_status_ctrl_hw_t io_bank0_status_ctrl_hw_t

// === GPIO interrupt control (pin = 0..29/RP2040 or 0..47/RP2350)

// pin interrupt hardware registers
// IRQ control registers, array of 4*u32 (cpu = processor core 0 or 1; or 2 = dormant wake)
//  Interrupts EDGE are latched in INTR registers and must be cleared.
//  Interrupts LEVEL are not latched, they become inactive on end condition.
#if RP2040
#define GPIO_IRQ_INTR ((volatile u32*)(IO_BANK0_BASE+0xf0)) // raw interrupt of both CPUs
#define GPIO_IRQ_INTE(cpu) ((volatile u32*)(IO_BANK0_BASE+(cpu)*0x30+0x100)) // interrupt enable
#define GPIO_IRQ_INTF(cpu) ((volatile u32*)(IO_BANK0_BASE+(cpu)*0x30+0x100+0x10)) // force interrupt
#define GPIO_IRQ_INTS(cpu) ((volatile u32*)(IO_BANK0_BASE+(cpu)*0x30+0x100+0x20)) // interrupt status
#else
#define GPIO_IRQ_INTR ((volatile u32*)(IO_BANK0_BASE+0x230)) // raw interrupt of both CPUs
#define GPIO_IRQ_INTE(cpu) ((volatile u32*)(IO_BANK0_BASE+(cpu)*0x48+0x248)) // interrupt enable
#define GPIO_IRQ_INTF(cpu) ((volatile u32*)(IO_BANK0_BASE+(cpu)*0x48+0x248+0x18)) // force interrupt
#define GPIO_IRQ_INTS(cpu) ((volatile u32*)(IO_BANK0_BASE+(cpu)*0x48+0x248+0x30)) // interrupt status
#endif

// pin interrupt hardware interface
typedef struct {
	io32	inte[GPIO_INT_REGNUM];	// 0x00: Interrupt Enable
	io32	intf[GPIO_INT_REGNUM];	// 0x10 (0x18): Interrupt Force
	io32	ints[GPIO_INT_REGNUM];	// 0x20 (0x30): Interrupt status after masking & forcing
} io_bank0_irq_ctrl_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(io_bank0_irq_ctrl_hw_t) == 0x30, "Incorrect io_bank0_irq_ctrl_hw_t!");
#else
STATIC_ASSERT(sizeof(io_bank0_irq_ctrl_hw_t) == 0x48, "Incorrect io_bank0_irq_ctrl_hw_t!");
#endif

// pin bank interface
typedef struct {
	io_bank0_status_ctrl_hw_t	io[GPIO_PIN_NUM];	// 0x000: pin control
#if !RP2040
	io32				_pad0[32];				// 0x180
	io32				irqsummary_proc0_secure[2];		// 0x200
	io32				irqsummary_proc0_nonsecure[2];		// 0x208
	io32				irqsummary_proc1_secure[2];		// 0x210
	io32				irqsummary_proc1_nonsecure[2];		// 0x218
	io32				irqsummary_dormant_wake_secure[2];	// 0x220
	io32				irqsummary_dormant_wake_nonsecure[2];	// 0x228
#endif
	io32				intr[GPIO_INT_REGNUM];		// 0x0F0 (0x230): Raw Interrupts
	union {
		struct {
			io_bank0_irq_ctrl_hw_t	proc0_irq_ctrl;		// 0x100 (0x248): interrupt control for proc0
			io_bank0_irq_ctrl_hw_t	proc1_irq_ctrl;		// 0x130 (0x290): interrupt control for proc1
			io_bank0_irq_ctrl_hw_t	dormant_wake_irq_ctrl;	// 0x160 (0x2D8): dormant interrupt control
		};
		io_bank0_irq_ctrl_hw_t	irq_ctrl[3];			// 0x100 (0x248):
	};
} io_bank0_hw_t;

#if RP2040
STATIC_ASSERT(sizeof(io_bank0_hw_t) == 0x190, "Incorrect io_bank0_hw_t!");
#else
STATIC_ASSERT(sizeof(io_bank0_hw_t) == 0x320, "Incorrect io_bank0_hw_t!");
#endif

#define io_bank0_hw ((io_bank0_hw_t*)IO_BANK0_BASE)
#define iobank0_hw io_bank0_hw

// === GPIO pin data (pin = 0..29/RP2040 or 0..47,56..63/RP2350 including USB and QSPI)

// GPIO pin data registers (bit 0..29 = pin 0..29)
//  (SIO does not support aliases for atomic access!)

#if RP2040
#define GPIO_IN_DATA		((volatile u32*)(SIO_BASE+4))		// GPIO input
#define GPIO_OUT_DATA		((volatile u32*)(SIO_BASE+0x10))	// GPIO output (read: last value written to output)
#define GPIO_OUT_SET		((volatile u32*)(SIO_BASE+0x14))	// GPIO output set
#define GPIO_OUT_CLR		((volatile u32*)(SIO_BASE+0x18))	// GPIO output clear
#define GPIO_OUT_XOR		((volatile u32*)(SIO_BASE+0x1C))	// GPIO output XOR
#define GPIO_OE			((volatile u32*)(SIO_BASE+0x20))	// GPIO output enable (read: last value written)
#define GPIO_OE_SET		((volatile u32*)(SIO_BASE+0x24))	// GPIO output enable set
#define GPIO_OE_CLR		((volatile u32*)(SIO_BASE+0x28))	// GPIO output enable clear
#define GPIO_OE_XOR		((volatile u32*)(SIO_BASE+0x2C))	// GPIO output enable XOR
#else
#define GPIO_IN_DATA		((volatile u32*)(SIO_BASE+4))		// GPIO input
#define GPIO_HI_IN_DATA		((volatile u32*)(SIO_BASE+8))		// GPIO input
#define GPIO_OUT_DATA		((volatile u32*)(SIO_BASE+0x10))	// GPIO output (read: last value written to output)
#define GPIO_HI_OUT_DATA	((volatile u32*)(SIO_BASE+0x14))	// GPIO output (read: last value written to output)
#define GPIO_OUT_SET		((volatile u32*)(SIO_BASE+0x18))	// GPIO output set
#define GPIO_HI_OUT_SET		((volatile u32*)(SIO_BASE+0x1C))	// GPIO output set
#define GPIO_OUT_CLR		((volatile u32*)(SIO_BASE+0x20))	// GPIO output clear
#define GPIO_HI_OUT_CLR		((volatile u32*)(SIO_BASE+0x24))	// GPIO output clear
#define GPIO_OUT_XOR		((volatile u32*)(SIO_BASE+0x28))	// GPIO output XOR
#define GPIO_HI_OUT_XOR		((volatile u32*)(SIO_BASE+0x2C))	// GPIO output XOR
#define GPIO_OE			((volatile u32*)(SIO_BASE+0x30))	// GPIO output enable (read: last value written)
#define GPIO_HI_OE		((volatile u32*)(SIO_BASE+0x34))	// GPIO output enable (read: last value written)
#define GPIO_OE_SET		((volatile u32*)(SIO_BASE+0x38))	// GPIO output enable set
#define GPIO_HI_OE_SET		((volatile u32*)(SIO_BASE+0x3C))	// GPIO output enable set
#define GPIO_OE_CLR		((volatile u32*)(SIO_BASE+0x40))	// GPIO output enable clear
#define GPIO_HI_OE_CLR		((volatile u32*)(SIO_BASE+0x44))	// GPIO output enable clear
#define GPIO_OE_XOR		((volatile u32*)(SIO_BASE+0x48))	// GPIO output enable XOR
#define GPIO_HI_OE_XOR		((volatile u32*)(SIO_BASE+0x4C))	// GPIO output enable XOR
#endif

// GPIO pin functions
#if RP2040
#define GPIO_FNC_XIP	0	// external Flash
#define GPIO_FNC_SPI	1	// SPI (SPI0 or SPI1)
#define GPIO_FNC_UART	2	// UART (UART0 or UART1)
#define GPIO_FNC_I2C	3	// I2C (I2C0 or I2C1)
#define GPIO_FNC_PWM	4	// PWM (PWM0..PWM7)
#define GPIO_FNC_SIO	5	// SIO (GPIO digital function)
#define GPIO_FNC_PIO0	6	// PIO0
#define GPIO_FNC_PIO1	7	// PIO1
#define GPIO_FNC_GPCK	8	// clock (only pins 20..25)
#define GPIO_FNC_USB	9	// USB
#define GPIO_FNC_NULL	31	// no function (use in case of ADC input)
#else
#define GPIO_FNC_HSTX	0	// HSTX (only pins 12..19)
#define GPIO_FNC_SPI	1	// SPI (SPI0 or SPI1)
#define GPIO_FNC_UART	2	// UART (UART0 or UART1)
#define GPIO_FNC_I2C	3	// I2C (I2C0 or I2C1)
#define GPIO_FNC_PWM	4	// PWM (PWM0..PWM11)
#define GPIO_FNC_SIO	5	// SIO (GPIO digital function)
#define GPIO_FNC_PIO0	6	// PIO0
#define GPIO_FNC_PIO1	7	// PIO1
#define GPIO_FNC_PIO2	8	// PIO2
#define GPIO_FNC_GPCK	9	// clock (pins 12..15 and 20..25)
#define GPIO_FNC_XIP	9	// XIP QMI CS1 (pins 0, 8, 19)
#define GPIO_FNC_TRACE	9	// CORESIGHT TRACECLK (pins 1..5)
#define GPIO_FNC_USB	10	// USB
#define GPIO_FNC_AUX	11	// UART AUX (select TX/RX instead of CTS/RTS)
#define GPIO_FNC_NULL	31	// no function (use in case of ADC input)
#endif

// interrupt events
#define IRQ_EVENT_LEVELLOW	B0	// level low (not latched interrupt request)
#define IRQ_EVENT_LEVELHIGH	B1	// level high (not latched interrupt request)
#define IRQ_EVENT_EDGELOW	B2	// edge low (request is latched in NTR register and must be cleared)
#define IRQ_EVENT_EDGEHIGH	B3	// edge high (request is latched in NTR register and must be cleared)

#define IRQ_EVENT_ALL		0x0f	// all events
#define IRQ_EVENT_NONE		0	// no event

// drive strength
#define GPIO_DRIVE_2MA		0
#define GPIO_DRIVE_4MA		1
#define GPIO_DRIVE_8MA		2
#define GPIO_DRIVE_12MA		3

// peripheral signal override
#define GPIO_OVER_NORMAL	0
#define GPIO_OVER_INVERT	1
#define GPIO_OVER_LOW		2
#define GPIO_OVER_HIGH		3
#define GPIO_OVER_MASK		3

// generic callback function type for GPIO events (install with GPIO_IRQSetCallback() function)
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events
typedef void (*gpio_irq_callback_t)(uint pin, u32 events);

// generic callback for GPIO types (NULL = not used)
extern gpio_irq_callback_t GPIO_Callbacks[CORE_NUM];

// ==== GPIO pad control (pin = 0..31/RP2040 or 0..49/RP2350, including SWCLK and SWD, or hw = pad control register from GPIO_PadHw())

// GPIO set voltage to 3.3V (DVDD >= 2V5; default state)
INLINE void GPIO_Voltage3V3() { pads_bank0_hw->voltage_select = 0; }

// GPIO set voltage to 1.8V (DVDD <= 1V8)
INLINE void GPIO_Voltage1V8() { pads_bank0_hw->voltage_select = B0; }

// get pointer to pad control interface (pin = 0..31 or 0..49)
INLINE io32* GPIO_PadHw(int pin) { return &pads_bank0_hw->io[pin]; }

// init pad to default reset state (pin = 0..29 or 0..47; partially valid for SWCLK and SWD)
//  - slow slew rate, enable schmitt, pull-down, 4mA, in/out enable (RP2350: input disable, enable isolation state)
#if RP2040
INLINE void GPIO_PadInit(int pin) { *GPIO_PAD(pin) = B1|B2|B4|B6; }
INLINE void GPIO_PadInit_hw(io32* hw) { *hw = B1|B2|B4|B6; }
#else
INLINE void GPIO_PadInit(int pin) { *GPIO_PAD(pin) = B1|B2|B4|B8; }
INLINE void GPIO_PadInit_hw(io32* hw) { *hw = B1|B2|B4|B8; }
#endif

#if !RP2040
// Enable isolation latches (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
//  If pad isolation is enabled, all output settings of the pad are latched.
//  Disable pad isolation after pad is configured.
INLINE void GPIO_IsolationEnable(int pin) { RegSet(GPIO_PAD(pin), B8); }
INLINE void GPIO_IsolationEnable_hw(io32* hw) { RegSet(hw, B8); }

// Disable isolation latches (pin = 0..31 or 0..49, including SWCLK and SWD)
//  If pad isolation is disable, all output settings are transparent to the pad.
INLINE void GPIO_IsolationDisable(int pin) { RegClr(GPIO_PAD(pin), B8); }
INLINE void GPIO_IsolationDisable_hw(io32* hw) { RegClr(hw, B8); }
#endif

// GPIO output enable; disable has priority over GPIO_DirOut (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_OutEnable(int pin) { RegClr(GPIO_PAD(pin), B7); }
INLINE void GPIO_OutEnable_hw(io32* hw) { RegClr(hw, B7); }

// GPIO output disable; disable has priority over GPIO_DirOut (pin = 0..31 or 0..49, including SWCLK and SWD; default state for SWCLK pin)
INLINE void GPIO_OutDisable(int pin) { RegSet(GPIO_PAD(pin), B7); }
INLINE void GPIO_OutDisable_hw(io32* hw) { RegSet(hw, B7); }

// enable pin input (default state on RP2040; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_InEnable(int pin) { RegSet(GPIO_PAD(pin), B6); }
INLINE void GPIO_InEnable_hw(io32* hw) { RegSet(hw, B6); }

// disable pin input (should be used on ADC input; default state on RP2350; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_InDisable(int pin) { RegClr(GPIO_PAD(pin), B6); }
INLINE void GPIO_InDisable_hw(io32* hw) { RegClr(hw, B6); }

// set output strength to 2 mA (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Drive2mA(int pin) { RegClr(GPIO_PAD(pin), B4|B5); }
INLINE void GPIO_Drive2mA_hw(io32* hw) { RegClr(hw, B4|B5); }

// set output strength to 4 mA (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Drive4mA(int pin) { RegMask(GPIO_PAD(pin), B4, B4|B5); }
INLINE void GPIO_Drive4mA_hw(io32* hw) { RegMask(hw, B4, B4|B5); }

// set output strength to 8 mA (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Drive8mA(int pin) { RegMask(GPIO_PAD(pin), B5, B4|B5); }
INLINE void GPIO_Drive8mA_hw(io32* hw) { RegMask(hw, B5, B4|B5); }

// set output strength to 12 mA (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Drive12mA(int pin) { RegSet(GPIO_PAD(pin), B4|B5); }
INLINE void GPIO_Drive12mA_hw(io32* hw) { RegSet(hw, B4|B5); }

// Set drive strength GPIO_DRIVE_* (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Drive(int pin, int drive) { RegMask(GPIO_PAD(pin), drive << 4, B4|B5); }
INLINE void GPIO_Drive_hw(io32* hw, int drive) { RegMask(hw, drive << 4, B4|B5); }

// set no pulls (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_NoPull(int pin) { RegClr(GPIO_PAD(pin), B2|B3); }
INLINE void GPIO_NoPull_hw(io32* hw) { RegClr(hw, B2|B3); }

// set pull down (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
// RP2350-A2 errata RP2350-E9:
//  Description:
//   If pad is set as input and voltage is within undefine range between VIL and VIH,
//   then pull-down will not pull voltage low, because leakage current will hold at 2V.
//  Workaround 1:
//   Use external pull-down 8.2 kOhm.
//  Workaroung 2:
//   Disabling input will reset the leakage. To read pulled-down input state, enable
//   input immediately before reading GPIO_InEnable(), and then re-disable GPIO_InDisable().
INLINE void GPIO_PullDown(int pin) { RegMask(GPIO_PAD(pin), B2, B2|B3); }
INLINE void GPIO_PullDown_hw(io32* hw) { RegMask(hw, B2, B2|B3); }

// set pull up (pin = 0..31 or 0..49, including SWCLK and SWD; default state for SWCLK and SWD pins)
INLINE void GPIO_PullUp(int pin) { RegMask(GPIO_PAD(pin), B3, B2|B3); }
INLINE void GPIO_PullUp_hw(io32* hw) { RegMask(hw, B3, B2|B3); }

// set bus keep (weak pull up and down; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_BusKeep(int pin) { RegSet(GPIO_PAD(pin), B2|B3); }
INLINE void GPIO_BusKeep_hw(io32* hw) { RegSet(hw, B2|B3); }

// enable schmitt trigger (uses hysteresis on input; default state; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_SchmittEnable(int pin) { RegSet(GPIO_PAD(pin), B1); }
INLINE void GPIO_SchmittEnable_hw(io32* hw) { RegSet(hw, B1); }

// disable schmitt trigger (will not use hysteresis on input; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_SchmittDisable(int pin) { RegClr(GPIO_PAD(pin), B1); }
INLINE void GPIO_SchmittDisable_hw(io32* hw) { RegClr(hw, B1); }

// use slow slew rate control on output (default state; pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Slow(int pin) { RegClr(GPIO_PAD(pin), B0); }
INLINE void GPIO_Slow_hw(io32* hw) { RegClr(hw, B0); }

// use fast slew rate control on output (pin = 0..31 or 0..49, including SWCLK and SWD)
INLINE void GPIO_Fast(int pin) { RegSet(GPIO_PAD(pin), B0); }
INLINE void GPIO_Fast_hw(io32* hw) { RegSet(hw, B0); }

// ==== GPIO pin control (pin = 0..29/RP2040 or 0..47/RP2350)

// get pointer to pin control interface (pin = 0..29 or 0..47)
INLINE io_bank0_status_ctrl_hw_t* GPIO_PinHw(int pin) { return &iobank0_hw->io[pin]; }

#if RP2040
// get output (0 or 1) from peripheral, before override (pin = 0..29 or 0..47)
INLINE u8 GPIO_OutPeri(int pin) { return (u8)((*GPIO_STATUS(pin) >> 8) & 1); }
INLINE u8 GPIO_OutPeri_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 8) & 1); }
#endif

// get output (0 or 1) to pad, after override (pin = 0..29 or 0..47)
INLINE u8 GPIO_OutPad(int pin) { return (u8)((*GPIO_STATUS(pin) >> 9) & 1); }
INLINE u8 GPIO_OutPad_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 9) & 1); }

#if RP2040
// get output enable (0 or 1) from peripheral, before override (pin = 0..29 or 0..47)
INLINE u8 GPIO_OePeri(int pin) { return (u8)((*GPIO_STATUS(pin) >> 12) & 1); }
INLINE u8 GPIO_OePeri_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 12) & 1); }
#endif

// get output enable (0 or 1) to pad, after override (pin = 0..29 or 0..47)
INLINE u8 GPIO_OePad(int pin) { return (u8)((*GPIO_STATUS(pin) >> 13) & 1); }
INLINE u8 GPIO_OePad_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 13) & 1); }

// get input (0 or 1) from pad, before override (pin = 0..29 or 0..47)
INLINE u8 GPIO_InPad(int pin) { return (u8)((*GPIO_STATUS(pin) >> 17) & 1); }
INLINE u8 GPIO_InPad_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 17) & 1); }

// get raw input (0 or 1) from pad, before override and before muxing, enables pin input (pin = 0..29 or 0..47)
INLINE u8 GPIO_RawIn(int pin)
{
	GPIO_InEnable(pin);
	return GPIO_InPad(pin);
}

#if RP2040
// get input (0 or 1) to peripheral, after override (pin = 0..29 or 0..47)
INLINE u8 GPIO_InPeri(int pin) { return (u8)((*GPIO_STATUS(pin) >> 19) & 1); }
INLINE u8 GPIO_InPeri_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 19) & 1); }
#endif

#if RP2040
// get interrupt (0 or 1) from pad, before override (pin = 0..29 or 0..47)
INLINE u8 GPIO_IrqPad(int pin) { return (u8)((*GPIO_STATUS(pin) >> 24) & 1); }
INLINE u8 GPIO_IrqPad_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 24) & 1); }
#endif

// get interrupt (0 or 1) to processor, after override (pin = 0..29 or 0..47)
INLINE u8 GPIO_IrqProc(int pin) { return (u8)((*GPIO_STATUS(pin) >> 26) & 1); }
INLINE u8 GPIO_IrqProc_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)((hw->status >> 26) & 1); }

#if RP2040
#define GPIO_OUTOVER_LSB	8
#else
#define GPIO_OUTOVER_LSB	12
#endif

// set Output pin override - normal (default state; pin = 0..29 or 0..47)
INLINE void GPIO_OutOverNormal(int pin)
	{ RegClr(GPIO_CTRL(pin), GPIO_OVER_MASK << GPIO_OUTOVER_LSB); }
INLINE void GPIO_OutOverNormal_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegClr(&hw->ctrl, GPIO_OVER_MASK << GPIO_OUTOVER_LSB); }

// set Output pin override - invert (pin = 0..29 or 0..47)
INLINE void GPIO_OutOverInvert(int pin)
	{ RegMask(GPIO_CTRL(pin), GPIO_OVER_INVERT << GPIO_OUTOVER_LSB, GPIO_OVER_MASK << GPIO_OUTOVER_LSB); }
INLINE void GPIO_OutOverInvert_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, GPIO_OVER_INVERT << GPIO_OUTOVER_LSB, GPIO_OVER_MASK << GPIO_OUTOVER_LSB); }

// set Output pin override - low (pin = 0..29 or 0..47)
INLINE void GPIO_OutOverLow(int pin)
	{ RegMask(GPIO_CTRL(pin), GPIO_OVER_LOW << GPIO_OUTOVER_LSB, GPIO_OVER_MASK << GPIO_OUTOVER_LSB); }
INLINE void GPIO_OutOverLow_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, GPIO_OVER_LOW << GPIO_OUTOVER_LSB, GPIO_OVER_MASK << GPIO_OUTOVER_LSB); }

// set Output pin override - high (pin = 0..29 or 0..47)
INLINE void GPIO_OutOverHigh(int pin)
	{ RegSet(GPIO_CTRL(pin), GPIO_OVER_HIGH << GPIO_OUTOVER_LSB); }
INLINE void GPIO_OutOverHigh_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegSet(&hw->ctrl, GPIO_OVER_HIGH << GPIO_OUTOVER_LSB); }

#if RP2040
#define GPIO_OEOVER_LSB		12
#else
#define GPIO_OEOVER_LSB		14
#endif

// set Output enable pin override - normal (default state; pin = 0..29 or 0..47)
INLINE void GPIO_OEOverNormal(int pin)
	{ RegClr(GPIO_CTRL(pin), GPIO_OVER_MASK << GPIO_OEOVER_LSB); }
INLINE void GPIO_OEOverNormal_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegClr(&hw->ctrl, GPIO_OVER_MASK << GPIO_OEOVER_LSB); }

// set Output enable pin override - invert (pin = 0..29 or 0..47)
INLINE void GPIO_OEOverInvert(int pin)
	{ RegMask(GPIO_CTRL(pin), GPIO_OVER_INVERT << GPIO_OEOVER_LSB, GPIO_OVER_MASK << GPIO_OEOVER_LSB); }
INLINE void GPIO_OEOverInvert_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, GPIO_OVER_INVERT << GPIO_OEOVER_LSB, GPIO_OVER_MASK << GPIO_OEOVER_LSB); }

// set Output enable pin override - disable (pin = 0..29 or 0..47)
INLINE void GPIO_OEOverDisable(int pin)
	{ RegMask(GPIO_CTRL(pin), GPIO_OVER_LOW << GPIO_OEOVER_LSB, GPIO_OVER_MASK << GPIO_OEOVER_LSB); }
INLINE void GPIO_OEOverDisable_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegMask(&hw->ctrl, GPIO_OVER_LOW << GPIO_OEOVER_LSB, GPIO_OVER_MASK << GPIO_OEOVER_LSB); }

// set Output enable pin override - enable (pin = 0..29 or 0..47)
INLINE void GPIO_OEOverEnable(int pin)
	{ RegSet(GPIO_CTRL(pin), GPIO_OVER_HIGH << GPIO_OEOVER_LSB); }
INLINE void GPIO_OEOverEnable_hw(io_bank0_status_ctrl_hw_t* hw)
	{ RegSet(&hw->ctrl, GPIO_OVER_HIGH << GPIO_OEOVER_LSB); }

// set Input pin override - normal (default state; pin = 0..29 or 0..47)
INLINE void GPIO_InOverNormal(int pin) { RegClr(GPIO_CTRL(pin), B16|B17); }
INLINE void GPIO_InOverNormal_hw(io_bank0_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B16|B17); }

// set Input pin override - invert (pin = 0..29 or 0..47)
INLINE void GPIO_InOverInvert(int pin) { RegMask(GPIO_CTRL(pin), B16, B16|B17); }
INLINE void GPIO_InOverInvert_hw(io_bank0_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B16, B16|B17); }

// set Input pin override - low (pin = 0..29 or 0..47)
INLINE void GPIO_InOverLow(int pin) { RegMask(GPIO_CTRL(pin), B17, B16|B17); }
INLINE void GPIO_InOverLow_hw(io_bank0_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B17, B16|B17); }

// set Input pin override - high (pin = 0..29 or 0..47)
INLINE void GPIO_InOverHigh(int pin) { RegSet(GPIO_CTRL(pin), B16|B17); }
INLINE void GPIO_InOverHigh_hw(io_bank0_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B16|B17); }

// set IRQ pin override - normal (default state; pin = 0..29 or 0..47)
INLINE void GPIO_IRQOverNormal(int pin) { RegClr(GPIO_CTRL(pin), B28|B29); }
INLINE void GPIO_IRQOverNormal_hw(io_bank0_status_ctrl_hw_t* hw) { RegClr(&hw->ctrl, B28|B29); }

// set IRQ pin override - invert (pin = 0..29 or 0..47)
INLINE void GPIO_IRQOverInvert(int pin) { RegMask(GPIO_CTRL(pin), B28, B28|B29); }
INLINE void GPIO_IRQOverInvert_hw(io_bank0_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B28, B28|B29); }

// set IRQ pin override - low (pin = 0..29 or 0..47)
INLINE void GPIO_IRQOverLow(int pin) { RegMask(GPIO_CTRL(pin), B29, B28|B29); }
INLINE void GPIO_IRQOverLow_hw(io_bank0_status_ctrl_hw_t* hw) { RegMask(&hw->ctrl, B29, B28|B29); }

// set IRQ pin override - high (pin = 0..29 or 0..47)
INLINE void GPIO_IRQOverHigh(int pin) { RegSet(GPIO_CTRL(pin), B28|B29); }
INLINE void GPIO_IRQOverHigh_hw(io_bank0_status_ctrl_hw_t* hw) { RegSet(&hw->ctrl, B28|B29); }

// set GPIO function GPIO_FNC_* (pin = 0..29 or 0..47)
// Resets pad overrides, enables input and output.
// To setup GPIO:
//  - reset GPIO to default state with GPIO_Reset()
//  - setup output level, output enable, pull-up/pull-down, schmitt, slew, drive strength
//  - set GPIO function
//  - if needed, disable input (ADC) and set overrides
void GPIO_Fnc(int pin, int fnc);

// set GPIO function GPIO_FNC_* with mask (bit '1' to set function of this pin)
// To use pin mask in range (first..last), use function PinRangeMask.
void GPIO_FncMask(gpio_mask_t mask, int fnc);

// set GPIO function GPIO_FNC_* raw, resets pad overrides, no other side effects (pin = 0..29 or 0..47)
INLINE void GPIO_FncRaw(int pin, int fnc) { *GPIO_CTRL(pin) = fnc; }

// get current GPIO function GPIO_FNC_* (pin = 0..29 or 0..47)
INLINE u8 GPIO_GetFnc(int pin) { return (u8)(*GPIO_CTRL(pin) & 0x1f); }
INLINE u8 GPIO_GetFnc_hw(const io_bank0_status_ctrl_hw_t* hw) { return (u8)(hw->ctrl & 0x1f); }

// === GPIO pin interrupt control (pin = 0..29/RP2040 or 0..47/RP2350)

// acknowledge IRQ interrupt for both CPU
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_EDGE* of events to acknowledge
//  Interrupts LEVEL are not latched, they become inactive on end condition.
void GPIO_IRQAck(int pin, int events);

// enable IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to enable

// After enabling interrupt from pin, following steps are necessary:
//   - set handler of IRQ_IO_BANK0 (use function GPIO_SetHandler), or use isr_io_bank0 handler
//   - not necessary, but rather clear pending interrupt of IRQ_IO_BANK0 (use function NVIC_IRQClear)
//   - enable NVIC interrupt of IRQ_IO_BANK0 (use function NVIC_IRQEnable)
//   - enable global interrupt ei()

void GPIO_IRQEnableCpu(int cpu, int pin, int events);
INLINE void GPIO_IRQEnable(int pin, int events) { GPIO_IRQEnableCpu(CpuID(), pin, events); }

// disable IRQ interrupt for selected/current CPU or dormant
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to disable
void GPIO_IRQDisableCpu(int cpu, int pin, int events);
INLINE void GPIO_IRQDisable(int pin, int events) { GPIO_IRQDisableCpu(CpuID(), pin, events); }

// set IRQ handler
INLINE void GPIO_SetHandler(irq_handler_t handler) { SetHandler(IRQ_IO_BANK0, handler); }

// force IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to force
void GPIO_IRQForceCpu(int cpu, int pin, int events);
INLINE void GPIO_IRQForce(int pin, int events) { GPIO_IRQForceCpu(CpuID(), pin, events); }

// clear force IRQ interrupt for selected/current CPU
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to unforce
void GPIO_IRQUnforceCpu(int cpu, int pin, int events);
INLINE void GPIO_IRQUnforce(int pin, int events) { GPIO_IRQUnforceCpu(CpuID(), pin, events); }

// check IRQ interrupt status for selected/current CPU (returns 1 if IRQ is pending)
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of incoming events
u8 GPIO_IRQIsPendingCpu(int cpu, int pin);
INLINE u8 GPIO_IRQIsPending(int pin) { return GPIO_IRQIsPendingCpu(CpuID(), pin); }

// check if IRQ is forced for selected/current CPU (returns 1 if IRQ is pending)
//   cpu = CPU core 0 or 1, use CpuID() to get current core, or 2 = dormant wake
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of forced events
u8 GPIO_IRQIsForcedCpu(int cpu, int pin);
INLINE u8 GPIO_IRQIsForced(int pin) { return GPIO_IRQIsForcedCpu(CpuID(), pin); }

// enable IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to enable
void GPIO_IRQEnableDorm(int pin, int events);

// disable IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to disable
void GPIO_IRQDisableDorm(int pin, int events);

// force IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to force
void GPIO_IRQForceDorm(int pin, int events);

// clear force IRQ interrupt for dormant wake
//   pin = 0..29 or 0..47
//   events = bit mask with IRQ_EVENT_* of events to unforce
void GPIO_IRQUnforceDorm(int pin, int events);

// check IRQ interrupt status for dormant wake (returns 1 if IRQ is pending)
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of incoming events
u8 GPIO_IRQIsPendingDorm(int pin);

// check if IRQ is forced for dormant wake (returns 1 if IRQ is pending)
//   pin = 0..29 or 0..47
//   returns events = bit mask with IRQ_EVENT_* of forced events
u8 GPIO_IRQIsForcedDorm(int pin);

// set generic callback for current core (NULL=disable callback)
// - it will install default IRQ handler (as used with GPIO_SetHandler)
// To use:
// - set callback using the GPIO_IRQSetCallback() function
// - enable interrupts on GPIO pins using the GPIO_IRQEnable() function
// - enable NVIC interrupt using the NVIC_IRQEnable(IRQ_IO_BANK0) command
void GPIO_IRQSetCallback(gpio_irq_callback_t cb);

// === GPIO pin data (pin = 0..29/RP2040 or 0..47,56..63/RP2350 including USB and QSPI)

// get input pin (returns 0 or 1; pin = 0..29 or 0..47,56..63)
INLINE u8 GPIO_In(int pin)
{
#if RP2040
	return (u8)((sio_hw->gpio_in >> pin) & 1);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	u64 in = GPIOC_InAll();
	if (pin >= 32)
	{
		pin -= 32;
		in >>= 32;
	}
	return (u8)(((u32)in >> pin) & 1);
#else
	if (pin >= 32)
		return (u8)((sio_hw->gpio_hi_in >> (pin - 32)) & 1);
	else
		return (u8)((sio_hw->gpio_in >> pin) & 1);
#endif
}

// get all input pins
INLINE gpio_mask_t GPIO_InAll()
{
#if RP2040
	return sio_hw->gpio_in;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	return GPIOC_InAll();
#else
	return sio_hw->gpio_in | ((u64)sio_hw->gpio_hi_in << 32);
#endif
}

// output 0 to pin (pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Out0(int pin)
{
#if RP2040
	sio_hw->gpio_clr = BIT(pin);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutClr(pin);
#else
	if (pin < 32)
		sio_hw->gpio_clr = BIT(pin);
	else
		sio_hw->gpio_hi_clr = BIT(pin - 32);
#endif
}

// output 1 to pin (pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Out1(int pin)
{
#if RP2040
	sio_hw->gpio_set = BIT(pin);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutSet(pin);
#else
	if (pin < 32)
		sio_hw->gpio_set = BIT(pin);
	else
		sio_hw->gpio_hi_set = BIT(pin - 32);
#endif
}

// flip output to pin (pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Flip(int pin)
{
#if RP2040
	sio_hw->gpio_togl = BIT(pin);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutXor(pin);
#else
	if (pin < 32)
		sio_hw->gpio_togl = BIT(pin);
	else
		sio_hw->gpio_hi_togl = BIT(pin - 32);
#endif
}

// output value to pin (val = 0 or val != 0; pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Out(int pin, int val)
{
#if USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_Out(pin, val);
#else
	if (val == 0)
		GPIO_Out0(pin);
	else
		GPIO_Out1(pin);
#endif
}

// output all pins
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_OutAll(gpio_mask_t value)
{
#if RP2040
	sio_hw->gpio_out = value;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutAll(value);
#else
	sio_hw->gpio_out = (u32)value;
	sio_hw->gpio_hi_out = (u32)(value >> 32);
#endif
}

// output masked values to pins
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_OutMask(gpio_mask_t mask, gpio_mask_t value)
{
#if RP2040
	sio_hw->gpio_togl = (sio_hw->gpio_out ^ value) & mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	value ^= GPIOC_GetOutAll();
	value &= mask;
	GPIOC_OutXorAll(value);
#else
	sio_hw->gpio_togl = (sio_hw->gpio_out ^ (u32)value) & (u32)mask;
	sio_hw->gpio_hi_togl = (sio_hw->gpio_hi_out ^ (u32)(value >> 32)) & (u32)(mask >> 32);
#endif
}

// clear output pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_ClrMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_clr = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutClrAll(mask);
#else
	sio_hw->gpio_clr = (u32)mask;
	sio_hw->gpio_hi_clr = (u32)(mask >> 32);
#endif
}

// set output pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_SetMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_set = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutSetAll(mask);
#else
	sio_hw->gpio_set = (u32)mask;
	sio_hw->gpio_hi_set = (u32)(mask >> 32);
#endif
}

// flip output pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_FlipMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_togl = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OutXorAll(mask);
#else
	sio_hw->gpio_togl = (u32)mask;
	sio_hw->gpio_hi_togl = (u32)(mask >> 32);
#endif
}

// get last output value to pin (returns 0 or 1; pin = 0..29 or 0..47,56..63)
INLINE u8 GPIO_GetOut(int pin)
{
#if RP2040
	return (u8)((sio_hw->gpio_out >> pin) & 1);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	u64 out = GPIOC_GetOutAll();
	if (pin >= 32)
	{
		pin -= 32;
		out >>= 32;
	}
	return (u8)(((u32)out >> pin) & 1);
#else
	if (pin >= 32)
		return (u8)((sio_hw->gpio_hi_out >> (pin - 32)) & 1);
	else
		return (u8)((sio_hw->gpio_out >> pin) & 1);
#endif
}

// get all last output values to pins
INLINE gpio_mask_t GPIO_GetOutAll()
{
#if RP2040
	return sio_hw->gpio_out;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	return GPIOC_GetOutAll();
#else
	return sio_hw->gpio_out | ((u64)sio_hw->gpio_hi_out << 32);
#endif
}

// set output direction of pin (enable output mode; pin = 0..29 or 0..47,56..63)
INLINE void GPIO_DirOut(int pin)
{
#if RP2040
	sio_hw->gpio_oe_set = BIT(pin);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OeSet(pin);
#else
	if (pin < 32)
		sio_hw->gpio_oe_set = BIT(pin);
	else
		sio_hw->gpio_hi_oe_set = BIT(pin - 32);
#endif
}

// set input direction of pin (disable output mode; pin = 0..29 or 0..47,56..63)
INLINE void GPIO_DirIn(int pin)
{
#if RP2040
	sio_hw->gpio_oe_clr = BIT(pin);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OeClr(pin);
#else
	if (pin < 32)
		sio_hw->gpio_oe_clr = BIT(pin);
	else
		sio_hw->gpio_hi_oe_clr = BIT(pin - 32);
#endif
}

// set pin output direction 0 in, <>0 out (pin = 0..29 or 0..47,56..63)
INLINE void GPIO_Dir(int pin, int val)
{
#if USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_Oe(pin, val);
#else
	if (val == 0)
		GPIO_DirIn(pin);
	else
		GPIO_DirOut(pin);
#endif
}

// set output direction of pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_DirOutMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_oe_set = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OeSetAll(mask);
#else
	sio_hw->gpio_oe_set = (u32)mask;
	sio_hw->gpio_hi_oe_set = (u32)(mask >> 32);
#endif
}

// set input direction of pins masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_DirInMask(gpio_mask_t mask)
{
#if RP2040
	sio_hw->gpio_oe_clr = mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OeClrAll(mask);
#else
	sio_hw->gpio_oe_clr = (u32)mask;
	sio_hw->gpio_hi_oe_clr = (u32)(mask >> 32);
#endif
}

// set direction masked
// To use pin mask in range (first..last), use function PinRangeMask.
INLINE void GPIO_SetDirMask(gpio_mask_t mask, gpio_mask_t outval)
{
#if RP2040
	sio_hw->gpio_oe_togl = (sio_hw->gpio_oe ^ outval) & mask;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	outval ^= GPIOC_GetOeAll();
	outval &= mask;
	GPIOC_OeXorAll(outval);
#else
	sio_hw->gpio_oe_togl = (sio_hw->gpio_oe ^ (u32)outval) & (u32)mask;
	sio_hw->gpio_hi_oe_togl = (sio_hw->gpio_hi_oe ^ (u32)(outval >> 32)) & (u32)(mask >> 32);
#endif
}

// set direction of all pins
INLINE void GPIO_SetDirAll(gpio_mask_t outval)
{
#if RP2040
	sio_hw->gpio_oe = outval;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	GPIOC_OeAll(outval);
#else
	sio_hw->gpio_oe = (u32)outval;
	sio_hw->gpio_hi_oe = (u32)(outval >> 32);
#endif
}

// get output direction of pin (returns 0=input or 1=output; pin = 0..29 or 0..47,56..63)
INLINE u8 GPIO_GetDir(int pin)
{
#if RP2040
	return (u8)((sio_hw->gpio_oe >> pin) & 1);
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	u64 oe = GPIOC_GetOeAll();
	if (pin >= 32)
	{
		pin -= 32;
		oe >>= 32;
	}
	return (u8)(((u32)oe >> pin) & 1);
#else
	if (pin >= 32)
		return (u8)((sio_hw->gpio_hi_oe >> (pin - 32)) & 1);
	else
		return (u8)((sio_hw->gpio_oe >> pin) & 1);
#endif
}

// get output direction of all pins (0=input, 1=output)
INLINE gpio_mask_t GPIO_GetDirAll()
{
#if RP2040
	return sio_hw->gpio_oe;
#elif USE_GPIOCOPROC	// use GPIO coprocessor (only RP2350 ARM; sdk_gpio_coproc.h)
	return GPIOC_GetOeAll();
#else
	return sio_hw->gpio_oe | ((u64)sio_hw->gpio_hi_oe << 32);
#endif
}

// initialize GPIO pin base function, reset pin, set input mode, LOW output value (pin = 0..29 or 0..47,56..63)
void GPIO_Init(int pin);

// initialize GPIO pin base function masked (bit '1' = initialize this pin)
// To use pin mask in range (first..last), use function PinRangeMask.
void GPIO_InitMask(gpio_mask_t mask);

// reset GPIO pin (return to reset state) (pin = 0..29 or 0..47,56..63)
void GPIO_Reset(int pin);

// reset GPIO pins masked (return to reset state)
// To use pin mask in range (first..last), use function PinRangeMask.
void GPIO_ResetMask(gpio_mask_t mask);

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

#if RP2350B // QFN-80
#define NUM_BANK0_GPIOS	48		// number of GPIO pins
typedef u64 raw_irq_mask_type_t;	// 64-bit pin mask
#else // QFN-60
#define NUM_BANK0_GPIOS	30		// number of GPIO pins
typedef u32 raw_irq_mask_type_t;	// 32-bit pin mask
#endif

#if RP2040
// functions
enum gpio_function {
	GPIO_FUNC_XIP = 0,
	GPIO_FUNC_SPI = 1,
	GPIO_FUNC_UART = 2,
	GPIO_FUNC_I2C = 3,
	GPIO_FUNC_PWM = 4,
	GPIO_FUNC_SIO = 5,
	GPIO_FUNC_PIO0 = 6,
	GPIO_FUNC_PIO1 = 7,
	GPIO_FUNC_GPCK = 8,
	GPIO_FUNC_USB = 9,
	GPIO_FUNC_NULL = 0x1f,
};
#else
//typedef enum gpio_function_rp2350 {
enum gpio_function {
	GPIO_FUNC_HSTX = 0,
	GPIO_FUNC_SPI = 1,
	GPIO_FUNC_UART = 2,
	GPIO_FUNC_I2C = 3,
	GPIO_FUNC_PWM = 4,
	GPIO_FUNC_SIO = 5,
	GPIO_FUNC_PIO0 = 6,
	GPIO_FUNC_PIO1 = 7,
	GPIO_FUNC_PIO2 = 8,
	GPIO_FUNC_GPCK = 9,
	GPIO_FUNC_XIP_CS1 = 9,
	GPIO_FUNC_CORESIGHT_TRACE = 9,
	GPIO_FUNC_USB = 10,
	GPIO_FUNC_UART_AUX = 11,
	GPIO_FUNC_NULL = 0x1f,
};// gpio_function_t;
#endif

// direction flag
#define GPIO_OUT 1
#define GPIO_IN 0

// GPIO Interrupt level definitions (GPIO events)
//  Level High: the GPIO pin is a logical 1 (not latched interrupt request)
//  Level Low: the GPIO pin is a logical 0 (not latched interrupt request)
//  Edge High: the GPIO has transitioned from a logical 0 to a logical 1 (interrupt request is latched in INTR register)
//  Edge Low: the GPIO has transitioned from a logical 1 to a logical 0 (interrupt request is latched in INTR register)
enum gpio_irq_level {
	GPIO_IRQ_LEVEL_LOW = 0x1u,
	GPIO_IRQ_LEVEL_HIGH = 0x2u,
	GPIO_IRQ_EDGE_FALL = 0x4u,
	GPIO_IRQ_EDGE_RISE = 0x8u,
};

// Slew rate limiting levels for GPIO outputs
enum gpio_slew_rate {
	GPIO_SLEW_RATE_SLOW = 0,	// Slew rate limiting enabled
	GPIO_SLEW_RATE_FAST = 1		// Slew rate limiting disabled
};

// Drive strength levels for GPIO outputs
enum gpio_drive_strength {
	GPIO_DRIVE_STRENGTH_2MA = 0,
	GPIO_DRIVE_STRENGTH_4MA = 1,
	GPIO_DRIVE_STRENGTH_8MA = 2,
	GPIO_DRIVE_STRENGTH_12MA = 3
};

enum gpio_override {
	GPIO_OVERRIDE_NORMAL	= 0,
	GPIO_OVERRIDE_INVERT	= 1,
	GPIO_OVERRIDE_LOW	= 2,
	GPIO_OVERRIDE_HIGH	= 3,
};

// === Pad Controls + IO Muxing

// Select GPIO function
INLINE void gpio_set_function(uint gpio, enum gpio_function fn) { GPIO_Fnc(gpio, fn); }

// Determine current GPIO function
INLINE enum gpio_function gpio_get_function(uint gpio) { return (enum gpio_function)GPIO_GetFnc(gpio); }

// Select up and down pulls on specific GPIO
INLINE void gpio_set_pulls(uint gpio, bool up, bool down)
{
	io32* pad = GPIO_PadHw(gpio);
	if (up) RegSet(pad, B3); else RegClr(pad, B3);
	if (down) RegSet(pad, B2); else RegClr(pad, B2);
}

// Set specified GPIO to be pulled up.
INLINE void gpio_pull_up(uint gpio) { GPIO_PullUp(gpio); }

// Determine if the specified GPIO is pulled up.
INLINE bool gpio_is_pulled_up(uint gpio) { return (pads_bank0_hw->io[gpio] & B3) != 0; }

// Set specified GPIO to be pulled down.
INLINE void gpio_pull_down(uint gpio) { GPIO_PullDown(gpio); }

// Determine if the specified GPIO is pulled down.
INLINE bool gpio_is_pulled_down(uint gpio) { return (pads_bank0_hw->io[gpio] & B2) != 0; }

// Disable pulls on specified GPIO
INLINE void gpio_disable_pulls(uint gpio) { GPIO_NoPull(gpio); }

// Set GPIO IRQ override
INLINE void gpio_set_irqover(uint gpio, enum gpio_override over) { RegMask(GPIO_CTRL(gpio), over << 28, B28|B29); }

// Set GPIO output override
INLINE void gpio_set_outover(uint gpio, enum gpio_override over) { RegMask(GPIO_CTRL(gpio), over << 8, B8|B9); }

// Select GPIO input override
INLINE void gpio_set_inover(uint gpio, enum gpio_override over) { RegMask(GPIO_CTRL(gpio), over << 16, B16|B17); }

// Select GPIO output enable override
INLINE void gpio_set_oeover(uint gpio, enum gpio_override over) { RegMask(GPIO_CTRL(gpio), over << 12, B12|B13); }

// Enable GPIO input
INLINE void gpio_set_input_enabled(uint gpio, bool enabled)
	{ if (enabled) GPIO_InEnable(gpio); else GPIO_InDisable(gpio); }

// Enable/disable GPIO input hysteresis (Schmitt trigger)
INLINE void gpio_set_input_hysteresis_enabled(uint gpio, bool enabled)
	{ if (enabled)  GPIO_SchmittEnable(gpio); else GPIO_SchmittDisable(gpio); }

// Determine whether input hysteresis is enabled on a specified GPIO
INLINE bool gpio_is_input_hysteresis_enabled(uint gpio) { return (*GPIO_PAD(gpio) & B1) != 0; }

// Set output slew rate for a specified GPIO
INLINE void gpio_set_slew_rate(uint gpio, enum gpio_slew_rate slew)
	{ if (slew == GPIO_SLEW_RATE_SLOW) GPIO_Slow(gpio); else GPIO_Fast(gpio); }

// Determine current slew rate for a specified GPIO
INLINE enum gpio_slew_rate gpio_get_slew_rate(uint gpio)
	{ return (enum gpio_slew_rate)(*GPIO_PAD(gpio) & B0); }

// Set drive strength for a specified GPIO
INLINE void gpio_set_drive_strength(uint gpio, enum gpio_drive_strength drive) { GPIO_Drive(gpio, drive); }

// Determine current drive strength for a specified GPIO
INLINE enum gpio_drive_strength gpio_get_drive_strength(uint gpio)
	{ return (enum gpio_drive_strength)((*GPIO_PAD(gpio) >> 4) & 3); }

// Enable or disable specific interrupt events for specified GPIO
// Events is a bitmask of the following \ref gpio_irq_level values:
// bit | constant            | interrupt
// ----|----------------------------------------------------------
//   0 | GPIO_IRQ_LEVEL_LOW  | Continuously while level is low
//   1 | GPIO_IRQ_LEVEL_HIGH | Continuously while level is high
//   2 | GPIO_IRQ_EDGE_FALL  | On each transition from high to low
//   3 | GPIO_IRQ_EDGE_RISE  | On each transition from low to high
INLINE void gpio_set_irq_enabled(uint gpio, uint32_t event_mask, bool enabled)
	{ if (enabled) GPIO_IRQEnable(gpio, event_mask); else GPIO_IRQDisable(gpio, event_mask); }

// Set the generic callback used for GPIO IRQ events for the current core
INLINE void gpio_set_irq_callback(gpio_irq_callback_t callback) { GPIO_IRQSetCallback(callback); }

// Convenience function which performs multiple GPIO IRQ related initializations
INLINE void gpio_set_irq_enabled_with_callback(uint gpio, u32 event_mask, bool enabled, gpio_irq_callback_t callback)
{
	gpio_set_irq_enabled(gpio, event_mask, enabled);
	gpio_set_irq_callback(callback);
	if (enabled) irq_set_enabled(IO_IRQ_BANK0, true);
}

// Enable dormant wake up interrupt for specified GPIO and events
INLINE void gpio_set_dormant_irq_enabled(uint gpio, uint32_t event_mask, bool enabled)
	{ if (enabled) GPIO_IRQEnableDorm(gpio, event_mask); else GPIO_IRQDisableDorm(gpio, event_mask); }

// Return the current interrupt status (pending events) for the given GPIO
INLINE uint32_t gpio_get_irq_event_mask(uint gpio) { return GPIO_IRQIsPending(gpio); }

// Acknowledge a GPIO interrupt for the specified events on the calling core
INLINE void gpio_acknowledge_irq(uint gpio, uint32_t event_mask) { GPIO_IRQAck(gpio, event_mask); }

// Initialise a GPIO for (enabled I/O and set func to GPIO_FUNC_SIO)
INLINE void gpio_init(uint gpio) { GPIO_Init(gpio); }

// Resets a GPIO back to the NULL function, i.e. disables it.
INLINE void gpio_deinit(uint gpio) { GPIO_Reset(gpio); }

// Initialise multiple GPIOs (enabled I/O and set func to GPIO_FUNC_SIO)
INLINE void gpio_init_mask(uint gpio_mask) { GPIO_InitMask(gpio_mask); }

// === Input

// Get state of a single specified GPIO
INLINE bool gpio_get(uint gpio) { return GPIO_In(gpio) != 0; }

// Get raw value of all GPIOs
INLINE u32 gpio_get_all(void) { return GPIO_InAll(); }

// === Output

// Drive high every GPIO appearing in mask
INLINE void gpio_set_mask(u32 mask) { GPIO_SetMask(mask); }

// Drive low every GPIO appearing in mask
INLINE void gpio_clr_mask(u32 mask) { GPIO_ClrMask(mask); }

// Toggle every GPIO appearing in mask
INLINE void gpio_xor_mask(u32 mask) { GPIO_FlipMask(mask); }

// Drive GPIO high/low depending on parameters
INLINE void gpio_put_masked(u32 mask, u32 value) { GPIO_OutMask(mask, value); }

// Drive all pins simultaneously
INLINE void gpio_put_all(u32 value) { GPIO_OutAll(value); }

// Drive a single GPIO high/low
INLINE void gpio_put(uint gpio, bool value) { GPIO_Out(gpio, value); }

// Determine whether a GPIO is currently driven high or low
INLINE bool gpio_get_out_level(uint gpio) { return GPIO_GetOut(gpio) != 0; }

// === Direction

// Set a number of GPIOs to output
INLINE void gpio_set_dir_out_masked(u32 mask) { GPIO_DirOutMask(mask); }

// Set a number of GPIOs to input
INLINE void gpio_set_dir_in_masked(u32 mask) { GPIO_DirInMask(mask); }

// Set multiple GPIO directions
INLINE void gpio_set_dir_masked(u32 mask, u32 value) { GPIO_SetDirMask(mask, value); }

// Set direction of all pins simultaneously.
INLINE  void gpio_set_dir_all_bits(u32 values) { GPIO_SetDirAll(values); }

// Set a single GPIO direction
INLINE void gpio_set_dir(uint gpio, bool out) { if (out) GPIO_DirOut(gpio); else GPIO_DirIn(gpio); }

// Check if a specific GPIO direction is OUT
INLINE bool gpio_is_dir_out(uint gpio) { return GPIO_GetDir(gpio) != 0; }

// Get a specific GPIO direction
INLINE uint gpio_get_dir(uint gpio) { return GPIO_GetDir(gpio); }

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_GPIO_H
