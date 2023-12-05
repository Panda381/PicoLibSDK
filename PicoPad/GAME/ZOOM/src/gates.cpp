
// ****************************************************************************
//
//                               Gates
//
// ****************************************************************************

#include "include.h"

sGate	Gates[GATES];	// gates
int	GateNext;	// distance counter to generate next gate

// reset gates on start of new scene
void ResetGates()
{
	int i;
	for (i = 0; i < GATES; i++) Gates[i].x = GATEFREE;
	GateNext = 0;
}

// display gates
void DispGate()
{
	// sector 3 has no gates
	if (SubLevel == 3) return;

	// display gates
	int i, y, n;
	sGate* g = Gates;
	for (i = GATES; i > 0; i--)
	{
		// is this gate used?
		if (g->x != GATEFREE)
		{
			// get Y coordinate of the gate, prepare size of the gate (0..7)
			y = GetPerspTab(g->z);
			n = g->z/840;
			if (n < 0) n = 0;
			if (n > 7) n = 7;
			y -= GATE_IMG_H;
			y -= n; // small correction of distant gates so that they don't jump when zoomed in

			// display gate
			DrawBlit4Pal(GateImg, GateImg_Pal, n*GATE_IMG_W, 0, g->x - GATE_IMG_W/2 - GATE_DISTX/2 + n*4,
				y, GATE_IMG_W, GATE_IMG_H, GATE_IMG_TOTW, GATE_IMG_KEY);
			DrawBlit4Pal(GateImg, GateImg_Pal, n*GATE_IMG_W, 0, g->x - GATE_IMG_W/2 + GATE_DISTX/2 - n*4,
				y, GATE_IMG_W, GATE_IMG_H, GATE_IMG_TOTW, GATE_IMG_KEY);
		}
		g++;
	}
}

// gate service (dz = delta Z coordinate, with FRAC fraction)
void Gating(int dz)
{
	// sector 3 has no gates
	if (SubLevel == 3) return;

	// shift gates
	int i, y, n, dx;
	sGate* g = Gates;
	for (i = GATES; i > 0; i--)
	{
		// is this gate used?
		if (g->x != GATEFREE)
		{
			// move gate
			g->z -= dz;

			// delete gate
			if (g->z < -4<<FRAC)
				g->x = GATEFREE;

			// test whether the plane flew through the gate
			else
			{
				// bomb release
				if (!g->bomb && (g->z < 15<<FRAC))
				{
					g->bomb = True;

					// generate bomb
					if (Level >= 2) // sector 3.x and more - bombs in both directions
					{
						GenBomb(ENEMY_BOMBL_MOVE, (g->x - GATE_DISTX/2)<<FRAC, g->z);
						GenBomb(ENEMY_BOMBR_MOVE, (g->x + GATE_DISTX/2)<<FRAC, g->z);
					}
					else if (Level == 1) // sector 2.x has 1 bomb
					{
						if (g->x >= WIDTH/2) // bomb to left
							GenBomb(ENEMY_BOMBL_MOVE, (g->x - GATE_DISTX/2)<<FRAC, g->z);
						else // bomb to right
							GenBomb(ENEMY_BOMBR_MOVE, (g->x + GATE_DISTX/2)<<FRAC, g->z);
					}
				}

				// if not defused
				if (!g->def && (ShipInCrash == 0))
				{
					// check coordinate
					dx = g->x - (ShipX >> FRAC);
					if ((dx >= - GATE_PASS_DX) && (dx <= GATE_PASS_DX) && (g->z < (5<<FRAC)))
					{
						// gate defused
						g->def = True;

						// decrease goals
						DecGoals();

						// add score
						AddScore(500<<FRAC);

						// play sound
						PlaySoundChan(0, GateSnd, count_of(GateSnd), False, 1, 0.4f, SNDFORM_PCM, 0);
					}
				}

				// crash gate
				dx = g->x - (ShipX >> FRAC);
				if ((((dx < - GATE_PASS_DX) && (dx >= - GATE_CRASH_DX)) ||
					((dx > GATE_PASS_DX) && (dx <= GATE_CRASH_DX)))
						&& (g->z < (7<<FRAC)) && (g->z > (2<<FRAC)))
				{
					// crash ship
					ShipCrash();
				}
			}
		}
		g++;
	}

	// generate new gate
	GateNext -= dz; // distance counter
	if (GateNext > 0) return; // not yet generate
	g = Gates;
	for (i = GATES; i > 0; i--)
	{
		// is this gate free?
		if (g->x == GATEFREE)
		{
			g->z = GATE_INITZ<<FRAC; // new start Z position
			g->x = RandS16MinMax((GATE_IMG_W + GATE_DISTX)/2, WIDTH - (GATE_IMG_W + GATE_DISTX)/2);
			g->def = False; // not defused
			g->bomb = False; // bomb not released
			GateNext = GATE_DISTZ<<FRAC; // reset distance counter
			return;
		}
		g++;
	}
}

