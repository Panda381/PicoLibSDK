
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

#include "../../global.h"	// globals

#if USE_MINIRING	// use Mini-Ring buffer (lib_miniring.c, lib_miniring.h)

#include "../../_sdk/inc/sdk_cpu.h"
#include "../inc/lib_miniring.h"

// initialize mini-ring buffer (not needed with static mini-ring buffer, created with MINIRING() macro)
//   ring = pointer to mini-ring buffer
//   buf = data buffer (of size 'size' bytes)
//   size = buffer size in bytes
void MiniRingInit(sMiniRing* ring, u8* buf, uint size)
{
	ring->buf = buf;
	ring->size = size;
	ring->write = 0;
	ring->read = 0;
}

// clear mini-ring buffer
void MiniRingClear(sMiniRing* ring)
{
	ring->write = 0;
	ring->read = 0;
	dmb();		// data memory barrier and compiler barrier
}	

// check if available space for write 1 byte into mini-ring buffer
Bool MiniRingWriteReady(sMiniRing* ring)
{
	uint i = ring->write;	// get write offset
	i++;			// increase write offset
	if (i >= ring->size) i = 0; // overflow
	dmb();		// data memory barrier and compiler barrier
	return (i != ring->read); // check if buffer is full
}

// write byte into mini-ring buffer, without waiting (returns False if buffer is full)
Bool MiniRingWrite(sMiniRing* ring, u8 data)
{
	uint i = ring->write;	// write offset
	uint i2 = i + 1;	// increase write offset
	if (i2 >= ring->size) i2 = 0; // overflow
	dmb();		// data memory barrier and compiler barrier
	if (i2 != ring->read)	// check if buffer is full
	{
		ring->buf[i] = data; // store data to the buffer
		dmb();		// data memory barrier and compiler barrier
		ring->write = i2; // new write offset
		dmb();		// data memory barrier and compiler barrier
		return True;	// ok
	}
	return False;		// buffer is full
}

// write data from buffer into mini-ring buffer, without waiting (data is lost if buffer is full,
//  returns number of bytes successfully written to the buffer)
uint MiniRingBufWrite(sMiniRing* ring, const u8* data, uint len)
{
	uint n = 0;
	while (len > 0)
	{
		if (!MiniRingWrite(ring, *data++)) break; // break if buffer is full
		n++;
		len--;
	}
	return n;
}

// write byte into mini-ring buffer, wait until ready
void MiniRingWriteWait(sMiniRing* ring, u8 data)
{
	while (!MiniRingWriteReady(ring)) {}
	MiniRingWrite(ring, data);
}

// write data from buffer into mini-ring buffer, wait until ready
void MiniRingBufWriteWait(sMiniRing* ring, const u8* data, uint len)
{
	for (; len > 0; len--) MiniRingWriteWait(ring, *data++);
}

// check if byte is ready to read from mini-ring buffer
Bool MiniRingReadReady(sMiniRing* ring)
{
	dmb();		// data memory barrier and compiler barrier
	return ring->read != ring->write;
}

// read byte from mini-ring buffer, without waiting (returns 0 if buffer is empty)
u8 MiniRingRead(sMiniRing* ring)
{
	dmb();		// data memory barrier and compiler barrier
	uint i = ring->read;	// get read offset
	if (i == ring->write) return 0; // not ready

	u8 data = ring->buf[i];	// read byte from the buffer
	i++;			// increase read offset
	if (i >= ring->size) i = 0; // overflow
	dmb();			// data memory barrier and compiler barrier
	ring->read = i;		// set new read offset
	dmb();			// data memory barrier and compiler barrier
	return data;
}

// read data from mini-ring buffer, without waiting (returns number of bytes successfully read from the buffer)
uint MiniRingBufRead(sMiniRing* ring, u8* data, uint len)
{
	uint n = 0;
	while (len > 0)
	{
		if (!MiniRingReadReady(ring)) break; // break if data not ready
		*data++ = MiniRingRead(ring);	// read byte from ring buffer
		n++;
		len--;
	}
	return n;
}

// read byte from mini-ring buffer, wait until ready
u8 MiniRingReadWait(sMiniRing* ring)
{
	while (!MiniRingReadReady(ring)) {}
	return MiniRingRead(ring);
}

// read data from mini-ring buffer, wait until ready
void MiniRingBufReadWait(sMiniRing* ring, u8* data, uint len)
{
	for (; len > 0; len--) *data++ = MiniRingReadWait(ring);
}

#endif // USE_MINIRING	// use Mini-Ring buffer (lib_miniring.c, lib_miniring.h)
