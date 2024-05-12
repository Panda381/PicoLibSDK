
// ****************************************************************************
//
//                            Random number generator
//
// ****************************************************************************
// - used for defined reproduction of enemies in the scene

#include "../include.h"

// Seed of random number generator to generate enemies
u32 EnemyRandSeed;

// generate 16-bit unsigned integer random number
u16 EnemyRand()
{
	u32 s = EnemyRandSeed*214013 + 2531011;
	EnemyRandSeed = s;
	return (u16)(s >> 16);
}

// generate 16-bit unsigned integer random number in range 0 to MAX (including)
u16 EnemyRandMax(u16 max)
{
	u16 res;
	u16 msk;

	// zero maximal value
	if (max == 0) return 0;

	// prepare mask
	msk = (u16)Mask(max);

	// generate random number
	do {
		res = EnemyRand() & msk;
	} while (res > max);

	return res;
}

// generate 16-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u16 EnemyRandMinMax(u16 min, u16 max)
{
	return EnemyRandMax(max - min) + min;
}
