
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

#include "../../global.h"	// globals

#if USE_UART	// use UART serial port (sdk_uart.c, sdk_uart.h)

#include "../inc/sdk_uart.h"
#include "../inc/sdk_clocks.h"
#include "../inc/sdk_timer.h"
#include "../inc/sdk_reset.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_gpio.h"
#include "../../_lib/inc/lib_ring.h"
#include "../../_lib/inc/lib_ringtx.h"
#include "../../_lib/inc/lib_ringrx.h"
#include "../../_lib/inc/lib_text.h"
#include "../../_lib/inc/lib_stream.h"
#include "../../_lib/inc/lib_print.h"

// set baudrate (range 45..3'000'000 with default clk_peri = 48 MHz; hardware support up to 921'600 bps)
void UART_Baud_hw(uart_hw_t* hw, u32 baudrate)
{
	// divisor = 2 * 64 * UARTCLK / (16 * baud_rate) = 2 * 4 * UARTCLK / baud_rate
	u32 div = (8 * CurrentFreq[CLK_PERI] + baudrate/2 ) / baudrate;

	// divisor integer part = divisor / 128
	u32 idiv = div >> 7;

	// divisor fractional part
	u32 fdiv = ((div & 0x7f) + 1) / 2;

	// limit ranges
	if (idiv == 0) // underflow, limit to minimal divisor
	{
		idiv = 1;
		fdiv = 0;
	}
	else if (idiv >= 65535) // overflow, limit to maximal divisor
	{
		idiv = 65535;
		fdiv = 0;
	}

	// set divisor
	hw->ibrd = idiv;
	hw->fbrd = fdiv;

	// dummy line control register write, to latch new divisor
	RegSet(&hw->lcr_h, 0);		
}

// get baudrate
u32 UART_GetBaud(int uart)
{
	// baud_rate_divisor = UARTCLK / (16 * baud_rate) ... fraction is in 1/64
	u32 k = 64 * *UART_IBRD(uart) + *UART_FBRD(uart);
	return (4 * CurrentFreq[CLK_PERI] + k/2) / k;
}

u32 UART_GetBaud_hw(const uart_hw_t* hw)
{
	// baud_rate_divisor = UARTCLK / (16 * baud_rate) ... fraction is in 1/64
	u32 k = 64 * hw->ibrd + hw->fbrd;
	return (4 * CurrentFreq[CLK_PERI] + k/2) / k;
}

// set parity UART_PARITY_* (default UART_PARITY_NONE)
void UART_Parity_hw(uart_hw_t* hw, int parity)
{
	// bit 7: SPS stick parity enable
	// bit 2: EPS even parity select
	// bit 1: PEN parity enable
	u32 k = 0; // no parity
	if (parity > 0)
	{
		k = B1; // PEN parity enable
		if ((parity & 1) != 0) k |= B2; // EPS even parity select
		if (parity >= 3) k |= B7; // SPS stick parity enable
	}
	RegMask(&hw->lcr_h, k, B1|B2|B7);
}

// enable loopback test
void UART_LoopEnable(int uart)
{
	RegSet(UART_CR(uart), B7); // reconnect to loopback
#if USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)
	WaitUs(50);  // wait to stabilize connection
#else
	int i;
	for (i = 1000; i > 0; i--) cb();
#endif
	UART_RecvFlush(uart); // flush fake received characters
}

void UART_LoopEnable_hw(uart_hw_t* hw)
{
	RegSet(&hw->cr, B7); // reconnect to loopback
#if USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)
	WaitUs(50);  // wait to stabilize connection
#else
	int i;
	for (i = 1000; i > 0; i--) cb();
#endif
	UART_RecvFlush_hw(hw); // flush fake received characters
}

// disable loopback test (default state)
void UART_LoopDisable(int uart)
{
	RegClr(UART_CR(uart), B7); // reconnect to pins
#if USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)
	WaitUs(50);  // wait to stabilize connection
#else
	int i;
	for (i = 1000; i > 0; i--) cb();
#endif
	UART_RecvFlush(uart); // flush fake received characters
}

