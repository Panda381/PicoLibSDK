
// ****************************************************************************
//
//                           DREQ data request channels
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

#ifndef _SDK_DREQ_H
#define _SDK_DREQ_H

#if RP2040

// extern data request channels
#define DREQ_PIO0_TX0	0x00	// 0
#define DREQ_PIO0_TX1	0x01	// 1
#define DREQ_PIO0_TX2	0x02	// 2
#define DREQ_PIO0_TX3	0x03	// 3
#define DREQ_PIO0_RX0	0x04	// 4
#define DREQ_PIO0_RX1	0x05	// 5
#define DREQ_PIO0_RX2	0x06	// 6
#define DREQ_PIO0_RX3	0x07	// 7
#define DREQ_PIO1_TX0	0x08	// 8
#define DREQ_PIO1_TX1	0x09	// 9
#define DREQ_PIO1_TX2	0x0a	// 10
#define DREQ_PIO1_TX3	0x0b	// 11
#define DREQ_PIO1_RX0	0x0c	// 12
#define DREQ_PIO1_RX1	0x0d	// 13
#define DREQ_PIO1_RX2	0x0e	// 14
#define DREQ_PIO1_RX3	0x0f	// 15
#define DREQ_SPI0_TX	0x10	// 16
#define DREQ_SPI0_RX	0x11	// 17
#define DREQ_SPI1_TX	0x12	// 18
#define DREQ_SPI1_RX	0x13	// 19
#define DREQ_UART0_TX	0x14	// 20
#define DREQ_UART0_RX	0x15	// 21
#define DREQ_UART1_TX	0x16	// 22
#define DREQ_UART1_RX	0x17	// 23
#define DREQ_PWM_WRAP0	0x18	// 24
#define DREQ_PWM_WRAP1	0x19	// 25
#define DREQ_PWM_WRAP2	0x1a	// 26
#define DREQ_PWM_WRAP3	0x1b	// 27
#define DREQ_PWM_WRAP4	0x1c	// 28
#define DREQ_PWM_WRAP5	0x1d	// 29
#define DREQ_PWM_WRAP6	0x1e	// 30
#define DREQ_PWM_WRAP7	0x1f	// 31
#define DREQ_I2C0_TX	0x20	// 32
#define DREQ_I2C0_RX	0x21	// 33
#define DREQ_I2C1_TX	0x22	// 34
#define DREQ_I2C1_RX	0x23	// 35
#define DREQ_ADC	0x24	// 36
#define DREQ_XIP_STREAM	0x25	// 37
#define DREQ_XIP_SSITX	0x26	// 38
#define DREQ_XIP_SSIRX	0x27	// 39

#else // RP2040

#define DREQ_PIO0_TX0	0x00	// 0
#define DREQ_PIO0_TX1	0x01	// 1
#define DREQ_PIO0_TX2	0x02	// 2
#define DREQ_PIO0_TX3	0x03	// 3
#define DREQ_PIO0_RX0	0x04	// 4
#define DREQ_PIO0_RX1	0x05	// 5
#define DREQ_PIO0_RX2	0x06	// 6
#define DREQ_PIO0_RX3	0x07	// 7
#define DREQ_PIO1_TX0	0x08	// 8
#define DREQ_PIO1_TX1	0x09	// 9
#define DREQ_PIO1_TX2	0x0A	// 10
#define DREQ_PIO1_TX3	0x0B	// 11
#define DREQ_PIO1_RX0	0x0C	// 12
#define DREQ_PIO1_RX1	0x0D	// 13
#define DREQ_PIO1_RX2	0x0E	// 14
#define DREQ_PIO1_RX3	0x0F	// 15
#define DREQ_PIO2_TX0	0x10	// 16
#define DREQ_PIO2_TX1	0x11	// 17
#define DREQ_PIO2_TX2	0x12	// 18
#define DREQ_PIO2_TX3	0x13	// 19
#define DREQ_PIO2_RX0	0x14	// 20
#define DREQ_PIO2_RX1	0x15	// 21
#define DREQ_PIO2_RX2	0x16	// 22
#define DREQ_PIO2_RX3	0x17	// 23
#define DREQ_SPI0_TX	0x18	// 24
#define DREQ_SPI0_RX	0x19	// 25
#define DREQ_SPI1_TX	0x1A	// 26
#define DREQ_SPI1_RX	0x1B	// 27
#define DREQ_UART0_TX	0x1C	// 28
#define DREQ_UART0_RX	0x1D	// 29
#define DREQ_UART1_TX	0x1E	// 30
#define DREQ_UART1_RX	0x1F	// 31
#define DREQ_PWM_WRAP0	0x20	// 32
#define DREQ_PWM_WRAP1	0x21	// 33
#define DREQ_PWM_WRAP2	0x22	// 34
#define DREQ_PWM_WRAP3	0x23	// 35
#define DREQ_PWM_WRAP4	0x24	// 36
#define DREQ_PWM_WRAP5	0x25	// 37
#define DREQ_PWM_WRAP6	0x26	// 38
#define DREQ_PWM_WRAP7	0x27	// 39
#define DREQ_PWM_WRAP8	0x28	// 40
#define DREQ_PWM_WRAP9	0x29	// 41
#define DREQ_PWM_WRAP10	0x2A	// 42
#define DREQ_PWM_WRAP11	0x2B	// 43
#define DREQ_I2C0_TX	0x2C	// 44
#define DREQ_I2C0_RX	0x2D	// 45
#define DREQ_I2C1_TX	0x2E	// 46
#define DREQ_I2C1_RX	0x2F	// 47
#define DREQ_ADC	0x30	// 48
#define DREQ_XIP_STREAM	0x31	// 49
#define DREQ_XIP_QMITX	0x32	// 50
#define DREQ_XIP_QMIRX	0x33	// 51
#define DREQ_HSTX	0x34	// 52
#define DREQ_CORESIGHT	0x35	// 53
#define DREQ_SHA256	0x36	// 54

#endif // RP2040

// internal DMA data request
#define DREQ_TIMER0	0x3B	// 59
#define DREQ_TIMER1	0x3C	// 60
#define DREQ_TIMER2	0x3D	// 61
#define DREQ_TIMER3	0x3E	// 62

#define DREQ_TIMER(timer) (DREQ_TIMER0+(timer))

#define DREQ_FORCE	0x3F	// 63, DMA permanent request

#endif // _SDK_DREQ_H
