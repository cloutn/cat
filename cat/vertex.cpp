////////////////////////////////////////////////////////////////////////////////
//	顶点
//	
//	2010.12.02 caolei
////////////////////////////////////////////////////////////////////////////////
#include "cat/vertex.h"

#include "scl/vector.h"

#include <memory.h>
#include <string.h>

namespace cat {

using scl::vector3;
using scl::matrix;

vertex& vertex::operator=( const vector3& v )
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

vertex::vertex( float ix, float iy, float iz, float inx, float iny, float inz, float iu, float iv)
{
	x = ix; y = iy; z = iz;
	nx = inx; ny = iny; nz = inz;
	u = iu; v = iv;
}

void vertex::clear()
{
	memset(this, 0, sizeof(vertex));
}

void vertex::mulMatrix( const matrix& m )
{
	vector3 temp = {x, y, z};
	x = temp.x * m.x1 + temp.y * m.x2 + temp.z * m.x3 + m.x4;
	y = temp.x * m.y1 + temp.y * m.y2 + temp.z * m.y3 + m.y4;
	z = temp.x * m.z1 + temp.y * m.z2 + temp.z * m.z3 + m.z4;

	//更新法线
	//TODO 这里是否需要更新？
	//vector3 ntemp = {nx, ny, nz};
	//nx = ntemp.x * m.x1 + ntemp.y * m.x2 + ntemp.z * m.x3 + m.x4;
	//ny = ntemp.x * m.y1 + ntemp.y * m.y2 + ntemp.z * m.y3 + m.y4;
	//nz = ntemp.x * m.z1 + ntemp.y * m.z2 + ntemp.z * m.z3 + m.z4;
}

void vertex::set(
	float ix,	float iy,	float iz, 
	float inx,	float iny,	float inz, 
	float iu,	float iv)
{
	x = ix; y = iy; z = iz;
	nx = inx; ny = iny; nz = inz;
	u = iu; v = iv;
}

vertex& vertex::operator+=(const vector3& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}


void vertex_color_uv::mul_matrix(const scl::matrix& m)
{
	const float t_x = x * m.x1 + y * m.x2 + z * m.x3 + m.x4;
	const float t_y = x * m.y1 + y * m.y2 + z * m.y3 + m.y4;
	const float t_z = x * m.z1 + y * m.z2 + z * m.z3 + m.z4;
	x = t_x;
	y = t_y;
	z = t_z;
}


} //namespace cat


