
// ****************************************************************************
//
//                               3D Vector
//
// ****************************************************************************

#ifndef _VECTOR_H
#define _VECTOR_H

// 3D vector
class V3
{
public:
	
	// vector coordinates
	float x, y, z;

	// constructor
	V3() {};
	V3(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }

	// compare vectors
	inline bool Equ(const V3 &v) { return (x == v.x) && (y == v.y) && (z == v.z); }

	// set value
	void Zero() { x = y = z = 0; }
	void SetX() { x = 1; y = 0; z = 0; }
	void SetY() { x = 0; y = 1; z = 0; }
	void SetZ() { x = 0; y = 0; z = 1; }
	void Set(float xx, float yy, float zz) { x = xx; y = yy; z = zz; }
	void Set(const V3 &v) { x = v.x; y = v.y; z = v.z; }
	V3& operator = (const V3 &v) { x = v.x; y = v.y; z = v.z; return *this; }

	// set normalized vector (set length to 1)
	void SetNorm(const V3 &v)
	{
		float r = (float)sqrtf_fast(v.x*v.x + v.y*v.y + v.z*v.z);
		if (r == 0)
		{
			x = y = 0;
			z = 1;
		}
		else
		{
			r = 1/r;
			x = v.x * r;
			y = v.y * r;
			z = v.z * r;
		}
	}

	// add two vectors
	void Add(float xx, float yy, float zz) { x += xx; y += yy; z += zz; }
	void Add(const V3 &v) { x += v.x; y += v.y; z += v.z; }
	V3 operator + (const V3 &v) const { return V3(x + v.x, y + v.y, z + v.z); }
	V3& operator += (const V3 &v) { x += v.x; y += v.y; z += v.z; return *this; }

	// subtract two vectors
	void Sub(float xx, float yy, float zz) { x -= xx; y -= yy; z -= zz; }
	void Sub(const V3 &v) { x -= v.x; y -= v.y; z -= v.z; }
	V3 operator - (const V3 &v) const { return V3(x - v.x, y - v.y, z - v.z); }
	V3& operator -= (const V3 &v) { x -= v.x; y -= v.y; z -= v.z; return *this; }

	// unary negation, turn vector into inverse direction
	void Inv() { x = -x; y = -y; z = -z; }
	V3 operator - () const { return V3(-x, -y, -z); }

	// multiplication by scalar value
	void Mul(float k) { x *= k; y *= k; z *= k; }
	V3 operator * (const float &k) const { return V3(x*k, y*k, z*k); }
	V3& operator *= (const float &k) { x *= k; y *= k; z *= k; return *this; }

	// division
	void Div(float k) { k = fdiv_fast(1.0f, k); x *= k; y *= k; z *= k; }
	V3 operator / (float k) const { k = fdiv_fast(1.0f, k); return V3(x*k, y*k, z*k); }
	V3& operator /= (float k) { k = fdiv_fast(1.0f, k); x *= k; y *= k; z *= k; return *this; }

	// length of vector
	float Len2() const { return x*x + y*y + z*z; }
	float Len() const { return (float)sqrtf_fast(x*x + y*y + z*z); }

	// distance of two vectors
	float Dist2(const V3 &v) const
	{
		float dx = x - v.x;
		float dy = y - v.y;
		float dz = z - v.z;
		return dx*dx + dy*dy + dz*dz;
	}
	float FASTCODE NOFLASH(Dist)(const V3 &v) const { return (float)sqrtf_fast(Dist2(v)); }

	// normalize vector (set length to 1)
	void Norm()
	{
		float r = (float)sqrtf_fast(x*x + y*y + z*z);
		if (r == 0)
			{ x = 0; y = 0; z = 1; }
		else
			{ r = fdiv_fast(1.0f, r); x *= r; y *= r; z *= r; }
	}

	// scalar (dot) product V1.V2 (result is cosinus of angle of two vectors * lengths of vectors)
	float FASTCODE NOFLASH(Dot)(const V3 &v) const { return x*v.x + y*v.y + z*v.z; }
	float FASTCODE NOFLASH(Angle)(const V3 &v)
	{
		float r = (float)sqrtf_fast(Len2()*v.Len2());
		if (r == 0) return 0;
		return (float)acosf((x*v.x + y*v.y + z*v.z)/r);
	}

	// vector (cross) product V1xV2, multiply two vectors and store result into this vector 
	// (result is perpendicular normal vector to this two vectors)
	void FASTCODE NOFLASH(Mul)(const V3 &v1, const V3 &v2)
	{
		float xx = v1.y * v2.z - v1.z * v2.y;
		float yy = v1.z * v2.x - v1.x * v2.z;
		float zz = v1.x * v2.y - v1.y * v2.x;
		x = xx;
		y = yy;
		z = zz;
	}
	V3 operator * (const V3 &v) const { return V3(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x); }
};

#endif // _VECTOR_H