void UART_LoopDisable_hw(uart_hw_t* hw)
{
	RegClr(&hw->cr, B7); // reconnect to pins
#if USE_TIMER	// use Timer with alarm (sdk_timer.c, sdk_timer.h)
	WaitUs(50);  // wait to stabilize connection
#else
	int i;
	for (i = 1000; i > 0; i--) cb();
#endif
	UART_RecvFlush_hw(hw); // flush fake received characters
}

// === extended functions

// soft reset UART (restore to its default state; takes 0.5 us)
void UART_SoftReset_hw(uart_hw_t* hw)
{
	hw->cr = B9|B8;		// disable UART
	hw->imsc = 0;		// disable interrupts
	hw->dmacr = 0;		// disable DMAs
	hw->icr = 0x7ff;	// clear interrupts
	hw->ifls = 2 | (2 << 3); // FIFO levels
	hw->lcr_h = 0;		// reset line control register
	hw->ibrd = 0;		// integer baud rate
	hw->fbrd = 0;		// fractional baud rate
	hw->rsr = 0x0f;		// clear receive errors
}

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
void UART_Init_hw(uart_hw_t* hw, u32 baudrate, u8 word, u8 parity, u8 stop, Bool flow)
{
	// reset UART to its default state
	UART_Reset_hw(hw);

	// set baudrate
	UART_Baud_hw(hw, baudrate);

	// set word length
	UART_Word_hw(hw, word);

	// set parity
	UART_Parity_hw(hw, parity);

	// set 2 stop bits (or 1 stop bit otherwise)
	if (stop > 1) UART_Stop2_hw(hw);

	// set hardware control
	if (flow)
	{
		UART_CTSEnable_hw(hw);
		UART_RTSEnable_hw(hw);
	}

	// enable FIFO
	UART_FifoEnable_hw(hw);

	// enable DREQ
	UART_EnableRxDMA_hw(hw);
	UART_EnableTxDMA_hw(hw);

	// enable UART
	UART_Enable_hw(hw);
}

// send one character with waiting
void UART_SendCharWait(int uart, char data)
{
	while (!UART_SendReady(uart)) {}
	UART_SendChar(uart, data);
}

void UART_SendCharWait_hw(uart_hw_t* hw, char data)
{
	while (!UART_SendReady_hw(hw)) {}
	UART_SendChar_hw(hw, data);
}

// receive one character with waiting
char UART_RecvCharWait(int uart)
{
	while (!UART_RecvReady(uart)) {}
	return UART_RecvChar(uart);
}

char UART_RecvCharWait_hw(uart_hw_t* hw)
{
	while (!UART_RecvReady_hw(hw)) {}
	return UART_RecvChar_hw(hw);
}

// flush received character
void UART_RecvFlush(int uart)
{
	while (UART_RecvReady(uart)) UART_RecvChar(uart);
}

void UART_RecvFlush_hw(uart_hw_t* hw)
{
	while (UART_RecvReady_hw(hw)) UART_RecvChar_hw(hw);
}

// send buffer with blocking
void UART_Send(int uart, const u8* src, int len)
{
	for (; len > 0; len--) UART_SendCharWait(uart, *src++);
}

void UART_Send_hw(uart_hw_t* hw, const u8* src, int len)
{
	for (; len > 0; len--) UART_SendCharWait_hw(hw, *src++);
}

// receive buffer without blocking, returns number of successfully read characters
u32 UART_Recv(int uart, u8* dst, int len)
{
	u32 num = 0;
	for (; len > 0; len--)
	{
		if (!UART_RecvReady(uart)) break;
		*dst++ = UART_RecvChar(uart);
		num++;
	}
	return num;
}

u32 UART_Recv_hw(uart_hw_t* hw, u8* dst, int len)
{
	u32 num = 0;
	for (; len > 0; len--)
	{
		if (!UART_RecvReady_hw(hw)) break;
		*dst++ = UART_RecvChar_hw(hw);
		num++;
	}
	return num;
}

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// callback - write data to UART
static u32 StreamWriteUart(sStream* str, const void* buf, u32 num)
{
	UART_Send((int)str->cookie, (const u8*)buf, num);
	return num;
}

