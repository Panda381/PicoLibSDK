
// ****************************************************************************
//
//                                    IRQ
//                           Interrupt Request RP2350
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

#define IRQ_INVALID		-16	// invalid exception - returned 

#if RISCV	// RISC-V mode

// RISC-V Exceptions
#define IRQ_MCALL		-14	// exception 11: call from machine mode (M-mode)
					// exception 10: not used
#define IRQ_SCALL		-12	// exception 9: call from supervisor mode (S-mode)
#define IRQ_UCALL		-11	// exception 8: call from user mode (U-mode)
#define IRQ_STOREFAULT		-10	// exception 7: store accesss fault (bus fault)
#define IRQ_STOREALIGN		-9	// exception 6: store address misaligned (alignment)
#define IRQ_LOADFAULT		-8	// exception 5: load access fault (bus fault)
#define IRQ_LOADALIGN		-7	// exception 4: load address misaligned (alignment)
#define IRQ_BREAKPOINT		-6	// exception 3: breakpoint
#define IRQ_INSTRILEGAL		-5	// exception 2: illegal instruction
#define IRQ_INSTRFAULT		-4	// exception 1: instruction access fault (bus fault)
#define IRQ_INSTRALIGN		-3	// exception 0: instruction fetch was misaligned (alignment)
#define IRQ_MTIMER		-2	// trap 7: machine-mode timer
#define IRQ_SOFTIRQ		-1	// trap 3: software IRQ

#else		// ARM mode

// ARM Cortex-M33 core interrupt request indices - Exceptions
#define IRQ_RESET		-15	// reset vector (priority -4)
#define IRQ_NMI			-14	// non maskable interrupt (priority -2)
#define IRQ_HARDFAULT		-13	// hard fault (priority -1)
#define IRQ_MEMFAULT		-12	// memory management fault (MPU mismatch)
#define IRQ_BUSFAULT		-11	// bus fault (memory access fault)
#define IRQ_USAGEFAULT		-10	// usage fault (undef instruction)
#define IRQ_SECUREFAULT		-9	// secure fault
#define IRQ_SVCALL		-5	// system service call (SVC instruction)
#define IRQ_DEBMONITOR		-4	// debug monitor
#define IRQ_PENDSV		-2	// pendable request for system service
#define IRQ_SYSTICK		-1	// system tick timer

#endif

// RP2350 device interrupt request indices - Interrupts
#define IRQ_TIMER0_0		0	// us timer IRQ 0
#define IRQ_TIMER0_1		1	// us timer IRQ 1
#define IRQ_TIMER0_2		2	// us timer IRQ 2
#define IRQ_TIMER0_3		3	// us timer IRQ 3
#define IRQ_TIMER1_0		4	// sys_clk timer IRQ 0
#define IRQ_TIMER1_1		5	// sys_clk timer IRQ 1
#define IRQ_TIMER1_2		6	// sys_clk timer IRQ 2
#define IRQ_TIMER1_3		7	// sys_clk timer IRQ 3
#define IRQ_PWM_WRAP_0		8	// PWM wrap 0
#define IRQ_PWM_WRAP_1		9	// PWM wrap 1
#define IRQ_DMA_0		10	// DMA IRQ 0
#define IRQ_DMA_1		11	// DMA IRQ 1
#define IRQ_DMA_2		12	// DMA IRQ 2
#define IRQ_DMA_3		13	// DMA IRQ 3
#define IRQ_USBCTRL		14	// USB ctrl
#define IRQ_PIO0_0		15	// PIO 0 IRQ 0
#define IRQ_PIO0_1		16	// PIO 0 IRQ 1
#define IRQ_PIO1_0		17	// PIO 1 IRQ 0
#define IRQ_PIO1_1		18	// PIO 1 IRQ 1
#define IRQ_PIO2_0		19	// PIO 2 IRQ 0
#define IRQ_PIO2_1		20	// PIO 2 IRQ 1
#define IRQ_IO_BANK0		21	// pad bank IO
#define IRQ_IO_BANK0_NS		22	// pad bank IO NS
#define IRQ_IO_QSPI		23	// qspi bank IO
#define IRQ_IO_QSPI_NS		24	// qspi bank IO NS
#define IRQ_SIO_FIFO		25	// SIO FIFO
#define IRQ_SIO_BELL		26	// SIO BELL
#define IRQ_SIO_FIFO_NS		27	// SIO FIFO NS
#define IRQ_SIO_BELL_NS		28	// SIO BELL NS
#define IRQ_SIO_MTIMECMP	29	// SIO MTIMECMP
#define IRQ_CLOCKS		30	// clocks
#define IRQ_SPI0		31	// SPI 0
#define IRQ_SPI1		32	// SPI 1
#define IRQ_UART0		33	// UART 0
#define IRQ_UART1		34	// UART 1
#define IRQ_ADC_FIFO		35	// ADC FIFO
#define IRQ_I2C0		36	// I2C 0
#define IRQ_I2C1		37	// I2C 1
#define IRQ_OTP			38	// OTP
#define IRQ_TRNG		39	// TRNG
#define IRQ_PROC0_CTI		40
#define IRQ_PROC1_CTI		41
#define IRQ_PLL_SYS		42	// PLL SYS
#define IRQ_PLL_USB		43	// PLL USB
#define IRQ_POWMAN_POW		44	// POWMAN POW
#define IRQ_POWMAN_TIMER	45	// POWMAN Timer
#define IRQ_SPAREIRQ_0		46	// user IRQ 0
#define IRQ_SPAREIRQ_1		47	// user IRQ 1
#define IRQ_SPAREIRQ_2		48	// user IRQ 2
#define IRQ_SPAREIRQ_3		49	// user IRQ 3
#define IRQ_SPAREIRQ_4		50	// user IRQ 4
#define IRQ_SPAREIRQ_5		51	// user IRQ 5

