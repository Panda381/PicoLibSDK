// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

#include "sdk_addressmap.h"	// Register address offsets
#include "sdk_dreq.h"		// DREQ data request channels
#include "sdk_sftable.h"	// offsets of floating-point functions

#include "inc/sdk_adc.h"	// ADC
#include "inc/sdk_bootrom.h"	// Boot ROM
#include "inc/sdk_clocks.h"	// Clocks
#include "inc/sdk_cpu.h"	// CPU control
#include "inc/sdk_divider.h"	// integer divider
#include "inc/sdk_dma.h"	// DMA controller
#include "inc/sdk_double.h"	// double-floating-point
#include "inc/sdk_fifo.h"	// inter-core FIFO
#include "inc/sdk_flash.h"	// Flash memory
#include "inc/sdk_float.h"	// single-floating-point
#include "inc/sdk_gpio.h"	// GPIO pins
#include "inc/sdk_i2c.h"	// I2C interface
#include "inc/sdk_interp.h"	// interpolator
#include "inc/sdk_irq.h"	// IRQ interrupt request
#include "inc/sdk_multicore.h"	// multicore
#include "inc/sdk_pio.h"	// PIO
#include "inc/sdk_pll.h"	// PLL phase-locked loop
#include "inc/sdk_pwm.h"	// PWM
#include "inc/sdk_qspi.h"	// QSPI flash pins
#include "inc/sdk_reset.h"	// reset and power-on
#include "inc/sdk_rosc.h"	// ROSC ring oscillator
#include "inc/sdk_rtc.h"	// RTC Real-time clock
#include "inc/sdk_spi.h"	// SPI interface
#include "inc/sdk_spinlock.h"	// spin lock
#include "inc/sdk_ssi.h"	// SSI synchronous serial interface
#include "inc/sdk_systick.h"	// SysTick system timer
#include "inc/sdk_timer.h"	// Timer
#include "inc/sdk_uart.h"	// UART serial port
#include "inc/sdk_vreg.h"	// voltage regulator
#include "inc/sdk_watchdog.h"	// Watchdog timer
#include "inc/sdk_xip.h"	// XIP flash control
#include "inc/sdk_xosc.h"	// XOSC cystal oscillator

#include "usb_inc/sdk_usbport.h"	// USB Mini-Port

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

/*
#include "usb_inc/sdk_usb_host_msc.h"	// USB Mass Storage Class (host)
#include "usb_inc/sdk_usb_host_vendor.h" // USB Vendor Device Class (host)
*/