// initialize stream to write to UART with blocking
void StreamWriteUartInit(sStream* str, int uart)
{
	Stream0Init(str); // clear stream descriptor
	str->cookie = uart; // cookie = uart number
	str->write = StreamWriteUart; // write function
}

// formatted print with blocking, with argument list (returns number of characters, without terminating 0)
u32 UART_PrintArg(int uart, const char* fmt, va_list args)
{
	// write and read stream
	sStream wstr, rstr;

	// initialize stream to read from
	StreamReadBufInit(&rstr, fmt, StrLen(fmt));

	// initialize stream to write to
	StreamWriteUartInit(&wstr, uart);
	
	// print string
	return StreamPrintArg(&wstr, &rstr, args);
}

// formatted print with blocking, with variadic arguments (returns number of characters, without terminating 0)
NOINLINE u32 UART_Print(int uart, const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = UART_PrintArg(uart, fmt, args);
	va_end(args);
	return n;
}

#endif // USE_STREAM

// === UART stdio

#if USE_UART_STDIO
//#define UART_STDIO_PORT	0	// UART stdio port 0 or 1
//#define UART_STDIO_TX		0	// UART stdio TX GPIO pin (function mode UART or AUX is auto-selected)
//#define UART_STDIO_RX		1	// UART stdio RX GPIO pin (function mode UART or AUX is auto-selected)
//#define UART_STDIO_TXBUF	128	// size of transmit ring buffer of UART stdio
//#define UART_STDIO_RXBUF	128	// size of receive ring buffer of UART stdio
//#define UART_STDIO_TXSPIN	28	// transmitter spinlock 0..31 (-1 = not used)
//#define UART_STDIO_RXSPIN	29	// receiver spinlock 0..31 (-1 = not used)

// internal function to force next send data
void UartStdioSendForce(sRing* ring)
{
	NVIC_IRQForce(IRQ_UART0 + UART_STDIO_PORT); // force UART interrupt
}

// send ring buffer
//   name, size in bytes, index of spinlock, force send handler, force receive handler, cookie
RING(UartStdioTxBuf, UART_STDIO_TXBUF, UART_STDIO_TXSPIN, UartStdioSendForce, NULL, 0);

// receive ring buffer
//   name, size in bytes, index of spinlock, force send handler, force receive handler, cookie
RING(UartStdioRxBuf, UART_STDIO_RXBUF, UART_STDIO_RXSPIN, NULL, NULL, 0);

// check if next character can be sent
Bool UartSendReady(void)
{
	return RingWriteReady(&UartStdioTxBuf, 1);
}

// check if next character can be received
Bool UartRecvReady(void)
{
	return RingReadReady(&UartStdioRxBuf, 1);
}

// print character to UART (wait if not ready)
void UartPrintChar(char ch)
{
	RingWrite8Wait(&UartStdioTxBuf, (u8)ch);
}

// print unformatted text to UART (returns number of characters, wait if not ready)
u32 UartPrintText(const char* txt)
{
	u32 n = StrLen(txt); // get text length
	RingWriteWait(&UartStdioTxBuf, txt, n);
	return n;
}

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// callback - write data to UART
u32 StreamWriteUartStdio(sStream* str, const void* buf, u32 num)
{
	RingWriteWait(&UartStdioTxBuf, buf, num);
	return num;
}

// formatted print string to UART, with argument list (returns number of characters)
u32 UartPrintArg(const char* fmt, va_list args)
{
	// write and read stream
	sStream wstr, rstr;

	// initialize stream to read from
	StreamReadBufInit(&rstr, fmt, StrLen(fmt));

	// initialize stream to write to
	Stream0Init(&wstr); // initialize nul stream
	wstr.write = StreamWriteUartStdio; // write callback
	
	// print string
	return StreamPrintArg(&wstr, &rstr, args);
}

