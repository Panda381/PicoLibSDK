
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

// prepare transformation matrix (for DrawImgMat function)
//  ws ... source image width
//  hs ... source image height
//  x0 ... reference point X on source image
//  y0 ... reference point Y on source image
//  wd ... destination image width (negative = flip image in X direction)
//  hd ... destination image height (negative = flip image in Y direction)
//  shearx ... shear image in X direction
//  sheary ... shear image in Y direction
//  r ... rotate image (angle in radians)
//  tx ... shift in X direction (ws = whole image width)
//  ty ... shift in Y direction (hs = whole image height)
//void Mat2D_PrepDrawImg(sMat2D* m, int ws, int hs, int x0, int y0, int wd, int hd,
//	float shearx, float sheary, float r, float tx, float ty);

// draw 16-bit image with 2D transformation matrix
//  src ... source image
//  ws ... source image width
//  hs ... source image height
//  x ... destination coordinate X
//  y ... destination coordinate Y
//  w ... destination width
//  h ... destination height
//  m ... transformation matrix (should be prepared using PrepDrawImg)
//  mode ... draw mode DRAWIMG_*
//  color ... key or border color (DRAWIMG_PERSP mode: horizon offset)
// Note to wrap and perspective mode: Width and height of source image must be power of 2!
//void DrawImgMat(const u16* src, int ws, int hs, int x, int y, int w, int h,
//	const sMat2D* m, u8 mode, u16 color);

// parameter
typedef struct {
	const char*	name;		// name
	float		val;		// current value
	float		min;		// minimal value
	float		max;		// maximal value
	float		step;		// step
} sParam;

#define PARN	10
enum {
	PAR_X0 = 0,
	PAR_Y0,
	PAR_WD,
	PAR_HD,
	PAR_SHEARX,
	PAR_SHEARY,
	PAR_ROT,
	PAR_TX,
	PAR_TY,
	PAR_MODE,
};

sParam Param[PARN] = {
	{ "x0", IMGW/2, IMGW/2-IMGW, IMGW/2+IMGW, 16 },
	{ "y0", IMGH/2, IMGH/2-IMGH, IMGH/2+IMGH, 16 },
	{ "wd", IMGW, -2*IMGW, 2*IMGW, 32 },
	{ "hd", IMGH, -2*IMGH, 2*IMGH, 32 },
	{ "shearx", 0, -1, 1, 0.125f },
	{ "sheary", 0, -1, 1, 0.125f },
	{ "rot", 0, -360, 360, 22.5f },
	{ "tx", 0, -IMGW, IMGW, 16 },
	{ "ty", 0, -IMGH, IMGH, 16 },
	{ "mode", 3, 0, 5, 1 },
};

const char* ModeTxt[] = { "Wrap", "NoBorder", "Clamp", "Color", "Transp", "Persp" };

// selected parameter
int ParSel = 0;

// transformation matrix
sMat2D Mat;

// drawing frame
#define FRAMEW	180
#define FRAMEH	160
#define FRAMEX	10
#define FRAMEY	10

// menu
#define MENUX	(FRAMEW+2*FRAMEX)

// print buffer
#define BUFN 50
char Buf[BUFN];

// draw menu
void DrawMenu()
{
	int i;
	sParam* p = Param;
	for (i = 0; i < PARN; i++)
	{
		MemPrint(Buf, BUFN, " %s=%g           ", p->name, p->val);
		if (i == PAR_MODE) MemPrint(Buf, BUFN, " %s=%s           ", p->name, ModeTxt[float2int(p->val+0.5f)]);

		if (i == ParSel)
			DrawTextBg(Buf, MENUX, i*FONTH, COL_BLACK, COL_WHITE);
		else
			DrawTextBg(Buf, MENUX, i*FONTH, COL_WHITE, COL_BLACK);
		p++;
	}

	DispUpdate();
}

// main function
int main()
{
	while (True)
	{
		// display help
		DrawText("A=select, C/B=change, C+B=quit", 0, 200, COL_WHITE);

		// draw menu
		DrawMenu();

		// prepare transformation matrix
		int mode = float2int(Param[PAR_MODE].val+0.5f);
		Bool persp = (mode == DRAWIMG_PERSP);
		Mat2D_PrepDrawImg(&Mat, IMGW, IMGH,
			float2int(Param[PAR_X0].val+0.5f), float2int(Param[PAR_Y0].val+0.5f),
			float2int(Param[PAR_WD].val+0.5f), float2int(Param[PAR_HD].val+0.5f),
			Param[PAR_SHEARX].val, Param[PAR_SHEARY].val, Param[PAR_ROT].val*PI/180,
			Param[PAR_TX].val, Param[PAR_TY].val);

		// draw image
		DrawImgMat(RaspberryImg, IMGW, IMGH, FRAMEX, FRAMEY, FRAMEW, FRAMEH,
			&Mat, mode, persp ? 30 : COL_DKGREEN);

		// draw frame
		DrawFrame(FRAMEX-1, FRAMEY-1, FRAMEW+2, FRAMEH+2, COL_RED);

		// display updagte
		DispUpdate();

		// keyboard
		switch (KeyGet())
		{
		// next entry of the menu
		case BTN_A:
			ParSel++;
			if (ParSel == PARN) ParSel = 0;
			break;

		// decrease menu value
		case BTN_C:
			if (KeyPressed(BTN_B)) ResetToBootLoader();
			{
				sParam* p = &Param[ParSel];
				p->val -= p->step;
				if (p->val <= p->min) p->val = p->min;
			}
			break;

		// increase menu value
		case BTN_B:
			if (KeyPressed(BTN_C)) ResetToBootLoader();
			{
				sParam* p = &Param[ParSel];
				p->val += p->step;
				if (p->val >= p->max) p->val = p->max;
			}
			break;
		}
	}
}
