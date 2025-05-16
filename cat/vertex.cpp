////////////////////////////////////////////////////////////////////////////////
//	顶点
//	
//	2010.12.02 caolei
////////////////////////////////////////////////////////////////////////////////
#include "cat/vertex.h"

#include "cat/IRender.h"

#include "scl/vector.h"
#include "scl/stringdef.h"

//#include <memory.h>
//#include <string.h>
#include <string.h>

namespace cat {

using scl::vector3;
using scl::matrix;

//vertex& vertex::operator=( const vector3& v )
//{
//	x = v.x;
//	y = v.y;
//	z = v.z;
//	return *this;
//}

//vertex::vertex( float ix, float iy, float iz, float inx, float iny, float inz, float iu, float iv)
//{
//	x = ix; y = iy; z = iz;
//	nx = inx; ny = iny; nz = inz;
//	u = iu; v = iv;
//}

//void vertex::clear()
//{
//	memset(this, 0, sizeof(vertex));
//}
//
//void vertex::mulMatrix( const matrix& m )
//{
//	vector3 temp = {x, y, z};
//	x = temp.x * m.x1 + temp.y * m.x2 + temp.z * m.x3 + m.x4;
//	y = temp.x * m.y1 + temp.y * m.y2 + temp.z * m.y3 + m.y4;
//	z = temp.x * m.z1 + temp.y * m.z2 + temp.z * m.z3 + m.z4;
//
//	//更新法线
//	//TODO 这里是否需要更新？
//	//vector3 ntemp = {nx, ny, nz};
//	//nx = ntemp.x * m.x1 + ntemp.y * m.x2 + ntemp.z * m.x3 + m.x4;
//	//ny = ntemp.x * m.y1 + ntemp.y * m.y2 + ntemp.z * m.y3 + m.y4;
//	//nz = ntemp.x * m.z1 + ntemp.y * m.z2 + ntemp.z * m.z3 + m.z4;
//}
//
//void vertex::set(
//	float ix,	float iy,	float iz, 
//	float inx,	float iny,	float inz, 
//	float iu,	float iv)
//{
//	x = ix; y = iy; z = iz;
//	nx = inx; ny = iny; nz = inz;
//	u = iu; v = iv;
//}
//
//vertex& vertex::operator+=(const vector3& v)
//{
//	x += v.x;
//	y += v.y;
//	z += v.z;
//	return *this;
//}



void vertex_color_uv::mul_matrix(const scl::matrix& m)
{
	const float t_x = x * m.x1 + y * m.x2 + z * m.x3 + m.x4;
	const float t_y = x * m.y1 + y * m.y2 + z * m.y3 + m.y4;
	const float t_z = x * m.z1 + y * m.z2 + z * m.z3 + m.z4;
	x = t_x;
	y = t_y;
	z = t_z;
}

const cat::VertexAttr* vertex_color_uv::get_attr()
{
	static const VertexAttr s_attrs[3] = {
			{ 0, 3, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), 0 },
			{ 5, 4, ELEM_TYPE_UINT8,	1, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, color) },
			{ 3, 2, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, u) }
	};
	return s_attrs;
}

int vertex_color::get_attr(VertexAttr* attrs, const int capacity)
{
	if (capacity < 2)
	{
		assert(false);
		return 0;
	}

	attrs[0] = { VertexAttrMapper::defaultLocation(VERTEX_ATTR_POSITION), 3, ELEM_TYPE_FLOAT, 0, sizeof(vertex_color), 0 };
	attrs[1] = { VertexAttrMapper::defaultLocation(VERTEX_ATTR_COLOR0), 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) };

	return 2;
}

VertexAttrMapper::VertexAttrMapper()
{
	for (int i = 0; i < VERTEX_ATTR_COUNT; ++i)
		locations[i] = i;
}

int VertexAttrMapper::location(VERTEX_ATTR attr) const
{
	if (attr <= VERTEX_ATTR_INVALID || attr >= VERTEX_ATTR_COUNT)
	{
		assert(false);
		return -1;
	}
	return locations[attr];
}

int VertexAttrMapper::gltfAttrNameToLocation(const char* const gltfAttrName) const
{
	VERTEX_ATTR attr = gltfAttrNameToEnum(gltfAttrName);
	return attr >= 0 ? locations[attr] : -1;
}

cat::VERTEX_ATTR VertexAttrMapper::gltfAttrNameToEnum(const char* const name)
{
	VERTEX_ATTR attr = VERTEX_ATTR_INVALID;
	if (0 == scl_strcasecmp(name, "POSITION"))	attr = VERTEX_ATTR_POSITION;
	else if (0 == scl_strcasecmp(name, "NORMAL"))	attr = VERTEX_ATTR_NORMAL;
	else if (0 == scl_strcasecmp(name, "TANGENT"))	attr = VERTEX_ATTR_TANGENT;
	else if (0 == scl_strcasecmp(name, "TEXCOORD_0"))	attr = VERTEX_ATTR_TEXCOORD0;
	else if (0 == scl_strcasecmp(name, "TEXCOORD_1"))	attr = VERTEX_ATTR_TEXCOORD1;
	else if (0 == scl_strcasecmp(name, "COLOR_0"))	attr = VERTEX_ATTR_COLOR0;
	else if (0 == scl_strcasecmp(name, "JOINTS_0"))	attr = VERTEX_ATTR_JOINTS_0;
	else if (0 == scl_strcasecmp(name, "WEIGHTS_0"))	attr = VERTEX_ATTR_WEIGHTS_0;
	return attr;
}

} //namespace cat


