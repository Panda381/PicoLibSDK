
// ****************************************************************************
//
//                        Register address offsets RP2040
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

// Included from both C and assembly.

// register memory map RP2040 - base address
#define ROM_BASE		0x00000000
#define XIP_BASE		0x10000000
#define XIP_MAIN_BASE		0x10000000
#define XIP_NOALLOC_BASE	0x11000000
#define XIP_NOCACHE_BASE	0x12000000
#define XIP_NOCACHE_NOALLOC_BASE 0x13000000
#define XIP_CTRL_BASE		0x14000000
#define XIP_SRAM_BASE		0x15000000
#define XIP_SRAM_END		0x15004000
#define XIP_SSI_BASE		0x18000000	// SSI flash interface
#define SRAM_BASE		0x20000000
#define SRAM_STRIPED_BASE	0x20000000
#define SRAM_STRIPED_END	0x20040000
#define SRAM4_BASE		0x20040000
#define SRAM5_BASE		0x20041000
#define SRAM_END		0x20042000
#define SRAM0_BASE		0x21000000
#define SRAM1_BASE		0x21010000
#define SRAM2_BASE		0x21020000
#define SRAM3_BASE		0x21030000
#define SYSINFO_BASE		0x40000000
#define SYSCFG_BASE		0x40004000
#define CLOCKS_BASE		0x40008000	// clocks
#define RESETS_BASE		0x4000c000	// subsystem resets
#define PSM_BASE		0x40010000	// power-on state machine
#define IO_BANK0_BASE		0x40014000	// GPIO pads
#define IO_QSPI_BASE		0x40018000	// QSPI pads
#define PADS_BANK0_BASE		0x4001c000	// GPIO pad control
#define PADS_QSPI_BASE		0x40020000	// QSPI pad control
#define XOSC_BASE		0x40024000	// XOSC crystal oscillator
#define PLL_SYS_BASE		0x40028000
#define PLL_USB_BASE		0x4002c000
#define BUSCTRL_BASE		0x40030000
#define UART0_BASE		0x40034000	// UART0 serial port
#define UART1_BASE		0x40038000	// UART1 serial port
#define SPI0_BASE		0x4003c000	// SPI0 interface
#define SPI1_BASE		0x40040000	// SPI1 interface
#define I2C0_BASE		0x40044000	// I2C0 interface
#define I2C1_BASE		0x40048000	// I2C1 interface
#define ADC_BASE		0x4004c000	// ADC controller
#define PWM_BASE		0x40050000	// PWM Pulse Width Modulator
#define TIMER_BASE		0x40054000	// us timer
#define WATCHDOG_BASE		0x40058000	// Watchdog timer
#define RTC_BASE		0x4005c000	// RTC real-time clock
#define ROSC_BASE		0x40060000	// ROSC ring oscillator
#define VREG_AND_CHIP_RESET_BASE 0x40064000
#define TBMAN_BASE		0x4006c000
#define DMA_BASE		0x50000000	// DMA controller
#define USBCTRL_DPRAM_BASE	0x50100000
#define USBCTRL_BASE		0x50100000
#define USBCTRL_REGS_BASE	0x50110000
#define PIO0_BASE		0x50200000	// PIO 0
#define PIO1_BASE		0x50300000	// PIO 1
#define XIP_AUX_BASE		0x50400000
#define SIO_BASE		0xd0000000	// SIO registers (does not support aliases for atomic access!)
#define PPB_BASE		0xe0000000	// Cortex-M0+ internal registers (system control block)

#define FLASH_BASE		0x10000000
#define PSRAM_BASE		0x11000000
#define SRAM_BASE		0x20000000
