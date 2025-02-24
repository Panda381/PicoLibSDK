
// ****************************************************************************
//                                 
//                        Project library configuration
//
// ****************************************************************************

#ifndef _CONFIG_H
#define _CONFIG_H

// *********
// At this place you can specify the switches and settings you want
// to change from the default configuration in config_def.h.
// *********

#define USE_EXTDISP	1		// use ExtDisp module: 0=use version 0 (with Pico), 1=use version 1 (with RP2040)

// set high system clock to generate DVI signal
#define PLL_KHZ			252000	// PLL system frequency in kHz (default 125000 kHz)
#define PRESET_VREG_VOLTAGE	VREG_VOLTAGE_1_20 // preset VREG voltage
#define PRESET_FLASH_CLKDIV	6	// preset flash CLK divider

//#define FONT			FontBold8x8	// default system font
//#define FONTW			8		// width of system font
//#define FONTH			8		// height of system font

//#define USE_ORIGSDK		1		// include interface of original-SDK

//#define USE_MINIVGA		1		// use mini-VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

//#define USE_DVI		1		// use DVI (HDMI) display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

#define USE_DVIVGA		1		// use DVI (HDMI) + VGA display with simple frame buffer:
						//	1=use only frame buffer
						//	2=add full back buffer
						//	3=add 1/2 back buffer
						//	4=add 1/4 back buffer
						//	5=add 1/8 back buffer

// SDK modules
#define USE_ADC		0		// use ADC controller (sdk_adc.c, sdk_adc.h)
#define USE_DMA		1		// use DMA controller (sdk_dma.c, sdk_dma.h)
#define USE_DOUBLE	0		// use Double-floating point 1=in RAM, 2=in Flash (sdk_double.c, sdk_double_asm.S, sdk_double.h)
#define USE_FIFO	1		// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)
#define USE_FLASH	1		// use Flash memory programming (sdk_flash.c, sdk_flash.h)
#define USE_FLOAT	0		// use Single-floating point 1=in RAM, 2=in Flash (sdk_float.c, sdk_float_asm.S, sdk_float.h)
#define USE_I2C		0		// use I2C interface (sdk_i2c.c, sdk_i2c.h)
#define USE_INTERP	1		// use interpolator (sdk_interp.c, sdk_interp.h)
#define USE_IRQ		1		// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
#define USE_MULTICORE	1		// use Multicore (sdk_multicore.c, sdk_multicore.h)
#define USE_PIO		1		// use PIO (sdk_pio.c, sdk_pio.h)
#define USE_PLL		1		// use PLL phase-locked loop (sdk_pll.c, sdk_pll.h)
#define USE_PWM		1		// use PWM (sdk_pwm.c, sdk_pwm.h)
#define USE_ROSC	0		// use ROSC ring oscillator (sdk_rosc.c, sdk_rosc.h)
#define USE_RTC		0		// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)
#define USE_SPI		0		// use SPI interface (sdk_spi.c, sdk_spi.h)
#define USE_SPINLOCK	0		// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)
//#define USE_SYSTICK	0		// use SysTick system timer (sdk_systick.c, sdk_systick.h)
#define USE_TIMER	1		// use Timer with alarm (sdk_timer.c, sdk_timer.h)
#define USE_UART	0		// use UART serial port (sdk_uart.c, sdk_uart.h)
#define USE_USBPORT	0		// use USB Mini-Port (sdk_usbport.c, sdk_usbport.h)
//#define USE_USB_STDIO	0		// use USB stdio (UsbPrint function)
#define USE_UART_STDIO	0		// use UART stdio (UartPrint function)
#define USE_DRAW_STDIO	1		// use DRAW stdio (DrawPrint function)
//#define USE_USB		0		// use USB (sdk_usb_*.c, sdk_usb_*.h)
#define USE_WATCHDOG	0		// use Watchdog timer (sdk_watchdog.c, sdk_watchdog.h)
#define USE_XOSC	1		// use XOSC crystal oscillator (sdk_xosc.c, sdk_xosc.h)

// library modules
#define USE_ALARM	0		// use SysTick alarm (lib_alarm.c, lib_alarm.h)
#define USE_CALENDAR	0		// use 32-bit calendar (lib_calendar.c, lib_calendar.h)
#define USE_CALENDAR64	0		// use 64-bit calendar (lib_calendar64.c, lib_calendar64.h)
#define USE_CANVAS	0		// use Canvas (lib_canvas.c, lib_canvas.h)
#define USE_COLOR	0		// use color vector (lib_color.c, lib_color.h)
#define USE_CONFIG	0		// use device configuration (lib_config.c, lib_config.h)
#define USE_CRC		1		// use CRC Check Sum (lib_crc.c, lib_crc.h)
//#define USE_DECNUM	0		// use DecNum (decnum.c, decnum.h)
#define USE_DRAW	1		// use drawing to frame buffer (lib_draw.c, lib_draw.h)
#define USE_ESCPKT	0		// use escape packet protocol (lib_escpkt.c, lib_escpkt.h)
#define USE_EVENT	0		// use Event Ring buffer (lib_event.c, lib_event.h)
#define USE_FAT		0		// use FAT file system (lib_fat.c, lib_fat.h)
#define USE_FRAMEBUF	1		// use default display frame buffer
//#define USE_LIST	0		// use Doubly Linked List (lib_list.c, lib_list.h)
#define USE_MAT2D	0		// use 2D transformation matrix (lib_mat2d.c, lib_mat2d.h)
//#define USE_MALLOC	0		// use Memory Allocator (lib_malloc.c, lib_malloc.h)
#define USE_MINIRING	0		// use Mini-Ring buffer (lib_miniring.c, lib_miniring.h)
#define USE_PRINT	1		// use Formatted print (lib_print.c, lib_print.h)
#define USE_PWMSND	0		// use PWM sound output; set 1.. = number of channels (lib_pwmsnd.c, lib_pwmsnd.h)
#define USE_RAND	0		// use Random number generator (lib_rand.c, lib_rand.h)
#define USE_RECT	0		// use Rectangle (lib_rect.c, lib_rect.h)
//#define USE_RING	0		// use Ring buffer (lib_ring.c, lib_ring.h)
#define USE_RINGRX	0		// use Ring buffer with DMA receiver (lib_ringrx.c, lib_ringrx.h)
#define USE_RINGTX	0		// use Ring buffer with DMA transmitter (lib_ringtx.c, lib_ringtx.h)
#define USE_SD		0		// use SD card (lib_sd.c, lib_sd.h)
#define USE_STREAM	1		// use Data stream (lib_stream.c, lib_stream.h)
#define USE_TEXT	0		// use Text strings, except StrLen and StrComp (lib_text.c, lib_textnum.c, lib_text.h)
#define USE_TEXTLIST	0		// use List of text strings (lib_textlist.c, lib_textlist.h)
#define USE_TPRINT	0		// use Text Print (lib_tprint.c, lib_tprint.h)
#define USE_TREE	0		// use Tree list (lib_tree.c, lib_tree.h)
#define USE_VIDEO	0		// use video player (lib_video.c, lib_video.h)

// extra
#define USE_BIGINT	0		// use Big Integers (bigint.c, bigint.h)
#define USE_REAL	0		// use real numbers
#define USE_STACKCHECK	0		// use Stack check (sdk_cpu.c, sdk_cpu.h)

//#define USE_USB_STDIO		1		// use USB stdio (UsbPrint function)

#include CONFIG_DEF_H				// default configuration

#endif // _CONFIG_H