// formatted print string to UART, with variadic arguments (returns number of characters)
NOINLINE u32 UartPrint(const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = UartPrintArg(fmt, args);
	va_end(args);
	return n;
}

#endif // USE_STREAM

// receive character (returns NOCHAR if no character)
char UartGetChar(void)
{
	return RingReadChar(&UartStdioRxBuf);
}

// UART handler
void UartStdioHandler(void)
{
	char ch;

	// send data
	while (UART_SendReady(UART_STDIO_PORT) && // free space in transmit FIFO
		RingReadReady(&UartStdioTxBuf, 1)) // some data are ready to send
			UART_SendChar(UART_STDIO_PORT, RingReadChar(&UartStdioTxBuf));

	// clear transmit interrupt request (when there is no more data to send)
	UART_ClearIRQ(UART_STDIO_PORT, UART_IRQ_TX);

	// receive data
	while (UART_RecvReady(UART_STDIO_PORT)) // received data ready
	{
		ch = UART_RecvChar(UART_STDIO_PORT); // receive character from UART
		if (RingWriteReady(&UartStdioRxBuf, 1)) // if space in receive buffer
			RingWrite8(&UartStdioRxBuf, ch); // write character
		// otherwise lost received data if receive buffer is full
	}
}

// initialize UART stdio
void UartStdioInit(void)
{
	// default initialize UART: 115200 Baud, 8 bits, no parity, 1 stop, no hw control
	UART_InitDef(UART_STDIO_PORT);

	// set TX and RX pins
#if RP2040
	GPIO_Fnc(UART_STDIO_TX, GPIO_FNC_UART);
	GPIO_Fnc(UART_STDIO_RX, GPIO_FNC_UART);
#else
	GPIO_Fnc(UART_STDIO_TX, (((UART_STDIO_TX) & B1) == 0) ? GPIO_FNC_UART : GPIO_FNC_AUX);
	GPIO_Fnc(UART_STDIO_RX, (((UART_STDIO_RX) & B1) == 0) ? GPIO_FNC_UART : GPIO_FNC_AUX);
#endif

	// set UART handler
	SetHandler(IRQ_UART0 + UART_STDIO_PORT, UartStdioHandler);

	// enable UART interrupt
	NVIC_IRQEnable(IRQ_UART0 + UART_STDIO_PORT);

	// enable interrupts from FIFO
// Receive interrupt is triggered when the FIFO threshold level is reached.
// We also need a receive timeout interrupt (occurs if a character does not
// arrive for 32 clock cycles) to get the rest of the received bytes out of the FIFO.
	UART_EnableIRQ(UART_STDIO_PORT, UART_IRQ_TX); // enable transmit interrupt
	UART_EnableIRQ(UART_STDIO_PORT, UART_IRQ_RX); // enable receive interrupt
	UART_EnableIRQ(UART_STDIO_PORT, UART_IRQ_TIMEOUT); // enable receive timeout interrupt
}

// terminate UART stdio
void UartStdioTerm(void)
{
	// disable interrupts from FIFO
	UART_DisableIRQ(UART_STDIO_PORT, UART_IRQ_TX); // disable transmit interrupt
	UART_DisableIRQ(UART_STDIO_PORT, UART_IRQ_RX); // disable receive interrupt
	UART_DisableIRQ(UART_STDIO_PORT, UART_IRQ_TIMEOUT); // enable receive timeout interrupt
	
	// disable UART interrupt
	NVIC_IRQDisable(IRQ_UART0 + UART_STDIO_PORT);

	// reset UART to its defalt state
	UART_Reset(UART_STDIO_PORT);

	// disable pins
	GPIO_Reset(UART_STDIO_TX);
	GPIO_Reset(UART_STDIO_RX);
}

#endif // USE_UART_STDIO

// === UART test sample (with Tx/Rx loopback)

#if UARTSAMPLE		// compilation switch from config_def.h - compile UART test sample

// --- transmitter, 1) polling mode

