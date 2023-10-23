
// ****************************************************************************
//
//                                Fuel
//
// ****************************************************************************

#ifndef _FUEL_H
#define _FUEL_H

#define FUEL_MAX	256	// max. fuel (without fraction)

extern int	Fuel;		// fuel, with FRAC fraction

extern const u8 FuelSnd[21354];

// display fuel
void DispFuel();

// subtract fuel
void SubFuel(int sub);

#endif // _FUEL_H
