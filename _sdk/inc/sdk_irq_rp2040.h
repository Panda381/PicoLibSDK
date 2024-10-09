
// ****************************************************************************
//
//                                    IRQ
//                           Interrupt Request RP2040
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

// ARM Cortex-M0+ core interrupt request indices - Exceptions
#define IRQ_INVALID	-16	// invalid exception - returned 

#define IRQ_RESET	-15	// reset vector (priority -4)
#define IRQ_NMI		-14	// non maskable interrupt (priority -2)
#define IRQ_HARDFAULT	-13	// hard fault (priority -1)
#define IRQ_SVCALL	-5	// system service call (SVC instruction)
#define IRQ_PENDSV	-2	// pendable request for system service
#define IRQ_SYSTICK	-1	// system tick timer
// RP2040 device interrupt request indices - Interrupts
#define IRQ_TIMER_0	0	// us timer IRQ 0
#define IRQ_TIMER_1	1	// us timer IRQ 1
#define IRQ_TIMER_2	2	// us timer IRQ 2
#define IRQ_TIMER_3	3	// us timer IRQ 3
#define IRQ_PWM_WRAP	4	// PWM wrap
#define IRQ_USBCTRL	5	// USB ctrl
#define IRQ_XIP		6
#define IRQ_PIO0_0	7	// PIO 0 IRQ 0
#define IRQ_PIO0_1	8	// PIO 0 IRQ 1
#define IRQ_PIO1_0	9	// PIO 1 IRQ 0
#define IRQ_PIO1_1	10	// PIO 1 IRQ 1
#define IRQ_DMA_0	11	// DMA IRQ 0
#define IRQ_DMA_1	12	// DMA IRQ 1
#define IRQ_IO_BANK0	13	// pad bank IO
#define IRQ_IO_QSPI	14	// qspi bank IO
#define IRQ_SIO_PROC0	15	// SIO PROC 0
#define IRQ_SIO_PROC1	16	// SIO PROC 1
#define IRQ_CLOCKS	17	// clocks
#define IRQ_SPI0	18	// SPI 0
#define IRQ_SPI1	19	// SPI 1
#define IRQ_UART0	20	// UART 0
#define IRQ_UART1	21	// UART 1
#define IRQ_ADC_FIFO	22	// ADC FIFO
#define IRQ_I2C0	23	// I2C 0
#define IRQ_I2C1	24	// I2C 1
#define IRQ_RTC		25	// RTC
#define IRQ_SPAREIRQ_0	26	// user IRQ 0
#define IRQ_SPAREIRQ_1	27	// user IRQ 1
#define IRQ_SPAREIRQ_2	28	// user IRQ 2
#define IRQ_SPAREIRQ_3	29	// user IRQ 3
#define IRQ_SPAREIRQ_4	30	// user IRQ 4
#define IRQ_SPAREIRQ_5	31	// user IRQ 5

#define IRQ_NUM		32	// number of IRQs (without system exceptions)
#define IRQ_USERNUM	6	// number of user's IRQs

#if USE_ORIGSDK		// include interface of original SDK
// original-SDK name convention
#define TIMER_IRQ_0	0
#define TIMER_IRQ_1	1
#define TIMER_IRQ_2	2
#define TIMER_IRQ_3	3
#define PWM_IRQ_WRAP	4
#define USBCTRL_IRQ	5
#define XIP_IRQ		6
#define PIO0_IRQ_0	7
#define PIO0_IRQ_1	8
#define PIO1_IRQ_0	9
#define PIO1_IRQ_1	10
#define DMA_IRQ_0	11
#define DMA_IRQ_1	12
#define IO_IRQ_BANK0	13
#define IO_IRQ_QSPI	14
#define SIO_IRQ_PROC0	15
#define SIO_IRQ_PROC1	16
#define CLOCKS_IRQ	17
#define SPI0_IRQ	18
#define SPI1_IRQ	19
#define UART0_IRQ	20
#define UART1_IRQ	21
#define ADC_IRQ_FIFO	22
#define I2C0_IRQ	23
#define I2C1_IRQ	24
#define RTC_IRQ		25
#endif // USE_ORIGSDK

