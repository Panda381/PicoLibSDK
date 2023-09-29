
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "../include.h"

#define PATH	"/PAINTER/"

#define MAXFILES 56	// max. files (14 rows, 4 columns)

#define LINEW	5	// width of thick line

const char PrivImg[] = "PAINTER.BMP";	// private image (do not list nor edit)

// temporary text buffer
#define TEMPN	50		// size of temporary text buffer
char Temp[TEMPN];		// temporary text buffer

// image file
typedef struct {
	char	name[12+1];	// image file name
	u8	namelen;	// lengt of file name
} sImgFile;

// list of image files
sImgFile FileList[MAXFILES];
int FileListNum = 0;
int FileListSel = 0;
Bool Modi; // file modified

// mouse cursor
COLTYPE CurBuf[CURSOR_W*CURSOR_H]; // buffer to save cursor background
int CurX = WIDTH/2; // cursor X position
int CurY = HEIGHT/2; // cursor Y position
Bool CurVisible; // cursor is visible
const COLTYPE* CurImg = CurArrowImg; // current cursor image
u8 MouseBtn = 0; // mouse buttons
u8 MousePress = 0; // mouse pressed buttons
u8 MouseRel = 0; // mouse released buttons

#define MOUSE_LBTN	B0	// mouse left button
#define MOUSE_RBTN	B1	// mouse right button
#define MOUSE_MBTN	B2	// mouse middle button

// color palette
#define COL_ROWS	6	// number of color rows
#define COL_COLS	36	// number of color columns
#define COL_DX		9	// color delta X
#define COL_DY		9	// color delta Y
COLTYPE PalBuf[PALETTE_IMG_W*PALETTE_IMG_H]; // buffer to save palette background
COLTYPE Color = COL_RED; // selected color
int ColRow = 0; // row of selected color
int ColCol = 0; // column of selected color

COLTYPE ColorAlt = COL_YELLOW; // alternate selected color
int ColRowAlt = 0; // row of alternate selected color
int ColColAlt = 9; // column of alternate selected color

// tools
#define TOOLS_ROWS	6	// number of tool rows
#define TOOLS_COLS	2	// number of tool columns
#define TOOLS_DX	30	// tool delta X
#define TOOLS_DY	30	// tool delta Y
COLTYPE ToolBuf[TOOLS_IMG_W*TOOLS_IMG_H]; // buffer to save tool background

enum {
	TOOL_PENCIL = 0,
	TOOL_BRUSH,
	TOOL_LINE,
	TOOL_LINE2,
	TOOL_RECT,
	TOOL_CIRCLE,
	TOOL_RECT2,
	TOOL_RING,
	TOOL_BOX,
	TOOL_ROUND,
	TOOL_BUCKET,
	TOOL_DROP,

	TOOL_NUM
};

int ToolSel = TOOL_BRUSH;	// selected tool
int ToolSelAlt = TOOL_PENCIL;	// alternate selected tool

// tool cursor
const COLTYPE* const CurImgList[TOOL_NUM] = {
	CurPencilImg,	// TOOL_PENCIL = 0,
	CurBrushImg,	// TOOL_BRUSH,
	CurLineImg,	// TOOL_LINE,
	CurLine2Img,	// TOOL_LINE2,
	CurRectImg,	// TOOL_RECT,
	CurCircleImg,	// TOOL_CIRCLE,
	CurRect2Img,	// TOOL_RECT2,
	CurRingImg,	// TOOL_RING,
	CurBoxImg,	// TOOL_BOX,
	CurRoundImg,	// TOOL_ROUND,
	CurBucketImg,	// TOOL_BUCKET,
	CurDropImg,	// TOOL_DROP,
};

