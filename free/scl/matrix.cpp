////////////////////////////////////////////////////////////////////////////////
//	3D matrix
//	2010.08.01 caolei
////////////////////////////////////////////////////////////////////////////////
#include "scl/matrix.h"

#include "scl/vector.h"
#include "scl/quaternion.h"
#include "scl/assert.h"

#ifdef SCL_ANDROID
#include <stdlib.h>
#else
#include <memory.h>
#endif

#include <math.h>

namespace scl {

//绕向量v旋转angle角度的矩阵
matrix matrix::rotate_axis(const vector3& v, float angle)
{
	matrix result;
	quaternion q = { 0 };
	q.from_pivot_radian(v, angle);
	q.to_matrix(result);
	return result;
}

//绕向量v = v2 - v1 旋转angle角度的矩阵
//v1是起始点，v2是结束点
matrix matrix::rotate_any_axis(const vector3& v1, const vector3& v2, float angle)
{
	matrix result;

	//先平移到以v1.x v1,y v1.z为原点的坐标系
	result = matrix::move(-v1.x, -v1.y, -v1.z);

	//绕v2 - v1旋转angle角度
	vector3 temp = { vector3_dec(v2, v1) };
	vector3 vq = { temp.x, temp.y, temp.z };
	quaternion q = { 0 };
	q.from_pivot_radian(vq, angle);
	matrix rotateAxis;
	q.to_matrix(rotateAxis);
	result.mul(rotateAxis);

	//平移回原来的坐标系
	result.mul(matrix::move(v1.x, v1.y, v1.z));

	return result;
}

//求从v1向量旋转到v2向量的矩阵
matrix matrix::rotate_between(const vector3& from_v1, const vector3& to_v2)
{
	matrix result;
	vector3 v1 = from_v1;
	vector3 v2 = to_v2;
	v1.normalize();
	v2.normalize();

	if (v1 == v2 || v1.empty() || v2.empty())
	{
		result = matrix::identity();
		return result;
	}

	//求出旋转轴
	vector3& axis = vector3::cross(v1, v2);

	//求出旋转角度
	float a = vector3::angle(v1, v2);

	//利用旋转轴pivot和旋转角度acos(cosa)计算旋转四元数
	quaternion q = { 0 };
	q.from_pivot_radian(axis, a);

	//利用四元数生成旋转矩阵
	q.to_matrix(result);

	return result;
}

matrix matrix::rotate_pivot_quaternion(const vector3& pivot, const quaternion& q)
{
	matrix transform;
	transform = matrix::move(-pivot.x, -pivot.y, -pivot.z);
	matrix rotateMatrix;
	q.to_matrix(rotateMatrix);
	transform.mul(rotateMatrix);
	transform.mul(matrix::move(pivot.x, pivot.y, pivot.z));
	return transform;
}

const float* matrix::operator[](int i) const
{
	assert(i < 4);
	const float* _a = m[i];
	return _a;
}

float* matrix::operator[](int i)
{
	assert(i < 4);
	float* _a = m[i];
	return _a;
}

void matrix::clear()
{
	memset(this, 0, sizeof(matrix));
}

void matrix::add(const matrix& m1, const matrix& m2, matrix& result)
{
	assert(&result != &m1 && &result != &m2);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] = m1.m[i][j] + m2.m[i][j];
}

void matrix::dec(const matrix& m1, const matrix& m2, matrix& result)
{
	assert(&result != &m1 && &result != &m2);
	for (int i = 0; i < 4; ++i)
		for (int j = 0; j < 4; ++j)
			result.m[i][j] = m1.m[i][j] - m2.m[i][j];
}

matrix matrix::operator+(const matrix& o) const
{
	matrix r;
	matrix::add(*this, o, r);
	return r;
}

matrix matrix::operator-(const matrix& o) const
{
	matrix r;
	matrix::dec(*this, o, r);
	return r;
}


matrix matrix::mul_all(const matrix& m1, const matrix& m2)
{
	matrix m = m1;
	m.mul(m2);
	return m;
}

