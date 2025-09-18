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

//TODO vector3和Vector可否统一？
class vector3
{
public:
	float x;
	float y;
	float z;

	//vector3() {}
	//vector3(const float x, const float y, const float z);

	void	clear();

	bool	equal	 		(const float _x, const float _y, const float _z, const float epsilon = MATH_FLOAT_EPSILON) const { return scl::float_equal(x, _x, epsilon) && scl::float_equal(y, _y, epsilon) && scl::float_equal(z, _z, epsilon); }
	bool	equal	 		(const vector3& other, const float epsilon = MATH_FLOAT_EPSILON) const { return equal(other.x, other.y, other.z, epsilon); }
	void	set				(const float x, const float y, const float z);
	void	add				(const float x, const float y, const float z);
	float	length			() const;
	float	length_sqr		() const;
	float	magnitude		() const { return length(); }
	float	magnitude_sqr	() const { return length_sqr(); }
	bool	empty			() const { return float_equal(length(), 0); }
	bool	is_zero			() const { return empty(); }
	void	normalize		();
	bool	is_normalized	() const;
	void	mul_matrix		(const matrix& m);
	vector3	cross			(const vector3& v);	
	float	dot				(const vector3& v) const;	
	vector3 absolute		() const ;

	vector3& 	operator-=	(const vector3& other);
	vector3& 	operator+=	(const vector3& other);
	vector3& 	operator*=	(const float v);
	vector3 	operator*	(const float v);
	vector3		operator-	() const;
	vector3		operator-	(const vector3& other) const;
	vector3		operator+	(const vector3& other) const;
	bool		operator==	(const vector3& other) const;
	bool		operator!=	(const vector3& other) const;
	vector3		operator*	(const matrix& m) const;

	//切换正负
	void		negative	() { x = -x; y = -y; z = -z; }


	//向量叉乘
	static vector3&		cross	(const vector3& v1, const vector3& v2);	

	//向量点乘
	static float		dot		(const vector3& v1, const vector3& v2);

	//求两个向量夹角的余弦值(使用了向量的点乘定义)
	static float		cosa	(const vector3& v1, const vector3& v2);

	//利用余弦定理计算夹角的余弦值
	static float		cosa2	(const vector3& v1, const vector3& v2);

	//求两个向量的夹角。（弧度制）
	static float		angle	(const vector3& v1, const vector3& v2);

	//线性插值
	static void			lerp	(const vector3& v1, const vector3& v2, const float t, vector3& result);

	static const vector3& zero			() { static vector3 _zero = { 0, 0, 0}; return _zero; }
	static const vector3& one			() { static vector3 _one = { 1, 1, 1}; return _one; }
	static const vector3& infinity		() { static vector3 _infinity = { SCL_FLOAT_INFINITY(), SCL_FLOAT_INFINITY(), SCL_FLOAT_INFINITY() }; return _infinity; }
	static const vector3& neg_infinity	() { static vector3 _neg_infinity = { SCL_FLOAT_NEG_INFINITY(), SCL_FLOAT_NEG_INFINITY(), SCL_FLOAT_NEG_INFINITY() }; return _neg_infinity; }

	template <typename StreamerT>
	void map(StreamerT& s)
	{
		s << x << y << z;
	}

	const float* value_ptr() const { return &(this->x); }
	float* value_ptr() { return &(this->x); }
};

class vector4
{
public:
	union {
		float x; 
		float r;
	};
	union {
		float y; 
		float g;
	};
	union {
		float z;
		float b;
	};
	union {
		float w;
		float a;
	};

	//vector4				() {}
	//vector4				(const float x, const float y, const float z, const float d = 0);
	//vector4				(const point& from, const point& to);

	void	clear		();
	void	set			(const float x, const float y, const float z, const float d);
	float	length		() const;	//向量长度
	//void	fromPoint	(const point& from, const point& to);
	void	normalize	();
	void	mul			(matrix& m);
	void	div			(const float v);

	vector4 operator-	(const vector4& vDec) const;
	
	//向量叉乘
	static vector4&		cross	(const vector4& v1, const vector4& v2);	

	//向量点乘
	static float		dot		(const vector4& v1, const vector4& v2);

	//求两个向量夹角的余弦值(使用了向量的点乘定义)
	static float		cosa	(const vector4& v1, const vector4& v2);

	//利用余弦定理计算夹角的余弦值
	static float		cosa2	(const vector4& v1, const vector4& v2);	

	//计算法向量
	//static vector4&		get_normal_vector(const point& p1, const point&p2, const point& p3); 
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

	//vector2i(const int vx, const int vy) : x(vx), y(vy) {} 防止构造函数造成效率损失
	//explicit vector2i(const vector2& v) { x = static_cast<int>(v.x); y = static_cast<int>(v.y); } 防止构造函数造成效率损失

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


//角度转为弧度
inline vector3 radian(const vector3& v)
{
	return { radian(v.x), radian(v.y), radian(v.z) };
}

//弧度转为角度
inline vector3 angle(const vector3& v)
{
	return { angle(v.x), angle(v.y), angle(v.z) };
}

}	//namespace scl