// BMP file header, 16 bits per pixel
const sBmp BmpHeader = {
	// BMP file header (size 14 bytes)
	0x4D42,			// u16	bfType;		// 0x00: magic, 'B' 'M' = 0x4D42
	70 + WIDTH*HEIGHT*2 + 2, // u32	bfSize;		// 0x02: file size, aligned to DWORD = 70 + 320*240*2 + 2 = 153672 = 0x25848
	0,			// u16	bfReserved1;	// 0x06: = 0
	0,			// u16	bfReserved2;	// 0x08: = 0
	70,			// 0x0A: offset of data bits after file header = 70 (0x46)
	// BMP info header (size 40 bytes)
	56,			// u32	biSize;		// 0x0E: size of this info header = 56 (0x38)
	WIDTH,			// s32	biWidth;	// 0x12: width = 320 (0x140)
	-HEIGHT,		// s32	biHeight;	// 0x16: height, negate if flip row order = -240 (0xFFFFFF10)
	1,			// u16	biPlanes;	// 0x1A: planes = 1
	16,			// u16	biBitCount;	// 0x1C: number of bits per pixel = 16
	3,			// u32	biCompression;	// 0x1E: compression = 3 (BI_BITFIELDS)
	WIDTH*HEIGHT*2+2,	// u32	biSizeImage;	// 0x22: size of data of image + aligned file = 320*240*2 + 2 = 153602 (0x25802)
	2834,			// s32	biXPelsPerMeter; // 0x26: X pels per meter = 2834 (= 0xB12)
	2834,			// s32	biYPelsPerMeter; // 0x2A: Y pels per meter = 2834 (= 0xB12)
	0,			// u32	biClrUsed;	// 0x2E: number of user colors (0 = all)
	0,			// u32	biClrImportant;	// 0x32: number of important colors (0 = all)
	// BMP color bit mask (size 16 bytes)
	0xF800,			// u32	biRedMask;	// 0x36: red mask = 0x0000F800
	0x07E0,			// u32	biGreenMask;	// 0x3A: green mask = 0x000007E0
	0x001F,			// u32	biBlueMask;	// 0x3E: blue mask = 0x0000001F
	0,			// u32	biAlphaMask;	// 0x42: alpha mask = 0x00000000
							// 0x46
};

// set cursor ON
void CurOn()
{
	// already visible
	if (CurVisible) return;
	CurVisible = True;

	// storing content under the cursor
	DrawGetImg(CurBuf, CurX, CurY, CURSOR_W, CURSOR_H);

	// draw cursor (with selected color)
	DrawBlitSubst(CurImg, 0, 0, CurX, CurY, CURSOR_W, CURSOR_H,
		CURSOR_W, CURSOR_KEY, CURSOR_SUBS, Color);
}

// set cursor OFF
void CurOff()
{
	// already not visible
	if (!CurVisible) return;
	CurVisible = False;

	// restore content
	DrawImg(CurBuf, 0, 0, CurX, CurY, CURSOR_W, CURSOR_H, CURSOR_W);
}

// move cursor
void CurMove(s8 dx, s8 dy)
{
	// new coordinates
	int x = CurX + dx;
	int y = CurY + dy;

	// limit coordinates
	if (x < 0) x = 0;
	if (x > WIDTH-1) x = WIDTH-1;
	if (y < 0) y = 0;
	if (y > HEIGHT-1) y = HEIGHT-1;

	// cursor not changes
	if ((x == CurX) && (y == CurY)) return;

	// synchornize
	VgaWaitVSync();

	// set cursor OFF
	CurOff();

	// set new coordinates
	CurX = x;
	CurY = y;

	// set cursor ON
	CurOn();
}

// cursor mouse move
void CurMouse()
{
	u32 k;
	int dx = 0;
	int dy = 0;

	// mouse buttons
	MousePress = 0; // mouse pressed buttons
	MouseRel = 0; // mouse released buttons

	// get mouse (get multiple messages at once - they can go faster than drawing graphics)
	while ((k = UsbGetMouse()) != 0)
	{
		// get shift coordinates
		dx += (s8)(k >> 8);
		dy += (s8)(k >> 16);

		// mouse buttons
		k &= 7; // mask only 3 used buttons (left, right and middle)
		MousePress |= (u8)(k ^ MouseBtn) & k; // mouse pressed buttons
		MouseRel |= (u8)(k ^ MouseBtn) & MouseBtn; // mouse released buttons
		MouseBtn = (u8)k;
	}

	// move cursor
	if ((dx != 0) || (dy != 0)) CurMove(dx, dy);
}

// wait mounting SD card (return False to break)
Bool WaitMount()
{
	// clear display
	DrawClear();

	// set font 8x16
	SelFont8x16();

	// prompt
	DrawText2("Insert SD card", (WIDTH-14*8*2)/2, (HEIGHT-8*2)/2, COL_WHITE);

	// display update
	DispUpdate();

	// waiting for mounting SD card
	while (!DiskMount())
	{
		// break
		if (KeyGet() == KEY_Y) return False;
	}

	return True;
}

