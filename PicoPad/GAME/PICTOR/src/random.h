
// ****************************************************************************
//
//                            Random number generator
//
// ****************************************************************************
// - used for defined reproduction of enemies in the scene

// Seed of random number generator to generate enemies
extern u32 EnemyRandSeed;

// generate 16-bit unsigned integer random number
u16 EnemyRand();

// generate 16-bit unsigned integer random number in range 0 to MAX (including)
u16 EnemyRandMax(u16 max);

// generate 16-bit unsigned integer random number in range MIN to MAX (including)
// If MIN > MAX, then number is generated out of interval.
u16 EnemyRandMinMax(u16 min, u16 max);
