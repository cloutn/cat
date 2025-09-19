////////////////////////////////////////////////////////////////////////////////
//	3D matrix
//	2010.08.01 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/math.h"

namespace scl {

class vector3;
class vector4;
class quaternion;

// NDC space Z Range
enum class z_range
{
	negative_one_to_one	= 0,
	one_to_negative_one	= 1,
	zero_to_one			= 2,
	one_to_zero			= 3,
};

////////////////////////////////////
//	matrix:
//		x1 y1 z1 d1
//		x2 y2 z2 d2
//		x3 y3 z3 d3
//		x4 y4 z4 d4
//
//	use row major storage. (like directX math lib)
//
////////////////////////////////////
class matrix
{
public:
	union 
	{
		//第一个括号是行，第二个括号是列，
		//例如第3行第2个 m[3 - 1][2 - 1] = y3;
		float m[4][4];
		struct
		{
			float x1; float y1; float z1; float d1;
			float x2; float y2; float z2; float d2;
			float x3; float y3; float z3; float d3;
			float x4; float y4; float z4; float d4;
		};
	};

	const float*	operator[]				(int index) const;
	float*			operator[]				(int index);
	const float*	ptr						() const { return &m[0][0]; }
	float*			ptr						() { return &m[0][0]; }

	void			clear					();

	inline void		set						(
		float _x1, float _y1, float	_z1, float _d1,
		float _x2, float _y2, float _z2, float _d2,
		float _x3, float _y3, float _z3, float _d3,
		float _x4, float _y4, float _z4, float _d4);

	//矩阵加法
	static void		add						(const matrix& m1, const matrix& m2, matrix& result);
	static void		dec						(const matrix& m1, const matrix& m2, matrix& result);
	matrix			operator+				(const matrix& o) const;
	matrix			operator-				(const matrix& o) const;

	//矩阵乘法
	inline matrix&	mul						(const matrix& other);
	inline matrix&	mul						(const matrix& m1, const matrix& m2);
	inline matrix&	mul						(const matrix& m1, const matrix& m2, const matrix& m3);
	static matrix	mul_all					(const matrix& m1, const matrix& m2);
	static matrix	mul_all					(const matrix& m1, const matrix& m2, const matrix& m3);
	static matrix	mul_all					(const matrix& m1, const matrix& m2, const matrix& m3, const matrix& m4);

	inline bool		operator==				(const matrix& other) const;
	matrix			operator*				(const matrix& other) const;

	////////////////////////////////////////////////////////////
	//static 函数
	////////////////////////////////////////////////////////////
	//旋转矩阵
	static matrix	rotate_x				(float a) { return rotate_x_radian(radian(a)); }		//角度制
	static matrix	rotate_y				(float a) { return rotate_y_radian(radian(a)); };	//角度制
	static matrix	rotate_z				(float a) { return rotate_z_radian(radian(a)); };	//角度制
	static matrix	rotate_xyz				(float x, float y, float z) { return rotate_xyz_radian(radian(x), radian(y), radian(z)); }	//角度制
	static matrix	rotate_xzy				(float x, float y, float z) { return rotate_xzy_radian(radian(x), radian(y), radian(z)); }	//角度制
	static matrix	rotate_yxz				(float x, float y, float z) { return rotate_yxz_radian(radian(x), radian(y), radian(z)); }	//角度制
	static matrix	rotate_yzx				(float x, float y, float z) { return rotate_yzx_radian(radian(x), radian(y), radian(z)); }	//角度制
	static matrix	rotate_zxy				(float x, float y, float z) { return rotate_zxy_radian(radian(x), radian(y), radian(z)); }	//角度制
	static matrix	rotate_zyx				(float x, float y, float z) { return rotate_zyx_radian(radian(x), radian(y), radian(z)); }	//角度制
	static matrix	rotate_x_radian			(float r);	//弧度制
	static matrix	rotate_y_radian			(float r);	//弧度制
	static matrix	rotate_z_radian			(float r);	//弧度制
	static matrix	rotate_xyz_radian		(float x, float y, float z);	//弧度制
	static matrix	rotate_xzy_radian		(float x, float y, float z);	//弧度制
	static matrix	rotate_yxz_radian		(float x, float y, float z);	//弧度制
	static matrix	rotate_yzx_radian		(float x, float y, float z);	//弧度制
	static matrix	rotate_zxy_radian		(float x, float y, float z);	//弧度制
	static matrix	rotate_zyx_radian		(float x, float y, float z);	//弧度制
	
	//平移，正方向为各坐标系的正半轴
	//例如x轴，当d > 0，表示右移d个单位(x轴右侧为正方向)
	static matrix	move					(float dx, float dy, float dz);
	static matrix	move_x					(float d) { return move(d, 0, 0); }
	static matrix	move_y					(float d) { return move(0, d, 0); }
	static matrix	move_z					(float d) { return move(0, 0, d); }
	void			set_move				(float x, float y, float z);

	static matrix	translate				(float dx, float dy, float dz) { return move(dx, dy, dz); }
	
	//缩放
	static matrix	scale					(float x, float y, float z);

	//单位矩阵
	static matrix	identity				();