#if UARTSAMPLE_TXMODE == 0	// 0 = polling mode

// check if next character can be sent
Bool UartSample_SendReady(void) { return UART_SendReady(UARTSAMPLE_UART); }

// send character (wait if not ready)
void UartSample_SendChar(char ch) { UART_SendCharWait(UARTSAMPLE_UART, ch); }

// send buffer (wait if not ready)
void UartSample_SendBuf(const char* buf, int len) { UART_Send(UARTSAMPLE_UART, buf, len); }

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// print to UART
NOINLINE u32 UartSample_Print(const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = UART_PrintArg(UARTSAMPLE_UART, fmt, args);
	va_end(args);
	return n;
}

#endif // USE_STREAM

// --- transmitter, 2) interrupt mode

#elif UARTSAMPLE_TXMODE == 1	// 1 = interrupt mode with ring buffer

// internal function to force next send data
void UartSample_SendForce(sRing* ring)
{
	NVIC_IRQForce(IRQ_UART0 + UARTSAMPLE_UART); // force UART interrupt
}

// send ring buffer
//   name, size in bytes, index of spinlock, force send handler, cookie
RING(UartSample_TxBuf, 1<<UARTSAMPLE_TXORDER, UARTSAMPLE_TXSPIN, UartSample_SendForce, NULL, 0);

// check if next character can be sent
Bool UartSample_SendReady(void) { return RingWriteReady(&UartSample_TxBuf, 1); }

// send character (wait if not ready)
void UartSample_SendChar(char ch) { RingWrite8Wait(&UartSample_TxBuf, (u8)ch); }

// send buffer (wait if not ready)
void UartSample_SendBuf(const char* buf, int len) { RingWriteWait(&UartSample_TxBuf, buf, len); }

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// print to UART
NOINLINE u32 UartSample_Print(const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = RingPrintArg(&UartSample_TxBuf, fmt, args);
	va_end(args);
	return n;
}

#endif // USE_STREAM

// --- transmitter, 3) DMA mode

#elif UARTSAMPLE_TXMODE >= 2	// 2 = DMA stream, 3 = DMA single

// DMA send ring buffer
//   name, port, index of spinlock, order of size, dma channel, data request, entry size
RINGTX(UartSample_TxBuf, UART_DR(UARTSAMPLE_UART), UARTSAMPLE_TXSPIN, UARTSAMPLE_TXORDER,
	UARTSAMPLE_TXDMA, UART_DREQ(UARTSAMPLE_UART,0), DMA_SIZE_8);

// check if next character can be sent
Bool UartSample_SendReady(void) { return RingTxReady(&UartSample_TxBuf, 1); }

// send character (wait if not ready)
void UartSample_SendChar(char ch) { RingTxWrite8Wait(&UartSample_TxBuf, (u8)ch); }

// send buffer (wait if not ready)
void UartSample_SendBuf(const char* buf, int len) { RingTxWriteSend(&UartSample_TxBuf, buf, len); }

// get free space in send buffer
int UartSample_SendFree(void) { return RingTxFree(&UartSample_TxBuf); }

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// print to UART
NOINLINE u32 UartSample_Print(const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = RingTxPrintArg(&UartSample_TxBuf, fmt, args);
	va_end(args);
	return n;
}

#endif // USE_STREAM

#endif // UARTSAMPLE_TXMODE

// --- receiver, 1) polling mode

#if UARTSAMPLE_RXMODE == 0	// 0 = polling mode

// check if next character can be received
Bool UartSample_RecvReady(void) { return UART_RecvReady(UARTSAMPLE_UART); }

// receive character (wait if not ready)
char UartSample_RecvChar(void) { return UART_RecvCharWait(UARTSAMPLE_UART); }

// receive buffer (without waiting), returns number of received characters
u32 UartSample_RecvBuf(char* buf, int len) { return UART_Recv(UARTSAMPLE_UART, buf, len); }

// --- receiver, 2) interrupt mode

#elif UARTSAMPLE_RXMODE == 1	// 1 = interrupt mode with ring buffer

