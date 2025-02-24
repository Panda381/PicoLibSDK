
// ****************************************************************************
//
//                               Ray-trace rendering
//
// ****************************************************************************

#ifndef _RENDER_H
#define _RENDER_H

#include "vector.h"		// 3D vector
#include "sphere.h"		// 3D sphere object

#define FLOORY -2			// floor Y position

// list of spheres
extern Sphere Spheres[];

extern float FloorPos;	// floot position

// render image
void FASTCODE NOFLASH(Render3D)();

#endif // _RENDER_H
