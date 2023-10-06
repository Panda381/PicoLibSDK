
// ****************************************************************************
//
//                               Ring Buffer
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

// Data transfer can be atomic without using lock, if one side performs only writes and the other only reads.

#if USE_RING	// use Ring buffer (lib_ring.c, lib_ring.h)

#ifndef _LIB_RING_H
#define _LIB_RING_H

#include "../../_sdk/inc/sdk_spinlock.h"	// spinlock
#include "lib_stream.h"

#ifdef __cplusplus
extern "C" {
#endif

struct sRing_;

// wake-up function to force handler to send or receive data
typedef void (*pRingForce)(struct sRing_*);

// ring buffer (size 32 bytes)
typedef struct sRing_ {
	u8*		buf;		// pointer to data buffer
	u32		size;		// size of buffer (number of bytes)
	volatile u32	write;		// write offset into buffer
	volatile u32	read;		// read offset from buffer
	int		spinlock; 	// index of spinlock (-1 = not used)
	pRingForce	force_send;	// wake-up function to send data (NULL = none)
	pRingForce	force_recv;	// wake-up function to receive data (NULL = none)
	u32		cookie;		// user data
} sRing;

// create ring buffer with static buffer (in *.c file)
//   name = name
//   size = size in bytes
//   spinlock = index of spinlock (-1 = not used)
//   force_send = wake-up function to force handler to send data (NULL = none)
//   force_recv = wake-up function to force handler to receive data (NULL = none)
//   cookie = user data
// Declare in *.h file: extern sRing name;
#define RING(name,size,spinlock,force_send,force_recv,cookie) u8 name ## _buf[size]; sRing name = \
	{ name ## _buf, (size), 0, 0, (spinlock), (force_send), (force_recv), (cookie) }

// initialize ring buffer (not needed with static ring buffer, created with RING() macro)
//   ring = pointer to ring buffer
//   buf = data buffer (of size 'size' bytes)
//   size = buffer size in bytes
//   spinlock = index of spinlock (-1 = not used)
//   force_send = wake-up function to force handler to send data (NULL = none)
//   force_recv = wake-up function to force handler to receive data (NULL = none)
//   cookie = user data (uart index)
void RingInit(sRing* ring, u8* buf, u32 size, int spinlock, pRingForce force_send, pRingForce force_recv, u32 cookie);

// update ring buffer send transmission (force handler to send data)
void RingWriteUpdate(sRing* ring);

// update ring buffer receive transmission (force handlers to receive data)
void RingReadUpdate(sRing* ring);

// clear ring buffer
// - Function can be called from inside the locked section.
void RingClear(sRing* ring);

// get free space for writing
// - Function can be called from inside the locked section.
u32 RingFree(sRing* ring);

// get bytes available for reading
// - Function can be called from inside the locked section.
u32 RingNum(sRing* ring);

// check if ring buffer is ready to write required number of bytes
// - Function can be called from inside the locked section.
Bool RingWriteReady(sRing* ring, u32 len);

// check if ring buffer is ready to read required number of bytes
// - Function can be called from inside the locked section.
Bool RingReadReady(sRing* ring, u32 len);

// write whole data into ring buffer, without waiting (returns False if buffer has not free space)
// Data is written only if there is enough space in the ring buffer. Cannot write only part of the data.
Bool RingWrite(sRing* ring, const void* src, u32 len);

// write part of data into ring buffer, without waiting (returns number of written bytes)
u32 RingWriteData(sRing* ring, const void* src, u32 len);

// write 8-bit data into ring buffer, without waiting (returns False on buffer overflow)
Bool RingWrite8(sRing* ring, u8 data);

// write 16-bit data into ring buffer, without waiting (returns False on buffer overflow)
Bool RingWrite16(sRing* ring, u16 data);

// write 32-bit data into ring buffer, without waiting (returns False on buffer overflow)
Bool RingWrite32(sRing* ring, u32 data);

// write data into ring buffer, wait until ready
void RingWriteWait(sRing* ring, const void* src, u32 len);

// write 8-bit data into ring buffer, wait until ready
void RingWrite8Wait(sRing* ring, u8 data);

// write 16-bit data into ring buffer, wait until ready
void RingWrite16Wait(sRing* ring, u16 data);

// write 32-bit data into ring buffer, wait until ready
void RingWrite32Wait(sRing* ring, u32 data);

// read whole data from ring buffer, without waiting (return False if buffer has not enough data)
// Data is read only if there is enough data in the ring buffer. Cannot read only part of the data.
Bool RingRead(sRing* ring, void* dst, u32 len);

// read part of data from ring buffer, without waiting (returns number of read bytes)
u32 RingReadData(sRing* ring, void* dst, u32 len);

// read 1 character from ring buffer, without waiting (returns 0 if buffer has not enough data)
char RingReadChar(sRing* ring);

// read 8-bit data from ring buffer, without waiting (returns False if buffer has not enough data)
Bool RingRead8(sRing* ring, u8* dst);

// read 16-bit data from ring buffer, without waiting (returns False if buffer has not enough data)
Bool RingRead16(sRing* ring, u16* dst);

// read 32-bit data from ring buffer, without waiting (returns False if buffer has not enough data)
Bool RingRead32(sRing* ring, u32* dst);

// read data from ring buffer, wait until ready
void RingReadWait(sRing* ring, void* dst, u32 len);

// read 8-bit data from ring buffer and wait until ready
u8 RingRead8Wait(sRing* ring);

// read 16-bit data from ring buffer and wait until ready
u16 RingRead16Wait(sRing* ring);

// read 32-bit data from ring buffer and wait until ready
u32 RingRead32Wait(sRing* ring);

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

// initialize stream to write to ring buffer
void StreamWriteRingInit(sStream* str, sRing* ring);

// formatted print string into ring buffer, with argument list
u32 RingPrintArg(sRing* ring, const char* fmt, va_list args);

// formatted print string into ring buffer, with variadic arguments
NOINLINE u32 RingPrint(sRing* ring, const char* fmt, ...);

#endif // USE_STREAM

#ifdef __cplusplus
}
#endif

#endif // _LIB_RING_H

#endif // USE_RING	// use Ring buffer (lib_ring.c, lib_ring.h)
