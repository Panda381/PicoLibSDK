
// ****************************************************************************
//
//                                 Main code
//
// ****************************************************************************

#include "include.h"

#define HOMEPATH_DEF	"/COLBOOK"		// default home path
char HomePath[APPPATH_PATHMAX+15];		// home path, with tailing '/'
int HomePathLen;

#define MAXFILES 60	// max. files (15 rows, 4 columns)

const char PrivImg[] = "COLBOOK.BMP";	// private image (do not list nor edit)

// image file
typedef struct {
	char	name[12+1];	// image file name
	u8	namelen;	// length of file name
} sImgFile;

// list of image files
sImgFile FileList[MAXFILES];
int FileListNum = 0;
int FileListSel = 0;
Bool Modi; // file modified

// cursor
COLTYPE CurBuf[BRUSH_IMG_W*BRUSH_IMG_H]; // buffer to save cursor background
int CurX = WIDTH/2; // cursor X position
int CurY = HEIGHT/2; // cursor Y position
Bool CurVisible; // cursor is visible

// palette
#define COL_ROWS	6	// number of color rows
#define COL_COLS	36	// number of color columns
#define COL_DX		9	// color delta X
#define COL_DY		9	// color delta Y
COLTYPE PalBuf[PALETTE_IMG_W*PALETTE_IMG_H]; // buffer to save palette background
COLTYPE Color = COL_RED; // selected color
int ColRow = 0; // row of selected color
int ColCol = 0; // column of selected color

// set cursor ON
void CurOn()
{
	// already visible
	if (CurVisible) return;
	CurVisible = True;

	// storing content under the cursor
	DrawGetImg(CurBuf, CurX, CurY, BRUSH_IMG_W, BRUSH_IMG_H);

	// draw cursor (with selected color)
	DrawBlitSubst(BrushImg, 0, 0, CurX, CurY, BRUSH_IMG_W, BRUSH_IMG_H,
		BRUSH_IMG_W, BRUSH_IMG_KEY, COL_WHITE, Color);
}

// set cursor OFF
void CurOff()
{
	// already not visible
	if (!CurVisible) return;
	CurVisible = False;

	// restore content
	DrawImg(CurBuf, 0, 0, CurX, CurY, BRUSH_IMG_W, BRUSH_IMG_H, BRUSH_IMG_W);
}

// move cursor
void CurMove(int x, int y)
{
	// limit coordinates
	if (x < 0) x = 0;
	if (x > WIDTH-1) x = WIDTH-1;
	if (y < 0) y = 0;
	if (y > HEIGHT-1) y = HEIGHT-1;

	// cursor not changes
	if ((x == CurX) && (y == CurY)) return;

	// set cursor OFF
	CurOff();

	// set new coordinates
	CurX = x;
	CurY = y;

	// set cursor ON
	CurOn();
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
	HomePath[HomePathLen] = 0;
	if (!SetDir(HomePath))
	{
		DrawText("No directory ", 0, 0, COL_WHITE);
		DrawText(HomePath, 13*8, 0, COL_WHITE);
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
	DispUpdate();
}

// set color ON
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

	Color = FrameBuf[ColCol*COL_DX+COL_DX/2-1 + (HEIGHT+(ColRow-COL_ROWS)*COL_DY+COL_DY/2)*WIDTHLEN];
}

// set color OFF
void ColSelOff()
{
	DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - PALETTE_IMG_H,
		PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);
}

// select color
void ColSel()
{
	int i;

	// cursor off
	CurOff();

	// save palette background
	DrawGetImg(PalBuf, 0, HEIGHT-PALETTE_IMG_H, PALETTE_IMG_W, PALETTE_IMG_H);

	// animate opening palette
#define DDY 5
	for (i = DDY; i < PALETTE_IMG_H; i += DDY)
	{
		DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - i,
			PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);
		DispUpdate();
#if USE_PICOPADVGA
		WaitMs(10);
#endif
	}
	DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - PALETTE_IMG_H,
		PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);

	// set color ON
	ColSelOn();

	// selection loop
	Bool loop = True;
	while (loop)
	{
		switch (KeyGet())
		{
		// Y, B, A exit
		case KEY_A:
		case KEY_B:
		case KEY_Y:
			loop = False;
			break;

		// left
		case KEY_LEFT:
			ColSelOff();
			ColCol--;
			if (ColCol < 0) ColCol = COL_COLS-1;
			ColSelOn();
			break;

		// right
		case KEY_RIGHT:
			ColSelOff();
			ColCol++;
			if (ColCol >= COL_COLS) ColCol = 0;
			ColSelOn();
			break;

		// up
		case KEY_UP:
			ColSelOff();
			ColRow--;
			if (ColRow < 0) ColRow = COL_ROWS-1;
			ColSelOn();
			break;

		// down
		case KEY_DOWN:
			ColSelOff();
			ColRow++;
			if (ColRow >= COL_ROWS) ColRow = 0;
			ColSelOn();
			break;
		}

		// display update
		DispUpdate();
	}

	// animate closing palette
	for (i = PALETTE_IMG_H; i > 0; i -= DDY)
	{
		DrawImg(PalBuf, 0, 0, 0, HEIGHT-PALETTE_IMG_H, PALETTE_IMG_W, PALETTE_IMG_H - i, PALETTE_IMG_W);
		DrawImgPal(PaletteImg, PaletteImg_Pal, 0, 0, 0, HEIGHT - i,
			PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);
		DispUpdate();
#if USE_PICOPADVGA
		WaitMs(10);
#endif
	}
	DrawImg(PalBuf, 0, 0, 0, HEIGHT-PALETTE_IMG_H, PALETTE_IMG_W, PALETTE_IMG_H, PALETTE_IMG_W);

	// cursor on
	CurOn();

	// flush keys
	KeyFlush();
}