matrix matrix::mul_all(const matrix& m1, const matrix& m2, const matrix& m3)
{
	matrix m = m1;
	m.mul(m2, m3);
	return m;
}

matrix matrix::mul_all(const matrix& m1, const matrix& m2, const matrix& m3, const matrix& m4)
{
	matrix m = m1;
	m.mul(m2, m3, m4);
	return m;
}

matrix matrix::operator*(const matrix& o) const
{
	matrix r = *this;
	r.mul(o);
	return r;
}

matrix matrix::identity()
{
	//这里三层外三层的括号是为了消除linux的警告！
	matrix m = { { {
		{ 1,	0,	0,	0 },
		{ 0,	1,	0,	0 },
		{ 0,	0,	1,	0 },
		{ 0,	0,	0,	1 }
	} } };

	//matrix m;
	//m.set(
	//	1,	0,	0,	0,
	//	0,	1,	0,	0,
	//	0,	0,	1,	0,
	//	0,	0,	0,	1);
	return m;
}


matrix matrix::move(float dx, float dy, float dz)
{
	matrix m = { 0 };
	m.set(
		1,	0,	0,	0,
		0,	1,	0,	0,
		0,	0,	1,	0,
		dx,	dy,	dz,	1
		);
	return m;
}

void matrix::set_move(float x, float y, float z)
{
	x4 = x;
	y4 = y;
	z4 = z;
}

matrix matrix::scale(float x, float y, float z)
{
	matrix m = { 0 };
	m.set
		(
		x,	0,	0,	0,
		0,	y,	0,	0,
		0,	0,	z,	0,
		0,	0,	0,	1
		);
	return m;
}

matrix matrix::rotate_x_radian(float a)
{
	matrix m = { 0 };
	float cosa = cosf(a);
	float sina = sinf(a);
	m.set
		(
		//xz为地平面，左手坐标系
		1,		0,		0,		0, 
		0,		cosa,	sina,	0,
		0,		-sina,	cosa,	0,
		0,		0,		0,		1
		);
	return m;
}

matrix matrix::rotate_y_radian(float a)
{
	matrix m = { 0 };
	float cosa = cosf(a);
	float sina = sinf(a);
	m.set
		(
		cosa,	0,	-sina,	0,
		0,		1,	0,		0,
		sina,	0,	cosa,	0,
		0,		0,	0,		1
		);								
	return m;
}

matrix matrix::rotate_z_radian(float a)
{
	matrix m = { 0 };
	float cosa = cosf(a);
	float sina = sinf(a);
	m.set
		(
		cosa,	sina,	0,	0,
		-sina,	cosa,	0,	0,
		0,		0,		1,	0,
		0,		0,		0,	1
		);
	return m;
}

matrix matrix::rotate_xyz_radian(float x, float y, float z)
{
	matrix m = { 0 };

	float cosx = cosf(x);
	float sinx = sinf(x);
	float cosy = cosf(y);
	float siny = sinf(y);
	float cosz = cosf(z);
	float sinz = sinf(z);

	m.set(
		cosy*cosz,						cosy*sinz,						-siny,			0,
		sinx*siny*cosz - cosx*sinz,		sinx*siny*sinz + cosx*cosz,		sinx*cosy,		0,
		cosx*siny*cosz + sinx*sinz,		cosx*siny*sinz - sinx*cosz, 	cosx*cosy,		0,
		0,								0,								0,				1);
	return m;
}

matrix matrix::rotate_xzy_radian(float x, float y, float z)
{
	matrix m = { 0 };

	float cosx = cosf(x);
	float sinx = sinf(x);
	float cosy = cosf(y);
	float siny = sinf(y);
	float cosz = cosf(z);
	float sinz = sinf(z);

	m.set(
		cosz*cosy,						sinz,							-cosz*siny,						0,
		-sinz*cosx*cosy + sinx*siny,	cosx*cosz,						sinz*cosx*siny + sinx*cosy,		0,
		sinx*sinz*cosy + cosx*siny,		-sinx*cosz,						-sinx*siny*sinz + cosx*cosy,	0,
		0,								0,								0,								1);
	return m;
}

