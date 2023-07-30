
// ****************************************************************************
//
//                               Event Ring Buffer
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

#if USE_EVENT		// use Event Ring buffer (lib_event.c, lib_event.h)

#include "../../_sdk/inc/sdk_cpu.h"
#include "../../_sdk/inc/sdk_spinlock.h"	// spinlock
#include "../inc/lib_event.h"

// initialize event ring buffer (not needed with static event ring buffer, created with EVENTRING() macro)
//   ring = pointer to event ring buffer
//   buf = data buffer (of size 'size' entries)
//   size = buffer size in number of entries
//   spinlock = index of spinlock (-1 = not used)
void EventRingInit(sEventRing* ring, sEvent* buf, uint size, int spinlock)
{
	ring->buf = buf;
	ring->size = size;
	ring->write = 0;
	ring->read = 0;
	ring->spinlock = spinlock;
}

// clear event ring buffer
void EventRingClear(sEventRing* ring)
{
	ring->write = 0;
	ring->read = 0;
	dmb();		// data memory barrier and compiler barrier
}	

// check if available space for write 1 entry into event ring buffer
Bool EventRingWriteReady(const sEventRing* ring)
{
	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	uint i = ring->write;	// get write offset
	i++;			// increase write offset
	if (i >= ring->size) i = 0; // overflow
	dmb();		// data memory barrier and compiler barrier
	Bool res = (i != ring->read); // check if buffer is full

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);

	return res;
}

// write event into event ring buffer, without waiting (returns False if buffer is full)
Bool EventRingWrite(sEventRing* ring, const sEvent* event)
{
	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	uint i = ring->write;	// write offset
	uint i2 = i + 1;	// increase write offset
	if (i2 >= ring->size) i2 = 0; // overflow
	dmb();		// data memory barrier and compiler barrier
	if (i2 != ring->read)	// check if buffer is full
	{
		memcpy(&ring->buf[i], event, sizeof(sEvent)); // store data to the buffer
		dmb();		// data memory barrier and compiler barrier
		ring->write = i2; // new write offset
		dmb();		// data memory barrier and compiler barrier
		SPINLOCK_UNLOCK(ring->spinlock); // unlock
		return True;	// ok
	}

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);
	return False;		// buffer is full
}

// write event into event ring buffer, wait until ready
void EventRingWriteWait(sEventRing* ring, const sEvent* event)
{
	while (!EventRingWrite(ring, event)) {}
}

// check if event is ready to read from event ring buffer
Bool EventRingReadReady(const sEventRing* ring)
{
	dmb();		// data memory barrier and compiler barrier
	return ring->read != ring->write;
}

// read event from event ring buffer, without waiting (returns False if buffer is empty)
Bool EventRingRead(sEventRing* ring, sEvent* event)
{
	// lock ring buffer and IRQ
	SPINLOCK_LOCK(ring->spinlock);

	dmb();		// data memory barrier and compiler barrier
	uint i = ring->read;	// get read offset
	if (i == ring->write)
	{
		// unlock ring buffer and IRQ
		SPINLOCK_UNLOCK(ring->spinlock);
		return False; // not ready
	}

	memcpy(event, &ring->buf[i], sizeof(sEvent)); // read data from the buffer
	i++;			// increase read offset
	if (i >= ring->size) i = 0; // overflow
	dmb();			// data memory barrier and compiler barrier
	ring->read = i;		// set new read offset
	dmb();			// data memory barrier and compiler barrier

	// unlock ring buffer and IRQ
	SPINLOCK_UNLOCK(ring->spinlock);
	return True;
}

// read event from event ring buffer, wait until ready
void EventRingReadWait(sEventRing* ring, sEvent* event)
{
	while (!EventRingRead(ring, event)) {}
}

#endif // USE_EVENT		// use Event Ring buffer (lib_event.c, lib_event.h)
