////////////////////////////////////////////////////////////////////////////////
//	顶点
//	
//	2010.12.02 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/matrix.h"
#include "scl/vector.h"

namespace cat {

class vertex
{
public:
	float x, y, z;
	float nx, ny, nz;
	float u, v;

	//TODO 干掉vertex的构造函数
	//vertex(float ix, float iy, float iz, float inx = 0, float iny = 0, float inz = 0, float iu = 0, float iv = 0);

	void set(
		float ix, float iy, float iz, 
		float inx = 0, float iny = 0, float inz = 0, float iu = 0, float iv = 0);

	void clear();
	void mulMatrix(const scl::matrix& m);

	vertex& operator=(const scl::vector3& v);
	vertex& operator+=(const scl::vector3& v);
};

class vertex_uv
{
public:
	float x, y, z;
	float u, v;

	void set(
		float ix,	float iy,	float iz, 
		float iu,	float iv)
	{
		x = ix; y = iy; z = iz;
		u = iu; v = iv;
	}

	void clear() { x = 0; y = 0; z = 0; u = 0; v = 0; }
};

class vertex_color
{
public:
	scl::vector3 position;
	uint32	color;
};

class vertex_coord
{
public:
	scl::vector4	position;
	uint32			color;
	scl::vector2	texcoord;
};

class vertex_color_uv
{
public:
	float x, y, z;
	uint32	color;
	float u, v;

	//static const uint32 FVF = (FVF_XYZ | FVF_DIFFUSE | FVF_TEX1);

	void set(
		float	_x,
		float	_y,	
		float	_z, 
		uint32	_color,
		float	_u,
		float	_v)
	{
		x		= _x; 
		y 		= _y; 
		z 		= _z;
		color	= _color;
		u		= _u; 
		v 		= _v;
	}

	void set(
		int		_x,
		int		_y,	
		int		_z, 
		uint32	_color,
		float	_u,
		float	_v)
	{
		set(static_cast<float>(_x), static_cast<float>(_y), static_cast<float>(_z), _color, _u, _v);	
	}

	void set_xyz	(float _x, float _y, float _z)	{ x = _x; y = _y; z = _z; }
	void set_xy		(float _x, float _y)			{ x = _x; y = _y; }
	void set_color	(uint32 _color)					{ color = _color; }
	void set_uv		(float _u, float _v)			{ u = _u; v = _v; }
	//void set_index	(uint8 _index)					{ index = _index; }
	void mul_matrix	(const scl::matrix& m);
	void clear		()								{ x = 0; y = 0; z = 0; color = 0; u = 0; v = 0; }
};


}  // namespace cat

