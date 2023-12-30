
///////////////////////////////////////////////////////////////////////////////
//                                                                           //
//                                   Includes                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////
// (c) 2020-2024 Miroslav Nemecek, Panda38@seznam.cz, hardyplotter2@gmail.com

// must not be compiled as Unicode (to ensure compatibility with other systems)

///////////////////////////////////////////////////////////////////////////////
// system includes

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdarg.h> // va_list

//#include <Windows.h>
//#include <tchar.h>

// definitions for non-Windows environment
typedef char TCHAR; // character type
#define _T(x) x		// char string
typedef const TCHAR* LPCTSTR; // pointer to constant text string
typedef unsigned int BOOL;
#define TRUE  1
#define FALSE 0

#pragma warning(disable : 4996) // unsafe function

///////////////////////////////////////////////////////////////////////////////
// base types

/*
typedef __int8				s8;
typedef __int16				s16;
typedef __int32				s32;
typedef __int64				s64;

typedef unsigned __int8		u8;
typedef unsigned __int16	u16;
typedef unsigned __int32	u32;
typedef unsigned __int64	u64;
*/

typedef signed char         s8;
typedef unsigned char       u8;
typedef signed short        s16;
typedef unsigned short      u16;
typedef signed int          s32;
typedef unsigned int        u32;
typedef signed long long    s64;
typedef unsigned long long  u64;
typedef unsigned int        uint;

typedef u8	BYTE;
typedef u16	WORD;
typedef u32	DWORD;

///////////////////////////////////////////////////////////////////////////////
// constants

// bits
#define	B0 (1<<0)
#define	B1 (1<<1)
#define	B2 (1<<2)
#define	B3 (1<<3)
#define	B4 (1<<4)
#define	B5 (1<<5)
#define	B6 (1<<6)
#define	B7 (1<<7)
#define	B8 (1<<8)
#define	B9 (1<<9)
#define	B10 (1<<10)
#define	B11 (1<<11)
#define	B12 (1<<12)
#define	B13 (1<<13)
#define	B14 (1<<14)
#define	B15 (1<<15)
#define B16 (1<<16)
#define B17 (1<<17)
#define B18 (1<<18)
#define	B19 (1<<19)
#define B20 (1<<20)
#define B21 (1<<21)
#define B22 (1<<22)
#define B23 (1<<23)
#define B24 (1<<24)
#define B25 (1<<25)
#define B26 (1<<26)
#define B27 (1<<27)
#define B28 (1<<28)
#define B29 (1<<29)
#define B30 (1<<30)
#define B31 ((u32)(1<<31))

#define	BIGINT	0x40000000

///////////////////////////////////////////////////////////////////////////////
// inplace "new" operator
//		example:	new (&m_List[inx]) CText;
//					m_List[i].~CText()

inline void* operator new (size_t size, void* p)
{
	size;
	return p;
}

inline void operator delete (void* adr, void* p)
{
	adr; p;
	return;
}

///////////////////////////////////////////////////////////////////////////////
// program includes

#include "text.h"			// text string
