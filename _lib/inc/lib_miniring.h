
// ****************************************************************************
//
//                               Mini-Ring Buffer
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

// Data transfer is atomic if one side performs only writes and the other only reads.

#if USE_MINIRING	// use Mini-Ring buffer (lib_miniring.c, lib_miniring.h)

#ifndef _LIB_MINIRING_H
#define _LIB_MINIRING_H

#ifdef __cplusplus
extern "C" {
#endif

// mini-ring buffer
typedef struct {
	u8*		buf;		// pointer to data buffer
	uint		size;		// size of data buffer in bytes
	volatile uint	write;		// write offset into buffer
	volatile uint	read;		// read offset from buffer
} sMiniRing;

// create mini-ring buffer with static buffer (in *.c file)
//   name = name
//   size = size of data buffer in bytes
// Declare in *.h file: extern sMiniRing name;
#define MINIRING(name,size) u8 name ## _buf[size]; sMiniRing name = \
	{ name ## _buf, (size), 0, 0 }

// initialize mini-ring buffer (not needed with static mini-ring buffer, created with MINIRING() macro)
//   ring = pointer to mini-ring buffer
//   buf = data buffer (of size 'size' bytes)
//   size = buffer size in bytes
void MiniRingInit(sMiniRing* ring, u8* buf, uint size);

// clear mini-ring buffer
void MiniRingClear(sMiniRing* ring);

// check if available space for write 1 byte into mini-ring buffer
Bool MiniRingWriteReady(sMiniRing* ring);

// write byte into mini-ring buffer, without waiting (returns False if buffer is full)
Bool MiniRingWrite(sMiniRing* ring, u8 data);

// write data from buffer into mini-ring buffer, without waiting (data is lost if buffer is full,
//  returns number of bytes successfully written to the buffer)
uint MiniRingBufWrite(sMiniRing* ring, const u8* data, uint len);

// write byte into mini-ring buffer, wait until ready
void MiniRingWriteWait(sMiniRing* ring, u8 data);

// write data from buffer into mini-ring buffer, wait until ready
void MiniRingBufWriteWait(sMiniRing* ring, const u8* data, uint len);

// check if byte is ready to read from mini-ring buffer
Bool MiniRingReadReady(sMiniRing* ring);

// read byte from mini-ring buffer, without waiting (returns 0 if buffer is empty)
u8 MiniRingRead(sMiniRing* ring);

// read data from mini-ring buffer, without waiting (returns number of bytes successfully read from the buffer)
uint MiniRingBufRead(sMiniRing* ring, u8* data, uint len);

// read byte from mini-ring buffer, wait until ready
u8 MiniRingReadWait(sMiniRing* ring);

// read data from mini-ring buffer, wait until ready
void MiniRingBufReadWait(sMiniRing* ring, u8* data, uint len);

#ifdef __cplusplus
}
#endif

#endif // _LIB_MINIRING_H

#endif // USE_MINIRING	// use Mini-Ring buffer (lib_miniring.c, lib_miniring.h)