matrix matrix::rotate_yxz_radian(float x, float y, float z)
{
	matrix m = { 0 };

	float cosx = cosf(x);
	float sinx = sinf(x);
	float cosy = cosf(y);
	float siny = sinf(y);
	float cosz = cosf(z);
	float sinz = sinf(z);

	m.set(
		cosy*cosz - sinx*siny*sinz,		cosy*sinz + sinx*siny*cosz,		-siny*cosx,			0,
		-cosx*sinz,						cosx*cosz,						sinx,				0,
		siny*cosz + sinx*cosy*sinz,		siny*sinz - sinx*cosy*cosz,		cosx*cosy,			0,
		0,								0,								0,					1);

	return m;
}

matrix matrix::rotate_yzx_radian(float x, float y, float z)
{
	matrix m = { 0 };

	float cosx = cosf(x);
	float sinx = sinf(x);
	float cosy = cosf(y);
	float siny = sinf(y);
	float cosz = cosf(z);
	float sinz = sinf(z);

	m.set(
		cosy*cosz,		cosy*sinz*cosx + sinx*siny,		cosy*sinz*sinx - siny*cosx,			0,
		-sinz,			cosz*cosx,						cosz*sinx,							0,
		siny*cosz,		siny*sinz*cosx - cosy*sinx,		sinx*siny*sinz + cosx*cosy,			0,
		0,				0,								0,									1);

	return m;
}

matrix matrix::rotate_zxy_radian(float x, float y, float z)
{
	matrix m = { 0 };

	float cosx = cosf(x);
	float sinx = sinf(x);
	float cosy = cosf(y);
	float siny = sinf(y);
	float cosz = cosf(z);
	float sinz = sinf(z);

	m.set(
		cosy*cosz+sinx*siny*sinz,		cosx*sinz,		-siny*cosz + sinx*cosy*sinz,		0,
		-sinz*cosy + sinx*siny*cosz,	cosx*cosz,		siny*sinz + sinx*cosy*cosz,			0,
		cosx*siny,						-sinx,			cosx*cosy,							0,
		0,								0,				0,									1);

	return m;
}

matrix matrix::rotate_zyx_radian(float x, float y, float z)
{
	matrix m = { 0 };

	float cosx = cosf(x);
	float sinx = sinf(x);
	float cosy = cosf(y);
	float siny = sinf(y);
	float cosz = cosf(z);
	float sinz = sinf(z);

	m.set(
		cosy*cosz,		sinx*siny*cosz + cosx*sinz,		-cosx*siny*cosz + sinx*sinz,		0,
		-cosy*sinz,		-sinx*siny*sinz + cosx*cosz,	cosx*siny*sinz + sinx*cosz,			0,
		siny,			-sinx*cosy,						cosx*cosy,							0,
		0,				0,								0,									1);

	return m;
}

matrix matrix::ortho(float fovy, float aspect, float nearZ, float farZ, float focus_z, z_range nearFarMapping)
{
	matrix m;
	ortho(m, fovy, aspect, nearZ, farZ, focus_z, nearFarMapping);
	return m;
}

void matrix::ortho(matrix& m, float fovy, float aspect, float nearZ, float farZ, float focus_z, z_range nearFarMapping)
{
	float frustumHeight = tanf(fovy / 360.0f * PI) * focus_z;
	float frustumWidth = frustumHeight * aspect;
	matrix::volume(m, -frustumWidth, frustumWidth, -frustumHeight, frustumHeight, nearZ, farZ, nearFarMapping);
}

//matrix matrix::volume(float left, float right, float bottom, float top, float nearZ, float farZ)
//{
//	matrix m;
//	volume(m, left, right, bottom, top, nearZ, farZ);
//	return m;
//}