#define IRQ_NUM			52	// number of IRQs (without system exceptions)
#define IRQ_USERNUM		6	// number of user's IRQs

#define IRQ_TIMER_0		IRQ_TIMER0_0
#define IRQ_TIMER_1		IRQ_TIMER0_1
#define IRQ_TIMER_2		IRQ_TIMER0_2
#define IRQ_TIMER_3		IRQ_TIMER0_3
#define IRQ_PWM_WRAP		IRQ_PWM_WRAP_0

#if USE_ORIGSDK		// include interface of original SDK
// original-SDK name convention
#define TIMER0_IRQ_0	0
#define TIMER0_IRQ_1	1
#define TIMER0_IRQ_2	2
#define TIMER0_IRQ_3	3
#define TIMER1_IRQ_0	4
#define TIMER1_IRQ_1	5
#define TIMER1_IRQ_2	6
#define TIMER1_IRQ_3	7
#define PWM_IRQ_WRAP_0	8
#define PWM_IRQ_WRAP_1	9
#define DMA_IRQ_0	10
#define DMA_IRQ_1	11
#define DMA_IRQ_2	12
#define DMA_IRQ_3	13
#define USBCTRL_IRQ	14
#define PIO0_IRQ_0	15
#define PIO0_IRQ_1	16
#define PIO1_IRQ_0	17
#define PIO1_IRQ_1	18
#define PIO2_IRQ_0	19
#define PIO2_IRQ_1	20
#define IO_IRQ_BANK0	21
#define IO_IRQ_BANK0_NS	22
#define IO_IRQ_QSPI	23
#define IO_IRQ_QSPI_NS	24
#define SIO_IRQ_FIFO	25
#define SIO_IRQ_BELL	26
#define SIO_IRQ_FIFO_NS	27
#define SIO_IRQ_BELL_NS	28
#define SIO_IRQ_MTIMECMP 29
#define CLOCKS_IRQ	30
#define SPI0_IRQ	31
#define SPI1_IRQ	32
#define UART0_IRQ	33
#define UART1_IRQ	34
#define ADC_IRQ_FIFO	35
#define I2C0_IRQ	36
#define I2C1_IRQ	37
#define OTP_IRQ		38
#define TRNG_IRQ	39
#define PROC0_IRQ_CTI	40
#define PROC1_IRQ_CTI	41
#define PLL_SYS_IRQ	42
#define PLL_USB_IRQ	43
#define POWMAN_IRQ_POW	44
#define POWMAN_IRQ_TIMER 45
#define SPAREIRQ_IRQ_0	46
#define SPAREIRQ_IRQ_1	47
#define SPAREIRQ_IRQ_2	48
#define SPAREIRQ_IRQ_3	49
#define SPAREIRQ_IRQ_4	50
#define SPAREIRQ_IRQ_5	51

