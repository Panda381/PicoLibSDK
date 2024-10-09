
// ****************************************************************************
//
//                              Decode number
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

#if USE_DECNUM		// use DecNum (decnum.c, decnum.h)

#ifndef _LIB_DECNUM_H
#define _LIB_DECNUM_H

#ifdef __cplusplus
extern "C" {
#endif

// temporary decode text buffer
#define DECNUMBUF_SIZE	16		// max. size of decode text buffer
extern char DecNumBuf[DECNUMBUF_SIZE];

// decode unsigned number into ASCIIZ text buffer (returns number of digits)
//  sep = thousand separator, 0=none
int DecUNum(char* buf, u32 num, char sep);

// decode signed number into ASCIIZ text buffer (returns number of digits)
//  sep = thousand separator, 0=none
int DecNum(char* buf, s32 num, char sep);

// decode hex digit in least significant 4bits
char DecHexDig(int num);

// decode hex number (dig = number of digits)
void DecHexNum(char* buf, u32 num, u8 dig);

// decode 2 digits of number
void Dec2Dig(char* buf, u8 num);

// decode 2 digits of number with space character
void Dec2DigSpc(char* buf, u8 num);

#ifdef __cplusplus
}
#endif

#endif // _LIB_DECNUM_H

#endif // USE_DECNUM		// use DecNum (decnum.c, decnum.h)