// receive ring buffer
//   name, size in bytes, index of spinlock, force send handler, cookie
RING(UartSample_RxBuf, 1<<UARTSAMPLE_RXORDER, UARTSAMPLE_RXSPIN, NULL, NULL, 0);

// check if next character can be received
Bool UartSample_RecvReady(void) { return RingReadReady(&UartSample_RxBuf, 1); }

// receive character (wait if not ready)
char UartSample_RecvChar(void) { return (char)RingRead8Wait(&UartSample_RxBuf); }

// receive buffer, returns number of received characters
u32 UartSample_RecvBuf(char* buf, int len) { return RingReadData(&UartSample_RxBuf, buf, len); }

// --- receiver, 3) DMA mode

#elif UARTSAMPLE_RXMODE == 2	// 2 = DMA with ring buffer

// DMA receive ring buffer
//   name, port, index of spinlock, order of size, dma channel, data request, entry size
RINGRX(UartSample_RxBuf, UART_DR(UARTSAMPLE_UART), UARTSAMPLE_RXSPIN, UARTSAMPLE_RXORDER,
	UARTSAMPLE_RXDMA, UART_DREQ(UARTSAMPLE_UART,1), DMA_SIZE_8);

// check if next character can be received
Bool UartSample_RecvReady(void) { return RingRxReady(&UartSample_RxBuf, 1); }

// receive character (wait if not ready)
char UartSample_RecvChar(void) { return (char)RingRxRead8Wait(&UartSample_RxBuf); }

// receive buffer, returns number of received characters
u32 UartSample_RecvBuf(char* buf, int len) { return RingRxReadData(&UartSample_RxBuf, buf, len); }

#endif // UARTSAMPLE_RXMODE

// --- initialize

// UART handler
void UartSample_Handler(void)
{
	char ch;

#if UARTSAMPLE_TXMODE == 1	// 1 = interrupt mode with ring buffer

	// send data
	while (UART_SendReady(UARTSAMPLE_UART) && // free space in transmit FIFO
		RingReadReady(&UartSample_TxBuf, 1)) // some data are ready to send
			UART_SendChar(UARTSAMPLE_UART, RingRead8Wait(&UartSample_TxBuf));

	// clear transmit interrupt request (when there is no more data to send)
	UART_ClearIRQ(UARTSAMPLE_UART, UART_IRQ_TX);

#endif // UARTSAMPLE_TXMODE

#if UARTSAMPLE_RXMODE == 1	// 1 = interrupt mode with ring buffer

	// receive data
	while (UART_RecvReady(UARTSAMPLE_UART)) // received data ready
	{
		ch = UART_RecvChar(UARTSAMPLE_UART); // receive character from UART
		if (RingWriteReady(&UartSample_RxBuf, 1)) // if space in receive buffer
			RingWrite8(&UartSample_RxBuf, ch); // send character
		// otherwise lost received data if receive buffer is full
	}

#endif // UARTSAMPLE_RXMODE

}

