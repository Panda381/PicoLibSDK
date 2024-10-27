
// ****************************************************************************
//
//                               3D Sphere object
//
// ****************************************************************************

#ifndef _SPHERE_H
#define _SPHERE_H

// sphere object
class Sphere
{
public:

	// sphere position (center)
	V3 pos;

	// sphere radius and radius^2
	float rad, rad2;

	// diffuse color (components 0..1)
	V3 col;

	// reflection (0..1)
	float ref;

	// transparency (0..1)
	float trans;

	// constructor
	Sphere() {}
	Sphere(const V3 &position, float radius, const V3 &color, float reflection = 0, float transparency = 0)
	{
		pos = position;
		rad = radius;
		rad2 = radius*radius;
		col = color;
		ref = reflection;
		trans = transparency;
	}

	// compute ray-sphere intersection (returns TRUE on success)
	//  orig = ray origin
	//  dir = ray direction
	//  OUTPUT t1 = distance of first intersection
	//  OUTPUT t2 = distance of second intersection
	bool FASTCODE NOFLASH(Intersect)(const V3 &orig, const V3 &dir, float* t1, float* t2) const
	{
		// direction vector to sphere center
		V3 l = pos - orig;

		// distance to center of intersection
		float tca = l.Dot(dir);

		// ray goes to oposide side, no intersection
		if (tca <= 0) return false;

		// distance from sphere center to the ray
		float d2 = l.Len2() - tca*tca;

		// ray is not intersecting the sphere
		if (d2 >= rad2) return false;

		// half length of intersection
		float thc = (float)sqrtf(rad2 - d2);

		// distance of intersections
		*t1 = tca - thc;
		*t2 = tca + thc;

		return true;
	}
};

#endif // _SPHERE_H
