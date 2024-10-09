
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

// 32 spinlocks with index 0..31

#if USE_SPINLOCK	// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)

#ifndef _SDK_SPINLOCK_H
#define _SDK_SPINLOCK_H

#include "../sdk_addressmap.h"		// Register address offsets
#include "sdk_cpu.h"
#include "sdk_timer.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SPINLOCK_NUM	32	// number of spinlocks
//   (SIO does not support aliases for atomic access!)
#define SPINLOCK_STATE	((volatile u32*)(SIO_BASE+0x5c)) // spinlock states
#define SPINLOCK(lock)	((volatile u32*)(SIO_BASE+(lock)*4+0x100)) // spinlock 0..31

// claimed spinlocks
extern u32 SpinClaimed;		// claimed spinlocks
extern u8 SpinStriped;		// next striped spinlock (claimed with round robin)

typedef volatile u32 spin_lock_t;

// get spinlock hardware from spinlock index
INLINE spin_lock_t* SpinGetHw(int lock) { return (spin_lock_t*)SPINLOCK(lock); }

// get spinlock index from spinlock hardware
INLINE u8 SpinGetInx(const spin_lock_t* hw) { return (u8)(hw - SpinGetHw(0)); }

// === claim spinlock
// Functions are not atomic safe! (not recommended to be used in both cores or in IRQ at the same time)

// next striped spinlock index (in range PICO_SPINLOCK_ID_STRIPED_FIRST .. PICO_SPINLOCK_ID_STRIPED_LAST)
u8 SpinNextStriped();

// claim spinlock (mark it as used)
INLINE void SpinClaim(int lock) { SpinClaimed |= BIT(lock); }

// claim spinlocks with mask (mark them as used)
INLINE void SpinClaimMask(u32 mask) { SpinClaimed |= mask; }

// unclaim spinlock (mark it as not used)
INLINE void SpinUnclaim(int lock) { SpinClaimed &= ~BIT(lock); }

// unclaim spinlocks with mask (mark them as not used)
INLINE void SpinUnclaimMask(u32 mask) { SpinClaimed &= ~mask; }

// check if spinlock is claimed
INLINE Bool SpinIsClaimed(int lock) { return (SpinClaimed & BIT(lock)) != 0; }

// claim free spinlock (in range PICO_SPINLOCK_ID_CLAIM_FREE_FIRST to PICO_SPINLOCK_ID_CLAIM_FREE_LAST; -1 = no free spinlock)
s8 SpinClaimFree();

// === lock spinlock

// try to lock spinlock - returns False if not locked (lock = 0..31)
INLINE Bool SpinTryLock(int lock)
{
	// reading from spinlock will claim the lock
	Bool res = *SPINLOCK(lock) != 0;
	dmb(); // data memory barrier
	return res;
}

INLINE Bool SpinTryLock_hw(spin_lock_t* hw)
{
	// reading from spinlock will claim the lock
	Bool res = *hw != 0;
	dmb(); // data memory barrier
	return res;
}

// lock spinlock and wait, if already locked (lock = 0..31)
INLINE void SpinLock(int lock)
{
	while (*SPINLOCK(lock) == 0) {}
	dmb(); // data memory barrier
}

INLINE void SpinLock_hw(spin_lock_t* hw)
{
	while (*hw == 0) {}
	dmb(); // data memory barrier
}

// lock spinlock with timeout (returns False on timeout)
Bool SpinLockTimeout_hw(spin_lock_t* hw, u32 us);
INLINE Bool SpinLockTimeout(int lock, u32 us) { return SpinLockTimeout_hw(SpinGetHw(lock), us); }

// unlock spinlock (default state; lock = 0..31)
INLINE void SpinUnlock(int lock)
{
	// writing any value releases the lock
	dmb(); // data memory barrier
	*SPINLOCK(lock) = 0;
}

INLINE void SpinUnlock_hw(spin_lock_t* hw)
{
	// writing any value releases the lock
	dmb(); // data memory barrier
	*hw = 0;
}

// check if spinlock is locked (lock = 0..31)
INLINE Bool SpinIsLocked(int lock) { return (*SPINLOCK_STATE & BIT(lock)) != 0; }

// initialize spinlock (all spinlocks are at unlocked state after reset; lock = 0..31)
INLINE void SpinInit(int lock) { SpinUnlock(lock); }
INLINE void SpinInit_hw(spin_lock_t* hw) { SpinUnlock_hw(hw); }

// reset all spinlocks
void SpinResetAll();

// get state of all spinlocks
INLINE u32 SpinGetAll() { return *SPINLOCK_STATE; }

// lock spinlock and IRQ (to share resources with interrupt; lock < 0 if not use spinlock)
#define SPINLOCK_LOCK(lock) IRQ_LOCK; if ((lock) >= 0) SpinLock(lock)

// unlock spinlock and IRQ
#define SPINLOCK_UNLOCK(lock) if ((lock) >= 0) SpinUnlock(lock); IRQ_UNLOCK

// ----------------------------------------------------------------------------
//                          Original-SDK interface
// ----------------------------------------------------------------------------

#if USE_ORIGSDK		// include interface of original SDK

// Get HW Spinlock instance from number
INLINE spin_lock_t* spin_lock_instance(uint lock_num) { return SpinGetHw(lock_num); }

// Get HW Spinlock number from instance
INLINE uint spin_lock_get_num(spin_lock_t *lock) { return SpinGetInx(lock); }

// Acquire a spin lock without disabling interrupts (hence unsafe)
INLINE void spin_lock_unsafe_blocking(spin_lock_t *lock) { SpinLock_hw(lock); }

