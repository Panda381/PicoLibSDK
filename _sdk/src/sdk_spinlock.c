
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

#include "../inc/sdk_timer.h"
#include "../inc/sdk_cpu.h"
#include "../inc/sdk_spinlock.h"

// claimed spinlocks
u32 SpinClaimed = 0;		// claimed spinlocks
u8 SpinStriped = PICO_SPINLOCK_ID_STRIPED_FIRST; // next striped spinlock (claimed with round robin)

// lock spinlock with timeout (returns False on timeout)
Bool SpinLockTimeout_hw(spin_lock_t* hw, u32 us)
{
	// start time
	u32 t = Time();

	// try to lock
	while (*hw == 0)
	{
		if ((u32)(Time() - t) >= us) return False;
	}
	dmb(); // data memory barrier

	// locked OK
	return True;
}

// reset all spinlocks
void SpinResetAll()
{
	SpinClaimed = 0;
	int lock;
	dmb(); // data memory barrier
	for (lock = 0; lock < SPINLOCK_NUM; lock++) SpinUnlock(lock);
}

// next striped spinlock index
u8 SpinNextStriped()
{
	u8 old = SpinStriped;
	u8 next = old + 1;
	if (next > PICO_SPINLOCK_ID_STRIPED_LAST) next = PICO_SPINLOCK_ID_STRIPED_FIRST;
	SpinStriped = next;
	return old;
}

// claim free spinlock (in range PICO_SPINLOCK_ID_CLAIM_FREE_FIRST to PICO_SPINLOCK_ID_CLAIM_FREE_LAST; -1 = no free spinlock)
s8 SpinClaimFree()
{
	int inx;
	u32 mask = BIT(PICO_SPINLOCK_ID_CLAIM_FREE_FIRST);
	for (inx = PICO_SPINLOCK_ID_CLAIM_FREE_FIRST; inx <= PICO_SPINLOCK_ID_CLAIM_FREE_LAST; inx++)
	{
		// check if spinlock is already claimed
		if ((SpinClaimed & mask) == 0)
		{
			// claim this spinlock
			SpinClaimed |= mask;
			return inx;
		}

		// shift to next spinlock
		mask <<= 1;
	}

	// no free spinlock
	return -1;
}

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// === semaphore

// Initialise a semaphore structure
void sem_init(semaphore_t *sem, s16 initial_permits, s16 max_permits)
{
	lock_init(&sem->core, next_striped_spin_lock_num());
	sem->permits = initial_permits;
	sem->max_permits = max_permits;
	__mem_fence_release();
}

// Release a permit on a semaphore
bool __time_critical_func(sem_release)(semaphore_t *sem)
{
	u32 save = spin_lock_blocking(sem->core.spin_lock);
	s32 count = sem->permits;
	if (count < sem->max_permits)
	{
		sem->permits = (s16)(count + 1);
		lock_internal_spin_unlock_with_notify(&sem->core, save);
		return true;
	}
	else
	{
		spin_unlock(sem->core.spin_lock, save);
		return false;
	}
}

// Reset semaphore to a specific number of available permits
void __time_critical_func(sem_reset)(semaphore_t *sem, s16 permits)
{
	u32 save = spin_lock_blocking(sem->core.spin_lock);
	if (permits > sem->permits)
	{
		sem->permits = permits;
		lock_internal_spin_unlock_with_notify(&sem->core, save);
	}
	else
	{
		sem->permits = permits;
		spin_unlock(sem->core.spin_lock, save);
	}
}

// Acquire a permit from the semaphore
void __time_critical_func(sem_acquire_blocking)(semaphore_t *sem)
{
	do {
		u32 save = spin_lock_blocking(sem->core.spin_lock);
		if (sem->permits > 0)
		{
			sem->permits--;
			spin_unlock(sem->core.spin_lock, save);
			break;
		}
		lock_internal_spin_unlock_with_wait(&sem->core, save);
	} while (true);
}

// Wait to acquire a permit from a semaphore until a specific time
bool __time_critical_func(sem_acquire_block_until)(semaphore_t *sem, absolute_time_t until)
{
	do {
		u32 save = spin_lock_blocking(sem->core.spin_lock);
		if (sem->permits > 0)
		{
			sem->permits--;
			spin_unlock(sem->core.spin_lock, save);
			return true;
		}
		if (lock_internal_spin_unlock_with_best_effort_wait_or_timeout(&sem->core, save, until))
			return false;
	} while (true);
}

// Attempt to acquire a permit from a semaphore without blocking
bool __time_critical_func(sem_try_acquire)(semaphore_t *sem)
{
	u32 save = spin_lock_blocking(sem->core.spin_lock);
	if (sem->permits > 0)
	{
		sem->permits--;
		spin_unlock(sem->core.spin_lock, save);
		return true;
	}
	spin_unlock(sem->core.spin_lock, save);
	return false;
}

#endif // USE_ORIGSDK

#endif // USE_SPINLOCK	// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)
