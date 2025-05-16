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

//	ShaderChannel
//	{
//	    None = -1,
//	    Vertex = 0,   // Vertex (vector3)
//	    Normal,       // Normal (vector3)
//	    Tangent,      // Tangent (vector4)
//	    Color,        // Vertex color
//	    TexCoord0,    // Texcoord 0
//	    TexCoord1,    // Texcoord 1
//	    TexCoord2,    // Texcoord 2
//	    TexCoord3,    // Texcoord 3
//	    TexCoord4,    // Texcoord 4
//	    TexCoord5,    // Texcoord 5
//	    TexCoord6,    // Texcoord 6
//	    TexCoord7,    // Texcoord 7
//	    BlendWeights, // Blend weights
//	    BlendIndices, // Blend indices
//	    Count,        // Keep this last!
//	};
//

//int _attrNameToIndex(const char* const name)
//{
//	int attrIndex = -1;
//	if		(0 == _stricmp(name, "POSITION"		))	attrIndex = 0;
//	else if (0 == _stricmp(name, "NORMAL"		))	attrIndex = 1;
//	else if (0 == _stricmp(name, "TANGENT"		))	attrIndex = 2;
//	else if (0 == _stricmp(name, "TEXCOORD_0"	))	attrIndex = 3;
//	else if (0 == _stricmp(name, "TEXCOORD_1"	))	attrIndex = 4;
//	else if (0 == _stricmp(name, "COLOR_0"		))	attrIndex = 5;
//	else if (0 == _stricmp(name, "JOINTS_0"		))	attrIndex = 6;
//	else if (0 == _stricmp(name, "WEIGHTS_0"	))	attrIndex = 7;
//	return attrIndex;
//}


// vertex attribute names
enum VERTEX_ATTR
{
	VERTEX_ATTR_INVALID = -1,
	VERTEX_ATTR_POSITION,
	VERTEX_ATTR_NORMAL,
	VERTEX_ATTR_TANGENT,
	VERTEX_ATTR_TEXCOORD0,
	VERTEX_ATTR_TEXCOORD1,
	VERTEX_ATTR_COLOR0,
	VERTEX_ATTR_JOINTS_0,
	VERTEX_ATTR_WEIGHTS_0,

	// extra vertex attributes
	VERTEX_ATTR_TEXCOORD2,
	VERTEX_ATTR_TEXCOORD3,
	VERTEX_ATTR_TEXCOORD4,
	VERTEX_ATTR_TEXCOORD5,
	VERTEX_ATTR_TEXCOORD6,
	VERTEX_ATTR_TEXCOORD7,
	VERTEX_ATTR_COLOR1,
	VERTEX_ATTR_JOINTS_1,
	VERTEX_ATTR_WEIGHTS_1,
	VERTEX_ATTR_COUNT,
};


//
// 顶点属性映射器
//
// 用于将指定的顶点属性映射到具体的 ShaderLocation。
// 如果遵循引擎本身的 shader location 的定义，那么这个转换就和枚举 VERTEX_ATTR 是一致的。
// 如果使用了第三方的 shader 需要调整 shader location，则需要自己构造 locations 数组的对应关系，并传递给 primitive。
// 注意，这种映射只在第一次生成 VertexAttr 的时候才生效。
//
class VertexAttrMapper
{
public:
	uint8 locations[VERTEX_ATTR_COUNT];

	VertexAttrMapper();

	int location(VERTEX_ATTR attr) const;

	int gltfAttrNameToLocation(const char* const gltfAttrName) const;

	static VERTEX_ATTR gltfAttrNameToEnum(const char* const name);

	static const VertexAttrMapper& default() 
	{
		static VertexAttrMapper mapper;
		return mapper;
	}

	static int defaultLocation(VERTEX_ATTR attr)
	{
		return default().location(attr);
	}
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