// load file list (returns False to break)
Bool LoadFileList()
{
	sFile find;
	sFileInfo info;
	sImgFile* img;
	sImgFile tmp;
	int i;

	// waiting for mounting SD card
	if (!WaitMount()) return False;

	// clear display
	DrawClear();
	DispUpdate();
	SelFont8x16();

	// set directory
	if (!SetDir(PATH))
	{
		DrawText("No directory " PATH, 0, 0, COL_WHITE);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// open search files
	if (!FindOpen(&find, ""))
	{
		DrawText("No image files", 0, 0, COL_WHITE);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// find files
	for (FileListNum = 0; FileListNum < MAXFILES; FileListNum++)
	{
		// find next file
		if (!FindNext(&find, &info, ATTR_ARCH, "*.BMP")) break;

		// check private image, check length of name
		if ((info.namelen < 4) || (memcmp(PrivImg, info.name, info.namelen+1) == 0))
		{
			FileListNum--;
			continue;
		}

		// fill up info
		img = &FileList[FileListNum];
		memcpy(img->name, info.name, 12+1);
		img->namelen = info.namelen;
	}

	// find close
	FindClose(&find);

	// no images
	if (FileListNum == 0)
	{
		DrawText("No image files", 0, 0, COL_WHITE);
		DispUpdate();
		WaitMs(2000);
		return False;
	}

	// check selected image
	if (FileListSel >= FileListNum) FileListSel = FileListNum-1;
	if (FileListSel < 0) FileListSel = 0;

	// sort files
	for (i = 0; i < FileListNum-1;)
	{
		if (strcmp(FileList[i].name, FileList[i+1].name) > 0)
		{
			memcpy(&tmp, &FileList[i], sizeof(sImgFile));
			memcpy(&FileList[i], &FileList[i+1], sizeof(sImgFile));
			memcpy(&FileList[i+1], &tmp, sizeof(sImgFile));
			if (i > 0)
				i--;
			else
				i++;
		}
		else
			i++;
	}

	return True;
}

// display file list (not clearing display)
void DispFileList()
{
	int i, j, x, y;
	u16 fgcol, bgcol;
	sImgFile* img;
	const char* s;

	// set font 8x16
	SelFont8x16();

	// no file
	if (FileListNum == 0)
	{
		DrawText("No image files", 0, 0, COL_WHITE);
		DispUpdate();
	}

	// display files
	for (i = 0; i < FileListNum; i++)
	{
		// prepare color
		fgcol = COL_WHITE;
		bgcol = COL_BLACK;

		if (i == FileListSel)
		{
			fgcol = COL_BLACK;
			bgcol = COL_WHITE;
		}

		// display coordinates
		y = (i / 4) * 16; // row
		x = (i & 3) * 80; // column

		// first space
		s = FileList[i].name;
		DrawCharBg(' ', x, y, fgcol, bgcol);
		x += 8;

		// display name
		for (j = 9; j > 0; j--)
		{
			if (*s == '.')		
				DrawCharBg(' ', x, y, fgcol, bgcol);
			else
				DrawCharBg(*s++, x, y, fgcol, bgcol);
			x += 8;
		}
	}

	// info
	DrawText("B = create new image", (WIDTH-20*8)/2, HEIGHT-16, COL_WHITE);

	// display update
	DispUpdate();
}

// flip tools (not updating visualization)
void ToolFlip()
{
	int k;

	k = ToolSel;
	ToolSel = ToolSelAlt;
	ToolSelAlt = k;

	k = ColRow;
	ColRow = ColRowAlt;
	ColRowAlt = k;

	k = ColCol;
	ColCol = ColColAlt;
	ColColAlt = k;

	COLTYPE c;
	c = Color;
	Color = ColorAlt;
	ColorAlt = c;
}

// set color selection ON
void ColSelOn()
{
	// 1st color
	if (ColCol == 0)
		DrawBlit4Pal(ColSel2Img, ColSel2Img_Pal, 0, 0, 0, HEIGHT+(ColRow-COL_ROWS)*COL_DY,
			COLSEL2_IMG_W, COLSEL2_IMG_H, COLSEL2_IMG_W, COLSEL2_IMG_KEY);
	else if (ColCol == COL_COLS-1)
		DrawBlit4Pal(ColSel3Img, ColSel3Img_Pal, 0, 0, WIDTH-COLSEL3_IMG_W, HEIGHT+(ColRow-COL_ROWS)*COL_DY,
			COLSEL3_IMG_W, COLSEL3_IMG_H, COLSEL3_IMG_W, COLSEL3_IMG_KEY);
	else
		DrawBlit4Pal(ColSelImg, ColSelImg_Pal, 0, 0, ColCol*COL_DX-2, HEIGHT+(ColRow-COL_ROWS)*COL_DY,
			COLSEL_IMG_W, COLSEL_IMG_H, COLSEL_IMG_W, COLSEL_IMG_KEY);

	// update selected color
	Color = FrameBuf[ColCol*COL_DX+COL_DX/2-1 + (HEIGHT+(ColRow-COL_ROWS)*COL_DY+COL_DY/2)*WIDTHLEN];
}

// set color selection OFF
void ColSelOff()
{
	DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - PALETTE_IMG_H,
		PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);
}

// set tool selection ON
void ToolSelOn()
{
	DrawFrame(WIDTH - TOOLS_IMG_W + (ToolSel & 1)*TOOLS_DX+2, (ToolSel >> 1)*TOOLS_DY+2, TOOLS_DX-4, TOOLS_DY-4, COL_RED);
}

// set tool selection OFF
void ToolSelOff()
{
	DrawImgPal(ToolsImg, ToolsImg_Pal, 0, 0, WIDTH - TOOLS_IMG_W, 0,
		TOOLS_IMG_W, TOOLS_IMG_H, TOOLS_IMG_W);
}

// select color or tool (activated by right mouse button)
void ColSel()
{
	int i;
	u8 key;

// ==== open panels

	// cursor off
	CurOff();

	// save palette background
	DrawGetImg(PalBuf, 0, HEIGHT-PALETTE_IMG_H, PALETTE_IMG_W, PALETTE_IMG_H);

	// save tool background
	DrawGetImg(ToolBuf, WIDTH-TOOLS_IMG_W, 0, TOOLS_IMG_W, TOOLS_IMG_H);

	// animate opening palette and tools
#if USE_PICOPADVGA
#define STEPS 10	// number of animation steps
#else
#define STEPS 5		// number of animation steps
#endif

#define DDY (PALETTE_IMG_H/STEPS) // Y increment of palettes
#define DDX (TOOLS_IMG_W/STEPS) // x increment of tools
	for (i = 1; i <= STEPS; i++)
	{
		// draw palette
		DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - i*DDY,
			PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);

		// draw tools
		DrawImgPal(ToolsImg, ToolsImg_Pal, 0, 0, WIDTH - i*DDX, 0,
			TOOLS_IMG_W, TOOLS_IMG_H, TOOLS_IMG_W);

		// display update
		DispUpdate();
#if USE_PICOPADVGA
		WaitMs(10);
#endif
	}

	// draw palette on final position
	DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - PALETTE_IMG_H,
		PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);

	// draw tools on final position
	DrawImgPal(ToolsImg, ToolsImg_Pal, 0, 0, WIDTH - TOOLS_IMG_W, 0,
		TOOLS_IMG_W, TOOLS_IMG_H, TOOLS_IMG_W);

	// set color selection ON
	ColSelOn();

	// set tools selection ON
	ToolSelOn();

	// cursor ON
	CurImg = CurArrowImg; // current cursor image = arrow
	CurOn();