void matrix::volume(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ, z_range nearFarMapping)
{
   float dx = right - left;
   float dy = top - bottom;
   float dz = farZ - nearZ;
   float f = farZ;
   float n = nearZ;

   if (dx == 0.0f || dy == 0.0f || dz == 0.0f)
      return;

	float A = 0;
	float B = 0;

	switch (nearFarMapping)
	{
	case z_range::negative_one_to_one:
		{
			A = 2.0f / (n - f);
			B = (n + f) / (n - f);
		}
		break;
	case z_range::one_to_negative_one:
		{
			A = -2.0f / (n - f);
			B =	-(n + f) / (n - f);
		}
		break;
	case z_range::zero_to_one:
		{
			A = 1.0f / (n - f);
			B = n / (n - f);
		}
		break;
	case z_range::one_to_zero:
		{
			A = -1.0f / (n - f);
			B = -f / (n - f);
		}
		break;
	default: assert(false); 
		break;
	}

   m.set(
		2.0f / dx,				0,						0,		0,
		0,						2.0f / dy,				0, 		0,
		0,						0,						A,		0,		
		-(right + left) / dx,	-(top + bottom) / dy,	B,		1);
}

//void matrix::volume(matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
//{
//   float dx = right - left;
//   float dy = top - bottom;
//   float dz = farZ - nearZ;
//   float f = farZ;
//   float n = nearZ;
//
//   if (dx == 0.0f || dy == 0.0f || dz == 0.0f)
//      return;
//
//   m.set(
//		2.0f / dx,				0,						0,						0,
//		0,						2.0f / dy,				0, 						0,
//		0,						0,						-2.0f / (f - n),				0,		
//		-(right + left) / dx,	-(top + bottom) / dy,	-(f + n) / (f - n),	1);
//
//}

//void matrix::volume_one_to_negative_one(matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
//{
//   float dx = right - left;
//   float dy = top - bottom;
//   float dz = farZ - nearZ;
//   float f = farZ;
//   float n = nearZ;
//
//   if (dx == 0.0f || dy == 0.0f || dz == 0.0f)
//      return;
//
//   m.set(
//		2.0f / dx,				0,						0,						0,
//		0,						2.0f / dy,				0, 						0,
//		0,						0,						-2.0f / (n - f),				0,		
//		-(right + left) / dx,	-(top + bottom) / dy,	-(n + f) / (n - f),	1);
//}

//void matrix::volume_zero_to_one(matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
//{
//   float dx = right - left;
//   float dy = top - bottom;
//   float dz = farZ - nearZ;
//   float f = farZ;
//   float n = nearZ;
//
//   if (dx == 0.0f || dy == 0.0f || dz == 0.0f)
//      return;
//
//   m.set(
//		2.0f / dx,				0,						0,						0,
//		0,						2.0f / dy,				0, 						0,
//		0,						0,						1.0f / (n - f),			0,		
//		-(right + left) / dx,	-(top + bottom) / dy,	n / (n - f),			1);
//}


//void matrix::volume_one_to_zero(matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
//{
//   float dx = right - left;
//   float dy = top - bottom;
//   float dz = farZ - nearZ;
//   float f = farZ;
//   float n = nearZ;
//
//   if (dx == 0.0f || dy == 0.0f || dz == 0.0f)
//      return;
//
//   m.set(
//		2.0f / dx,				0,						0,						0,
//		0,						2.0f / dy,				0, 						0,
//		0,						0,						1.0f / (f - n),			0,		
//		-(right + left) / dx,	-(top + bottom) / dy,	f / (f - n),			1);
//}

matrix matrix::perspective(float fovy, float aspect, float nearZ, float farZ, z_range nearFarMapping)
{
	matrix m;
	perspective(m, fovy, aspect, nearZ, farZ, nearFarMapping);
	return m;
}

void matrix::perspective(matrix& m, float fovy, float aspect, float nearZ, float farZ, z_range nearFarMapping)
{
   float frustumH = tanf ( fovy / 360.0f * PI ) * nearZ;
   float frustumW = frustumH * aspect;
   matrix::frustum(m, -frustumW, frustumW, -frustumH, frustumH, nearZ, farZ, nearFarMapping);
}

