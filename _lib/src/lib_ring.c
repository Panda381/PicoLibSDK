
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

#include "../../global.h"	// globals

#if USE_RING	// use Ring buffer (lib_ring.c, lib_ring.h)

#include "../../_sdk/inc/sdk_cpu.h"
#include "../inc/lib_ring.h"
#include "../inc/lib_text.h"
#include "../inc/lib_print.h"

// initialize ring buffer (not needed with static ring buffer, created with RING() macro)
//   ring = pointer to ring buffer
//   buf = data buffer (of size 'size' bytes)
//   size = buffer size in bytes
//   spinlock = index of spinlock (-1 = not used)
//   force_send = wake-up function to force handler to send data (NULL = none)
//   force_recv = wake-up function to force handler to receive data (NULL = none)
//   cookie = user data (uart index)
void RingInit(sRing* ring, u8* buf, u32 size, int spinlock, pRingForce force_send, pRingForce force_recv, u32 cookie)
{
	ring->buf = buf;
	ring->size = size;
	ring->write = 0;
	ring->read = 0;
	ring->spinlock = spinlock;
	ring->force_send = force_send;
	ring->force_recv = force_recv;
	ring->cookie = cookie;
	if (spinlock >= 0) SpinInit(spinlock);	
}

// update ring buffer send transmission (force handler to send data)
void RingWriteUpdate(sRing* ring)
{
	// wake-up sending handler
	if (ring->force_send != NULL) ring->force_send(ring);
}

// update ring buffer receive transmission (force handlers to receive data)
void RingReadUpdate(sRing* ring)
{
	// wake-up receive handler
	if (ring->force_recv != NULL) ring->force_recv(ring);
}

// clear ring buffer
// - Function can be called from inside the locked section.
void RingClear(sRing* ring)
{
	ring->write = 0;
	ring->read = 0;
	dmb();		// data memory barrier and compiler barrier
}	

// get free space for writing
// - Function can be called from inside the locked section.
u32 RingFree(sRing* ring)
{
	dmb();		// data memory barrier and compiler barrier
	u32 wr = ring->write;
	u32 rd = ring->read;
	if (rd <= wr) rd += ring->size;
	return rd - 1 - wr;
}

// get bytes available for reading
// - Function can be called from inside the locked section.
u32 RingNum(sRing* ring)
{
	dmb();		// data memory barrier and compiler barrier
	u32 wr = ring->write;
	u32 rd = ring->read;
	if (wr < rd) wr += ring->size;
	return wr - rd;
}

// check if ring buffer is ready to write required number of bytes
// - Function can be called from inside the locked section.
Bool RingWriteReady(sRing* ring, u32 len)
{
	return RingFree(ring) >= len;
}

// check if ring buffer is ready to read required number of bytes
// - Function can be called from inside the locked section.
Bool RingReadReady(sRing* ring, u32 len)
{
	return RingNum(ring) >= len;
}

// write whole data into ring buffer, without waiting (returns False if buffer has not free space)
// Data is written only if there is enough space in the ring buffer. Cannot write only part of the data.
Bool RingWrite(sRing* ring, const void* src, u32 len)
{
	u32 wr, rd, num;
	Bool ok;
	const u8* s;

// The first test is performed without locking the locks,
// to give the service enough time to perform the operations

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	if (rd <= wr) rd += ring->size;

	// check free space (start transfer if buffer is full)
	num = rd - 1 - wr;
	if (num < len)
	{
		RingWriteUpdate(ring);
		return False;
	}

	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	if (rd <= wr) rd += ring->size;

	// check free space
	num = rd - 1 - wr;
	ok = False;
	if (num >= len)
	{
		s = (const u8*)src;

		// write data into buffer
		for (; len > 0; len--)
		{
			ring->buf[wr++] = *s++;
			if (wr >= ring->size) wr -= ring->size;
		}
		dmb();		// data memory barrier and compiler barrier
		ring->write = wr;
		dmb();		// data memory barrier and compiler barrier
		ok = True;
	}

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);

	// wake-up sending handler
	RingWriteUpdate(ring);
	return ok;
}

