
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

#if USE_STREAM	// use Data stream (lib_stream.c, lib_stream.h)

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

#endif // USE_STREAM

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

/*
// queue structure
typedef struct {
	lock_core_t	core;		// core lock
	u8*		data;		// pointer to data
	u16		wptr;		// write offset
	u16		rptr;		// read offset
	u16		element_size;	// element size
	u16		element_count;	// count of elements
	u16		max_level;	// max. count of elements (0 = unlimited)
} queue_t;
*/

// Initialise a queue with a specific spinlock for concurrency protection
void queue_init_with_spinlock(queue_t *q, uint element_size, uint element_count, uint spinlock_num)
{
	lock_init(&q->core, spinlock_num);
	q->data = (u8*)calloc(element_count + 1, element_size);
	if (q->data == NULL) panic("queue_init_with_spinlock calloc!");
	q->element_count = (u16)element_count;
	q->element_size = (u16)element_size;
	q->wptr = 0;
	q->rptr = 0;
}

// get element pointer
static INLINE void *element_ptr(queue_t *q, uint index)
	{ return q->data + index * q->element_size; }

// increment element index
static INLINE u16 inc_index(queue_t *q, u16 index)
{
	index++;
	if (index > q->element_count) index = 0;
	u16 level = queue_get_level_unsafe(q);
	if (level > q->max_level) q->max_level = level;
	return index;
}

// add element
static bool queue_add_internal(queue_t *q, const void *data, bool block)
{
	do {
		// lock core spinlock
		u32 save = spin_lock_blocking(q->core.spin_lock);

		// if not full
		if (queue_get_level_unsafe(q) != q->element_count)
		{
			// write element data into write pointer
			memcpy(element_ptr(q, q->wptr), data, q->element_size);

			// increment write index
			q->wptr = inc_index(q, q->wptr);

			// unlock and send notification
			lock_internal_spin_unlock_with_notify(&q->core, save);
			return true;
		}

		// queue is full, wait or end
		if (block)
			lock_internal_spin_unlock_with_wait(&q->core, save);
		else
		{
			spin_unlock(q->core.spin_lock, save);
			return false;
		}
	} while (true);
}

// remove element
static bool queue_remove_internal(queue_t *q, void *data, bool block)
{
	do {
		// lock core spinlock
		u32 save = spin_lock_blocking(q->core.spin_lock);

		// if not empty
		if (queue_get_level_unsafe(q) != 0)
		{
			// copy element to destination buffer
			memcpy(data, element_ptr(q, q->rptr), q->element_size);

			// increment read index
			q->rptr = inc_index(q, q->rptr);

			// unlock and send notification
			lock_internal_spin_unlock_with_notify(&q->core, save);
			return true;
		}

		// queue is empty, wait or end
		if (block)
			lock_internal_spin_unlock_with_wait(&q->core, save);
		else
		{
			spin_unlock(q->core.spin_lock, save);
			return false;
		}
	} while (true);
}

// peek element
static bool queue_peek_internal(queue_t *q, void *data, bool block)
{
	do {
		// lock core spinlock
		u32 save = spin_lock_blocking(q->core.spin_lock);

		// if not empty
		if (queue_get_level_unsafe(q) != 0)
		{
			// copy element to destination buffer
			memcpy(data, element_ptr(q, q->rptr), q->element_size);

			// unlock and send notification
			lock_internal_spin_unlock_with_notify(&q->core, save);
			return true;
		}

		// queue is empty, wait or end
		if (block)
			lock_internal_spin_unlock_with_wait(&q->core, save);
		else
		{
			spin_unlock(q->core.spin_lock, save);
			return false;
		}
	} while (true);
}

// Non-blocking add value queue if not full
bool queue_try_add(queue_t *q, const void *data)
	{ return queue_add_internal(q, data, false); }

// Non-blocking removal of entry from the queue if non empty
bool queue_try_remove(queue_t *q, void *data)
	{ return queue_remove_internal(q, data, false); }

// Non-blocking peek at the next item to be removed from the queue
bool queue_try_peek(queue_t *q, void *data)
	{ return queue_peek_internal(q, data, false); }

// Blocking add of value to queue
void queue_add_blocking(queue_t *q, const void *data)
	{ queue_add_internal(q, data, true); }

// Blocking remove entry from queue
void queue_remove_blocking(queue_t *q, void *data)
	{ queue_remove_internal(q, data, true); }

// Blocking peek at next value to be removed from queue
void queue_peek_blocking(queue_t *q, void *data)
	{ queue_peek_internal(q, data, true); }

#endif // USE_ORIGSDK

#endif // USE_RING	// use Ring buffer (lib_ring.c, lib_ring.h)