#define TIMER_IRQ_0	TIMER0_IRQ_0
#define TIMER_IRQ_1	TIMER0_IRQ_0
#define TIMER_IRQ_2	TIMER0_IRQ_0
#define TIMER_IRQ_3	TIMER0_IRQ_0
#define PWM_IRQ_WRAP	PWM_IRQ_WRAP_0

#endif // USE_ORIGSDK

#if USE_ORIGSDK		// include interface of original SDK
// original-SDK interrupt number definition
#define Reset_IRQn		-15		// Reset Vector, invoked on Power up and warm reset
#define NonMaskableInt_IRQn	-14		// Non maskable Interrupt, cannot be stopped or preempted
#define HardFault_IRQn		-13		// Hard Fault, all classes of Fault
#define MemoryManagement_IRQn	-12		// Memory Management, MPU mismatch, including Access Violation and No Match
#define BusFault_IRQn		-11		// Bus Fault, Pre-Fetch-, Memory Access Fault, other address/memory related Fault
#define UsageFault_IRQn		-10		// Usage Fault, i.e. Undef Instruction, Illegal State Transition
#define SecureFault_IRQn	-9		// Secure Fault Handler
#define SVCall_IRQn		-5		// System Service Call via SVC instruction
#define DebugMonitor_IRQn	-4		// Debug Monitor
#define PendSV_IRQn		-2		// Pendable request for system service
#define SysTick_IRQn		-1		// System Tick Timer

#define TIMER0_IRQ_0_IRQn	0		// TIMER0_IRQ_0
#define TIMER0_IRQ_1_IRQn	1		// TIMER0_IRQ_1
#define TIMER0_IRQ_2_IRQn	2		// TIMER0_IRQ_2
#define TIMER0_IRQ_3_IRQn	3		// TIMER0_IRQ_3
#define TIMER1_IRQ_0_IRQn	4		// TIMER1_IRQ_0
#define TIMER1_IRQ_1_IRQn	5		// TIMER1_IRQ_1
#define TIMER1_IRQ_2_IRQn	6		// TIMER1_IRQ_2
#define TIMER1_IRQ_3_IRQn	7		// TIMER1_IRQ_3
#define PWM_IRQ_WRAP_0_IRQn	8		// PWM_IRQ_WRAP_0
#define PWM_IRQ_WRAP_1_IRQn	9		// PWM_IRQ_WRAP_1
#define DMA_IRQ_0_IRQn		10		// DMA_IRQ_0
#define DMA_IRQ_1_IRQn		11		// DMA_IRQ_1
#define DMA_IRQ_2_IRQn		12		// DMA_IRQ_2
#define DMA_IRQ_3_IRQn		13		// DMA_IRQ_3
#define USBCTRL_IRQ_IRQn	14		// USBCTRL_IRQ
#define PIO0_IRQ_0_IRQn		15		// PIO0_IRQ_0
#define PIO0_IRQ_1_IRQn		16		// PIO0_IRQ_1
#define PIO1_IRQ_0_IRQn		17		// PIO1_IRQ_0
#define PIO1_IRQ_1_IRQn		18		// PIO1_IRQ_1
#define PIO2_IRQ_0_IRQn		19		// PIO2_IRQ_0
#define PIO2_IRQ_1_IRQn		20		// PIO2_IRQ_1
#define IO_IRQ_BANK0_IRQn	21		// IO_IRQ_BANK0
#define IO_IRQ_BANK0_NS_IRQn	22		// IO_IRQ_BANK0_NS
#define IO_IRQ_QSPI_IRQn	23		// IO_IRQ_QSPI
#define IO_IRQ_QSPI_NS_IRQn	24		// IO_IRQ_QSPI_NS
#define SIO_IRQ_FIFO_IRQn	25		// SIO_IRQ_FIFO
#define SIO_IRQ_BELL_IRQn	26		// SIO_IRQ_BELL
#define SIO_IRQ_FIFO_NS_IRQn	27		// SIO_IRQ_FIFO_NS
#define SIO_IRQ_BELL_NS_IRQn	28		// SIO_IRQ_BELL_NS
#define SIO_IRQ_MTIMECMP_IRQn	29		// SIO_IRQ_MTIMECMP
#define CLOCKS_IRQ_IRQn		30		// CLOCKS_IRQ
#define SPI0_IRQ_IRQn		31		// SPI0_IRQ
#define SPI1_IRQ_IRQn		32		// SPI1_IRQ
#define UART0_IRQ_IRQn		33		// UART0_IRQ
#define UART1_IRQ_IRQn		34		// UART1_IRQ
#define ADC_IRQ_FIFO_IRQn	35		// ADC_IRQ_FIFO
#define I2C0_IRQ_IRQn		36		// I2C0_IRQ
#define I2C1_IRQ_IRQn		37		// I2C1_IRQ
#define OTP_IRQ_IRQn		38		// OTP_IRQ
#define TRNG_IRQ_IRQn		39		// TRNG_IRQ
#define PLL_SYS_IRQ_IRQn	42		// PLL_SYS_IRQ
#define PLL_USB_IRQ_IRQn	43		// PLL_USB_IRQ
#define POWMAN_IRQ_POW_IRQn	44		// POWMAN_IRQ_POW
#define POWMAN_IRQ_TIMER_IRQn	45		// POWMAN_IRQ_TIMER
#endif // USE_ORIGSDK

