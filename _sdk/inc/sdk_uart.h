
// ****************************************************************************
//
//                                     UART
//                  Universal Asynchronous Receiver-Transmitter
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

// UART controller is based on ARM Primecell UART PL011 revision r1p5
//  ... IrDA and modem mode of PL011 on RP2040 is not supported

// UART signals (active in LOW; inputs should be pull-ups):
//   TX ... -> (output) transmitted data -> connect to RX
//   RX ... <- (input) received data -> connect to TX
//   CTS ... <- (input) input flow control (= can send data) -> connect to RTS
//   RTS ... -> (output) output flow control (= can receive data) -> connect to CTS

// After init, connect UART to GPIO pins using GPIO_Fnc(pin, GPIO_FNC_UART).
// After connecting the GPIO, wait a moment with WaitUs(50) and
// then flush any fake received characters with UART_RecvFlush.

//   GPIO0  ... UART0 TX
//   GPIO1  ... UART0 RX
//   GPIO2  ... UART0 CTS
//   GPIO3  ... UART0 RTS

//   GPIO4  ... UART1 TX
//   GPIO5  ... UART1 RX
//   GPIO6  ... UART1 CTS
//   GPIO7  ... UART1 RTS

//   GPIO8  ... UART1 TX
//   GPIO9  ... UART1 RX
//   GPIO10 ... UART1 CTS
//   GPIO11 ... UART1 RTS

//   GPIO12 ... UART0 TX
//   GPIO13 ... UART0 RX
//   GPIO14 ... UART0 CTS
//   GPIO15 ... UART0 RTS

//   GPIO16 ... UART0 TX
//   GPIO17 ... UART0 RX
//   GPIO18 ... UART0 CTS
//   GPIO19 ... UART0 RTS

//   GPIO20 ... UART1 TX
//   GPIO21 ... UART1 RX
//   GPIO22 ... UART1 CTS
//  (GPIO23 ... UART1 RTS)

//  (GPIO24 ... UART1 TX)
//  (GPIO25 ... UART1 RX)
//   GPIO26 ... UART1 CTS
//   GPIO27 ... UART1 RTS

//   GPIO28 ... UART0 TX
//  (GPIO29 ... UART0 RX)

// using external compilation switch and parameters (from config_def.h):
//#define UARTSAMPLE	1	// flag in sdk_uart.*, compile UART test sample: 1 = compile, 0 = not

#if USE_UART	// use UART serial port (sdk_uart.c, sdk_uart.h)

#ifndef _SDK_UART_H
#define _SDK_UART_H

//#include "resource.h"		// Resources
#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_reset.h"
#include "../../_lib/inc/lib_ring.h"		// ring buffer
#include "../../_lib/inc/lib_stream.h"		// ring buffer
#include "../sdk_dreq.h"

#if USE_ORIGSDK		// include interface of original SDK
#if RP2040		// 1=use MCU RP2040
#include "orig_rp2040/orig_uart.h"	// constants of original SDK
#else
#include "orig_rp2350/orig_uart.h"	// constants of original SDK
#endif
#include "sdk_timer.h"
#include "sdk_gpio.h"
#endif // USE_ORIGSDK

