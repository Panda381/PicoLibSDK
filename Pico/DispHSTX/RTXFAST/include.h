
// ****************************************************************************
//                                 
//                              Includes
//
// ****************************************************************************

// ----------------------------------------------------------------------------
//                                   Includes
// ----------------------------------------------------------------------------

#include INCLUDES_H		// all includes

typedef float flt;

#ifndef WIDTH
#define WIDTH 320
#endif

#ifndef HEIGHT
#define HEIGHT 240
#endif

extern u16 ALIGNED FrameBuf[WIDTH*HEIGHT];

#include "src/render.h"		// ray-trace rendering
#include "src/main.h"		// main code
