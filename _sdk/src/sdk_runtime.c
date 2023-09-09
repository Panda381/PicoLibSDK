
// ****************************************************************************
//
//                             Runtime Initialize
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

#include "../../global.h"	// globals
#include "../inc/sdk_reset.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_gpio.h"
#include "../inc/sdk_irq.h"
#include "../inc/sdk_bootrom.h"
#include "../inc/sdk_spinlock.h"
#include "../inc/sdk_systick.h"
#include "../inc/sdk_rtc.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_float.h"
#include "../../_lib/inc/lib_alarm.h"
#include "../../_lib/inc/lib_config.h"
#include "../../_lib/inc/lib_malloc.h"
#include "../../_lib/inc/lib_rand.h"
#include "../usb_inc/sdk_usb_phy.h" // physical layer
#include "../usb_inc/sdk_usb_dev.h" // devices
#include "../usb_inc/sdk_usb_dev_cdc.h"
#include "../inc/sdk_rosc.h"
#include "../inc/sdk_uart.h"

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
// copy of vector table in RAM
u32 __attribute__((section(".ram_vector_table"))) RamVectorTable[48];
#endif

// Device init
void DeviceInit();

// runtime initialize
void RuntimeInit()
{
	// reset all peripherals to put system into a known state, except QSPI and PLLs
	ResetPeripheryOnMask(RESET_ALLBITS & ~(BIT(RESET_IO_QSPI) | BIT(RESET_PADS_QSPI) |
					BIT(RESET_PLL_USB) | BIT(RESET_PLL_SYS)));

	// restart peripherals which are clocked only by clk_sys and clk_ref, and wait to complete
	ResetPeripheryOffWaitMask(RESET_ALLBITS & ~(BIT(RESET_ADC) | BIT(RESET_RTC) | BIT(RESET_SPI0) |
			BIT(RESET_SPI1) | BIT(RESET_UART0) | BIT(RESET_UART1) | BIT(RESET_USBCTRL)));

	// pre-initialize constructors
	extern void (*__preinit_array_start)();
	extern void (*__preinit_array_end)();
	void(**p)();
	for (p = &__preinit_array_start; p < &__preinit_array_end; p++) (*p)();

	// initialize ROM functions (memset etc.)
	RomFncInit();

#if USE_FLOAT		// use float support
	// initialize floating-point service
	FloatInit();
#endif // USE_FLOAT		// use float support

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
	// set all interrupt priorities of NVIC of this CPU core to default value
	NVIC_IRQPrioDef();
#endif

	// initialize clocks
	ClockInit();

	// start all remaining peripherals
	ResetPeripheryOffWaitMask(RESET_ALLBITS);

#if USE_IRQ	// use IRQ interrupts (sdk_irq.c, sdk_irq.h)
	// copy vector table to RAM (must use built-in memcpy)
	memcpy(RamVectorTable, (u32*)GetVTOR(), sizeof(RamVectorTable));
	SetVTOR((irq_handler_t*)RamVectorTable);
#endif

#if USE_STACKCHECK	// use Stack check (sdk_cpu.c, sdk_cpu.h)
	// feed stack of current CPU core with the food
	StackFeed();
#endif

#if USE_SPINLOCK	// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)
	// reset all spinlocks
	SpinResetAll();
#endif

#if USE_CONFIG			// use device configuration (lib_config.c, lib_config.h)
	// load config from flash
	ConfigLoad();
#endif

#if USE_RAND	// use Random number generator (lib_rand.c, lib_rand.h)
	// initialize seed of random number generator (for current CPU core) (randomize it)
	//  Seed will not be fully random if ROSC is used as a CPU clock source.
	RandInit();
	RandSeed[1] += RandSeed[0];
#if USE_ROSC	// use ROSC ring oscillator (sdk_rosc.c, sdk_rosc.h)
	RandSeed[0] += RoscRand32();
#endif
#endif

#if USE_ALARM	// use SysTick alarm (lib_alarm.c, lib_alarm.h)
	// Initialize alarm lists for both cores
	AlarmInit();
#endif

#if USE_SYSTICK		// use SysTick system timer (sdk_systick.c, sdk_systick.h)
	// initialize SysTick timer
	SysTickInit();
#endif

#if USE_RTC	// use RTC Real-time clock (sdk_rtc.c, sdk_rtc.h)
	// initialize RTC timer
	RtcInit();
#endif

#if USE_MALLOC	// use Memory Allocator (lib_malloc.c, lib_malloc.h)
	// initialize memory allocator
	MemInit();
#endif

	// initialize constructors
	extern void (*__init_array_start)();
	extern void (*__init_array_end)();
	for (p = &__init_array_start; p < &__init_array_end; p++) (*p)();

// default LED pin
#ifdef LED_PIN

	// select GPIO function
	GPIO_Init(LED_PIN);

	// set direction to output
	GPIO_DirOut(LED_PIN);

	// set LED to 0
	GPIO_Out0(LED_PIN);

#endif // LED_PIN

	// Device init
	DeviceInit();

	// stdio
#if USE_USB_STDIO		// use USB stdio (UsbPrint function)
	UsbDevInit(&UsbDevCdcSetupDesc);
#endif

#if USE_UART_STDIO
	// initialize UART stdio
	UartStdioInit();
#endif
}

// Device terminate
void DeviceTerm();

// runtime terminate
void RuntimeTerm()
{
#if USE_UART_STDIO
	// terminate UART stdio
	UartStdioTerm();
#endif

	// terminate USB
#if USE_USB
	UsbTerm();
#endif

	// Device terminate
	DeviceTerm();

// default LED pin
#ifdef LED_PIN
	GPIO_Reset(LED_PIN);
#endif // LED_PIN

#if USE_SYSTICK		// use SysTick system timer (sdk_systick.c, sdk_systick.h)
	// terminate SysTick timer
	SysTickTerm();
#endif
}
