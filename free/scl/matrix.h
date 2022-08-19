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

////////////////////////////////////
//	matrix:
//		x1 y1 z1 d1
//		x2 y2 z2 d2
//		x3 y3 z3 d3
//		x4 y4 z4 d4
////////////////////////////////////
class matrix
{
public:
	union 
	{
		//��һ���������У��ڶ����������У�
		//�����3�е�2�� m[3 - 1][2 - 1] = y3;
		float m[4][4];
		struct
		{
			float x1; float y1; float z1; float d1;
			float x2; float y2; float z2; float d2;
			float x3; float y3; float z3; float d3;
			float x4; float y4; float z4; float d4;
		};
	};

	float& operator[] (int index);

	//matrix() { memset(this, 0, sizeof(matrix)); }
	void clear();

	inline void set(
		float _x1, float _y1, float	_z1, float _d1,
		float _x2, float _y2, float _z2, float _d2,
		float _x3, float _y3, float _z3, float _d3,
		float _x4, float _y4, float _z4, float _d4 );

	//inline void set(const float** new_m);


	//����ӷ�
	static void add(const matrix& m1, const matrix& m2, matrix& result);
	static void dec(const matrix& m1, const matrix& m2, matrix& result);
	matrix operator+(const matrix& o);
	matrix operator-(const matrix& o);

	//����˷�
	inline void mul(const matrix& other);

	inline bool operator==(const matrix& other) const;

	////////////////////////////////////////////////////////////
	//static ����
	////////////////////////////////////////////////////////////
	//��ת����
	static matrix& rotate_x(float a) { return rotate_x_radian(radian(a)); }	//�Ƕ���
	static matrix& rotate_y(float a) { return rotate_y_radian(radian(a)); };	//�Ƕ���
	static matrix& rotate_z(float a) { return rotate_z_radian(radian(a)); };	//�Ƕ���
	static matrix& rotate_x_radian(float r);	//������
	static matrix& rotate_y_radian(float r);	//������
	static matrix& rotate_z_radian(float r);	//������
	
	//ƽ�ƣ�������Ϊ������ϵ��������
	//����x�ᣬ��d > 0����ʾ����d����λ(x���Ҳ�Ϊ������)
	static matrix& move(float dx, float dy, float dz);
	static matrix& move_x(float d) { return move(d, 0, 0); }
	static matrix& move_y(float d) { return move(0, d, 0); }
	static matrix& move_z(float d) { return move(0, 0, d); }
	
	//����
	static matrix& scale(float x, float y, float z);

	//��λ����
	static matrix& identity();

	//������v��תangle�Ƕȵľ���
	static matrix& rotate_axis(const vector3& v, float angle);

	//������v = v2 - v1 ��תangle�Ƕȵľ���
	//v1����ʼ�㣬v2�ǽ�����
	static matrix& rotate_any_axis(const vector3& v1, const vector3& v2, float angle);

	//���v1������ת��v2�����ľ���
	static matrix& rotate_between(const vector3& v1, const vector3& v2);

	//��ĳ����Ϊ��׼�㣬��Ԫ������ת����
	static matrix& rotate_pivot_quaternion(const vector3& pivot, const quaternion& q);

	//ͶӰ����
	static void ortho		(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ);
	static void perspective	(scl::matrix& m, float fovy, float aspect, float nearZ, float farZ);
	static void frustum		(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ);
	//static void frustum2	(scl::matrix& m, float left, float right, float bottom, float top, float nearZ, float farZ);

	//���������
	static void lookat		(scl::matrix& result, float posX, float posY, float posZ, float lookAtX, float lookAtY, float lookAtZ, float upX, float upY, float upZ);
	static matrix lookat2	(scl::vector3 pos, scl::vector3 lookAt, scl::vector3 up);

	//�������
	static bool inverse		(scl::matrix& m, scl::matrix& result);
};

////////////////////////////////////
//	
//	Matrixʵ��	
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


inline void matrix::mul(const matrix& other)
{
    matrix result;
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
}


inline bool matrix::operator==(const matrix& other) const
{
	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			if (!float_equal(m[i][j], other.m[i][j], 0.0001f))
			{
				return false;
			}
		}
	}
	return true;
}

} //namespace scl