//#if USE_ORIGSDK		// include interface of original SDK
// original-SDK symbolic names of interrupt services
// ARM:
//  isr_nmi
//  isr_hardfault
//  isr_memfault
//  isr_busfault
//  isr_usagefault
//  isr_securefault
//  isr_svcall
//  isr_debmonitor
//  isr_pendsv
//  isr_systick
// RISC-V:
//  isr_mcall
//  isr_scall
//  isr_ucall
//  isr_storefault
//  isr_storealign
//  isr_loadfault
//  isr_loadalign
//  isr_breakpoint
//  isr_instrilegal
//  isr_instrfault
//  isr_instralign
//  isr_mtimer
//  isr_softirq
#define isr_timer0_0	isr_irq0
#define isr_timer0_1	isr_irq1
#define isr_timer0_2	isr_irq2
#define isr_timer0_3	isr_irq3
#define isr_timer1_0	isr_irq4
#define isr_timer1_1	isr_irq5
#define isr_timer1_2	isr_irq6
#define isr_timer1_3	isr_irq7
#define isr_pwm_wrap_0	isr_irq8
#define isr_pwm_wrap_1	isr_irq9
#define isr_dma_0	isr_irq10
#define isr_dma_1	isr_irq11
#define isr_dma_2	isr_irq12
#define isr_dma_3	isr_irq13
#define isr_usbctrl	isr_irq14
#define isr_pio0_0	isr_irq15
#define isr_pio0_1	isr_irq16
#define isr_pio1_0	isr_irq17
#define isr_pio1_1	isr_irq18
#define isr_pio2_0	isr_irq19
#define isr_pio2_1	isr_irq20
#define isr_io_bank0	isr_irq21
#define isr_io_bank0_ns	isr_irq22
#define isr_io_qspi	isr_irq23
#define isr_io_qspi_ns	isr_irq24
#define isr_sio_fifo	isr_irq25
#define isr_sio_bell	isr_irq26
#define isr_sio_fifo_ns	isr_irq27
#define isr_sio_bell_ns	isr_irq28
#define isr_sio_mtimecmp isr_irq29
#define isr_clocks	isr_irq30
#define isr_spi0	isr_irq31
#define isr_spi1	isr_irq32
#define isr_uart0	isr_irq33
#define isr_uart1	isr_irq34
#define isr_adc_fifo	isr_irq35
#define isr_i2c0	isr_irq36
#define isr_i2c1	isr_irq37
#define isr_otp		isr_irq38
#define isr_trng	isr_irq39
#define isr_proc0_cti	isr_irq40
#define isr_proc1_cti	isr_irq41
#define isr_pll_sys	isr_irq42
#define isr_pll_usb	isr_irq43
#define isr_powman_pow	isr_irq44
#define isr_powman_timer isr_irq45
#define isr_spare_0	isr_irq46
#define isr_spare_1	isr_irq47
#define isr_spare_2	isr_irq48
#define isr_spare_3	isr_irq49
#define isr_spare_4	isr_irq50
#define isr_spare_5	isr_irq51
//#endif // USE_ORIGSDK