#ifdef __cplusplus
extern "C" {
#endif

#define PICO_DEFAULT_UART		UART_STDIO_PORT
#define PICO_DEFAULT_UART_TX_PIN	UART_STDIO_TX
#define PICO_DEFAULT_UART_RX_PIN	UART_STDIO_RX
#define PICO_DEFAULT_UART_BAUD_RATE	115200

// UART hardware registers
#define UART_BASE(uart) (UART0_BASE+(uart)*(UART1_BASE - UART0_BASE))	// UART base address (uart = 0 or 1)
#define UART_DR(uart)	((volatile u32*)(UART_BASE(uart)+0))	// data register (uart = 0 or 1)
#define UART_RSR(uart)	((volatile u32*)(UART_BASE(uart)+4))	// receive status register (uart = 0 or 1)
#define UART_FR(uart)	((volatile u32*)(UART_BASE(uart)+0x18))	// flag register (uart = 0 or 1)
#define UART_ILPR(uart)	((volatile u32*)(UART_BASE(uart)+0x20))	// IrDA Low-Power Divisor Register (uart = 0 or 1)
#define UART_IBRD(uart)	((volatile u32*)(UART_BASE(uart)+0x24))	// integer baud rate register (uart = 0 or 1)
#define UART_FBRD(uart)	((volatile u32*)(UART_BASE(uart)+0x28))	// fractional baud rate register (uart = 0 or 1)
#define UART_LCR(uart)	((volatile u32*)(UART_BASE(uart)+0x2C))	// line control register (uart = 0 or 1)
#define UART_CR(uart)	((volatile u32*)(UART_BASE(uart)+0x30))	// control register (uart = 0 or 1)
#define UART_IFLS(uart)	((volatile u32*)(UART_BASE(uart)+0x34))	// interrupt FIFO level select register (uart = 0 or 1)
#define UART_IMSC(uart)	((volatile u32*)(UART_BASE(uart)+0x38))	// interrupt mask register (uart = 0 or 1)
#define UART_RIS(uart)	((volatile u32*)(UART_BASE(uart)+0x3C))	// raw interrupt status register (uart = 0 or 1)
#define UART_MIS(uart)	((volatile u32*)(UART_BASE(uart)+0x40))	// masked interrupt status register (uart = 0 or 1)
#define UART_ICR(uart)	((volatile u32*)(UART_BASE(uart)+0x44))	// interrupt clear register (uart = 0 or 1)
#define UART_DMACR(uart) ((volatile u32*)(UART_BASE(uart)+0x48)) // DMA control register (uart = 0 or 1)

#define UART_DREQ(uart,rx) (DREQ_UART0_TX + (uart)*2 + (rx))

// UART hardware interface
typedef struct {
	io32	dr;		// 0x00: Data Register, UARTDR
	io32	rsr;		// 0x04: Receive Status Register/Error Clear Register, UARTRSR/UARTECR
	io32	_pad0[4];	// 0x08:
	io32	fr;		// 0x18: Flag Register, UARTFR
	io32	_pad1;		// 0x1C:
	io32	ilpr;		// 0x20: IrDA Low-Power Counter Register, UARTILPR
	io32	ibrd;		// 0x24: Integer Baud Rate Register, UARTIBRD
	io32	fbrd;		// 0x28: Fractional Baud Rate Register, UARTFBRD
	io32	lcr_h;		// 0x2C: Line Control Register, UARTLCR_H
	io32	cr;		// 0x30: Control Register, UARTCR
	io32	ifls;		// 0x34: Interrupt FIFO Level Select Register, UARTIFLS
	io32	imsc;		// 0x38: Interrupt Mask Set/Clear Register, UARTIMSC
	io32	ris;		// 0x3C: Raw Interrupt Status Register, UARTRIS
	io32	mis;		// 0x40: Masked Interrupt Status Register, UARTMIS
	io32	icr;		// 0x44: Interrupt Clear Register, UARTICR
	io32	dmacr;		// 0x48: DMA Control Register, UARTDMACR
} uart_hw_t;

#define uart0_hw ((uart_hw_t*)UART0_BASE)
#define uart1_hw ((uart_hw_t*)UART1_BASE)

STATIC_ASSERT(sizeof(uart_hw_t) == 0x4C, "Incorrect uart_hw_t!");

#define uart0	uart0_hw
#define uart1	uart1_hw

// parity (values are hardcoded in functions)
#define UART_PARITY_NONE	0	// no parity bit (PEN=0, EPS=0, SPS=0)
#define UART_PARITY_EVEN	1	// even parity (PEN=1, EPS=1, SPS=0)
#define UART_PARITY_ODD		2	// odd parity (PEN=1, EPS=0, SPS=0)
#define UART_PARITY_ZERO	3	// zero parity (PEN=1, EPS=1, SPS=1)
#define UART_PARITY_ONE		4	// one parity (PEN=1, EPS=0, SPS=1)

// FIFO trigger points (every FIFO is 32 entries deep)
#define UART_LEVEL_VERYLOW	0	// 1/8 (4 + 28 entries)
#define UART_LEVEL_LOW		1	// 1/4 (2/8) (8 + 24 entries)
#define UART_LEVEL_MID		2	// 1/2 (4/8) (16 + 16 entries) ... default
#define UART_LEVEL_HIGH		3	// 3/4 (6/8) (24 + 8 entries)
#define UART_LEVEL_VERYHIGH	4	// 7/8 (28 + 4 entries)

// UART interrupt
#define UART_IRQ_OVERRUN	10	// overrun error
#define UART_IRQ_BREAK		9	// break error
#define UART_IRQ_PARITY		8	// parity error
#define UART_IRQ_FRAME		7	// framing error
#define UART_IRQ_TIMEOUT	6	// receive timeout (if receive FIFO is not empty and no more data comes during 32-bit period)
#define UART_IRQ_TX		5	// transmit
#define UART_IRQ_RX		4	// receive
#define UART_IRQ_CTS		1	// CTS

// get UART hardware interface from UART index
INLINE uart_hw_t* UART_GetHw(int uart) { return (uart == 0) ? uart0_hw : uart1_hw; }

// get UART index from UART hardware interface
INLINE u8 UART_GetInx(const uart_hw_t* hw) { return (hw == uart0_hw) ? 0 : 1; }

// === UART data

// receive character with error flags (error flags are cleared when condition disappears)
//  bit 11: OE overrun error (receive data is lost)
//  bit 10: BE break error (long "LOW" on RX input)
//  bit 9: parity error (incorrect parity of received character)
//  bit 8: framing error (invalid stop bit)
//  bit 0..7: data byte
// Every entry in receive FIFO contains 8 bit character with 4 error bits.
INLINE u16 UART_RecvCharFlags(int uart) { return (u16)(*UART_DR(uart) & 0xfff); }
INLINE u16 UART_RecvCharFlags_hw(uart_hw_t* hw) { return (u16)(hw->dr & 0xfff); }

// receive character, does not wait if not ready
INLINE char UART_RecvChar(int uart) { return (char)*UART_DR(uart); }
INLINE char UART_RecvChar_hw(uart_hw_t* hw) { return (char)hw->dr; }

// send character, does not wait if not ready
INLINE void UART_SendChar(int uart, char data) { *UART_DR(uart) = (u8)data; }
INLINE void UART_SendChar_hw(uart_hw_t* hw, char data) { hw->dr = (u8)data; }

// === UART errors

// check overrun error (returns True if receive FIFO is full and data is lost)
INLINE Bool UART_OverrunError(int uart) { return ((*UART_RSR(uart) >> 3) & 1) != 0; }
INLINE Bool UART_OverrunError_hw(const uart_hw_t* hw) { return ((hw->rsr >> 3) & 1) != 0; }

// clear overrun error
INLINE void UART_OverrunClear(int uart) { *UART_RSR(uart) = B3; }
INLINE void UART_OverrunClear_hw(uart_hw_t* hw) { hw->rsr = B3; }

// check break error (returns True if break condition 'LOW' detected)
INLINE Bool UART_BreakError(int uart) { return ((*UART_RSR(uart) >> 2) & 1) != 0; }
INLINE Bool UART_BreakError_hw(const uart_hw_t* hw) { return ((hw->rsr >> 2) & 1) != 0; }

// clear break error
INLINE void UART_BreakClear(int uart) { *UART_RSR(uart) = B2; }
INLINE void UART_BreakClear_hw(uart_hw_t* hw) { hw->rsr = B2; }

// check parity error (returns True if incorrect parity received)
INLINE Bool UART_ParityError(int uart) { return ((*UART_RSR(uart) >> 1) & 1) != 0; }
INLINE Bool UART_ParityError_hw(const uart_hw_t* hw) { return ((hw->rsr >> 1) & 1) != 0; }

// clear parity error
INLINE void UART_ParityClear(int uart) { *UART_RSR(uart) = B1; }
INLINE void UART_ParityClear_hw(uart_hw_t* hw) { hw->rsr = B1; }

// check framing error (returns True if invalid stop bit)
INLINE Bool UART_FrameError(int uart) { return (*UART_RSR(uart) & 1) != 0; }
INLINE Bool UART_FrameError_hw(const uart_hw_t* hw) { return (hw->rsr & 1) != 0; }

// clear framing error
INLINE void UART_FrameClear(int uart) { *UART_RSR(uart) = B0; }
INLINE void UART_FrameClear_hw(uart_hw_t* hw) { hw->rsr = B0; }

// === UART FIFO

// check if transmit FIFO is empty
INLINE Bool UART_TxEmpty(int uart) { return ((*UART_FR(uart) >> 7) & 1) != 0; }
INLINE Bool UART_TxEmpty_hw(const uart_hw_t* hw) { return ((hw->fr >> 7) & 1) != 0; }

// check if receive FIFO is full
INLINE Bool UART_RxFull(int uart) { return ((*UART_FR(uart) >> 6) & 1) != 0; }
INLINE Bool UART_RxFull_hw(const uart_hw_t* hw) { return ((hw->fr >> 6) & 1) != 0; }

// check if transmit FIFO is full
INLINE Bool UART_TxFull(int uart) { return ((*UART_FR(uart) >> 5) & 1) != 0; }
INLINE Bool UART_TxFull_hw(const uart_hw_t* hw) { return ((hw->fr >> 5) & 1) != 0; }

// check if receive FIFO is empty
INLINE Bool UART_RxEmpty(int uart) { return ((*UART_FR(uart) >> 4) & 1) != 0; }
INLINE Bool UART_RxEmpty_hw(const uart_hw_t* hw) { return ((hw->fr >> 4) & 1) != 0; }

// check if UART is busy transmitting data from shift register
INLINE Bool UART_Busy(int uart) { return ((*UART_FR(uart) >> 3) & 1) != 0; }
INLINE Bool UART_Busy_hw(const uart_hw_t* hw) { return ((hw->fr >> 3) & 1) != 0; }

// check if CTS (clear to send) input is LOW (active)
INLINE Bool UART_CTS(int uart) { return (*UART_FR(uart) & 1) != 0; }
INLINE Bool UART_CTS_hw(const uart_hw_t* hw) { return (hw->fr & 1) != 0; }

// === UART setup

// set integer baud rate divisor (16 bits)
INLINE void UART_IBaud(int uart, int ibaud) { *UART_IBRD(uart) = ibaud; }
INLINE void UART_IBaud_hw(uart_hw_t* hw, int ibaud) { hw->ibrd = ibaud; }

// get integer baud rate divisor (16 bits)
INLINE u16 UART_GetIBaud(int uart) { return (u16)*UART_IBRD(uart); }
INLINE u16 UART_GetIBaud_hw(const uart_hw_t* hw) { return (u16)hw->ibrd; }

// set fractional baud rate divisor (= 1/64 fraction, 6 bits)
INLINE void UART_FBaud(int uart, int fbaud) { *UART_FBRD(uart) = fbaud; }
INLINE void UART_FBaud_hw(uart_hw_t* hw, int fbaud) { hw->fbrd = fbaud; }

// get fractional baud rate divisor (= 1/64 fraction, 6 bits)
INLINE u8 UART_GetFBaud(int uart) { return (u8)(*UART_FBRD(uart) & 0x3f); }
INLINE u8 UART_GetFBaud_hw(const uart_hw_t* hw) { return (u8)(hw->fbrd & 0x3f); }

// set baudrate (range 45..3'000'000 with default clk_peri = 48 MHz; hardware support up to 921'600 bps)
void UART_Baud_hw(uart_hw_t* hw, u32 baudrate);
INLINE void UART_Baud(int uart, u32 baudrate) { UART_Baud_hw(UART_GetHw(uart), baudrate); }

// get baudrate
u32 UART_GetBaud(int uart);
u32 UART_GetBaud_hw(const uart_hw_t* hw);

// set parity UART_PARITY_* (default UART_PARITY_NONE)
void UART_Parity_hw(uart_hw_t* hw, int parity);
INLINE void UART_Parity(int uart, int parity) { UART_Parity_hw(UART_GetHw(uart), parity); }

// set word length to 5..8 (default 5 bits)
INLINE void UART_Word(int uart, int len) { RegMask(UART_LCR(uart), (u32)(len-5)<<5, B5|B6); }
INLINE void UART_Word_hw(uart_hw_t* hw, int len) { RegMask(&hw->lcr_h, (u32)(len-5)<<5, B5|B6); }

// enable FIFO (every FIFO is 32 entries deep)
INLINE void UART_FifoEnable(int uart) { RegSet(UART_LCR(uart), B4); }
INLINE void UART_FifoEnable_hw(uart_hw_t* hw) { RegSet(&hw->lcr_h, B4); }

// disable FIFO (default state; every FIFO is 1 entry deep)
INLINE void UART_FifoDisable(int uart) { RegClr(UART_LCR(uart), B4); }
INLINE void UART_FifoDisable_hw(uart_hw_t* hw) { RegClr(&hw->lcr_h, B4); }

// set 1 stop bit (default value)
INLINE void UART_Stop1(int uart) { RegClr(UART_LCR(uart), B3); }
INLINE void UART_Stop1_hw(uart_hw_t* hw) { RegClr(&hw->lcr_h, B3); }

// set 2 stop bits
INLINE void UART_Stop2(int uart) { RegSet(UART_LCR(uart), B3); }
INLINE void UART_Stop2_hw(uart_hw_t* hw) { RegSet(&hw->lcr_h, B3); }

// send break signal ON (break signal = set TX output to LOW; min. for 2 frames; get UART_BreakError state)
INLINE void UART_BreakOn(int uart) { RegSet(UART_LCR(uart), B0); }
INLINE void UART_BreakOn_hw(uart_hw_t* hw) { RegSet(&hw->lcr_h, B0); }

// send break signal OFF (break signal = set TX output to LOW; min. for 2 frames; get UART_BreakError state)
INLINE void UART_BreakOff(int uart) { RegClr(UART_LCR(uart), B0); }
INLINE void UART_BreakOff_hw(uart_hw_t* hw) { RegClr(&hw->lcr_h, B0); }

// CTS hardware flow control enable (if enabled, data is only transmitted when CTR signal is LOW)
INLINE void UART_CTSEnable(int uart) { RegSet(UART_CR(uart), B15); }
INLINE void UART_CTSEnable_hw(uart_hw_t* hw) { RegSet(&hw->cr, B15); }

// CTS hardware flow control disable (default state; if disabled, data is always transmitted)
INLINE void UART_CTSDisable(int uart) { RegClr(UART_CR(uart), B15); }
INLINE void UART_CTSDisable_hw(uart_hw_t* hw) { RegClr(&hw->cr, B15); }

// RTS hardware flow control enable (if enabled, data is requested by setting the RTS signal
//   to LOW only if there is free space in the receive FIFO)
INLINE void UART_RTSEnable(int uart) { RegSet(UART_CR(uart), B14); }
INLINE void UART_RTSEnable_hw(uart_hw_t* hw) { RegSet(&hw->cr, B14); }

// RTS hardware flow control disable (default state; if disabled, RTS signal is not controlled)
INLINE void UART_RTSDisable(int uart) { RegClr(UART_CR(uart), B14); }
INLINE void UART_RTSDisable_hw(uart_hw_t* hw) { RegClr(&hw->cr, B14); }

// set RTS output ON (output RTS signal when RTS control is disabled: set RTS output to LOW)
INLINE void UART_RTSOutON(int uart) { RegSet(UART_CR(uart), B11); }
INLINE void UART_RTSOutON_hw(uart_hw_t* hw) { RegSet(&hw->cr, B11); }

// set RTS output OFF (default state; output RTS signal when RTS control is disabled: set RTS output to HIGH)
INLINE void UART_RTSOutOFF(int uart) { RegClr(UART_CR(uart), B11); }
INLINE void UART_RTSOutOFF_hw(uart_hw_t* hw) { RegClr(&hw->cr, B11); }

// enable receiver (default state)
INLINE void UART_RxEnable(int uart) { RegSet(UART_CR(uart), B9); }
INLINE void UART_RxEnable_hw(uart_hw_t* hw) { RegSet(&hw->cr, B9); }

// disable receiver
INLINE void UART_RxDisable(int uart) { RegClr(UART_CR(uart), B9); }
INLINE void UART_RxDisable_hw(uart_hw_t* hw) { RegClr(&hw->cr, B9); }

// enable transmitter (default state)
INLINE void UART_TxEnable(int uart) { RegSet(UART_CR(uart), B8); }
INLINE void UART_TxEnable_hw(uart_hw_t* hw) { RegSet(&hw->cr, B8); }

// disable transmitter
INLINE void UART_TxDisable(int uart) { RegClr(UART_CR(uart), B8); }
INLINE void UART_TxDisable_hw(uart_hw_t* hw) { RegClr(&hw->cr, B8); }

// enable loopback test
void UART_LoopEnable(int uart);
void UART_LoopEnable_hw(uart_hw_t* hw);

// disable loopback test (default state)
void UART_LoopDisable(int uart);
void UART_LoopDisable_hw(uart_hw_t* hw);

// UART enable
INLINE void UART_Enable(int uart) { RegSet(UART_CR(uart), B0); }
INLINE void UART_Enable_hw(uart_hw_t* hw) { RegSet(&hw->cr, B0); }

// UART disable (default state)
INLINE void UART_Disable(int uart) { RegClr(UART_CR(uart), B0); }
INLINE void UART_Disable_hw(uart_hw_t* hw) { RegClr(&hw->cr, B0); }

// set receive interrupt FIFO level select UART_LEVEL_* (default UART_LEVEL_MID)
INLINE void UART_RxLevel(int uart, int level) { RegMask(UART_IFLS(uart), (u32)level<<3, B3|B4|B5); }
INLINE void UART_RxLevel_hw(uart_hw_t* hw, int level) { RegMask(&hw->ifls, (u32)level<<3, B3|B4|B5); }

// set transmit interrupt FIFO level select UART_LEVEL_* (default UART_LEVEL_MID)
INLINE void UART_TxLevel(int uart, int level) { RegMask(UART_IFLS(uart), level, B0|B1|B2); }
INLINE void UART_TxLevel_hw(uart_hw_t* hw, int level) { RegMask(&hw->ifls, level, B0|B1|B2); }

// === UART interrupt

// enable interrupt UART_IRQ_*
INLINE void UART_EnableIRQ(int uart, int irq) { RegSet(UART_IMSC(uart), BIT(irq)); }
INLINE void UART_EnableIRQ_hw(uart_hw_t* hw, int irq) { RegSet(&hw->imsc, BIT(irq)); }

// disable interrupt UART_IRQ_*
INLINE void UART_DisableIRQ(int uart, int irq) { RegClr(UART_IMSC(uart), BIT(irq)); }
INLINE void UART_DisableIRQ_hw(uart_hw_t* hw, int irq) { RegClr(&hw->imsc, BIT(irq)); }

// check if raw interrupt (before enabling) UART_IRQ_* is raised
INLINE Bool UART_RawIRQ(int uart, int irq) { return (*UART_RIS(uart) & BIT(irq)) != 0; }
INLINE Bool UART_RawIRQ_hw(const uart_hw_t* hw, int irq) { return (hw->ris & BIT(irq)) != 0; }

// check if masked interrupt (after enabling) UART_IRQ_* is raised
INLINE Bool UART_IRQ(int uart, int irq) { return (*UART_MIS(uart) & BIT(irq)) != 0; }
INLINE Bool UART_IRQ_hw(const uart_hw_t* hw, int irq) { return (hw->mis & BIT(irq)) != 0; }

// clear interrupt UART_IRQ_*
INLINE void UART_ClearIRQ(int uart, int irq) { RegSet(UART_ICR(uart), BIT(irq)); }
INLINE void UART_ClearIRQ_hw(uart_hw_t* hw, int irq) { RegSet(&hw->icr, BIT(irq)); }

// === UART DMA

// enable DMA on error
INLINE void UART_EnableErrDMA(int uart) { RegSet(UART_DMACR(uart), B2); }
INLINE void UART_EnableErrDMA_hw(uart_hw_t* hw) { RegSet(&hw->dmacr, B2); }

// disable DMA on error (default state)
INLINE void UART_DisableErrDMA(int uart) { RegClr(UART_DMACR(uart), B2); }
INLINE void UART_DisableErrDMA_hw(uart_hw_t* hw) { RegClr(&hw->dmacr, B2); }

// enable transmit DMA
INLINE void UART_EnableTxDMA(int uart) { RegSet(UART_DMACR(uart), B1); }
INLINE void UART_EnableTxDMA_hw(uart_hw_t* hw) { RegSet(&hw->dmacr, B1); }

// disable transmit DMA (default state)
INLINE void UART_DisableTxDMA(int uart) { RegClr(UART_DMACR(uart), B1); }
INLINE void UART_DisableTxDMA_hw(uart_hw_t* hw) { RegClr(&hw->dmacr, B1); }

// enable receive DMA
INLINE void UART_EnableRxDMA(int uart) { RegSet(UART_DMACR(uart), B0); }
INLINE void UART_EnableRxDMA_hw(uart_hw_t* hw) { RegSet(&hw->dmacr, B0); }

// disable receive DMA (default state)
INLINE void UART_DisableRxDMA(int uart) { RegClr(UART_DMACR(uart), B0); }
INLINE void UART_DisableRxDMA_hw(uart_hw_t* hw) { RegClr(&hw->dmacr, B0); }

// === extended functions

// soft reset UART (restore to its default state; takes 0.5 us)
void UART_SoftReset_hw(uart_hw_t* hw);
INLINE void UART_SoftReset(int uart) { UART_SoftReset_hw(UART_GetHw(uart)); }

// hard reset UART (send reset signal; takes 0.5 us)
INLINE void UART_HardReset(int uart) { ResetPeriphery(RESET_UART0 + uart); }
INLINE void UART_HardReset_hw(uart_hw_t* hw) { ResetPeriphery(RESET_UART0 + UART_GetInx(hw)); }

// reset UART to its default state
INLINE void UART_Reset(int uart) { UART_HardReset(uart); }
INLINE void UART_Reset_hw(uart_hw_t* hw) { UART_HardReset_hw(hw); }

// initialize UART (and reset to default state) ... GPIO pins are not initialized
//   uart ... uart number 0 or 1
//   baudrate ... baudrate 45 to 3'000'000 Baud; hardware support up to 921'600 bps
//   word ... word length 5 to 8 bits
//   parity ... parity UART_PARITY_*
//   stop ... number of stop bits 1 or 2
//   flow ... use hardware flow control CTS/RTS
// After init, connect UART to GPIO pins using GPIO_Fnc(pin, GPIO_FNC_UART).
// After connecting the GPIO, wait a moment with WaitUs(50) and
// then flush any fake received characters with UART_RecvFlush.
void UART_Init_hw(uart_hw_t* hw, u32 baudrate, u8 word, u8 parity, u8 stop, Bool flow);
INLINE void UART_Init(int uart, u32 baudrate, u8 word, u8 parity, u8 stop, Bool flow)
	{ UART_Init_hw(UART_GetHw(uart), baudrate, word, parity, stop, flow); }

// default initialize UART: 115200 Baud, 8 bits, no parity, 1 stop, no hw control ... GPIO pins are not initialized
// After init, connect UART to GPIO pins using GPIO_Fnc(pin, GPIO_FNC_UART).
// After connecting the GPIO, wait a moment with WaitUs(50) and
// then flush any fake received characters with UART_RecvFlush.
INLINE void UART_InitDef(int uart) { UART_Init(uart, 115200, 8, UART_PARITY_NONE, 1, False); }
INLINE void UART_InitDef_hw(uart_hw_t* hw) { UART_Init_hw(hw, 115200, 8, UART_PARITY_NONE, 1, False); }

// waiting for end of transmission (including transmission shift register)
INLINE void UART_TxWait(int uart) { while (UART_Busy(uart)) {} }
INLINE void UART_TxWait_hw(const uart_hw_t* hw) { while (UART_Busy_hw(hw)) {} }

// check if next character can be transmitted
INLINE Bool UART_SendReady(int uart) { return !UART_TxFull(uart); }
INLINE Bool UART_SendReady_hw(const uart_hw_t* hw) { return !UART_TxFull_hw(hw); }

// check if next character can be received
INLINE Bool UART_RecvReady(int uart) { return !UART_RxEmpty(uart); }
INLINE Bool UART_RecvReady_hw(const uart_hw_t* hw) { return !UART_RxEmpty_hw(hw); }

// send one character with waiting
void UART_SendCharWait(int uart, char data);
void UART_SendCharWait_hw(uart_hw_t* hw, char data);

// receive one character with waiting
char UART_RecvCharWait(int uart);
char UART_RecvCharWait_hw(uart_hw_t* hw);

// flush received character
void UART_RecvFlush(int uart);
void UART_RecvFlush_hw(uart_hw_t* hw);

// send buffer with blocking
void UART_Send(int uart, const u8* src, int len);
void UART_Send_hw(uart_hw_t* hw, const u8* src, int len);

// receive buffer without blocking, returns number of successfully read characters
u32 UART_Recv(int uart, u8* dst, int len);
u32 UART_Recv_hw(uart_hw_t* hw, u8* dst, int len);

// === UART stream

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// initialize stream to write to UART with blocking
void StreamWriteUartInit(sStream* str, int uart);

// formatted print with blocking, with argument list (returns number of characters, without terminating 0)
u32 UART_PrintArg(int uart, const char* fmt, va_list args);

// formatted print with blocking, with variadic arguments (returns number of characters, without terminating 0)
NOINLINE u32 UART_Print(int uart, const char* fmt, ...);

#endif // USE_STREAM

// === UART stdio

#if USE_UART_STDIO

// check if next character can be sent
Bool UartSendReady(void);

// check if next character can be received
Bool UartRecvReady(void);

// print character to UART (wait if not ready)
void UartPrintChar(char ch);

// print unformatted text to UART (returns number of characters, wait if not ready)
u32 UartPrintText(const char* txt);

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// formatted print string to UART, with argument list (returns number of characters)
u32 UartPrintArg(const char* fmt, va_list args);

// formatted print string to UART, with variadic arguments (returns number of characters)
NOINLINE u32 UartPrint(const char* fmt, ...);

#endif // USE_STREAM

// receive character (returns NOCHAR if no character)
char UartGetChar(void);

// initialize UART stdio
void UartStdioInit(void);

// terminate UART stdio
void UartStdioTerm(void);

#endif // USE_UART_STDIO

// === UART test sample (with Tx/Rx loopback)
//
//  In this test example, signals Tx and Rx are connected by an internal loopback.
//  To use real signals, disconnect loopback with 'UART_LoopDisable' function and
//  connect signals to GPIOs with GPIO_Fnc(pin, GPIO_FNC_UART).
//
//  When polling mode is used for both transmit and receive, some test data may
//  be lost due to limited FIFO capacity (which is 32 characters).

// To setup UART test sample, edit flags UARTSAMPLE* in resource.h

#if UARTSAMPLE		// compilation switch from resource.h - compile UART test sample

// --- transmitter

// check if next character can be sent
Bool UartSample_SendReady(void);

// send character (wait if not ready)
void UartSample_SendChar(char ch);

// send buffer (wait if not ready)
void UartSample_SendBuf(const char* buf, int len);

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)
// print to UART
NOINLINE u32 UartSample_Print(const char* fmt, ...);
#endif // USE_STREAM

