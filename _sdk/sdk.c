
// ****************************************************************************
//
//                      SDK files - for faster bulk compilation
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

#include "../global.h"	// globals

#if USE_ADC
#include "src/sdk_adc.c"
#endif

#include "src/sdk_bootrom.c"

#include "src/sdk_clocks.c"

#include "src/sdk_cpu.c"

#if USE_DMA
#include "src/sdk_dma.c"
#endif

#if USE_DOUBLE
#include "src/sdk_double.c"
#endif

#include "src/sdk_fatal.c"

#if USE_FIFO
#include "src/sdk_fifo.c"
#endif

#if USE_FLASH
#include "src/sdk_flash.c"
#endif

#if USE_FLOAT
#include "src/sdk_float.c"
#endif

#include "src/sdk_gpio.c"

#if USE_HSTX && !RP2040
#include "src/sdk_hstx.c"
#endif

#if USE_I2C
#include "src/sdk_i2c.c"
#endif

#if USE_INTERP
#include "src/sdk_interp.c"
#endif

#if USE_IRQ
#include "src/sdk_irq.c"
#endif

#include "src/sdk_multicore.c"

#if USE_PIO
#include "src/sdk_pio.c"
#endif

#if USE_PLL
#include "src/sdk_pll.c"
#endif

#if USE_PSRAM
#include "src/sdk_psram.c"
#endif

#if RP2350
#include "src/sdk_powman.c"
#endif

#if USE_PWM
#include "src/sdk_pwm.c"
#endif

#if RP2350
#include "src/sdk_qmi.c"
#endif

#include "src/sdk_qspi.c"

#include "src/sdk_reset.c"

#if USE_ROSC
#include "src/sdk_rosc.c"
#endif

#if RP2040 && USE_RTC
#include "src/sdk_rtc.c"
#endif

#include "src/sdk_runtime.c"

#if USE_SPI
#include "src/sdk_spi.c"
#endif

#if USE_SPINLOCK
#include "src/sdk_spinlock.c"
#endif

#if RP2040
#include "src/sdk_ssi.c"
#endif

#if USE_SYSTICK
#include "src/sdk_systick.c"
#endif

#if !RP2040
#include "src/sdk_ticks.c"
#endif

#if USE_TIMER
#include "src/sdk_timer.c"
#endif

#if USE_UART
#include "src/sdk_uart.c"
#endif

#if RP2040
#include "src/sdk_vreg.c"
#endif

#if USE_WATCHDOG || USE_TIMER
#include "src/sdk_watchdog.c"
#endif

#if RP2040
#include "src/sdk_xip.c"
#endif

#if USE_XOSC
#include "src/sdk_xosc.c"
#endif




#if USE_USBPORT
#include "usb_src/sdk_usbport.c"
#endif




#if USE_USB

#if USE_USB_DEV

#include "usb_src/sdk_usb_dev.c"

#if USE_USB_DEV_CDC
#include "usb_src/sdk_usb_dev_cdc.c"
#endif

#if USE_USB_DEV_HID
#include "usb_src/sdk_usb_dev_hid.c"
#endif

#endif // USE_USB_DEV

#if USE_USB_HOST

#include "usb_src/sdk_usb_host.c"

#if USE_USB_HOST_CDC
#include "usb_src/sdk_usb_host_cdc.c"
#endif

#if USE_USB_HOST_HID
#include "usb_src/sdk_usb_host_hid.c"
#endif

#endif // USE_USB_HOST

#include "usb_src/sdk_usb_phy.c"

#endif // USE_USB
