
// ****************************************************************************
//
//                         FileSel - File selection
//
// ****************************************************************************
// PicoLibSDK - Alternative SDK library for Raspberry Pico and RP2040
// Copyright (c) 2023 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com
// 	https://github.com/Panda381/PicoLibSDK
//	https://www.breatharian.eu/hw/picolibsdk/index_en.html
//	https://github.com/pajenicko/picopad
//	https://picopad.eu/en/
// License:
//	This source code is freely available for any purpose, including commercial.
//	It is possible to take and modify the code or parts of it, without restriction.


// File with text description - same name as main file, but with TXT extension. Format:
//	- text width is 26 characters
//	- text height is 15 row, if using also BMP preview file, or 30 rows, if not using BMP
//	- ^ is prefix of control characters
//		^^ ... print ^ character
//		^1..^9 ... print character with code 1..9
//		^A..^V ... print character with code 10..31
//		^0 ... normal gray text
//		^W ... white text
//		^X ... green text
//		^Y ... yellow text
//		^Z ... red text
//		^[ ... start invert
//		^] ... stop invert
// File with preview image - same name as main file, but with BMP extension. Format:
// 	- Size recommended 1/2 width and 1/2 height of the screen. If TXT is not used, height can be full screen.
//	- Pixel format is the same as screen format


#if USE_FILESEL		// use file selection (lib_filesel.c, lib_filesel.h)

#include "../../_display/_include.h"	// display interface (colors)

#include "lib_text.h"
#include "lib_draw.h"
#include "lib_fat.h"

#ifndef _LIB_FILESEL_H
#define _LIB_FILESEL_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef FILESEL_DISPSIZE
#define FILESEL_DISPSIZE	1	// 1=display size instead of TXT+BMP flags
#endif

// file selection colors
typedef struct {
	COLTYPE		titlefg;	// title foreground color - current directory or description (COL_BLACK)
	COLTYPE		titlebg;	// title background color - current directory or description (COL_WHITE)
	COLTYPE		filefg;		// file foreground color (COL_CYAN)
	COLTYPE		filebg;		// file background color (COL_BLUE)
	COLTYPE		dirfg;		// directory foreground color (COL_WHITE)
	COLTYPE		dirbg;		// directory background color (COL_BLUE)
	COLTYPE		curfg;		// cursor coreground color (COL_BLACK)
	COLTYPE		curbg;		// cursor background color (COL_CYAN)
	COLTYPE		infofg;		// info text foreground color (COL_GREEN)
	COLTYPE		infobg;		// info text background color (COL_BLUE)
	COLTYPE		textfg;		// text foreground color (COL_GRAY)
	COLTYPE		textbg;		// text background color (COL_BLACK)
	COLTYPE		biginfofg;	// big info text foreground color (COL_YELLOW)
	COLTYPE		biginfobg;	// big info text background color (COL_BLACK)
	COLTYPE		bigerrfg;	// big error foreground color (COL_YELLOW)
	COLTYPE		bigerrbg;	// big error background color (COL_RED)
} sFileSelColors;

extern const sFileSelColors FileSelColBlue; // colors template - blue theme
extern const sFileSelColors FileSelColGreen; // colors template - green theme

// current path
#define FILESEL_PATHMAX		240	// max. length of path (it must be <= 256 with CRC and magic)
extern char FileSelPath[FILESEL_PATHMAX+1]; // current path (with terminating 0, without last path character if not root)
extern int FileSelPathLen; // length of path
extern int FileSelRootLen; // length of root

// file extension
extern char FileSelExt[4]; // file extension in uppercase (1 to 3 characters, with terminating 0)
extern int FileSelExtLen; // length of file extension (1 to 3 characters)

extern char FileSelExt2[4]; // file extension 2 in uppercase (0 to 3 characters, with terminating 0)
extern int FileSelExtLen2; // length of file extension 2 (0 to 3 characters)

extern char FileSelExt3[4]; // file extension 3 in uppercase (0 to 3 characters, with terminating 0)
extern int FileSelExtLen3; // length of file extension 3 (0 to 3 characters)

// temporary buffer
#define FILESEL_TEMPBUF 512 // size of temporary buffer
extern char FileSelTempBuf[FILESEL_TEMPBUF+1];
extern int FileSelTempBufNum; // number of bytes in temporary buffer

// selected last name
extern char FileSelLastName[9]; // last selected name (without extension, with terminating 0)
extern int FileSelLastNameLen; // length of last name (without extension), 0 = not used
extern int FileSelLastNameTop; // top file of last name
extern u8 FileSelLastNameAttr; // attributes of last name
extern u8 FileSelLastNameExt; // index of extension of last name 0..2
extern u32 FileSelLastNameSize; // size of last name

extern sFile FileSelPrevFile; // preview file (name[0] = 0 if not open)

// display big info text
void FileSelDispBigInfo(const char* text);

// display big error text, wait for key press
void FileSelDispBigErr(const char* text);

// initialize file selection
//  root ... path to root directory (terminated with 0; max. 240 characters)
//   - use uppercase
//   - use '/' character as path separator
//   - start path with '/' root entry
//   - do not end path with '/' character (except root path)
//  title... title on file panel, used instead of root path (terminated with 0; max. 20 characters)
//  ext ... file extension (1 to 3 characters in uppercase, terminated with 0)
//  col ... colors (recommended &FileSelColGreen or &FileSelColBlue)
void FileSelInit3(const char* root, const char* title, const char* ext, const char* ext2, const char* ext3, const sFileSelColors* col);

INLINE void FileSelInit2(const char* root, const char* title, const char* ext, const char* ext2, const sFileSelColors* col)
	{ FileSelInit3(root, title, ext, ext2, "", col); }

INLINE void FileSelInit(const char* root, const char* title, const char* ext, const sFileSelColors* col)
	{ FileSelInit3(root, title, ext, "", "", col); }

// select file (returns True if file selected, or False to break)
//   If file selected with True:
//  - FileSelPath contains current directory (function sets this directory as active, using SetDir(FileSelPath))
//  - FileSelExt contains file extension
//  - FileSelLastName contains file name without extension
//  - FileSelTempBuf contains file name with extension (can be used to 
//  - function displays info "Loading..."
// Open the file as in this example:
//   if (!FileSel()) return; ... select file, or return from the program
//   if (!FileOpen(&FileSelPrevFile, FileSelTempBuf))
//       FileSelDispBigErr("Cannot open file");
//   else
//   {
//     ...loading file
//     FileClose(&FileSelPrevFile);
//     ...process file
//   }
//   ... repeat with FileSel() again
Bool FileSel();

#ifdef __cplusplus
}
#endif

#endif // _LIB_FILESEL_H

#endif // USE_FILESEL		// use file selection (lib_filesel.c, lib_filesel.h)