// ==== select tools

	// selection loop
	while (True)
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// right mouse button - return
		if ((MousePress & MOUSE_RBTN) != 0) break;

		// left mouse button - select
		if ((MousePress & MOUSE_LBTN) != 0)
		{
			// select color
			if (CurY >= HEIGHT-PALETTE_IMG_H)
			{
				// cursor off
				CurOff();

				// set color selection OFF
				ColSelOff();

				// current row
				ColRow = (CurY - (HEIGHT - PALETTE_IMG_H)) / COL_DY;

				// current column
				ColCol = (CurX + 2) / COL_DX;

				// set color selection ON
				ColSelOn();

				// cursor ON
				CurOn();
				break;
			}

			// select tool
			if ((CurX >= WIDTH-TOOLS_IMG_W) && (CurY < TOOLS_IMG_H))
			{
				// cursor off
				CurOff();

				// set tools selection OFF
				ToolSelOff();

				// current tool
				ToolSel = (CurX - (WIDTH-TOOLS_IMG_W))/TOOLS_DX + (CurY/TOOLS_DY)*2;

				// set tools selection ON
				ToolSelOn();

				// cursor ON
				CurOn();
				break;
			}
		}

		// flip tools (key X or middle mouse button)
		key = KeyGet();
		if ((key == KEY_X) || ((MousePress & MOUSE_MBTN) != 0))
		{
			// synchornize
			VgaWaitVSync();

			// disable cursors
			CurOff();
			ColSelOff();
			ToolSelOff();

			// flip tools
			ToolFlip();

			// enable cursors
			ToolSelOn();
			ColSelOn();
			CurOn();

			// destoy key
			key = NOKEY;
		}

		// any button - exit
		if (key != NOKEY)
		{
#if USE_SCREENSHOT		// use screen shots
			if (key == KEY_Y) ScreenShot();
#endif
			break;
		}
	}

// ==== close panels

	// cursor off
	CurOff();

	// animate closing palette and tools
	for (i = STEPS; i > 0; i--)
	{
		// draw content below palette
		DrawImg(PalBuf, 0, 0, 0, HEIGHT-PALETTE_IMG_H, PALETTE_IMG_W, PALETTE_IMG_H - i*DDY, PALETTE_IMG_W);

		// draw palette
		DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - i*DDY,
			PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);

		// draw content below tools
		DrawImg(ToolBuf, 0, 0, WIDTH-TOOLS_IMG_W, 0, TOOLS_IMG_W - i*DDX, TOOLS_IMG_H, TOOLS_IMG_W);

		// draw tools
		DrawImgPal(ToolsImg, ToolsImg_Pal, 0, 0, WIDTH - i*DDX, 0,
			TOOLS_IMG_W, TOOLS_IMG_H, TOOLS_IMG_W);

		// display update
		DispUpdate();
