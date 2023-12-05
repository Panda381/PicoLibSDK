
// ****************************************************************************
//
//                                   FIFO
//                             Inter-core FIFO
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

#if USE_FIFO	// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)

#include "../inc/sdk_fifo.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_timer.h"

// write message to other core (wait if not ready)
void FifoWrite(u32 data)
{
	// wait TX FIFO to ready
	while (!FifoWriteReady()) {}

	// send message
	fifo_hw->write = data;

	// send event to the other code
	sev();
}

// write message to other core with time-out in [us] (wait if not ready, max. 71 minutes, returns False on time-out)
Bool FifoWriteTime(u32 data, u32 us)
{
	// prepare current time
	u32 start = Time();

	// wait TX FIFO to ready
	while (!FifoWriteReady())
	{
		// check time-out (returns False on time-out)
		if ((u32)(Time() - start) >= us) return False;
	}

	// send message
	fifo_hw->write = data;

	// send event to the other code
	sev();
	return True;
}

// read message from other core (wait if not ready)
u32 FifoRead()
{
	// wait RX FIFO to ready
	while (!FifoReadReady())
	{
		// wait for event
		wfe();
	}

	// receive message
	return fifo_hw->read;
}

// read message from other core with time-out in [us] (wait if not ready, max. 71 minutes, returns False on time-out)
Bool FifoReadTime(u32* data, u32 us)
{
	// prepare current time
	u32 start = Time();

	// wait RX FIFO to ready
	while (!FifoReadReady())
	{
		// wait for event
		wfe();

		// check time-out (returns False on time-out)
		if ((u32)(Time() - start) >= us) return False;
	}

	// receive message
	*data = fifo_hw->read;
	return True;
}

// FIFO hanshake - send code and wait for same response
void FifoHandshake(u32 code)
{
	// save and lock interrupts
	IRQ_LOCK;

	// send code
	FifoWrite(code);

	// wait for same response
	while (FifoRead() != code) {}

	// unlock interrupts
	IRQ_UNLOCK;
}

#endif // USE_FIFO	// use Inter-core FIFO (sdk_fifo.c, sdk_fifo.h)
