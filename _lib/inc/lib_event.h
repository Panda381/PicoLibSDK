
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

// Data transfer is atomic if one side performs only writes and the other only reads.

#if USE_EVENT		// use Event Ring buffer (lib_event.c, lib_event.h)

#ifndef _LIB_EVENT_H
#define _LIB_EVENT_H

#ifdef __cplusplus
extern "C" {
#endif

// event structure (16 bytes)
typedef struct {
	u32	data[4];	// event data (aligned to u32)
} sEvent;

// event ring buffer
typedef struct {
	sEvent*		buf;		// pointer to data buffer
	uint		size;		// size of data buffer in number of entries
	volatile uint	write;		// write offset into buffer
	volatile uint	read;		// read offset from buffer
	int		spinlock; 	// index of spinlock (-1 = not used)
} sEventRing;

// create event ring buffer with static buffer (in *.c file)
//   name = name
//   size = size of data buffer in number of entries
//   spinlock = index of spinlock (-1 = not used)
// Declare in *.h file: extern sEventRing name;
#define EVENTRING(name,size,spinlock) sEvent name ## _buf[size]; sEventRing name = \
	{ name ## _buf, (size), 0, 0, (spinlock) }

// initialize event ring buffer (not needed with static event ring buffer, created with EVENTRING() macro)
//   ring = pointer to event ring buffer
//   buf = data buffer (of size 'size' entries)
//   size = buffer size in number of entries
//   spinlock = index of spinlock (-1 = not used)
void EventRingInit(sEventRing* ring, sEvent* buf, uint size, int spinlock);

// clear event ring buffer
void EventRingClear(sEventRing* ring);

// check if available space for write 1 entry into event ring buffer
Bool EventRingWriteReady(const sEventRing* ring);

// write event into event ring buffer, without waiting (returns False if buffer is full)
Bool EventRingWrite(sEventRing* ring, const sEvent* event);

// write event into event ring buffer, wait until ready
void EventRingWriteWait(sEventRing* ring, const sEvent* event);

// check if event is ready to read from event ring buffer
Bool EventRingReadReady(const sEventRing* ring);

// read event from event ring buffer, without waiting (returns False if buffer is empty)
Bool EventRingRead(sEventRing* ring, sEvent* event);

// read event from event ring buffer, wait until ready
void EventRingReadWait(sEventRing* ring, sEvent* event);

#ifdef __cplusplus
}
#endif

#endif // _LIB_EVENT_H

#endif // USE_EVENT	// use Event Ring buffer (lib_event.c, lib_event.h)