//void matrix::frustum(matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ)
//{
//   float dx = right - left;
//   float dy = top - bottom;
//   float dz = farZ - nearZ;
//
//   if (dx <= 0.0f || dy <= 0.0f || dz <= 0.0f)
//      return;
//
//   m.set(
//		2.0f * nearZ / dx,		0,						0,							0,
//		0,						2.0f * nearZ / dy,		0,							0,
//		2 * (right + left) / dx, 2 * (top + bottom) / dy,	-(nearZ + farZ) / dz,		-1.0f,
//		0,						0,						2.0f * nearZ * farZ / dz,	0);
//}

//matrix matrix::frustum(float l, float r, float b, float t, float n, float f)
//{
//	matrix m;
//	frustum(m, l, r, b, t, n, f);
//	return m;
//}

//void matrix::frustum(matrix& m, float l, float r, float b, float t, float n, float f)
//{
//   if (r <= l || t <= b || f <= n)
//      return;
//
//   m.set(
//		2 * n / (r - l),		0,						0,							0,
//		0,						2 * n / (t - b),		0,							0,
//		(r + l) / (r - l),		(t + b) / (t - b),		-(n + f) / (f - n),		-1.0f,
//		0,						0,						-2 * n * f / (f - n),		0);
//}


// https://www.songho.ca/opengl/gl_projectionmatrix.html
void matrix::frustum(matrix& m, float l, float r, float b, float t, float n, float f, z_range nearFarMapping)
{
	if (r <= l || t <= b || f <= n)
		return;

	float A = 0;
	float B = 0;

	switch (nearFarMapping)
	{
	case z_range::negative_one_to_one:
		{
			A = -(f + n) / (f - n);
			B = -2 * n * f / (f - n);
		}
		break;
	case z_range::one_to_negative_one:
		{
			A = (f + n) / (f - n);
			B = 2 * n * f / (f - n);
		}
		break;
	case z_range::zero_to_one:
		{
			A = f / (n - f);
			B = n * f / (n - f);
		}
		break;
	case z_range::one_to_zero:
		{
			A = n / (f - n);
			B = n * f / (f - n);
		}
		break;
	default:
		assert(false);
		break;
	}

	m.set(
		2 * n / (r - l),		0,						0,		0,
		0,						2 * n / (t - b),		0,		0,
		(r + l) / (r - l),		(t + b) / (t - b),		A,		-1.0f,
		0,						0,						B,		0);
}

// Infinite perspective projection matrix
// https://www.songho.ca/opengl/gl_projectionmatrix.html
void matrix::frustum_infinite(matrix& m, float l, float r, float b, float t, float n, z_range nearFarMapping)
{
	if (r <= l || t <= b || n <= 0)
		return;

	float A = 0;
	float B = 0;

	// When far plane is set to infinity, the projection matrix can be simplified
	switch (nearFarMapping)
	{
	case z_range::negative_one_to_one:
		{
			A = -1.0f;
			B = -2.0f * n;
		}
		break;
	case z_range::one_to_negative_one:
		{
			A = 1.0f;
			B = 2.0f * n;
		}
		break;
	case z_range::zero_to_one:
		{
			A = 0.0f;
			B = -n;
		}
		break;
	case z_range::one_to_zero:
		{
			A = 0.0f;
			B = n;
		}
		break;
	default:
		assert(false);
		break;
	}

	m.set(
		2 * n / (r - l),		0,						0,		0,
		0,						2 * n / (t - b),		0,		0,
		(r + l) / (r - l),		(t + b) / (t - b),		A,		-1.0f,
		0,						0,						B,		0);
}