// initialize sample UART (uses UART0, 512 bytes of RAM and 2 spinlocks)
void UartSample_Init(void)
{
	// default initialize UART: 115200 Baud, 8 bits, no parity, 1 stop, no hw control
	UART_InitDef(UARTSAMPLE_UART);

	// activate loopback test
	UART_LoopEnable(UARTSAMPLE_UART);

#if UARTSAMPLE_RXMODE == 2	// 2 = DMA with ring buffer

	// enable Rx DMA
	UART_EnableRxDMA(UARTSAMPLE_UART);

	// start DMA read transfer
	RingRxStart(&UartSample_RxBuf);

#endif // UARTSAMPLE_RXMODE

#if UARTSAMPLE_TXMODE >= 2	// 2 = DMA stream, 3 = DMA single

	// fill up send buffer with 'SYN' character
	memset(UartSample_TxBuf_buf, CH_SYN, (1<<UARTSAMPLE_TXORDER));

	// prepare ring buffer
	RingTxPrep(&UartSample_TxBuf);

	// enable Tx DMA
	UART_EnableTxDMA(UARTSAMPLE_UART);

#endif // UARTSAMPLE_TXMODE

#if UARTSAMPLE_TXMODE == 2	// 2 = DMA stream

	// start DMA write transfer
	RingTxStart(&UartSample_TxBuf);

#endif // UARTSAMPLE_TXMODE

#if (UARTSAMPLE_TXMODE == 1) || (UARTSAMPLE_RXMODE == 1) // 1 = interrupt mode with ring buffer
	// set UART handler
	SetHandler(IRQ_UART0 + UARTSAMPLE_UART, UartSample_Handler);

	// enable UART interrupt
	NVIC_IRQEnable(IRQ_UART0 + UARTSAMPLE_UART);
#endif // UARTSAMPLE_TXMODE, UARTSAMPLE_RXMODE

	// enable interrupts from FIFO
// Receive interrupt is triggered when the FIFO threshold level is reached.
// We also need a receive timeout interrupt (occurs if a character does not
// arrive for 32 clock cycles) to get the rest of the received bytes out of the FIFO.

#if UARTSAMPLE_TXMODE == 1	// 1 = interrupt mode with ring buffer
	UART_EnableIRQ(UARTSAMPLE_UART, UART_IRQ_TX); // enable transmit interrupt
#endif // UARTSAMPLE_TXMODE

#if UARTSAMPLE_RXMODE == 1	// 1 = interrupt mode with ring buffer
	UART_EnableIRQ(UARTSAMPLE_UART, UART_IRQ_RX); // enable receive interrupt
	UART_EnableIRQ(UARTSAMPLE_UART, UART_IRQ_TIMEOUT); // enable receive timeout interrupt
#endif // UARTSAMPLE_RXMODE

	// enable global interrupt
	//ei(); // should be already enabled, but don't forget to it
}

// terminate sample UART
void UartSample_Term(void)
{
#if UARTSAMPLE_TXMODE == 2	// 2 = DMA with ring buffer

	// stop DMA write transfer
	RingTxStop(&UartSample_TxBuf);

	// disable Tx DMA
	UART_DisableTxDMA(UARTSAMPLE_UART);

#endif // UARTSAMPLE_TXMODE

#if UARTSAMPLE_RXMODE == 2	// 2 = DMA with ring buffer

	// stop DMA read transfer
	RingRxStop(&UartSample_RxBuf);

	// disable Rx DMA
	UART_DisableRxDMA(UARTSAMPLE_UART);

#endif // UARTSAMPLE_RXMODE

#if (UARTSAMPLE_TXMODE == 1) || (UARTSAMPLE_RXMODE == 1) // 1 = interrupt mode with ring buffer
	// disable interrupts from FIFO
	UART_DisableIRQ(UARTSAMPLE_UART, UART_IRQ_TX); // disable transmit interrupt
	UART_DisableIRQ(UARTSAMPLE_UART, UART_IRQ_RX); // disable receive interrupt
	UART_DisableIRQ(UARTSAMPLE_UART, UART_IRQ_TIMEOUT); // enable receive timeout interrupt
	
	// disable UART interrupt
	NVIC_IRQDisable(IRQ_UART0 + UARTSAMPLE_UART);
#endif // UARTSAMPLE_TXMODE, UARTSAMPLE_RXMODE

	// reset UART to its defalt state
	UART_Reset(UARTSAMPLE_UART);
}

#endif // UARTSAMPLE

#if USE_ORIGSDK		// include interface of original SDK

// Write string to UART for transmission, doing any CR/LF conversions
void uart_puts(uart_inst_t *uart, const char *s)
{
	Bool last_cr = False;
	while (*s != 0)
	{
		if (last_cr)
			uart_putc_raw(uart, *s);
		else
			uart_putc(uart, *s);
		last_cr = *s++ == '\r';
	}
}

#endif // USE_ORIGSDK

#endif // USE_UART	// use UART serial port (sdk_uart.c, sdk_uart.h)