#if UARTSAMPLE_TXMODE >= 2	// 2 = DMA stream, 3 = DMA single

// get free space in send buffer
int UartSample_SendFree(void);

#endif // UARTSAMPLE_TXMODE

// --- receiver

// check if next character can be received
Bool UartSample_RecvReady(void);

// receive character (wait if not ready)
char UartSample_RecvChar(void);

// receive buffer (without waiting), returns number of received characters
u32 UartSample_RecvBuf(char* buf, int len);

// --- initialize

// initialize sample UART
void UartSample_Init(void);

// terminate sample UART
void UartSample_Term(void);

#endif // UARTSAMPLE

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

#undef uart0
#undef uart1
typedef uart_hw_t uart_inst_t;
#define uart0 ((uart_inst_t*)uart0_hw)
#define uart1 ((uart_inst_t*)uart1_hw)

#ifndef uart_default
#define uart_default uart0
#endif

// Convert UART instance to hardware instance number
INLINE uint uart_get_index(uart_inst_t *uart) { return UART_GetInx(uart); }
INLINE uart_inst_t *uart_get_instance(uint instance) { return UART_GetHw(instance); }
INLINE uart_hw_t *uart_get_hw(uart_inst_t *uart) { return (uart_hw_t*)uart; }

// Parity enumeration
#undef UART_PARITY_NONE
#undef UART_PARITY_EVEN
#undef UART_PARITY_ODD
#undef UART_PARITY_ZERO
#undef UART_PARITY_ONE