#if USE_PICOPADVGA
		WaitMs(10);
#endif
	}

	// draw content below palette on final position
	DrawImg(PalBuf, 0, 0, 0, HEIGHT-PALETTE_IMG_H, PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);

	// draw content below tools on final position
	DrawImg(ToolBuf, 0, 0, WIDTH-TOOLS_IMG_W, 0, TOOLS_IMG_W, TOOLS_IMG_H, TOOLS_IMG_W);

	// cursor on
	CurImg = CurImgList[ToolSel]; // current cursor image
	CurOn();

	// display update
	DispUpdate();

	// flush keys
	KeyFlush();
}

// load selected image
void LoadImg()
{
	sFile file;
	Modi = False; // not modified

	// prepare filename
	MemPrint(Temp, TEMPN, "%s%s", PATH, FileList[FileListSel].name);

	// open file
	if (!FileOpen(&file, Temp)) return;

	// skip header
	FileSeek(&file, 0x46);

	// load image
	FileRead(&file, FrameBuf, FRAMESIZE*sizeof(FRAMETYPE));

	// close file
	FileClose(&file);

	// set cursor ON
	CurVisible = False;
	CurImg = CurImgList[ToolSel]; // current cursor image
	CurOn();

	// update image
	DispUpdateAll();
}

// save image (if modified)
void SaveImg()
{
	// check if modified
	if (!Modi) return;

	sFile file;
	Modi = False; // not modified

	// set cursor off
	CurOff();

	// prepare filename
	MemPrint(Temp, TEMPN, "%s%s", PATH, FileList[FileListSel].name);

	// open file
	if (!FileOpen(&file, Temp)) return;

	// write BMP file header
	FileWrite(&file, &BmpHeader, sizeof(sBmp));

	// write image
	FileWrite(&file, FrameBuf, FRAMESIZE*sizeof(FRAMETYPE)+2);

	// close file
	FileClose(&file);
}

// edit pencil and brush
void EditBrush(int width)
{
	// save old position
	int oldx = CurX;
	int oldy = CurY;

	// while left mouse button is pressed
	while ((MouseBtn & MOUSE_LBTN) != 0) 
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// draw line
		if ((CurX != oldx) || (CurY != oldy))
		{
			// synchronize
			VgaWaitVSync();

			// set cursor off
			CurOff();

			// draw line
			DrawLineW(oldx, oldy, CurX, CurY, Color, width, True);
			oldx = CurX;
			oldy = CurY;

			// cursor on
			CurOn();

			// display update
			DispUpdate();
		}
	}

	// modified
	Modi = True;
}

// edit line
void EditLine(int width)
{
	// save origin position
	int origx = CurX;
	int origy = CurY;

	// save old position
	int oldx = CurX;
	int oldy = CurY;

	// draw line for the first time
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawLineInvW(oldx, oldy, origx, origy, width, True); // draw line inverted
	CurOn();		// cursor on
	DispUpdate();		// display update

	// while left mouse button is pressed
	while ((MouseBtn & MOUSE_LBTN) != 0) 
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// draw line
		if ((CurX != oldx) || (CurY != oldy))
		{
			// redraw new line
			VgaWaitVSync();		// synchronize
			CurOff();		// set cursor off
			DrawLineInvW(oldx, oldy, origx, origy, width, True); // clear old line inverted
			oldx = CurX;		// save new cursor position
			oldy = CurY;
			DrawLineInvW(oldx, oldy, origx, origy, width, True); // draw new line inverted
			CurOn();		// cursor on
			DispUpdate();		// display update
		}
	}

	// draw new line
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawLineInvW(oldx, oldy, origx, origy, width, True); // clear old line inverted
	DrawLineW(CurX, CurY, origx, origy, Color, width, True); // draw new line
	CurOn();		// cursor on
	DispUpdate();		// display update

	// modified
	Modi = True;
}