// Release a spin lock without re-enabling interrupts
INLINE void spin_unlock_unsafe(spin_lock_t *lock) { SpinUnlock_hw(lock); }

// Acquire a spin lock safely
INLINE u32 spin_lock_blocking(spin_lock_t *lock)
{
	u32 save = LockIRQ();
	SpinLock_hw(lock);
	return save;
}

// Check to see if a spinlock is currently acquired elsewhere.
INLINE bool is_spin_locked(spin_lock_t *lock) { return SpinIsLocked(SpinGetInx(lock)); }

// Release a spin lock safely
INLINE void spin_unlock(spin_lock_t *lock, u32 saved_irq) { SpinUnlock_hw(lock); UnlockIRQ(saved_irq); }

// Initialise a spin lock
INLINE spin_lock_t *spin_lock_init(uint lock_num)
{
	spin_lock_t* lock = SpinGetHw(lock_num);
	SpinUnlock_hw(lock);
	return lock;
}

// Release all spin locks
INLINE void spin_locks_reset(void) { SpinResetAll(); }

// Return a spin lock number from the _striped_ range
INLINE uint next_striped_spin_lock_num(void) { return SpinNextStriped(); }

// Mark a spin lock as used
INLINE void spin_lock_claim(uint lock_num) { SpinClaim(lock_num); }

// Mark multiple spin locks as used
INLINE void spin_lock_claim_mask(u32 lock_num_mask) { SpinClaimMask(lock_num_mask); }

// Mark a spin lock as no longer used
INLINE void spin_lock_unclaim(uint lock_num) { SpinUnclaim(lock_num); }

// Claim a free spin lock
INLINE int spin_lock_claim_unused(bool required) { return SpinClaimFree(); }

// Determine if a spin lock is claimed
INLINE bool spin_lock_is_claimed(uint lock_num) { return SpinIsClaimed(lock_num); }

#define remove_volatile_cast(t, x) ({__mem_fence_acquire(); (t)(x); })

// implementation for locking primitives protected by a spin lock. The spin lock is only used to protect
// access to the remaining lock state (in primitives using lock_core); it is never left locked outside
// of the function implementations
struct lock_core {
	spin_lock_t*	spin_lock;	// spin lock protecting this lock's state
};

typedef struct lock_core lock_core_t;

// Initialise a lock structure
INLINE void lock_init(lock_core_t *core, uint lock_num) { core->spin_lock = SpinGetHw(lock_num); }

// type to use to store the 'owner' of a lock.
#define lock_owner_id_t s8

// marker value to use for a lock_owner_id_t which does not refer to any valid owner
#define LOCK_INVALID_OWNER_ID ((lock_owner_id_t)-1)

// return the owner id for the caller
#define lock_get_caller_owner_id() ((lock_owner_id_t)get_core_num())
#define lock_is_owner_id_valid(id) ((id)>=0)

// Atomically unlock the lock's spin lock, and wait for a notification.
#define lock_internal_spin_unlock_with_wait(lock, save) spin_unlock((lock)->spin_lock, save), __wfe()

// Atomically unlock the lock's spin lock, and send a notification
#define lock_internal_spin_unlock_with_notify(lock, save) spin_unlock((lock)->spin_lock, save), __sev()

// Atomically unlock the lock's spin lock, and wait for a notification or a timeout
#define lock_internal_spin_unlock_with_best_effort_wait_or_timeout(lock, save, until) ( \
	spin_unlock((lock)->spin_lock, save), 	\
	best_effort_wfe_or_timeout(until) )

// yield to other processing until some time before the requested time
#define sync_internal_yield_until_before(until) ((void)0)

// === semaphore

typedef struct __attribute__((packed)) __attribute((aligned)) semaphore
{
	struct lock_core	core;
	s16			permits;
	s16			max_permits;
} semaphore_t;

// Initialise a semaphore structure
void sem_init(semaphore_t *sem, s16 initial_permits, s16 max_permits);

// Return number of available permits on the semaphore
INLINE int sem_available(semaphore_t *sem) { return *(volatile typeof(sem->permits)*) &sem->permits; }

// Release a permit on a semaphore
bool __time_critical_func(sem_release)(semaphore_t *sem);

// Reset semaphore to a specific number of available permits
void __time_critical_func(sem_reset)(semaphore_t *sem, s16 permits);

// Acquire a permit from the semaphore
void __time_critical_func(sem_acquire_blocking)(semaphore_t *sem);

// Wait to acquire a permit from a semaphore until a specific time
bool __time_critical_func(sem_acquire_block_until)(semaphore_t *sem, absolute_time_t until);

// Acquire a permit from a semaphore, with timeout
INLINE bool sem_acquire_timeout_ms(semaphore_t *sem, u32 timeout_ms)
	{ return sem_acquire_block_until(sem, make_timeout_time_ms(timeout_ms)); }

// Acquire a permit from a semaphore, with timeout
INLINE bool sem_acquire_timeout_us(semaphore_t *sem, u32 timeout_us)
	{ return sem_acquire_block_until(sem, make_timeout_time_us(timeout_us)); }

// Attempt to acquire a permit from a semaphore without blocking
bool __time_critical_func(sem_try_acquire)(semaphore_t *sem);

#endif // USE_ORIGSDK

#ifdef __cplusplus
}
#endif

#endif // _SDK_SPINLOCK_H

#endif // USE_SPINLOCK	// use Spinlock (sdk_spinlock.c, sdk_spinlock.h)