#if USE_ORIGSDK		// include interface of original SDK
// original-SDK interrupt number definition
#define Reset_IRQn		-15	// Reset Vector, invoked on Power up and warm reset
#define NonMaskableInt_IRQn	-14	// Non maskable Interrupt, cannot be stopped or preempted
#define HardFault_IRQn		-13	// Hard Fault, all classes of Fault
#define SVCall_IRQn		-5	// system Service Call via SVC instruction
#define PendSV_IRQn		-2	// Pendable request for system service
#define SysTick_IRQn		-1	// System Tick Timer

#define TIMER_IRQ_0_IRQn	0	// TIMER_IRQ_0
#define TIMER_IRQ_1_IRQn	1	// TIMER_IRQ_1
#define TIMER_IRQ_2_IRQn	2	// TIMER_IRQ_2
#define TIMER_IRQ_3_IRQn	3	// TIMER_IRQ_3
#define PWM_IRQ_WRAP_IRQn	4	// PWM_IRQ_WRAP
#define USBCTRL_IRQ_IRQn	5	// USBCTRL_IRQ
#define XIP_IRQ_IRQn		6	// XIP_IRQ
#define PIO0_IRQ_0_IRQn		7	// PIO0_IRQ_0
#define PIO0_IRQ_1_IRQn		8	// PIO0_IRQ_1
#define PIO1_IRQ_0_IRQn		9	// PIO1_IRQ_0
#define PIO1_IRQ_1_IRQn		10	// PIO1_IRQ_1
#define DMA_IRQ_0_IRQn		11	// DMA_IRQ_0
#define DMA_IRQ_1_IRQn		12	// DMA_IRQ_1
#define IO_IRQ_BANK0_IRQn	13	// IO_IRQ_BANK0
#define IO_IRQ_QSPI_IRQn	14	// IO_IRQ_QSPI
#define SIO_IRQ_PROC0_IRQn	15	// SIO_IRQ_PROC0
#define SIO_IRQ_PROC1_IRQn	16	// 16 SIO_IRQ_PROC1
#define CLOCKS_IRQ_IRQn		17	// CLOCKS_IRQ
#define SPI0_IRQ_IRQn		18	// SPI0_IRQ
#define SPI1_IRQ_IRQn		19	// SPI1_IRQ
#define UART0_IRQ_IRQn		20	// UART0_IRQ
#define UART1_IRQ_IRQn		21	// UART1_IRQ
#define ADC_IRQ_FIFO_IRQn	22	// ADC_IRQ_FIFO
#define I2C0_IRQ_IRQn		23	// I2C0_IRQ
#define I2C1_IRQ_IRQn		24	// 24 I2C1_IRQ
#define RTC_IRQ_IRQn		25	// RTC_IRQ
#endif // USE_ORIGSDK

//#if USE_ORIGSDK		// include interface of original SDK
// original-SDK symbolic names of interrupt services
//  isr_nmi
//  isr_hardfault
//  isr_svcall
//  isr_pendsv
//  isr_systick
#define isr_timer_0	isr_irq0
#define isr_timer_1	isr_irq1
#define isr_timer_2	isr_irq2
#define isr_timer_3	isr_irq3
#define isr_pwm_wrap	isr_irq4
#define isr_usbctrl	isr_irq5
#define isr_xip		isr_irq6
#define isr_pio0_0	isr_irq7
#define isr_pio0_1	isr_irq8
#define isr_pio1_0	isr_irq9
#define isr_pio1_1	isr_irq10
#define isr_dma_0	isr_irq11
#define isr_dma_1	isr_irq12
#define isr_io_bank0	isr_irq13
#define isr_io_qspi	isr_irq14
#define isr_sio_proc0	isr_irq15
#define isr_sio_proc1	isr_irq16
#define isr_clocks	isr_irq17
#define isr_spi0	isr_irq18
#define isr_spi1	isr_irq19
#define isr_uart0	isr_irq20
#define isr_uart1	isr_irq21
#define isr_adc_fifo	isr_irq22
#define isr_i2c0	isr_irq23
#define isr_i2c1	isr_irq24
#define isr_rtc		isr_irq25
#define isr_spare_0	isr_irq26
#define isr_spare_1	isr_irq27
#define isr_spare_2	isr_irq28
#define isr_spare_3	isr_irq29
#define isr_spare_4	isr_irq30
#define isr_spare_5	isr_irq31
//#endif // USE_ORIGSDK
