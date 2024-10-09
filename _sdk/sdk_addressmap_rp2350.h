
// ****************************************************************************
//
//                        Register address offsets RP2350
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

// register memory map RP2350 - base address
#define ROM_BASE			0x00000000
#define XIP_BASE			0x10000000
#define XIP_SRAM_BASE			0x13ffc000
#define XIP_END				0x14000000
#define XIP_NOCACHE_NOALLOC_BASE	0x14000000
#define XIP_SRAM_END			0x14000000
#define XIP_NOCACHE_NOALLOC_END		0x18000000
#define XIP_MAINTENANCE_BASE		0x18000000
#define XIP_NOCACHE_NOALLOC_NOTRANSLATE_BASE 0x1c000000
#define SRAM0_BASE			0x20000000
#define XIP_NOCACHE_NOALLOC_NOTRANSLATE_END 0x20000000
#define SRAM_BASE			0x20000000
#define SRAM_STRIPED_BASE		0x20000000
#define SRAM4_BASE			0x20040000
#define SRAM8_BASE			0x20080000
#define SRAM_STRIPED_END		0x20080000
#define SRAM_SCRATCH_X_BASE		0x20080000
#define SRAM9_BASE			0x20081000
#define SRAM_SCRATCH_Y_BASE		0x20081000
#define SRAM_END			0x20082000
#define SYSINFO_BASE			0x40000000
#define SYSCFG_BASE			0x40008000
#define CLOCKS_BASE			0x40010000
#define PSM_BASE			0x40018000
#define RESETS_BASE			0x40020000
#define IO_BANK0_BASE			0x40028000
#define IO_QSPI_BASE			0x40030000
#define PADS_BANK0_BASE			0x40038000
#define PADS_QSPI_BASE			0x40040000
#define XOSC_BASE			0x40048000
#define PLL_SYS_BASE			0x40050000
#define PLL_USB_BASE			0x40058000
#define ACCESSCTRL_BASE			0x40060000
#define BUSCTRL_BASE			0x40068000
#define UART0_BASE			0x40070000
#define UART1_BASE			0x40078000
#define SPI0_BASE			0x40080000
#define SPI1_BASE			0x40088000
#define I2C0_BASE			0x40090000
#define I2C1_BASE			0x40098000
#define ADC_BASE			0x400a0000
#define PWM_BASE			0x400a8000
#define TIMER0_BASE			0x400b0000
#define TIMER1_BASE			0x400b8000
#define HSTX_CTRL_BASE			0x400c0000
#define XIP_CTRL_BASE			0x400c8000
#define XIP_QMI_BASE			0x400d0000
#define WATCHDOG_BASE			0x400d8000
#define BOOTRAM_BASE			0x400e0000
#define BOOTRAM_END			0x400e0400
#define ROSC_BASE			0x400e8000
#define TRNG_BASE			0x400f0000
#define SHA256_BASE			0x400f8000
#define POWMAN_BASE			0x40100000
#define TICKS_BASE			0x40108000
#define OTP_BASE			0x40120000
#define OTP_DATA_BASE			0x40130000
#define OTP_DATA_RAW_BASE		0x40134000
#define OTP_DATA_GUARDED_BASE		0x40138000
#define OTP_DATA_RAW_GUARDED_BASE	0x4013c000
#define CORESIGHT_PERIPH_BASE		0x40140000
#define CORESIGHT_ROMTABLE_BASE		0x40140000
#define CORESIGHT_AHB_AP_CORE0_BASE	0x40142000
#define CORESIGHT_AHB_AP_CORE1_BASE	0x40144000
#define CORESIGHT_TIMESTAMP_GEN_BASE	0x40146000
#define CORESIGHT_ATB_FUNNEL_BASE	0x40147000
#define CORESIGHT_TPIU_BASE		0x40148000
#define CORESIGHT_CTI_BASE		0x40149000
#define CORESIGHT_APB_AP_RISCV_BASE	0x4014a000
#define DFT_BASE			0x40150000
#define GLITCH_DETECTOR_BASE		0x40158000
#define TBMAN_BASE			0x40160000
#define DMA_BASE			0x50000000
#define USBCTRL_BASE			0x50100000
#define USBCTRL_DPRAM_BASE		0x50100000
#define USBCTRL_REGS_BASE		0x50110000
#define PIO0_BASE			0x50200000
#define PIO1_BASE			0x50300000
#define PIO2_BASE			0x50400000
#define XIP_AUX_BASE			0x50500000
#define HSTX_FIFO_BASE			0x50600000
#define CORESIGHT_TRACE_BASE		0x50700000
#define SIO_BASE			0xd0000000
#define SIO_NONSEC_BASE			0xd0020000
#define PPB_BASE			0xe0000000
#define PPB_NONSEC_BASE			0xe0020000
#define EPPB_BASE			0xe0080000