typedef enum {
	UART_PARITY_NONE = 0,
	UART_PARITY_EVEN = 1,
	UART_PARITY_ODD = 2,
	UART_PARITY_ZERO = 3,
	UART_PARITY_ONE = 4,
} uart_parity_t;

// Put the UART into a known state, and enable it. Must be called before other functions.
INLINE uint uart_init(uart_inst_t *uart, uint baudrate)
	{ UART_Init_hw(uart, baudrate, 8, UART_PARITY_NONE, 1, False); return UART_GetBaud_hw(uart); }

// DeInitialise a UART
INLINE void uart_deinit(uart_inst_t *uart) { UART_HardReset_hw(uart); }

// Set UART baud rate
INLINE uint uart_set_baudrate(uart_inst_t *uart, uint baudrate)
	{ UART_Baud_hw(uart, baudrate); return UART_GetBaud_hw(uart); }

// Set UART flow control CTS/RTS
INLINE void uart_set_hw_flow(uart_inst_t *uart, bool cts, bool rts)
{
	if (cts) UART_CTSEnable_hw(uart); else UART_CTSDisable_hw(uart);
	if (rts) UART_RTSEnable_hw(uart); else UART_RTSDisable_hw(uart);
}

// Set UART data format
INLINE void uart_set_format(uart_inst_t *uart, uint data_bits, uint stop_bits, uart_parity_t parity)
{
	UART_Word_hw(uart, data_bits);
	if (stop_bits > 1) UART_Stop2_hw(uart); else UART_Stop1_hw(uart);
	UART_Parity_hw(uart, parity);
}

