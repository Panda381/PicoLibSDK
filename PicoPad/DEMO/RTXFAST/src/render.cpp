
// ****************************************************************************
//
//                               Ray-trace rendering
//
// ****************************************************************************

#include "../include.h"

// list of spheres
Sphere Spheres[] = {
	// position, radius, color, reflection, transparency (transparency not realized)
	Sphere(V3(-3.2f,-0.4f,1), 1, V3(1,1,0), 0.5f, 0), // yellow sphere
	Sphere(V3(-1.4f,0.4f,0), 0.8f, V3(1,0,0), 0.5f, 0),  // red sphere
	Sphere(V3(1.0f,-0.1f,-4), 2, V3(0,0,(float)0.4f), (float)0.7f, 0), // blue sphere
	Sphere(V3(2.8f,0.2f,1), 1.5, V3(0,(float)0.5f,0), (float)0.6f, 0), // green sphere
};
#define OBJNUM count_of(Spheres)

V3 Camera(0, 0, 10);		// camera position
V3 Light(-6, 4, 10);		// light position
float Ambient = (float)0.3f;	// intensity of ambient light
V3 BackCol((float)0.4f, (float)0.6f, 1); // background color in horizon

float FloorPos = 0;	// floot position

#define TOOFAR 1e3
#define DEPTHMAX 1			// max. depth of trace

// trace ray
void FASTCODE NOFLASH(Trace)(V3* rgb, const V3 &orig, const V3 &dir, int depth, const Sphere* disable)
{
	// find nearest intersection
	float t1best = TOOFAR, t2best = TOOFAR;
	const Sphere* sbest = NULL;
	float t1, t2;
	int i;
	for (i = 0; i < OBJNUM; i++)
	{
		if (&Spheres[i] == disable) continue;
		t1 = TOOFAR;
		t2 = TOOFAR;
		if (Spheres[i].Intersect(orig, dir, &t1, &t2))
		{
			if (t1 < 0) t1 = t2;
			if (t1 < t1best)
			{
				t1best = t1;
				t2best = t2;
				sbest = &Spheres[i];
			}
		}
	}

	// if object not found, return sky color or continue with floor plane
	V3 col, pos, norm;
	float refl;
	if (sbest == NULL)
	{
		// sky (black in top direction)
		float k = dir.y;
		if (dir.y >= 0)
		{
			k *= 2.5;
			if (k > 1) k = 1;
			*rgb = BackCol*(1-k);
			return;
		}

		// floor - substitute with plane parameters
		t1best = (orig.y - FLOORY) / dir.y;
		pos = orig - (dir*t1best);
		norm.Set(0,1,0);
		col = ((int)(ceilf(pos.x) + ceilf(pos.z + FloorPos)) & 1) ? V3(1,1,1) : V3(1,0.25,0.25);
		refl = (float)0.4;
	}
	else
	{
		// coordinate and normal in intersection point
		pos = orig + (dir*t1best);
		norm = pos - sbest->pos;
		norm.Norm();
		col = sbest->col;
		refl = sbest->ref;
	}

	// if normal and ray direction are not opposited, we are inside sphere, then reverse normal
	bool inside = false;
	if (dir.Dot(norm) > 0)
	{
		inside = true;
		norm.Inv();
	}

	// vector to light
	V3 light = Light - pos;
	light.Norm();

	// check if point is in shadow
	float intens = 1;
	for (i = 0; i < OBJNUM; i++)
	{
		if (sbest == &Spheres[i]) continue;
		if (Spheres[i].Intersect(pos, light, &t1, &t2))
		{
			intens = 0;
			break;
		}
	}

	// get diffusion color
	intens = intens*norm.Dot(light)*(1-Ambient);
	if (intens <= 0) intens = 0;
	*rgb = col * (intens + Ambient);

	// add reflection
	if ((refl > 0) && (depth < DEPTHMAX))
	{
		// reflection vector
		V3 rdir = dir - norm*2*dir.Dot(norm);
		rdir.Norm();

		// add reflection
		V3 rgb2;
		Trace(&rgb2, pos, rdir, depth + 1, sbest);
		*rgb *= 1 - refl;
		*rgb += rgb2 * refl;
	}
}

// render image
void FASTCODE NOFLASH(Render3D)()
{
	// local variables
	int tmp;
	int x, y;			// current X and Y coordinates in bitmap
	float xx, yy;		// current X and Y coordinates in viewing plane
	V3 rgbV;			// result pixel color as vactor 0..1
	V3 orig(0, 0, 10);	// camera position
	V3 dir;				// ray direction
	float fov = 45*PI/180; // field of view in degrees
	float tfov = (float)tanf(fov/2); // height/2 of viewing plane
	float ar = WIDTH/(float)HEIGHT; // aspect ratio
	int red, green, blue;
	int redold, greenold, blueold;

	// render picture
	u16* dst = &FrameBuf[(CpuID() == 0) ? 0 : WIDTH];
	y = (CpuID() == 0) ? (HEIGHT-1) : (HEIGHT-2);
	for (; y >= 0; y -= 2)
	{
		x = 0;

		// ray direction vector
		xx = (float)(2*(x + 0.5)/WIDTH - 1) * tfov * ar;
		yy = (float)(2*(y + 0.5)/HEIGHT - 1) * tfov;
		dir.Set(xx, yy, -1);
		dir.Norm();

		// trace this ray
		Trace(&rgbV, Camera, dir, 0, NULL);

		// convert vector to RGB pixel
		tmp = (int)(rgbV.x*256 + 0.5);
		if (tmp < 0) tmp = 0;
		if (tmp > 255) tmp = 255;
		red =  tmp; // red

		tmp = (int)(rgbV.y*256 + 0.5);
		if (tmp < 0) tmp = 0;
		if (tmp > 255) tmp = 255;
		green =  tmp; // green

		tmp = (int)(rgbV.z*256 + 0.5);
		if (tmp < 0) tmp = 0;
		if (tmp > 255) tmp = 255;
		blue =  tmp; // blue

		*dst++ = COLOR(red, green, blue);
		*dst++ = COLOR(red, green, blue);
		redold = red;
		greenold = green;
		blueold = blue;

		for (x = 2; x < WIDTH; x += 2)
		{
			// ray direction vector
			xx = (float)(2*(x + 0.5)/WIDTH - 1) * tfov * ar;
			yy = (float)(2*(y + 0.5)/HEIGHT - 1) * tfov;
			dir.Set(xx, yy, -1);
			dir.Norm();

			// trace this ray
			Trace(&rgbV, Camera, dir, 0, NULL);

			// convert vector to RGB pixel
			tmp = (int)(rgbV.x*256 + 0.5);
			if (tmp < 0) tmp = 0;
			if (tmp > 255) tmp = 255;
			red =  tmp; // red

			tmp = (int)(rgbV.y*256 + 0.5);
			if (tmp < 0) tmp = 0;
			if (tmp > 255) tmp = 255;
			green =  tmp; // green

			tmp = (int)(rgbV.z*256 + 0.5);
			if (tmp < 0) tmp = 0;
			if (tmp > 255) tmp = 255;
			blue =  tmp; // blue

			*dst++ = COLOR((red+redold)>>1, (green+greenold)>>1, (blue+blueold)>>1);
			*dst++ = COLOR(red, green, blue);
			redold = red;
			greenold = green;
			blueold = blue;
		}

		dst += WIDTH;
	}
}
