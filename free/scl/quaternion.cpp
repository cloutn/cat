////////////////////////////////////////////////////////////////////////////////
//	四元数
//	
//	2010.12.01 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/math.h"

#include "scl/quaternion.h"
#include "scl/matrix.h"
#include "scl/vector.h"
#include "scl/assert.h"

#include <math.h>

namespace scl {

void quaternion::from_pivot_radian(const vector3& v, const float angle)
{
	vector3 nv = v;
	nv.normalize();
	float sina = ::sinf(angle / 2);
	x = sina * nv.x;
	y = sina * nv.y;
	z = sina * nv.z;
	w = ::cosf(angle / 2);
}

void quaternion::from_vector4(const vector4& v)
{
	vector3 nv = {v.x, v.y, v.z};
	nv.normalize();
	float sina = ::sinf(v.w / 2);
	x = sina * nv.x;
	y = sina * nv.y;
	z = sina * nv.z;
	w = ::cosf(v.w / 2);
}

void quaternion::to_matrix(matrix& matrix) const
{
	matrix.x1 = 1 - 2*(y*y + z*z);
	matrix.y1 = 2*(x*y + w*z);
	matrix.z1 = 2*(x*z - w*y);
	matrix.d1 = 0;

	matrix.x2 = 2*(x*y - w*z);
	matrix.y2 = 1 - 2*(x*x + z*z);
	matrix.z2 = 2*(y*z + w*x);
	matrix.d2 = 0;

	matrix.x3 = 2*(w*y + x*z);
	matrix.y3 = 2*(y*z - w*x);
	matrix.z3 = 1 - 2*(x*x + y*y);
	matrix.d3 = 0;

	matrix.x4 = 0;
	matrix.y4 = 0;
	matrix.z4 = 0;
	matrix.d4 = 1;
}

//!!!the quaternion must have been normalized!!!
void quaternion::to_vector4(vector4& v) const
{
	//!!!the quaternion must have been normalized!!!

	float sqrLength = x*x + y*y + z*z;
	if (sqrLength > 0) 
	{
		float invLength = 1 / ::sqrtf(sqrLength);
		v.x = x * invLength;
		v.y = y * invLength;
		v.z = z * invLength;
		if (w <= 1.0f)
		{
			v.w = 2.0f * ::acosf(w);
		}
		else
		{
			v.w = 0;
		}
	}
	else 
	{
		v.w = 0;
		v.x = 1;
		v.y = 0;
		v.z = 0;
	}
}

void quaternion::normalize()
{
	float n = ::sqrtf(x*x + y*y + z*z + w*w);
	x /= n;
	y /= n;
	z /= n;
	w /= n;
}

bool quaternion::operator==( const quaternion& other ) const
{
	return scl::float_equal(x, other.x) &&
		scl::float_equal(y, other.y) &&
		scl::float_equal(z, other.z) &&
		scl::float_equal(w, other.w);
}

bool quaternion::operator!=( const quaternion& other ) const
{
	return	!scl::float_equal(x, other.x) || 
			!scl::float_equal(y, other.y) || 
			!scl::float_equal(z, other.z) || 
			!scl::float_equal(w, other.w);
}

float quaternion::dot(const quaternion& q1, const quaternion& q2)
{
	return q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;
}

void quaternion::lerp(const quaternion& q1, const quaternion& q2, const float t, quaternion& result)
{
	result.x = q1.x + t * (q2.x - q1.x);	
	result.y = q1.y + t * (q2.y - q1.y);	
	result.z = q1.z + t * (q2.z - q1.z);	
	result.w = q1.w + t * (q2.w - q1.w);	
}

scl::quaternion quaternion::inverse(const quaternion& q)
{
	quaternion i = q;
	i.inverse();
	return i;
}

void quaternion::slerp(const quaternion& _q0, const quaternion& _q1, const float t, quaternion& result)
{
	quaternion q0 = _q0;
	quaternion q1 = _q1;
	float cosa = quaternion::dot(q0, q1);
	if (cosa < 0)
	{
		q1.reverse();
		cosa = -cosa;
	}

	if (cosa > 0.9999f)
	{
		// angle a is nearly 0, use linear interpolation instead.
		quaternion::lerp(q0, q1, t, result);
	}
	else 
	{
		// Assume q_result = k0 * q0 + k1 * q1, so we need to caculate k0 and k1.
		float sina			= sqrtf(1 - cosa * cosa);
		float a				= atan2f(sina, cosa);
		float one_div_sina	= 1 / sina;
		float k0			= sinf((1 - t) * a) * one_div_sina;
		float k1			= sinf(t * a) * one_div_sina;

		result.x			= k0 * q0.x + k1 * q1.x;
		result.y 			= k0 * q0.y + k1 * q1.y;
		result.z 			= k0 * q0.z + k1 * q1.z;
		result.w 			= k0 * q0.w + k1 * q1.w;
	}
}

void quaternion::clear()
{
	x = y = z = 0;
	w = 1;
}

void quaternion::from_euler_radian(const float _x, const float _y, const float _z)
{
	const float cosx = cosf(_x/2);
	const float sinx = sinf(_x/2);
	const float cosy = cosf(_y/2);
	const float siny = sinf(_y/2);
	const float cosz = cosf(_z/2);
	const float sinz = sinf(_z/2);

	//x = sinx*cosy*cosz + cosx*siny*sinz;
	//y = cosx*siny*cosz - sinx*cosy*sinz;
	//z = cosx*cosy*sinz - sinx*siny*cosz;
	//w = cosx*cosy*cosz + sinx*siny*sinz;

	x = sinx*cosy*cosz - cosx*siny*sinz;
	y = cosx*siny*cosz + sinx*cosy*sinz;
	z = cosx*cosy*sinz - sinx*siny*cosz;
	w = cosx*cosy*cosz + sinx*siny*sinz;

	// xyz order
    // x = s1 * c2 * c3 - c1 * s2 * s3,
    // y = c1 * s2 * c3 + s1 * c2 * s3,
    // z = c1 * c2 * s3 - s1 * s2 * c3
    // w = c1 * c2 * c3 + s1 * s2 * s3,

	// zyx order
    // x = s1 * c2 * c3 + c1 * s2 * s3,
    // y = c1 * s2 * c3 - s1 * c2 * s3,
    // z = c1 * c2 * s3 + s1 * s2 * c3,
    // w = c1 * c2 * c3 - s1 * s2 * s3,
}


void quaternion::to_euler_radian(float& _x, float& _y, float& _z) const
{
	// 算法：
	// 从 quaternion::to_matrix		能得到矩阵和四元数的 xyzw 的关系
	// 从 matrix::rotate_xyz_radian 能得到矩阵和欧拉角 xyz 关系
	// 对比两个矩阵，能得到如下关系:
	// z1 = -siny =  2*(x*z - w*y);
	// z2 / z3 = sinx/cosx = 2*(y*z + w*x) / (1 - 2*(x*x + y*y)) //注意 cosy 不能为0
	// y1 / x1 = sinz/cosz = 2*(x*y + w*z) / (1 - 2*(y*y + z*z)) //注意 cosy 不能为0

	const float epsilon = 0.000001f;

	float mat_z1 = -2*(x*z - w*y);
	_y = asin(mat_z1);

	float mat_z2 = 2*(y*z + w*x); 
	float mat_z3 = 1 - 2*(x*x + y*y);
	if (scl::float_equal(mat_z2, 0, epsilon) && scl::float_equal(mat_z3, 0, epsilon)) // glm code : quaternion.inl:34
		_x =  2 * atan2(x, w);
	else
		_x = atan2(mat_z2, mat_z3);

	float mat_y1 = 2*(x*y + w*z);
	float mat_x1 = 1 - 2*(y*y + z*z);
	if (scl::float_equal(mat_y1, 0, epsilon) && scl::float_equal(mat_x1, 0, epsilon)) // glm code : quaternion.inl:21
		_z =  0;
	else
		_z = atan2(mat_y1, mat_x1);

	//float ay	= 2 * (w*z + x*y);
	//float ax	= 1 - 2 * (z*z + x*x);
	//float b		= 2 * (w*x - y*z);
	//float cy	= 2 * (w*y + z*x);
	//float cx	= 1 - 2 * (x*x+y*y);
	//
	//_z = atan2f(ay, ax);
	//_x = asinf(b);
	//_y = atan2f(cy, cx);
}

void quaternion::to_euler_radian(scl::vector3& v) const
{
	to_euler_radian(v.x, v.y, v.z);
}

void quaternion::from_euler_angle(const float x, const float y, const float z)
{
	from_euler_radian(radian(x), radian(y), radian(z));
}

void quaternion::from_matrix(const scl::matrix& m)
{
	quaternion& q = *this;

	using T = float;
	T fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
	T fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
	T fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
	T fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

	int biggestIndex = 0;
	T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
	if(fourXSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourXSquaredMinus1;
		biggestIndex = 1;
	}
	if(fourYSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourYSquaredMinus1;
		biggestIndex = 2;
	}
	if(fourZSquaredMinus1 > fourBiggestSquaredMinus1)
	{
		fourBiggestSquaredMinus1 = fourZSquaredMinus1;
		biggestIndex = 3;
	}

	T biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<T>(1)) * static_cast<T>(0.5);
	T mult = static_cast<T>(0.25) / biggestVal;