// edit rectangle
void EditRect(int width)
{
	// save origin position
	int origx = CurX;
	int origy = CurY;

	// save old position
	int oldx = CurX;
	int oldy = CurY;
	int w = 1;
	int h = 1;

	// draw line for the first time
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawFrameInvW(origx, origy, w, h, width); // draw rectangle inverted
	CurOn();		// cursor on
	DispUpdate();		// display update

	// while left mouse button is pressed
	while ((MouseBtn & MOUSE_LBTN) != 0) 
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// draw line
		if ((CurX != oldx) || (CurY != oldy))
		{
			// redraw new rectangle
			VgaWaitVSync();		// synchronize
			CurOff();		// set cursor off
			DrawFrameInvW(origx, origy, w, h, width); // clear old rectangle inverted
			oldx = CurX;		// save new cursor position
			oldy = CurY;
			w = oldx - origx;
			if (w >= 0) w++;
			h = oldy - origy;
			if (h >= 0) h++;
			DrawFrameInvW(origx, origy, w, h, width); // draw new rectangle inverted
			CurOn();		// cursor on
			DispUpdate();		// display update
		}
	}

	// draw new rectangle
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawFrameInvW(origx, origy, w, h, width); // clear old rectangle inverted
	oldx = CurX;		// save new cursor position
	oldy = CurY;
	w = oldx - origx;
	if (w >= 0) w++;
	h = oldy - origy;
	if (h >= 0) h++;
	DrawFrameW(origx, origy, w, h, Color, width); // draw new rectangle
	CurOn();		// cursor on
	DispUpdate();		// display update

	// modified
	Modi = True;
}

// edit circle and ring
void EditRing(int width)
{
	// save origin position
	int origx = CurX;
	int origy = CurY;

	// save old position
	int oldx = CurX;
	int oldy = CurY;
	int r = 1;

	// draw line for the first time
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawRingInv(origx, origy, r-width/2, r+(width+1)/2); // draw ring inverted
	CurOn();		// cursor on
	DispUpdate();		// display update

	// while left mouse button is pressed
	while ((MouseBtn & MOUSE_LBTN) != 0) 
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// draw ring
		if ((CurX != oldx) || (CurY != oldy))
		{
			// redraw new line
			VgaWaitVSync();		// synchronize
			CurOff();		// set cursor off
			DrawRingInv(origx, origy, r-width/2, r+(width+1)/2); // clear old ring inverted
			oldx = CurX;		// save new cursor position
			oldy = CurY;
			r = float2int(sqrtf((oldx-origx)*(oldx-origx) + (oldy-origy)*(oldy-origy)) + 0.5f);
			if (r == 0) r = 1;
			DrawRingInv(origx, origy, r-width/2, r+(width+1)/2); // draw new ring inverted
			CurOn();		// cursor on
			DispUpdate();		// display update
		}
	}

	// draw new ring
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawRingInv(origx, origy, r-width/2, r+(width+1)/2); // clear old ring inverted
	oldx = CurX;		// save new cursor position
	oldy = CurY;
	r = float2int(sqrtf((oldx-origx)*(oldx-origx) + (oldy-origy)*(oldy-origy)) + 0.5f);
	if (r == 0) r = 1;
	DrawRing(origx, origy, r-width/2, r+(width+1)/2, Color); // draw new ring
	CurOn();		// cursor on
	DispUpdate();		// display update

	// modified
	Modi = True;
}

// edit fill box
void EditBox()
{
	// save origin position
	int origx = CurX;
	int origy = CurY;

	// save old position
	int oldx = CurX;
	int oldy = CurY;
	int w = 1;
	int h = 1;

	// draw line for the first time
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawRectInv(origx, origy, w, h); // draw box inverted
	CurOn();		// cursor on
	DispUpdate();		// display update

	// while left mouse button is pressed
	while ((MouseBtn & MOUSE_LBTN) != 0) 
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// draw box
		if ((CurX != oldx) || (CurY != oldy))
		{
			// redraw new line
			VgaWaitVSync();		// synchronize
			CurOff();		// set cursor off
			DrawRectInv(origx, origy, w, h); // clear old box inverted
			oldx = CurX;		// save new cursor position
			oldy = CurY;
			w = oldx - origx;
			if (w >= 0) w++;
			h = oldy - origy;
			if (h >= 0) h++;
			DrawRectInv(origx, origy, w, h); // draw new box inverted
			CurOn();		// cursor on
			DispUpdate();		// display update
		}
	}

	// draw new box
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawRectInv(origx, origy, w, h); // clear old box inverted
	oldx = CurX;		// save new cursor position
	oldy = CurY;
	w = oldx - origx;
	if (w >= 0) w++;
	h = oldy - origy;
	if (h >= 0) h++;
	DrawRect(origx, origy, w, h, Color); // draw new box
	CurOn();		// cursor on
	DispUpdate();		// display update

	// modified
	Modi = True;
}