	//绕向量v旋转angle角度的矩阵
	static matrix	rotate_axis				(const vector3& v, float angle);

	//绕向量v = v2 - v1 旋转angle角度的矩阵
	//v1是起始点，v2是结束点
	static matrix	rotate_any_axis			(const vector3& v1, const vector3& v2, float angle);

	//求从v1向量旋转到v2向量的矩阵
	static matrix	rotate_between			(const vector3& v1, const vector3& v2);

	//以某个点为基准点，四元数做旋转参数
	static matrix	rotate_pivot_quaternion	(const vector3& pivot, const quaternion& q);

	//投影矩阵
	static void		perspective				(scl::matrix& m, float fovy, float aspect, float near_z, float far_z, z_range _z_range = z_range::negative_one_to_one);
	static matrix	perspective				(float fovy, float aspect, float near_z, float far_z, z_range _z_range = z_range::negative_one_to_one);
	static void		frustum					(matrix& m, float l, float r, float b, float t, float n, float f, z_range _z_range);
	static matrix	frustum					(float l, float r, float b, float t, float n, float f, z_range _z_range) { matrix m; frustum(m, l, r, b, t, n, f, _z_range); return m;}
	static void		frustum_infinite		(matrix& m, float l, float r, float b, float t, float n, z_range _z_range);
	static matrix	frustum_infinite		(float l, float r, float b, float t, float n, z_range _z_range) { matrix m; frustum_infinite(m, l, r, b, t, n, _z_range); return m;}

	// focus_z is used to determine the Z distance to the camera in otho, if you dont' care it, set it to near_z.
	static void		ortho					(scl::matrix& m, float fovy, float aspect, float near_z, float far_z, float focus_z, z_range _z_range = z_range::negative_one_to_one);
	static matrix	ortho					(float fovy, float aspect, float near_z, float far_z, float focus_z, z_range _z_range = z_range::negative_one_to_one);
	static void		volume					(scl::matrix& m, float left, float right, float bottom, float top, float near_z, float far_z, z_range _z_range);
	static matrix	volume					(float left, float right, float bottom, float top, float near_z, float far_z, z_range _z_range) { matrix m; volume(m, left, right, bottom, top, near_z, far_z, _z_range); return m;}

	//摄像机矩阵
	static void		lookat					(scl::matrix& result, float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ);
	static matrix	lookat					(float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ);
	static matrix	lookat2					(scl::vector3 pos, scl::vector3 lookAt, scl::vector3 up);
	static void		decompose_lookat		(const scl::matrix& lookatMatrix, float& posX, float& posY, float& posZ, float& lookAtX, float& lookAtY, float& lookAtZ, float& upX, float& upY, float& upZ);

	//求逆矩阵
	static bool		inverse					(scl::matrix& m, scl::matrix& result);

	//
	static vector3	extract_move			(const scl::matrix& m);
	vector3			extract_move			();

	//求一个矩阵对应的缩放、旋转、平移
	static bool		decompose				(const scl::matrix& m, scl::vector3* translate, scl::vector3* scale, scl::vector3* rotateEuler, scl::matrix* rotateMatrix, scl::quaternion* rotateQuaternion);

	// 求解一个旋转矩阵的欧拉角表示，注意，当 y = 90 的时候，decompose 可能无法正常工作，参见 glm 和 directX math 都没有处理该问题
	static void		decompose_rotation_xyz_radian(const scl::matrix& m, scl::vector3& euler);
	static void		decompose_rotation_xyz	(const scl::matrix& m, scl::vector3& euler);
};

////////////////////////////////////
//	
//	Matrix实现	
//	
////////////////////////////////////
inline void matrix::set(
	float _x1, float _y1, float	_z1, float _d1,
	float _x2, float _y2, float _z2, float _d2,
	float _x3, float _y3, float _z3, float _d3,
	float _x4, float _y4, float _z4, float _d4 )
{
	x1 = _x1; y1 = _y1; z1 = _z1; d1 = _d1;
	x2 = _x2; y2 = _y2; z2 = _z2; d2 = _d2;
	x3 = _x3; y3 = _y3; z3 = _z3; d3 = _d3;
	x4 = _x4; y4 = _y4; z4 = _z4; d4 = _d4;
}


inline matrix& matrix::mul(const matrix& other)
{
	matrix result = { 0 };
    for (int row = 0; row < 4; ++row)
    {
        for (int col = 0; col < 4; ++col)
        {
            result.m[row][col] =
            m[row][0] * other.m[0][col]
            + m[row][1] * other.m[1][col]
            + m[row][2] * other.m[2][col]
            + m[row][3] * other.m[3][col];
        }
    }
    *this = result;
	return *this;
}

inline matrix& matrix::mul(const matrix& m1, const matrix& m2)
{
	mul(m1);
	mul(m2);
	return *this;
}


inline matrix& matrix::mul(const matrix& m1, const matrix& m2, const matrix& m3)
{
	mul(m1);
	mul(m2);
	mul(m3);
	return *this;
}

inline bool matrix::operator==(const matrix& other) const
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (!float_equal(m[i][j], other.m[i][j], MATH_FLOAT_EPSILON))
			{
				return false;
			}
		}
	}
	return true;
}

} //namespace scl

