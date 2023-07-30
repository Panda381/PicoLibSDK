
// ****************************************************************************
//
//                              Spinlock
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

#if USE_SPINLOCK	// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)

#include "../inc/sdk_spinlock.h"
#include "../inc/sdk_cpu.h"

// reset all spinlocks
void SpinResetAll()
{
	u8 lock;
	dmb(); // data memory barrier
	for (lock = 0; lock < SPINLOCK_NUM; lock++) SpinUnlock(lock);
}

#endif // USE_SPINLOCK	// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)