// edit fill circle
void EditRound()
{
	// save origin position
	int origx = CurX;
	int origy = CurY;

	// save old position
	int oldx = CurX;
	int oldy = CurY;
	int r = 1;

	// draw line for the first time
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawFillCircleInv(origx, origy, r); // draw circle inverted
	CurOn();		// cursor on
	DispUpdate();		// display update

	// while left mouse button is pressed
	while ((MouseBtn & MOUSE_LBTN) != 0) 
	{
		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// draw circle
		if ((CurX != oldx) || (CurY != oldy))
		{
			// redraw new line
			VgaWaitVSync();		// synchronize
			CurOff();		// set cursor off
			DrawFillCircleInv(origx, origy, r); // clear old circle inverted
			oldx = CurX;		// save new cursor position
			oldy = CurY;
			r = float2int(sqrtf((oldx-origx)*(oldx-origx) + (oldy-origy)*(oldy-origy)) + 0.5f);
			if (r == 0) r = 1;
			DrawFillCircleInv(origx, origy, r); // draw new circle inverted
			CurOn();		// cursor on
			DispUpdate();		// display update
		}
	}

	// draw new circle
	VgaWaitVSync();		// synchronize
	CurOff();		// set cursor off
	DrawFillCircleInv(origx, origy, r); // clear old circle inverted
	oldx = CurX;		// save new cursor position
	oldy = CurY;
	r = float2int(sqrtf((oldx-origx)*(oldx-origx) + (oldy-origy)*(oldy-origy)) + 0.5f);
	if (r == 0) r = 1;
	DrawFillCircle(origx, origy, r, Color); // draw new circle
	CurOn();		// cursor on
	DispUpdate();		// display update

	// modified
	Modi = True;
}

// edit dropper
void EditDrop()
{
	// synchronize
	VgaWaitVSync();

	// set cursor off
	CurOff();

	// get color under cursor
	u16 c = *CurBuf;
	int b = c & 0x1f; // blue 5 bits
	int g = (c >> 5) & 0x3f; // green 6 bits
	int r = c >> 11; // red 5 bits

	// search nearest color in palettes
	u16 c2;
	int r2, g2, b2, col, row, dist;
	int distbest = 100000000;
	for (col = 0; col < COL_COLS; col++)
	{
		for (row = 0; row < COL_ROWS; row++)
		{
			// get color from palettes
			c2 = PaletteImg_Pal[PaletteImg[col*COL_DX+COL_DX/2-1 + (row*COL_DY+COL_DY/2)*PALETTE_IMG_W]];
			b2 = c2 & 0x1f; // blue 5 bits
			g2 = (c2 >> 5) & 0x3f; // green 6 bits
			r2 = c2 >> 11; // red 5 bits

			// compare distance
			b2 -= b;
			g2 -= g;
			r2 -= r;
			dist = b2*b2 + g2*g2 + r2*r2;

			// new best color
			if (dist < distbest)
			{
				distbest = dist;
				ColCol = col;
				ColRow = row;
				Color = c2;
			}
		}
	}

	// cursor on
	CurOn();
}

// edit image
void EditImg()
{
	int i;

	// connectint USB mouse
	printf("\f\27Connecting USB mouse (Y=quit)...");
	while (!UsbMouseIsMounted())
	{
		if (KeyGet() == KEY_Y) return;
	}

	// load selected image
	LoadImg();

	// edit loop
	while (True)
	{
		switch (KeyGet())
		{
		// Y exit
		case KEY_Y:
#if USE_SCREENSHOT		// use screen shots
			ScreenShot();
#endif
			// save image (if modified)
			SaveImg();
			return;

		// A previous image
		case KEY_A:
			// save image (if modified)
			SaveImg();

			// previous image
			DrawClear(); // clear screen
			DispUpdate();
			i = FileListSel;
			i--;
			if (i < 0) i = FileListNum - 1;
			FileListSel = i;
			LoadImg(); // load image
			break;

		// B next image
		case KEY_B:
			// save image (if modified)
			SaveImg();

			// next image
			DrawClear(); // clear screen
			DispUpdate();
			i = FileListSel;
			i++;
			if (i >= FileListNum) i = 0;
			FileListSel = i;
			LoadImg(); // load image
			break;

		// X flip tools
		case KEY_X:
			// synchronize
			VgaWaitVSync();

			// set cursor off
			CurOff();

			// flip tools
			ToolFlip();

			// cursor on
			CurImg = CurImgList[ToolSel]; // current cursor image
			CurOn();
			break;
		}

		// cursor mouse move
		CurMouse();

		// display update
		DispUpdate();

		// left mouse button - edit
		if ((MousePress & MOUSE_LBTN) != 0) 
		{
			switch (ToolSel)
			{
			// pencil
			case TOOL_PENCIL:
				EditBrush(1);
				break;

			// brush
			case TOOL_BRUSH:
				EditBrush(LINEW);
				break;

			// line
			case TOOL_LINE:
				EditLine(1);
				break;

			// thick line
			case TOOL_LINE2:
				EditLine(LINEW);
				break;

			// rectangle
			case TOOL_RECT:
				EditRect(1);
				break;

			// circle
			case TOOL_CIRCLE:
				EditRing(1);
				break;

			// thick rectangle
			case TOOL_RECT2:
				EditRect(LINEW);
				break;

			// ring
			case TOOL_RING:
				EditRing(LINEW);
				break;

			// fill box
			case TOOL_BOX:
				EditBox();
				break;

			// fill circle
			case TOOL_ROUND:
				EditRound();
				break;

			// bucket
			case TOOL_BUCKET:
				// synchronize
				VgaWaitVSync();

				// set cursor off
				CurOff();

				// fill color
				DrawFill(CurX, CurY, Color);

				// cursor on
				CurOn();
				break;

			// dropper
			case TOOL_DROP:
				EditDrop();
				break;
			}
		}

		// right mouse button - select color or tool
		if ((MousePress & MOUSE_RBTN) != 0) 
		{
			// select color or tool
			ColSel();
		}

		// middle mouse button - flip tools
		if ((MousePress & MOUSE_MBTN) != 0)
		{
			// synchronize
			VgaWaitVSync();

			// set cursor off
			CurOff();

			// flip tools
			ToolFlip();

			// cursor on
			CurImg = CurImgList[ToolSel]; // current cursor image
			CurOn();
		}

		// display update
		DispUpdate();
	}
}