// write part of data into ring buffer, without waiting (returns number of written bytes)
u32 RingWriteData(sRing* ring, const void* src, u32 len)
{
	u32 wr, rd, num, ok;
	const u8* s;

// The first test is performed without locking the locks,
// to give the service enough time to perform the operations

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	if (rd <= wr) rd += ring->size;

	// check free space (start transfer if buffer is full)
	num = rd - 1 - wr;
	if (num == 0)
	{
		RingWriteUpdate(ring);
		return 0;
	}

	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	if (rd <= wr) rd += ring->size;

	// check free space
	num = rd - 1 - wr;
	if (num > len) num = len;
	ok = num;
	if (num > 0)
	{
		s = (const u8*)src;

		// write data into buffer
		for (; num > 0; num--)
		{
			ring->buf[wr++] = *s++;
			if (wr >= ring->size) wr -= ring->size;
		}
		dmb();		// data memory barrier and compiler barrier
		ring->write = wr;
		dmb();		// data memory barrier and compiler barrier
	}

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);

	// wake-up sending handler
	RingWriteUpdate(ring);
	return ok;
}

// write 8-bit data into ring buffer, without waiting (returns False on buffer overflow)
Bool RingWrite8(sRing* ring, u8 data) { return RingWrite(ring, &data, 1); }

// write 16-bit data into ring buffer, without waiting (returns False on buffer overflow)
Bool RingWrite16(sRing* ring, u16 data) { return RingWrite(ring, &data, 2); }

// write 32-bit data into ring buffer, without waiting (returns False on buffer overflow)
Bool RingWrite32(sRing* ring, u32 data) { return RingWrite(ring, &data, 4); }

// write data into ring buffer, wait until ready
void RingWriteWait(sRing* ring, const void* src, u32 len)
{
	u32 n;
	const u8* s = (const u8*)src;
	while (len > 0)
	{
		n = RingWriteData(ring, s, len);
		len -= n;
		s += n;
	}
}

// write 8-bit data into ring buffer, wait until ready
void RingWrite8Wait(sRing* ring, u8 data) { while (!RingWrite8(ring, data)) {} }

// write 16-bit data into ring buffer, wait until ready
void RingWrite16Wait(sRing* ring, u16 data) { while (!RingWrite16(ring, data)) {} }

// write 32-bit data into ring buffer, wait until ready
void RingWrite32Wait(sRing* ring, u32 data) { while (!RingWrite32(ring, data)) {} }

// read whole data from ring buffer, without waiting (returns False if buffer has not enough data)
// Data is read only if there is enough data in the ring buffer. Cannot read only part of the data.
Bool RingRead(sRing* ring, void* dst, u32 len)
{
	u32 wr, rd, num, size;
	Bool ok;
	u8* d;

	// wake-up receive handler
	RingReadUpdate(ring);

// The first test is performed without locking the locks,
// to give the service enough time to perform the operations

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	size = ring->size;
	if (wr < rd) wr += size;

	// check data size
	num = wr - rd;
	if (num < len) return False;

	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	if (wr < rd) wr += size;

	// check data size
	num = wr - rd;
	ok = False;
	if (num >= len)
	{
		d = (u8*)dst;

		// read data from buffer
		for (; len > 0; len--)
		{
			*d++ = ring->buf[rd++];
			if (rd >= size) rd -= size;
		}
		dmb();		// data memory barrier and compiler barrier
		ring->read = rd;
		dmb();		// data memory barrier and compiler barrier
		ok = True;
	}

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);

	return ok;
}