	switch(biggestIndex)
	{
	case 0:
		{
			q.x = (m[1][2] - m[2][1]) * mult; 
			q.y = (m[2][0] - m[0][2]) * mult;
			q.z = (m[0][1] - m[1][0]) * mult;
			q.w = biggestVal;
		}
		break;
	case 1:
		{
			q.x = biggestVal; 
			q.y = (m[0][1] + m[1][0]) * mult; 
			q.z = (m[2][0] + m[0][2]) * mult;
			q.w = (m[1][2] - m[2][1]) * mult; 
		}
		break;
	case 2:
		{
			q.x = (m[0][1] + m[1][0]) * mult;
			q.y = biggestVal;
			q.z = (m[1][2] + m[2][1]) * mult;
			q.w = (m[2][0] - m[0][2]) * mult;
		}
		break;
	case 3:
		{
			q.w = (m[0][1] - m[1][0]) * mult;
			q.x = (m[2][0] + m[0][2]) * mult;
			q.y = (m[1][2] + m[2][1]) * mult;
			q.z = biggestVal;
		}
		break;
	default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
		assert(false);
		q = {  0, 0, 0, 1 };
		break;
	}
}

void quaternion::from_matrix2(const scl::matrix& a)
{
	// https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
	quaternion& q = *this;
	float trace = a[0][0] + a[1][1] + a[2][2]; // I removed + 1.0f; see discussion with Ethan https://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/ethan.htm
	if (trace > 0) // I changed M_EPSILON to 0
	{
		float s = 0.5f / sqrtf(trace + 1.0f);
		q.w = 0.25f / s;
		q.x = (a[1][2] - a[2][1]) * s;
		q.y = (a[2][0] - a[0][2]) * s;
		q.z = (a[0][1] - a[1][0]) * s;
	}
	else 
	{
		if (a[0][0] > a[1][1] && a[0][0] > a[2][2]) 
		{
			float s = 2.0f * sqrtf(1.0f + a[0][0] - a[1][1] - a[2][2]);
			q.w = (a[1][2] - a[2][1]) / s;
			q.x = 0.25f * s;
			q.y = (a[1][0] + a[0][1]) / s;
			q.z = (a[2][0] + a[0][2]) / s;
		}
		else if (a[1][1] > a[2][2]) 
		{
			float s = 2.0f * sqrtf(1.0f + a[1][1] - a[0][0] - a[2][2]);
			q.w = (a[2][0] - a[0][2]) / s;
			q.x = (a[1][0] + a[0][1]) / s;
			q.y = 0.25f * s;
			q.z = (a[2][1] + a[1][2]) / s;
		}
		else 
		{
			float s = 2.0f * sqrtf(1.0f + a[2][2] - a[0][0] - a[1][1]);
			q.w = (a[0][1] - a[1][0]) / s;
			q.x = (a[2][0] + a[0][2]) / s;
			q.y = (a[2][1] + a[1][2]) / s;
			q.z = 0.25f * s;
		}
	}
}

void quaternion::to_euler_angle(float& _x, float& _y, float& _z) const
{
	to_euler_radian(_x, _y, _z);
	_x = angle(_x);
	_y = angle(_y);
	_z = angle(_z);
}

void quaternion::to_euler_angle(scl::vector3& v) const
{
	to_euler_angle(v.x, v.y, v.z);
}

}	//namespace scl