// create new file (returns False on error)
Bool NewFile()
{
	int i;
	sFile f;

	// search not existing file
	for (i = 1; i < 100; i++)
	{
		// prepare full filename
		MemPrint(Temp, TEMPN, "%sNEW%d.BMP", PATH, i);

		// check if file exists
		if (!FileExist(Temp)) break;
	}
	if (i == 100) return False; // directory is full

	// create file
	if (!FileCreate(&f, Temp)) return False;

	// wait info
	DrawClear();
	DrawTextBg4("WAIT", (WIDTH-4*32)/2, (HEIGHT-64)/2, COL_WHITE, COL_BLACK);
	WaitMs(300);
	DispUpdate();

	// write BMP file header
	FileWrite(&f, &BmpHeader, sizeof(sBmp));

	// clear screen
	DrawClearCol(COL_WHITE);

	// write image data
	FileWrite(&f, FrameBuf, WIDTH*HEIGHT*sizeof(COLTYPE)+2);

	// clear screen
	DrawClear();

	// close file
	FileClose(&f);

	// flush disk writes
	DiskFlush();

	// load file list (returns False to break)
	if (!LoadFileList()) ResetToBootLoader();

	// prepare short filename
	MemPrint(Temp, TEMPN, "NEW%d.BMP", i);

	// search this new file
	for (i = 0; i < FileListNum; i++)
	{
		// check file name
		if (memcmp(Temp, FileList[i].name, FileList[i].namelen+1) == 0)
		{
			// select this file
			FileListSel = i;
			break;
		}
	}

	// clear display
	DrawClear();

	// display file list
	DispFileList();

	return True;
}

// main function
int main()
{
	FileListSel = 0;
	Bool ok;
	int i;

	// initialize USB
	UsbHostInit();

// ==== select file to edit

	// image selection loop
	while (True)
	{
		// load file list (returns False to break)
		if (!LoadFileList()) ResetToBootLoader();

		// clear display
		DrawClear();

		// display file list
		DispFileList();

		// image selection control
		ok = False;
		while (!ok)
		{
			switch (KeyGet())
			{
			// Y exit
			case KEY_Y:
				ResetToBootLoader();
				break;

			// UP
			case KEY_UP:
				i = FileListSel;
				i -= 4;
				if (i < 0)
				{
					do i += 4; while (i < FileListNum);
					i -= 4;
					if (i < 0) i = 0;
				}
				FileListSel = i;
				DispFileList();
				break;

			// DOWN
			case KEY_DOWN:
				i = FileListSel;
				i += 4;
				if (i >= FileListNum)
				{
					do i -= 4; while (i >= 0);
					i += 4;
					if (i >= FileListNum) i = FileListNum-1;
				}
				FileListSel = i;
				DispFileList();
				break;

			// LEFT
			case KEY_LEFT:
				i = FileListSel;
				i--;
				if (i < 0) i = FileListNum-1;
				FileListSel = i;
				DispFileList();
				break;

			// RIGHT
			case KEY_RIGHT:
				i = FileListSel;
				i++;
				if (i >= FileListNum) i = 0;
				FileListSel = i;
				DispFileList();
				break;

			// A: select file
			case KEY_A:
				if (FileListSel < FileListNum) ok = True;
				break;

			// B: create new image
			case KEY_B:
				NewFile(); // create new file
				break;
			}
		}

		// edit image
		EditImg();
	}
}
