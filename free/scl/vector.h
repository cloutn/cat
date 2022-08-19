////////////////////////////////////////////////////////////////////////////////
//	3D vector
//	2010.08.01 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/math.h"
#include "scl/point.h"

namespace scl	{

class vector2
{
public:
	float x; float y;

	bool	equal		(const float _x, const float _y) const { return scl::float_equal(x, _x) && scl::float_equal(y, _y); }
	void	set			(const float _x, const float _y) { x = _x; y = _y; }
	void	set			(const int _x, const int _y) { x = static_cast<float>(_x); y = static_cast<float>(_y); }
	void	clear		() { x = 0; y = 0; }
	bool	is			(const int _x, const int _y) const { return x == _x && y == _y; }
	void	add			(const float _x, const float _y) { x += _x; y += _y; }
	bool	operator==	(const vector2& other) const { return scl::float_equal(x, other.x) && scl::float_equal(y, other.y); }
	bool	operator!=	(const vector2& other) const { return !scl::float_equal(x, other.x) || !scl::float_equal(y, other.y); }

	static const scl::vector2&	zero();
	template <typename StreamerT> void map(StreamerT& s) { s << x << y; }
};

//TODO vector3��Vector�ɷ�ͳһ��
class vector3
{
public:
	float x;
	float y;
	float z;

	//vector3() {}
	//vector3(const float x, const float y, const float z);

	void	clear();

	bool	equal			(const float _x, const float _y, const float _z) const { return scl::float_equal(x, _x) && scl::float_equal(y, _y) && scl::float_equal(z, _z); }
	void	set				(const float x, const float y, const float z);
	void	add				(const float x, const float y, const float z);
	float	length			() const;
	bool	empty			() const { return float_equal(length(), 0); }
	void	normalize		();
	void	mul_matrix		(const matrix& m);
	vector3	cross			(const vector3& v);	
	vector3	dot				(const vector3& v);	

	vector3& 	operator-=	(const vector3& other);
	vector3& 	operator+=	(const vector3& other);
	vector3& 	operator*=	(const float v);
	vector3		operator-	() const;
	vector3		operator-	(const vector3& other) const;
	vector3		operator+	(const vector3& other) const;
	bool		operator==	(const vector3& other) const;
	bool		operator!=	(const vector3& other) const;

	//�л�����
	void		negative	() { x = -x; y = -y; z = -z; }


	//�������
	static vector3&		cross	(const vector3& v1, const vector3& v2);	

	//�������
	static float		dot		(const vector3& v1, const vector3& v2);

	//�����������нǵ�����ֵ(ʹ���������ĵ�˶���)
	static float		cosa	(const vector3& v1, const vector3& v2);

	//�������Ҷ������нǵ�����ֵ
	static float		cosa2	(const vector3& v1, const vector3& v2);

	//�����������ļнǡ��������ƣ�
	static float		angle	(const vector3& v1, const vector3& v2);

	//���Բ�ֵ
	static void			lerp	(const vector3& v1, const vector3& v2, const float t, vector3& result);

	static const vector3& zero() { static vector3 _zero = { 0, 0, 0}; return _zero; }
	static const vector3& one() { static vector3 _one = { 1, 1, 1}; return _one; }

	template <typename StreamerT>
	void map(StreamerT& s)
	{
		s << x << y << z;
	}
};

class vector4
{
public:
	float x; float y; float z;
	float d;

	//vector4				() {}
	//vector4				(const float x, const float y, const float z, const float d = 0);
	//vector4				(const point& from, const point& to);

	void	clear		();
	void	set			(const float x, const float y, const float z, const float d);
	float	length		() const;	//��������
	void	fromPoint	(const point& from, const point& to);
	void	normalize	();
	void	mul			(matrix& m);
	void	div			(const float v);

	vector4 operator-	(const vector4& vDec) const;
	
	//�������
	static vector4&		cross	(const vector4& v1, const vector4& v2);	

	//�������
	static float		dot		(const vector4& v1, const vector4& v2);

	//�����������нǵ�����ֵ(ʹ���������ĵ�˶���)
	static float		cosa	(const vector4& v1, const vector4& v2);

	//�������Ҷ������нǵ�����ֵ
	static float		cosa2	(const vector4& v1, const vector4& v2);	

	//���㷨����
	static vector4&		get_normal_vector(const point& p1, const point&p2, const point& p3); 
};

inline vector3 c_vector3(const float x, const float y, const float z)
{
	vector3 v = { x, y, z };
	return v;
}

class vector3i
{
public:
	int x;
	int y;
	int z;

	void set(const int vx, const int vy, const int vz) { x = vx; y = vy; z = vz;}
	void clear() { x = 0; y = 0; z = 0; }
};

class vector2i
{
public:
	int x;
	int y;

	//vector2i(const int vx, const int vy) : x(vx), y(vy) {} ��ֹ���캯�����Ч����ʧ
	//explicit vector2i(const vector2& v) { x = static_cast<int>(v.x); y = static_cast<int>(v.y); } ��ֹ���캯�����Ч����ʧ

	void	set			(const int vx, const int vy) { x = vx; y = vy; }
	void	clear		() { x = 0; y = 0; }
	bool	is			(const int _x, const int _y) const { return x == _x && y == _y; }
	bool	operator==	(const vector2i& other) const { return x == other.x && y == other.y; }
	bool	operator!=	(const vector2i& other) const { return x != other.x || y != other.y; }

	static const scl::vector2i&	zero();
	template <typename StreamerT> void map(StreamerT& s) { s << x << y; }
};

#define vector3_dec(v1, v2) v1.x - v2.x, v1.y - v2.y, v1.z - v2.z
#define vector3_add(v1, v2) v1.x + v2.x, v1.y + v2.y, v1.z + v2.z

#define vector2_dec(v1, v2) v1.x - v2.x, v1.y - v2.y
#define vector2_add(v1, v2) v1.x + v2.x, v1.y + v2.y

}	//namespace scl

