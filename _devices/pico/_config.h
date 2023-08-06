// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.

// To use UART console, connect UART-USB converter to pins GPIO0 (TX output from Pico)
// and GPIO1 (RX input to Pico) and set COM port to 115200 Baud, 8 bits, no parity,
// 1 stop bit, no flow control.

// UART stdio
#ifndef UART_STDIO_PORT
#define UART_STDIO_PORT	0		// UART stdio port 0 or 1
#endif

#ifndef UART_STDIO_TX
#define UART_STDIO_TX	0		// UART stdio TX GPIO pin
#endif

#ifndef UART_STDIO_RX
#define UART_STDIO_RX	1		// UART stdio RX GPIO pin
#endif
