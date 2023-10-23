
// ****************************************************************************
//
//                               Ground
//
// ****************************************************************************

#include "include.h"

short GrassPhase = 0;		// grass phase, with FRAC fraction

/*
height of horizon: H (= height of viewer above ground)
distance of viewer before projection plane: D
distance of object from projection plane: Z
height of projection above ground: Y
lowering horizon: L

Y/Z = (H+L)/(D+Z)
Y = (H+L)/(D+Z)*Z
y = GRASS_Y_MAX - Y = GRASS_Y_MAX - (GRASS_H+HORIZ_LOW)/(DIST_VIEWER+z)*z
*/

// perspective projection table (conversion table from object distance to Y coordinate on the screen)
short PerspTab[DIST_MAX+1-DIST_MIN];

// calculate perspective projection table
void CalcPerspTab()
{
	int i, z, y;
	for (i = 0; i < count_of(PerspTab); i++)
	{
		z = i + DIST_MIN;
		y = GRASS_Y_MAX - ((GRASS_H+HORIZ_LOW)*z + (DIST_VIEWER+z)/2)/(DIST_VIEWER+z);
		if (y < GRASS_Y_MIN) y = GRASS_Y_MIN;
		if (y > GRASS_Y_MAX) y = GRASS_Y_MAX;
		PerspTab[i] = (short)y;
	}
}

const u8* const GrassImgList[4] = { Grass1Img, Grass2Img, Grass3Img, Grass4Img };
const u16* const GrassImgPalList[4] = { Grass1Img_Pal, Grass2Img_Pal, Grass3Img_Pal, Grass4Img_Pal };
const COLTYPE GrassColList[4] = { COL_LTGREEN, COL_LTYELLOW, COL_LTRED, COL_WHITE };

// get Y coordinate from perspective table
//  z ... distance with FRAC fraction
int GetPerspTab(int z)
{
	// limit Z coordinate
	z -= DIST_MIN<<FRAC;
	if (z < 0) return HEIGHT - (z>>3);
	if (z >= ((count_of(PerspTab)-1)<<FRAC)) z = ((count_of(PerspTab)-1)<<FRAC);

	// index in the table
	int z1 = z >> FRAC;
	int z2 = z1+1;
	if (z2 > (count_of(PerspTab)-1)) z2 = (count_of(PerspTab)-1);

	// interpolate
	int y1 = PerspTab[z1];
	int y2 = PerspTab[z2];
	int d = z & ((1 << FRAC)-1);
	return y1 + (y2 - y1)*d/(1 << FRAC);
}

// display ground
void DispGround()
{
	u8 n = Level & 3;

	// display grass
	DrawImgRle(GrassImgList[n], GrassImgPalList[n], 0, HORIZON, GRASS_IMG_W, GRASS_IMG_H);

	// display lines
	int d, y;
	COLTYPE c = GrassColList[n];
	for (d = (DIST_MIN<<FRAC)+GrassPhase; d <= (DIST_MAX<<FRAC); d += (GROUND_DY<<FRAC))
	{
		y = GetPerspTab(d);
		DrawRect(0, y, WIDTH, 1, c);
	}
}

