
// ****************************************************************************
//
//                                Fuel
//
// ****************************************************************************

#include "include.h"

int	Fuel;		// fuel, with FRAC fraction

// display fuel
void DispFuel()
{
	int i, j;

	// display fuel
	i = (WIDTH - FUEL_MAX - 2)/2;
	j = Fuel >> FRAC; // fuel
	DrawRect(i, 34, FUEL_MAX + 2, 4, (SubLevel == 3) ? COL_BLUE : COL_BLACK);
	DrawRect(i + 1, 35, j, 2, (SubLevel == 3) ? COL_YELLOW : COL_GREEN);

	// low fuel
	if (j < 32)
	{
		if (j < 12) // red "LOW FUEL" blinking
			DrawTextBg((((Time() >> 18) & 1) == 0) ? "        " : "LOW FUEL",
				(WIDTH - 8*8)/2, 32, COL_RED, COL_BLACK); 
		else // yellow "LOW FUEL"
			DrawTextBg("LOW FUEL", (WIDTH - 8*8)/2, 32, COL_YELLOW, COL_BLACK); 
	}
}

// subtract fuel
void SubFuel(int sub)
{
	int f = Fuel;
	Fuel = f - sub;
	if (Fuel <= 0)
	{
		Fuel = 0;
		ShipCrash();
	}

	// play low fuel warning
	if (Fuel < (12<<FRAC))
	{
		if (f >= (12<<FRAC))
			PlaySoundChan(SNDCHAN_FUEL, FuelSnd, count_of(FuelSnd), True, 1, 1, SNDFORM_PCM, 0);
	}
	else
		StopSoundChan(SNDCHAN_FUEL);
}