// load selected image
void LoadImg()
{
	sFile file;
	Modi = False; // not modified

	// prepare filename
	memcpy(&HomePath[HomePathLen], FileList[FileListSel].name, 13);

	// open file
	Bool res = FileOpen(&file, HomePath);
	HomePath[HomePathLen] = 0;
	if (!res) return;

	// skip header
	FileSeek(&file, 0x46);

	// load image
	FileRead(&file, FrameBuf, FRAMESIZE*sizeof(FRAMETYPE));

	// close file
	FileClose(&file);

	// set cursor ON
	CurVisible = False;
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
	memcpy(&HomePath[HomePathLen], FileList[FileListSel].name, 13);

	// open file
	Bool res = FileOpen(&file, HomePath);
	HomePath[HomePathLen] = 0;
	if (!res) return;

	// skip header
	FileSeek(&file, 0x46);

	// write image
	FileWrite(&file, FrameBuf, FRAMESIZE*sizeof(FRAMETYPE));

	// close file
	FileClose(&file);
}

// edit image
void EditImg()
{
	int i;

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

		// A or previous image
		case KEY_A:
			if (KeyPressed(KEY_X))
			{
				// save image (if modified)
				SaveImg();

				DrawClear();
				DispUpdate();
				i = FileListSel;
				i--;
				if (i < 0) i = FileListNum - 1;
				FileListSel = i;
				LoadImg();
			}
			else
			{
				// fill area (not for black color)
				CurOff();
				if (DrawGetPoint(CurX, CurY) != COL_BLACK)
					DrawFill(CurX, CurY, Color);
				else if (DrawGetPoint(CurX+1, CurY) != COL_BLACK)
					DrawFill(CurX+1, CurY, Color);
				else if (DrawGetPoint(CurX-1, CurY) != COL_BLACK)
					DrawFill(CurX-1, CurY, Color);
				else if (DrawGetPoint(CurX, CurY+1) != COL_BLACK)
					DrawFill(CurX, CurY+1, Color);
				else if (DrawGetPoint(CurX, CurY-1) != COL_BLACK)
					DrawFill(CurX, CurY-1, Color);
				CurOn();
				DispUpdate();
				Modi = True; // modified
			}
			break;

		// B select color or next image
		case KEY_B:
			if (KeyPressed(KEY_X))
			{
				// save image (if modified)
				SaveImg();

				// next image
				DrawClear();
				DispUpdate();
				i = FileListSel;
				i++;
				if (i >= FileListNum) i = 0;
				FileListSel = i;
				LoadImg();
			}
			else
			{
				// select color
				ColSel();
			}
			break;

		// cursor left
		case KEY_LEFT:
			CurMove(CurX - (KeyPressed(KEY_X) ? 1 : 5), CurY);
			break;

		// cursor right
		case KEY_RIGHT:
			CurMove(CurX + (KeyPressed(KEY_X) ? 1 : 5), CurY);
			break;

		// cursor up
		case KEY_UP:
			CurMove(CurX, CurY - (KeyPressed(KEY_X) ? 1 : 5));
			break;

		// cursor down
		case KEY_DOWN:
			CurMove(CurX, CurY + (KeyPressed(KEY_X) ? 1 : 5));
			break;
		}

		// display update
		DispUpdate();
	}
}

int main()
{
	FileListSel = 0;
	Bool ok;
	int i;

	// prepare home path
	HomePathLen = GetHomePath(HomePath, HOMEPATH_DEF);
	if (HomePathLen > 1)
	{
		HomePath[HomePathLen] = PATHCHAR;
		HomePathLen++;
		HomePath[HomePathLen] = 0;
	}
	while (True)
	{
		// load file list (returns False to break)
		if (!LoadFileList()) ResetToBootLoader();

		// clear display
		DrawClear();

		// display file list
		DispFileList();

		// image selection
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

			// select image
			case KEY_A:
				if (FileListSel < FileListNum) ok = True;
				break;
			}
		}

		// edit image
		EditImg();
	}
}
