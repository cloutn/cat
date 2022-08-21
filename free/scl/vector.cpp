////////////////////////////////////////////////////////////////////////////////
//	3D vector
//	2010.08.01 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/vector.h"
#include "scl/assert.h"

#ifdef SCL_ANDROID
#include <stdlib.h>
#else
#include <memory.h>
#endif

#include <math.h>

namespace scl {

////////////////////////////////////
// vector3
////////////////////////////////////


//vector3::vector3(const float x, const float y, const float z)
//: x(x), y(y), z(z)
//{
//}

float vector3::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

void vector3::normalize()
{
	if (float_equal(x, 0) && float_equal(y, 0) && float_equal(z, 0))
		return;
	float len = length(); 
	assert(len > 0);
	x /= len; 
	y /= len; 
	z /= len;
}

vector3& vector3::operator-=( const vector3& other )
{
	this->x -= other.x;
	this->y -= other.y;
	this->z -= other.z;
	return *this;
}

scl::vector3 vector3::operator-(const vector3& other) const
{
	vector3 v = { x - other.x, y - other.y, z - other.z };
	return v;
}

scl::vector3 vector3::operator+(const vector3& other) const
{
	vector3 v = { x + other.x, y + other.y, z + other.z };
	return v;
}

vector3& vector3::operator+=( const vector3& other )
{
	this->x += other.x;
	this->y += other.y;
	this->z += other.z;
	return *this;
}

vector3& vector3::operator*=( const float v )
{
	this->x *= v;
	this->y *= v;
	this->z *= v;
	return *this;
}

vector3 vector3::operator*(const float v)
{
	vector3 r = *this;
	r *= v;
	return r;
}

vector3 vector3::operator-() const
{
	vector3 v = {-x, -y, -z};
	return v;
}

bool vector3::operator==(const vector3& other) const
{
	return float_equal(x, other.x) 
		&& float_equal(y, other.y)
		&& float_equal(z, other.z);
}

bool vector3::operator!=(const vector3& other) const
{
	return !operator==(other);
}

void vector3::mul_matrix( const matrix& m )
{
	vector3 temp = *this;
	x = temp.x * m.x1 + temp.y * m.x2 + temp.z * m.x3 + m.x4;
	y = temp.x * m.y1 + temp.y * m.y2 + temp.z * m.y3 + m.y4;
	z = temp.x * m.z1 + temp.y * m.z2 + temp.z * m.z3 + m.z4;
}

vector3& vector3::cross( const vector3& v1, const vector3& v2 )
{
	//计算方法说明：
	//vector a = (a1, a2, a3)	here as v1(x, y, z)
	//vector b = (b1, b2, b3)	here as v2(x, y, z)
	//a × b = [a2b3 ? a3b2, a3b1 ? a1b3, a1b2 ? a2b1]

	static vector3 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	return result;
}

scl::vector3 vector3::cross(const vector3& v)
{
	scl::vector3 r = cross(*this, v);
	return r;
}

float vector3::dot( const vector3& v1, const vector3& v2 )
{
	//TODO vector.d没有处理？
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

scl::vector3 vector3::dot(const vector3& v)
{
	scl::vector3 r = cross(*this, v);
	return r;
}

float vector3::cosa( const vector3& v1, const vector3& v2 )
{
	//计算叉积
	float dotProduct = vector3::dot(v1, v2);
	float cosa = dotProduct / ( v1.length() * v2.length() );
	return cosa;
}

float vector3::cosa2( const vector3& v1, const vector3& v2 )
{
	vector3 v3 = { v2.x - v1.x, v2.y - v1.y, v2.z - v1.z };
	float l1 = v1.length();
	float l2 = v2.length();
	float l3 = v3.length();
	float cosa = (l1 * l1 + l2 * l2 - l3 * l3) / (2 * l1 * l2);
	return cosa;
}

void vector3::set( const float x, const float y, const float z )
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void vector3::add( const float x, const float y, const float z )
{
	this->x += x;
	this->y += y;
	this->z += z;
}

void vector3::clear()
{
	memset(this, 0, sizeof(vector3));
}

float vector3::angle(const vector3& v1, const vector3& v2)
{
	return acosf(cosa(v1, v2));
}

void vector3::lerp(const vector3& v1, const vector3& v2, const float t, vector3& result)
{
	result.x = lerpf(v1.x, v2.x, t);
	result.y = lerpf(v1.y, v2.y, t);
	result.z = lerpf(v1.z, v2.z, t);
}

////////////////////////////////////
// vector4
////////////////////////////////////
//vector4::vector4()
//{
//	x = 0; y = 0; z = 0;
//	d = 0;
//}

//vector4::vector4( const float x, const float y, const float z, const float d ) //d = 0
//{
//	this->x = x;
//	this->y = y;
//	this->z = z;
//	this->d = d;
//}
//
//vector4::vector4( const point& from, const point& to )
//{
//	fromPoint(from, to);
//}

float vector4::length() const
{
	return sqrtf(x * x + y * y + z * z);
}

void vector4::fromPoint( const point& from, const point& to )
{
	x = to.x - from.x;
	y = to.y - from.y;
	z = to.z - from.z;

	//TODO d应该怎么处理?
	d = to.d - from.d;
}

vector4& vector4::cross( const vector4& v1, const vector4& v2 )
{
	//计算方法说明：
	//vector a = (a1, a2, a3)	here as v1(x, y, z)
	//vector b = (b1, b2, b3)	here as v2(x, y, z)
	//a × b = [a2b3 ? a3b2, a3b1 ? a1b3, a1b2 ? a2b1]

	static vector4 result;
	result.x = v1.y * v2.z - v1.z * v2.y;
	result.y = v1.z * v2.x - v1.x * v2.z;
	result.z = v1.x * v2.y - v1.y * v2.x;
	result.d = 0;
	return result;
}

float vector4::dot( const vector4& v1, const vector4& v2 )
{
	//TODO vector.d没有处理？
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

float vector4::cosa( const vector4& v1, const vector4& v2 )
{
	//计算叉积
	float dotProduct = dot(v1, v2);
	float cosa = dotProduct / ( v1.length() * v2.length() );
	return cosa;
	//return Math::Angle(acos(cosa));
}

float vector4::cosa2( const vector4& v1, const vector4& v2 )
{
	vector4 v3 = v2 - v1;
	float l1 = v1.length();
	float l2 = v2.length();
	float l3 = v3.length();
	float cosa = (l1 * l1 + l2 * l2 - l3 * l3) / (2 * l1 * l2);
	return cosa;
}

vector4 vector4::operator-( const vector4& vDec ) const
{
	vector4 v;
	v.x = x - vDec.x;
	v.y = y - vDec.y;
	v.z = z - vDec.z;
	v.d = 0;
	return v;
}

void vector4::normalize()
{
	float len = length();
	x /= len;
	y /= len;
	z /= len;
}

void vector4::clear()
{
	memset(this, 0, sizeof(vector4));
}

void vector4::set( const float x, const float y, const float z, const float d)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->d = d;
}

void vector4::mul( matrix& m )
{
	vector4 temp;
	temp.x = x * m.x1 + y * m.x2 + z * m.x3 + d * m.x4;
	temp.y = x * m.y1 + y * m.y2 + z * m.y3 + d * m.y4;
	temp.z = x * m.z1 + y * m.z2 + z * m.z3 + d * m.z4;
	temp.d = x * m.d1 + y * m.d2 + z * m.d3 + d * m.d4;
	*this = temp;
}

void vector4::div(const float v)
{
	if (v == 0)
		return;
	x /= v;
	y /= v;
	z /= v;
	d /= v;
}

vector4& vector4::get_normal_vector( const point& p1, const point&p2, const point& p3 )
{
	vector4 v1;
	v1.fromPoint( p1, p2 );
	vector4 v2;
	v2.fromPoint( p2, p3 );
	vector4& q = vector4::cross(v1, v2);
	q.normalize();
	return q;
}



const scl::vector2& vector2::zero()
{
	static const scl::vector2 _zero = { 0, 0 };
	return _zero;
}


const scl::vector2i& vector2i::zero()
{
	static const scl::vector2i _zero = { 0, 0 };
	return _zero;
}

}	//namespace scl



