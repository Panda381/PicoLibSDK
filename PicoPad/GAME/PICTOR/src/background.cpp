
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// background
int BackInx;		// index of background
sBackTemp Back;		// current background template
int BackPhase1, BackPhase2, BackPhase3; // background phases

// background templates
const sBackTemp BackTemp[BG_NUM] = {
	// name			pal1			img1		pal2			img2		dy			pal3			img3		
	{ "Meadow",		MeadowImg_Pal,		MeadowImg,	Meadow2Img_Pal,		Meadow2Img,	BG2_DY_MEADOW,		Meadow3Img_Pal,		Meadow3Img,	},
	{ "Jungle",		JungleImg_Pal,		JungleImg,	Jungle2Img_Pal,		Jungle2Img,	BG2_DY_JUNGLE,		Jungle3Img_Pal,		Meadow3Img,	},
	{ "Sandy Beach",	SandbeachImg_Pal,	SandbeachImg,	Sandbeach2Img_Pal,	Sandbeach2Img,	BG2_DY_SANDBEACH,	Sandbeach3Img_Pal,	Meadow3Img,	},
	{ "Underwater",		UnderwaterImg_Pal,	UnderwaterImg,	Underwater2Img_Pal,	Underwater2Img,	BG2_DY_UNDERWATER,	Underwater3Img_Pal,	Meadow3Img,	},
	{ "Ice Land",		IcelandImg_Pal,		IcelandImg,	Iceland2Img_Pal,	Iceland2Img,	BG2_DY_ICELAND,		Iceland3Img_Pal,	Meadow3Img,	},
	{ "Fiery Land",		FirelandImg_Pal,	FirelandImg,	Fireland2Img_Pal,	Fireland2Img,	BG2_DY_FIRELAND,	Fireland3Img_Pal,	Meadow3Img,	},
	{ "Haunted Hill",	HauntedImg_Pal,		HauntedImg,	Haunted2Img_Pal,	Haunted2Img,	BG2_DY_HAUNTED,		Haunted3Img_Pal,	Meadow3Img,	},
	{ "Spacecraft",		SpacecraftImg_Pal,	SpacecraftImg,	Spacecraft2Img_Pal,	Spacecraft2Img,	BG2_DY_SPACECRAFT,	Spacecraft3Img_Pal,	Spacecraft3Img,	},
	{ "Galaxy",		GalaxyImg_Pal,		GalaxyImg,	Galaxy2Img_Pal,		Galaxy2Img,	BG2_DY_GALAXY,		Galaxy3Img_Pal,		Meadow3Img,	},
	{ "Alien Planet",	PlanetImg_Pal,		PlanetImg,	Planet2Img_Pal,		Planet2Img,	BG2_DY_PLANET,		Planet3Img_Pal,		Meadow3Img,	},
	{ "Candy Land",		CandylandImg_Pal,	CandylandImg,	Candyland2Img_Pal,	Candyland2Img,	BG2_DY_CANDYLAND,	Candyland3Img_Pal,	Meadow3Img,	},
	{ "Surreal Land",	SurrealImg_Pal,		SurrealImg,	Surreal2Img_Pal,	Surreal2Img,	BG2_DY_SURREAL,		Surreal3Img_Pal,	Meadow3Img,	},
};

// initialize backgrounds on new level
void InitBack()
{
	// prepare background template
	BackInx = Level % BG_NUM;
	memcpy(&Back, &BackTemp[BackInx], sizeof(Back));

	// clear background phase
	BackPhase1 = 0;
	BackPhase2 = 0;
	BackPhase3 = 0;
}

// display backgrounds
void DispBack()
{
	// draw background 1
	if (BackPhase1 < BG_WIDTH - WIDTH)
	{
		DrawImgPalDbl(Back.img1, Back.pal1, 	// image data and palettes
			BackPhase1, 0,			// source X, Y
			0, BG_Y,			// destination X, Y
			WIDTH, BG_HEIGHT,		// width, height
			BG_WIDTH);			// source total width
	}
	else
	{
		DrawImgPalDbl(Back.img1, Back.pal1, 	// image data and palettes
			BackPhase1, 0,			// source X, Y
			0, BG_Y,			// destination X, Y
			BG_WIDTH - BackPhase1, BG_HEIGHT, // width, height
			BG_WIDTH);			// source total width

		DrawImgPalDbl(Back.img1, Back.pal1, 	// image data and palettes
			0, 0,				// source X, Y
			BG_WIDTH - BackPhase1, BG_Y,	// destination X, Y
			WIDTH - (BG_WIDTH - BackPhase1), BG_HEIGHT, // width, height
			BG_WIDTH);			// source total width
	}

	// draw background 2
	if (BackPhase2 < BG2_WIDTH - WIDTH)
	{
		DrawBlitPal(Back.img2, Back.pal2, 	// image data and palettes
			BackPhase2, 0,			// source X, Y
			0, BG2_Y+Back.dy,		// destination X, Y
			WIDTH, BG2_HEIGHT-Back.dy,	// width, height
			BG2_WIDTH, BG2_TRANST);		// source total width
	}
	else
	{
		DrawBlitPal(Back.img2, Back.pal2, 	// image data and palettes
			BackPhase2, 0,			// source X, Y
			0, BG2_Y+Back.dy,		// destination X, Y
			BG2_WIDTH - BackPhase2, BG2_HEIGHT-Back.dy, // width, height
			BG2_WIDTH, BG2_TRANST);		// source total width

		DrawBlitPal(Back.img2, Back.pal2,	// image data and palettes
			0, 0,				// source X, Y
			BG2_WIDTH - BackPhase2, BG2_Y+Back.dy,	// destination X, Y
			WIDTH - (BG2_WIDTH - BackPhase2), BG2_HEIGHT-Back.dy, // width, height
			BG2_WIDTH, BG2_TRANST);		// source total width
	}

	// draw background 3
	if (BackPhase3 < BG3_WIDTH - WIDTH)
	{
		DrawBlitPal(Back.img3, Back.pal3, 	// image data and palettes
			BackPhase3, 0,			// source X, Y
			0, BG3_Y,			// destination X, Y
			WIDTH, BG3_HEIGHT,		// width, height
			BG3_WIDTH, BG3_TRANST);		// source total width
	}
	else
	{
		DrawBlitPal(Back.img3, Back.pal3, 	// image data and palettes
			BackPhase3, 0,			// source X, Y
			0, BG3_Y,			// destination X, Y
			BG3_WIDTH - BackPhase3, BG3_HEIGHT, // width, height
			BG3_WIDTH, BG3_TRANST);		// source total width

		DrawBlitPal(Back.img3, Back.pal3, 	// image data and palettes
			0, 0,				// source X, Y
			BG3_WIDTH - BackPhase3, BG3_Y,	// destination X, Y
			WIDTH - (BG3_WIDTH - BackPhase3), BG3_HEIGHT, // width, height
			BG3_WIDTH, BG3_TRANST);		// source total width
	}
}

// shift backgrounds
void ShiftBack()
{
	// background layer 1
	BackPhase1 += BG_SPEED1;
	if (BackPhase1 >= BG_WIDTH) BackPhase1 -= BG_WIDTH;

	// background layer 2
	BackPhase2 += BG_SPEED2;
	if (BackPhase2 >= BG2_WIDTH) BackPhase2 -= BG2_WIDTH;

	// background layer 3
	BackPhase3 += BG_SPEED3;
	if (BackPhase3 >= BG3_WIDTH) BackPhase3 -= BG3_WIDTH;
}
