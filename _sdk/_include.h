// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#ifndef _SDK_INCLUDE_H
#define _SDK_INCLUDE_H

#include "sdk_addressmap.h"	// Register address offsets
#include "sdk_dreq.h"		// DREQ data request channels
#include "sdk_sftable.h"	// offsets of floating-point functions

#if USE_ADC
#include "inc/sdk_adc.h"	// ADC
#endif

#include "inc/sdk_bootrom.h"	// Boot ROM
#include "inc/sdk_clocks.h"	// Clocks
#include "inc/sdk_cpu.h"	// CPU control
#include "inc/sdk_divider.h"	// integer divider

#if USE_DMA
#include "inc/sdk_dma.h"	// DMA controller
#endif

#include "inc/sdk_double.h"	// double-floating-point

#include "inc/sdk_fatal.h"	// fatal error message

#if USE_FIFO
#include "inc/sdk_fifo.h"	// inter-core FIFO
#endif

#if USE_FLASH
#include "inc/sdk_flash.h"	// Flash memory
#endif

#include "inc/sdk_float.h"	// single-floating-point

#include "inc/sdk_gpio.h"	// GPIO pins

#if USE_HSTX && !RP2040
#include "inc/sdk_hstx.h"	// HSTX High-speed serial transmit
#endif

#if USE_I2C
#include "inc/sdk_i2c.h"	// I2C interface
#endif

#if USE_INTERP
#include "inc/sdk_interp.h"	// interpolator
#endif

#if USE_IRQ
#include "inc/sdk_irq.h"	// IRQ interrupt request
#endif

#include "inc/sdk_multicore.h"	// multicore

#if USE_PIO
#include "inc/sdk_pio.h"	// PIO
#endif

#if USE_PLL
#include "inc/sdk_pll.h"	// PLL phase-locked loop
#endif

#if USE_PSRAM
#include "inc/sdk_psram.h"	// PSRAM Memory
#endif

#if RP2350
#include "inc/sdk_powman.h"	// Power Manager
#endif

#if USE_PWM
#include "inc/sdk_pwm.h"	// PWM
#endif

#if RP2350
#include "inc/sdk_qmi.h"	// QMI QSPI memory interface
#endif

#include "inc/sdk_qspi.h"	// QSPI flash pins
#include "inc/sdk_reset.h"	// reset and power-on

#if USE_ROSC
#include "inc/sdk_rosc.h"	// ROSC ring oscillator
#endif

#if RP2040 && USE_RTC
#include "inc/sdk_rtc.h"	// RTC Real-time clock
#endif

#include "inc/sdk_scb.h"	// ARM system control block

#if USE_SHA256 && !RP2040
#include "inc/sdk_sha256.h"	// SHA-256 accelerator
#endif

#include "inc/sdk_sio.h"	// SIO registers

#if USE_SPI
#include "inc/sdk_spi.h"	// SPI interface
#endif

#if USE_SPINLOCK
#include "inc/sdk_spinlock.h"	// spin lock
#endif

#if RP2040
#include "inc/sdk_ssi.h"	// SSI synchronous serial interface
#endif

#if USE_SYSTICK
#include "inc/sdk_systick.h"	// SysTick system timer
#endif

#if !RP2040
#include "inc/sdk_ticks.h"	// Tick generators
#endif

#if USE_TIMER
#include "inc/sdk_timer.h"	// Timer
#endif

#if USE_TMDS && !RP2040
#include "inc/sdk_tmds.h"	// TMDS encoder
#endif

#if USE_TRNG && !RP2040
#include "inc/sdk_trng.h"	// TRNG True Random Number Generator
#endif

#if USE_UART
#include "inc/sdk_uart.h"	// UART serial port
#endif

#if RP2040
#include "inc/sdk_vreg.h"	// voltage regulator
#endif

#include "inc/sdk_watchdog.h"	// Watchdog timer

#include "inc/sdk_xip.h"	// XIP flash control

#if USE_XOSC
#include "inc/sdk_xosc.h"	// XOSC cystal oscillator
#endif




#if USE_USBPORT
#include "usb_inc/sdk_usbport.h"	// USB Mini-Port
#endif

#if USE_USB
#include "usb_inc/sdk_usb_def.h"	// USB definitions
#include "usb_inc/sdk_usb_phy.h"	// USB physical layer
#include "usb_inc/sdk_usb_dev.h"	// USB device class driver
#include "usb_inc/sdk_usb_dev_cdc.h"	// USB Communication Device Class (device)
#include "usb_inc/sdk_usb_dev_hid.h"	// USB Human Interface Device Class (device)
/*
#include "usb_inc/sdk_usb_dev_audio.h"	// USB Audio Device Class (device)
#include "usb_inc/sdk_usb_dev_bth.h"	// USB Bluetooth Device Class (device)
#include "usb_inc/sdk_usb_dev_dfu.h"	// USB Firmware Upgrade Device Class (device)
#include "usb_inc/sdk_usb_dev_midi.h"	// USB Midi Device Class (device)
#include "usb_inc/sdk_usb_dev_msc.h"	// USB Mass Storage Class (device)
#include "usb_inc/sdk_usb_dev_net.h"	// USB Network Adapter Device Class (device)
#include "usb_inc/sdk_usb_dev_tmc.h"	// USB Test and Measurement Device Class (device)
#include "usb_inc/sdk_usb_dev_vendor.h"	// USB Vendor Device Class (device)
#include "usb_inc/sdk_usb_dev_video.h"	// USB Video Device Class (device)
*/

#include "usb_inc/sdk_usb_host.h"	// USB Host Class Driver
#include "usb_inc/sdk_usb_host_cdc.h"	// USB Communication Device Class (host)
#include "usb_inc/sdk_usb_host_hid.h"	// USB Human Interface Device Class (host)
#endif

/*
#include "usb_inc/sdk_usb_host_msc.h"	// USB Mass Storage Class (host)
#include "usb_inc/sdk_usb_host_vendor.h" // USB Vendor Device Class (host)
*/

#endif // _SDK_INCLUDE_H