// Setup UART interrupts
INLINE void uart_set_irq_enables(uart_inst_t *uart, bool rx_has_data, bool tx_needs_data)
{
	// enable interrupts
	u32 k = 0;
	if (tx_needs_data) k |= B5;	// TXIM transmit interrupt
	if (rx_has_data) k |= B4 | B6;	// RXIM receive interrupt, RTIM receive timeout
	uart->imsc = k;

	// set FIFO minimum thresholds
	uart->ifls = 0;
}

// Test if specific UART is enabled
INLINE bool uart_is_enabled(uart_inst_t *uart) { return ((uart->cr & B0) != 0); }

// Enable/Disable the FIFOs on specified UART
INLINE void uart_set_fifo_enabled(uart_inst_t *uart, bool enabled)
	{ if (enabled) UART_FifoEnable_hw(uart); else UART_FifoDisable_hw(uart); }

// Determine if space is available in the TX FIFO
INLINE bool uart_is_writable(uart_inst_t *uart) { return UART_SendReady_hw(uart); }

// Wait for the UART TX fifo to be drained
INLINE void uart_tx_wait_blocking(uart_inst_t *uart) { UART_TxWait_hw(uart); }

// Determine whether data is waiting in the RX FIFO
INLINE bool uart_is_readable(uart_inst_t *uart) { return UART_RecvReady_hw(uart); }