//void matrix::frustum_one_to_zero(matrix& m, float l, float r, float b, float t, float n, float f)
//{
//	if (r <= l || t <= b || f <= n)
//		return;
//
//	m.set(
//		2 * n / (r - l),		0,						0,							0,
//		0,						2 * n / (t - b),		0,							0,
//		(r + l) / (r - l),		(t + b) / (t - b),		n / (f - n),				-1.0f,
//		0,						0,						n * f / (f - n),			0);
//}
//
//void matrix::frustum_zero_to_one(matrix& m, float l, float r, float b, float t, float n, float f)
//{
//	if (r <= l || t <= b || f <= n)
//		return;
//
//	m.set(
//		2 * n / (r - l),		0,						0,							0,
//		0,						2 * n / (t - b),		0,							0,
//		(r + l) / (r - l),		(t + b) / (t - b),		f / (n - f),				-1.0f,
//		0,						0,						n * f / (n - f),			0);
//}
//
//void matrix::frustum_one_to_negative_one(matrix& m, float l, float r, float b, float t, float n, float f)
//{
//   m.set(
//		2 * n / (r - l),		0,						0,							0,
//		0,						2 * n / (t - b),		0,							0,
//		(r + l) / (r - l),		(t + b) / (t - b),		(f + n) / (f - n),		-1.0f,
//		0,						0,						2 * n * f / (f - n),		0);
//}