// read part of data from ring buffer, without waiting (returns number of read bytes)
u32 RingReadData(sRing* ring, void* dst, u32 len)
{
	u32 wr, rd, num, ok, size;
	u8* d;

	// wake-up receive handler
	RingReadUpdate(ring);

// The first test is performed without locking the locks,
// to give the service enough time to perform the operations

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	size = ring->size;
	if (wr < rd) wr += size;

	// check data size
	num = wr - rd;
	if (num == 0) return 0;

	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	// get ring offsets
	dmb();		// data memory barrier and compiler barrier
	wr = ring->write;
	rd = ring->read;
	if (wr < rd) wr += size;

	// check data size
	num = wr - rd;
	if (num > len) num = len;
	ok = num;
	if (num > 0)
	{
		d = (u8*)dst;

		// read data from buffer
		for (; num > 0; num--)
		{
			*d++ = ring->buf[rd++];
			if (rd >= size) rd -= size;
		}
		dmb();		// data memory barrier and compiler barrier
		ring->read = rd;
		dmb();		// data memory barrier and compiler barrier
	}

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);

	return ok;
}

// read 1 character from ring buffer, without waiting (returns 0 if buffer has not enough data)
char RingReadChar(sRing* ring)
{
	char ch = 0;
	RingRead(ring, &ch, 1);
	return ch;
}

// read 8-bit data from ring buffer, without waiting (returns False if buffer has not enough data)
Bool RingRead8(sRing* ring, u8* dst) { return RingRead(ring, dst, 1); }

// read 16-bit data from ring buffer, without waiting (returns False if buffer has not enough data)
Bool RingRead16(sRing* ring, u16* dst) { return RingRead(ring, dst, 2); }

// read 32-bit data from ring buffer, without waiting (returns False if buffer has not enough data)
Bool RingRead32(sRing* ring, u32* dst) { return RingRead(ring, dst, 4); }

// read data from ring buffer, wait until ready
void RingReadWait(sRing* ring, void* dst, u32 len)
{
	u32 n;
	u8* d = (u8*)dst;
	while (len > 0)
	{
		n = RingReadData(ring, d, len);
		len -= n;
		d += n;
	}
}

// read 8-bit data from ring buffer and wait until ready
u8 RingRead8Wait(sRing* ring)
{
	u8 data;
	while (!RingRead8(ring, &data)) {}
	return data;
}

// read 16-bit data from ring buffer and wait until ready
u16 RingRead16Wait(sRing* ring)
{
	u16 data;
	while (!RingRead16(ring, &data)) {}
	return data;
}

// read 32-bit data from ring buffer and wait until ready
u32 RingRead32Wait(sRing* ring)
{
	u32 data;
	while (!RingRead32(ring, &data)) {}
	return data;
}

// callback - write data to ring buffer
static u32 StreamWriteRing(sStream* str, const void* buf, u32 num)
{
	// destination ring
	sRing* ring = (sRing*)str->cookie;

	// write data into ring buffer, wait until ready
	RingWriteWait(ring, buf, num);

	return num;
}

// initialize stream to write to ring buffer
void StreamWriteRingInit(sStream* str, sRing* ring)
{
	Stream0Init(str); // clear stream descriptor
	str->cookie = (u32)ring; // cookie = pointer to ring descriptor
	str->write = StreamWriteRing; // write function
}

// formatted print string into ring buffer, with argument list
u32 RingPrintArg(sRing* ring, const char* fmt, va_list args)
{
	// write and read stream
	sStream wstr, rstr;

	// initialize stream to read from
	StreamReadBufInit(&rstr, fmt, StrLen(fmt));

	// initialize stream to write to
	StreamWriteRingInit(&wstr, ring);
	
	// print string
	return StreamPrintArg(&wstr, &rstr, args);
}

// formatted print string into ring buffer, with variadic arguments
NOINLINE u32 RingPrint(sRing* ring, const char* fmt, ...)
{
	u32 n;
	va_list args;
	va_start(args, fmt);
	n = RingPrintArg(ring, fmt, args);
	va_end(args);
	return n;
}

#endif // USE_RING	// use Ring buffer (lib_ring.c, lib_ring.h)