// Write to the UART for transmission.
INLINE void uart_write_blocking(uart_inst_t *uart, const u8 *src, size_t len) { UART_Send_hw(uart, src, len); }

// Read from the UART
INLINE  void uart_read_blocking(uart_inst_t *uart, uint8_t *dst, size_t len) { UART_Recv_hw(uart, dst, len); }

// Write single character to UART for transmission.
INLINE void uart_putc_raw(uart_inst_t *uart, char c) { UART_SendCharWait_hw(uart, c); }

// Write single character to UART for transmission, with optional CR/LF conversions
INLINE void uart_putc(uart_inst_t *uart, char c)
	{ if (c == '\n') UART_SendCharWait_hw(uart, '\r'); UART_SendCharWait_hw(uart, c); }

// Write string to UART for transmission, doing any CR/LF conversions
void uart_puts(uart_inst_t *uart, const char *s);

// Read a single character from the UART
INLINE char uart_getc(uart_inst_t *uart) { return UART_RecvCharWait_hw(uart); }

// Assert a break condition on the UART transmission.
INLINE void uart_set_break(uart_inst_t *uart, bool en)
	{ if (en) UART_BreakOn_hw(uart); else UART_BreakOff_hw(uart); }

// Set CR/LF conversion on UART
INLINE void uart_set_translate_crlf(uart_inst_t *uart, bool translate) {}