matrix matrix::lookat(float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
{
	matrix result;
	lookat(result, posX, posY, posZ, lookAtX, lookAtY, lookAtZ, upX, upY, upZ);
	return result;
}

void matrix::lookat(matrix& result, float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ)
{
	float axisX[3] = { 0 };
	float axisY[3] = { 0 };
	float axisZ[3] = { 0 };
	float length = 0;

	// axisZ = lookAt - pos
	axisZ[0] = lookAtX - posX;
	axisZ[1] = lookAtY - posY;
	axisZ[2] = lookAtZ - posZ;

	// normalize axisZ
	length = sqrtf(axisZ[0] * axisZ[0] + axisZ[1] * axisZ[1] + axisZ[2] * axisZ[2]);

	if (length != 0.0f)
	{
		axisZ[0] /= length;
		axisZ[1] /= length;
		axisZ[2] /= length;
	}

	// axisX = up X axisZ
	axisX[0] = upY * axisZ[2] - upZ * axisZ[1];
	axisX[1] = upZ * axisZ[0] - upX * axisZ[2];
	axisX[2] = upX * axisZ[1] - upY * axisZ[0];

	// normalize axisX
	length = sqrtf(axisX[0] * axisX[0] + axisX[1] * axisX[1] + axisX[2] * axisX[2]);

	if (length != 0.0f)
	{
		axisX[0] /= length;
		axisX[1] /= length;
		axisX[2] /= length;
	}

	// axisY = axisZ x axisX
	axisY[0] = axisZ[1] * axisX[2] - axisZ[2] * axisX[1];
	axisY[1] = axisZ[2] * axisX[0] - axisZ[0] * axisX[2];
	axisY[2] = axisZ[0] * axisX[1] - axisZ[1] * axisX[0];

	// normalize axisY
	length = sqrtf(axisY[0] * axisY[0] + axisY[1] * axisY[1] + axisY[2] * axisY[2]);

	if (length != 0.0f)
	{
		axisY[0] /= length;
		axisY[1] /= length;
		axisY[2] /= length;
	}

	result.clear();

	result.m[0][0] = -axisX[0];
	result.m[0][1] = axisY[0];
	result.m[0][2] = -axisZ[0];

	result.m[1][0] = -axisX[1];
	result.m[1][1] = axisY[1];
	result.m[1][2] = -axisZ[1];

	result.m[2][0] = -axisX[2];
	result.m[2][1] = axisY[2];
	result.m[2][2] = -axisZ[2];

	// translate (-posX, -posY, -posZ)
	result.m[3][0] = axisX[0] * posX + axisX[1] * posY + axisX[2] * posZ;
	result.m[3][1] = -axisY[0] * posX - axisY[1] * posY - axisY[2] * posZ;
	result.m[3][2] = axisZ[0] * posX + axisZ[1] * posY + axisZ[2] * posZ;
	result.m[3][3] = 1.0f;
}

matrix matrix::lookat2(scl::vector3 eye, scl::vector3 target, scl::vector3 upDir)
{
	// code snippet from http://www.songho.ca/opengl/gl_camera.html

    // compute the forward vector from target to eye
    scl::vector3 forward = eye - target;
    forward.normalize();                 // make unit length

    // compute the left vector
    scl::vector3 left = upDir.cross(forward); // cross product
    left.normalize();

    // recompute the orthonormal up vector
    scl::vector3 up = forward.cross(left);    // cross product

    // set translation part
    float transX = -left.x * eye.x - left.y * eye.y - left.z * eye.z;
    float transY = -up.x * eye.x - up.y * eye.y - up.z * eye.z;
    float transZ = -forward.x * eye.x - forward.y * eye.y - forward.z * eye.z;

    // init 4x4 matrix
	matrix matrix = 
	{
		left.x,		up.x,		forward.x,	0,
		left.y,		up.y,		forward.y,	0,
		left.z,		up.z,		forward.z,	0,
		transX,		transY, 	transZ,		1
	};

    //// set rotation part, inverse rotation matrix: M^-1 = M^T for Euclidean transform

	return matrix;
}

void matrix::decompose_lookat(const scl::matrix& result, float& posX, float& posY, float& posZ, float& lookAtX, float& lookAtY, float& lookAtZ, float& upX, float& upY, float& upZ)
{
	scl::vector3 foward;
	scl::vector3 left;
	scl::vector3 up;

	left.x = result.m[0][0];
	left.y = result.m[1][0];
	left.z = result.m[2][0];

	up.x = result.m[0][1];
	up.y = result.m[1][1];
	up.z = result.m[2][1];

	foward.x = result.m[0][2];
	foward.y = result.m[1][2];
	foward.z = result.m[2][2];

	// camera view rotate is orthogonal (which each column has unit length and perpendicular to the other column.)
	// so the inverse matrix is equal to its transpose matrix
	// 旋转矩阵是正交矩阵，所以逆矩阵和转置矩阵相同
	scl::matrix rotateInverse = {
		left.x,		left.y,		left.z,		0,
		up.x,		up.y,		up.z,		0,
		foward.x,	foward.y,	foward.z,	0,
		0,			0,			0,			1
	};

	scl::matrix transform = result;
	transform.mul(rotateInverse);

	posX = -transform.x4;
	posY = -transform.y4;
	posZ = -transform.z4;

	lookAtX = posX - foward.x;
	lookAtY = posY - foward.y;
	lookAtZ = posZ - foward.z;

	upX = up.x;
	upY = up.y;
	upZ = up.z;
}

// code snippet from http://www.songho.ca/opengl/gl_camera.html
//typedef matrix Matrix4;
//typedef scl::vector3 Vector3;
//
//Matrix4 lookAt3(Vector3& eye, Vector3& target, Vector3& upDir)
//{
//    // compute the forward vector from target to eye
//    Vector3 forward = eye - target;
//    forward.normalize();                 // make unit length
//
//    // compute the left vector
//    Vector3 left = upDir.cross(forward); // cross product
//    left.normalize();
//
//    // recompute the orthonormal up vector
//    Vector3 up = forward.cross(left);    // cross product
//
//    // init 4x4 matrix
//    Matrix4 matrix;
//    matrix.identity();
//
//    // set rotation part, inverse rotation matrix: M^-1 = M^T for Euclidean transform
//    matrix[0] = left.x;
//    matrix[4] = left.y;
//    matrix[8] = left.z;
//    matrix[1] = up.x;
//    matrix[5] = up.y;
//    matrix[9] = up.z;
//    matrix[2] = forward.x;
//    matrix[6] = forward.y;
//    matrix[10]= forward.z;
//
//    // set translation part
//    matrix[12]= -left.x * eye.x - left.y * eye.y - left.z * eye.z;
//    matrix[13]= -up.x * eye.x - up.y * eye.y - up.z * eye.z;
//    matrix[14]= -forward.x * eye.x - forward.y * eye.y - forward.z * eye.z;
//
//    return matrix;
//}

bool matrix::inverse(matrix& m, matrix& result)
{
	matrix leftM = m;
	matrix rightM = matrix::identity();
	int i, j;
	for (i = 0; i < 4; ++i)
	{
		int rowmaxpos = i;
		for (j = i + 1; j < 4; ++j)
		{
			if (scl::absolute(leftM.m[j][i]) > scl::absolute(leftM.m[rowmaxpos][i]))
				rowmaxpos = j;
		}
		if (rowmaxpos != i)
		{
			for (j = 0; j < 4; ++j)//按从大到小的顺序排列矩阵
			{
				//swap
				float temp = leftM.m[rowmaxpos][j];
				leftM.m[rowmaxpos][j] = leftM.m[i][j];
				leftM.m[i][j] = temp;
				//swap
				temp = rightM.m[rowmaxpos][j];
				rightM.m[rowmaxpos][j] = rightM.m[i][j];
				rightM.m[i][j] = temp;
			}
		}
		float divisor = leftM.m[i][i];
		//assert(!scl::float_equal(divisor, 0.0f, 0.00000001f));//must be can-inverse matrix
		if (scl::float_equal(divisor, 0.0f, 0.00000001f))//must be can-inverse matrix
			return false;

		for (j = 0; j < 4; ++j)//归一化矩阵
		{
			leftM.m[i][j] /= divisor;
			rightM.m[i][j] /= divisor;
		}
		for (j = 0; j < 4; ++j)//高斯消元法处理行列式
		{
			if (j == i)
				continue;
			float multiple = leftM.m[j][i];
			for (int k = 0; k < 4; ++k)
			{
				leftM.m[j][k] -= leftM.m[i][k] * multiple;
				rightM.m[j][k] -= rightM.m[i][k] * multiple;
			}
		}
	}
	result = rightM;
	return true;
}


vector3 matrix::extract_move(const scl::matrix& m)
{
	return { m.x4, m.y4, m.z4 };
}

vector3 matrix::extract_move()
{
	return extract_move(*this);
}

void matrix::decompose_rotation_xyz_radian(const matrix& m, scl::vector3& euler)
{
	euler.y		= atan2(-m.z1, sqrt(m.x1*m.x1 + m.y1*m.y1));
	float cosy	= cosf(euler.y);
	if (scl::float_equal(cosy, 0, 0.00001f))
	{
		// sinx*cosz - cosx*sinz,   sinx*sinz + cosx*cosz
		// cosx*cosz + sinx*sinz,	cosx*sinz - sinx*cosz

		//euler.x = atan2(m[2][1], m[1][1]);	// https://learnopencv.com/rotation-matrix-to-euler-angles/
		euler.x = atan2f(-m[2][0], m[1][1]);	// glm code : matrix_decompose.inl:146
		euler.z = 0; 
	}
	else
	{
		euler.x		= atan2f(m[1][2], m[2][2]);
		euler.z		= atan2f(m[0][1], m[0][0]);
	}
}

void matrix::decompose_rotation_xyz(const matrix& m, scl::vector3& eulerAngle)
{
	vector3 radian;
	decompose_rotation_xyz_radian(m, radian);
	eulerAngle = scl::angle(radian);
}


bool matrix::decompose(const matrix& m, scl::vector3* translate, scl::vector3* scale, scl::vector3* rotateEuler, matrix* rotateMatrix, scl::quaternion* rotateQuaternion)
{
	if (NULL != translate)
		*translate = { m.x4, m.y4, m.z4 };

	vector3 svx = { m.x1, m.y1, m.z1 };
	float sx = svx.length();

	vector3 svy = { m.x2, m.y2, m.z2 };
	float sy = svy.length();

	vector3 svz = { m.x3, m.y3, m.z3 };
	float sz = svz.length();

	vector3 s = { sx, sy, sz };
	if (NULL != scale)
		*scale = s;

	matrix matRotate = 
	{
		m.x1/s.x,		m.y1/s.x,		m.z1/s.x,		0,
		m.x2/s.y,		m.y2/s.y,		m.z2/s.y,		0,
		m.x3/s.z,		m.y3/s.z,		m.z3/s.z,		0,
		0,				0,				0,				1,
	};

	if (NULL != rotateMatrix)
		*rotateMatrix = matRotate;

	if (NULL != rotateQuaternion)
		rotateQuaternion->from_matrix(matRotate);

	if (NULL != rotateEuler)
		decompose_rotation_xyz(matRotate, *rotateEuler);

	return true;

}

} //namespace scl
