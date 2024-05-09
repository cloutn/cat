////////////////////////////////////////////////////////////////////////////////
//	顶点
//	
//	2010.12.02 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"
#include "scl/matrix.h"
#include "scl/vector.h"
#include "scl/bitset.h"

namespace cat {

class VertexAttr;

//class vertex
//{
//public:
//	float x, y, z;
//	float nx, ny, nz;
//	float u, v;
//
//	//TODO 干掉vertex的构造函数
//	//vertex(float ix, float iy, float iz, float inx = 0, float iny = 0, float inz = 0, float iu = 0, float iv = 0);
//
//	void set(
//		float ix, float iy, float iz, 
//		float inx = 0, float iny = 0, float inz = 0, float iu = 0, float iv = 0);
//
//	void clear();
//	void mulMatrix(const scl::matrix& m);
//
//	vertex& operator=(const scl::vector3& v);
//	vertex& operator+=(const scl::vector3& v);
//};

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
	scl::vector3	position;
	uint32			color;

	static int get_attr(VertexAttr* attrs, const int capacity);
};

class vertex_color_uv
{
public:
	float	x, y, z;
	uint32	color;
	float	u, v;

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
	void mul_matrix	(const scl::matrix& m);
	void clear		()								{ x = 0; y = 0; z = 0; color = 0; u = 0; v = 0; }

	static const VertexAttr*	get_attr();
	static int					get_attr_count() { return 3; }

};

//enum VERTEX_SLOT
//{
//	VERTEX_SLOT_POSITION,
//	VERTEX_SLOT_NORMAL,
//	VERTEX_SLOT_TANGENT,
//	VERTEX_SLOT_COLOR,
//	VERTEX_SLOT_TEXCOORD_0,
//	VERTEX_SLOT_TEXCOORD_1,
//	VERTEX_SLOT_TEXCOORD_2,
//	VERTEX_SLOT_TEXCOORD_3,
//	VERTEX_SLOT_JOINTS,
//	VERTEX_SLOT_WEIGHTS,
//
//	VERTEX_SLOT_COUNT,
//};
//
////enum ShaderChannel
////{
////    kShaderChannelNone = -1,
////    kShaderChannelVertex = 0,   // Vertex (vector3)
////    kShaderChannelNormal,       // Normal (vector3)
////    kShaderChannelTangent,      // Tangent (vector4)
////    kShaderChannelColor,        // Vertex color
////    kShaderChannelTexCoord0,    // Texcoord 0
////    kShaderChannelTexCoord1,    // Texcoord 1
////    kShaderChannelTexCoord2,    // Texcoord 2
////    kShaderChannelTexCoord3,    // Texcoord 3
////    kShaderChannelTexCoord4,    // Texcoord 4
////    kShaderChannelTexCoord5,    // Texcoord 5
////    kShaderChannelTexCoord6,    // Texcoord 6
////    kShaderChannelTexCoord7,    // Texcoord 7
////    kShaderChannelBlendWeights, // Blend weights
////    kShaderChannelBlendIndices, // Blend indices
////    kShaderChannelCount,        // Keep this last!
////};
//
//template <int SIZE>
//class base_vertex
//{
//public:
//	void set_position(float x, float y, float z)
//	{
//		
//	}
//
//private:
//	//scl::bitset<VERTEX_SLOT_COUNT> m_bits;
//	float m_data[SIZE];
//};

}  // namespace cat