// Wait for the default UART's TX FIFO to be drained
INLINE void uart_default_tx_wait_blocking(void) { uart_tx_wait_blocking(uart_default); }

// Wait for up to a certain number of microseconds for the RX FIFO to be non empty
INLINE bool uart_is_readable_within_us(uart_inst_t *uart, u32 us)
{
	u32 t = Time();
	do {
		if (UART_RecvReady_hw(uart)) return true;
	} while ((u32)(Time() - t) <= us);
	return false;
}

// Return the DREQ to use for pacing transfers to/from a particular UART instance
INLINE uint uart_get_dreq(uart_inst_t *uart, bool is_tx)
	{ return UART_DREQ(UART_GetInx(uart), is_tx ? 0 : 1); }

// Set up the default UART and assign it to the default GPIO's
INLINE void setup_default_uart()
{
#if !USE_UART_STDIO
	uart_init(UART_GetHw(PICO_DEFAULT_UART), PICO_DEFAULT_UART_BAUD_RATE);

#if RP2040
	if (PICO_DEFAULT_UART_TX_PIN >= 0)
		gpio_set_function(PICO_DEFAULT_UART_TX_PIN, GPIO_FUNC_UART);
	if (PICO_DEFAULT_UART_RX_PIN >= 0)
		gpio_set_function(PICO_DEFAULT_UART_RX_PIN, GPIO_FUNC_UART);
#else
	if (PICO_DEFAULT_UART_TX_PIN >= 0)
		gpio_set_function(PICO_DEFAULT_UART_TX_PIN, (((PICO_DEFAULT_UART_TX_PIN) & B1) == 0) ? GPIO_FUNC_UART : GPIO_FUNC_UART_AUX);
	if (PICO_DEFAULT_UART_RX_PIN >= 0)
		gpio_set_function(PICO_DEFAULT_UART_RX_PIN, (((PICO_DEFAULT_UART_RX_PIN) & B1) == 0) ? GPIO_FUNC_UART : GPIO_FUNC_UART_AUX);
#endif

#endif
}

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_UART_H

#endif // USE_UART	// use UART serial port (sdk_uart.c, sdk_uart.h)
