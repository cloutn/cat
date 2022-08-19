#pragma once

#include "scl/math.h"
#include "scl/matrix.h"

namespace scl	{

////////////////////////////////////
//	point:
//		x	y	z
//	��space��x y Ϊ���棬zΪ��ֱ����
//  ��screen��x y Ϊ�����Ͻ���Ϊ(0,0)��ʼ������꣬z��Ч
////////////////////////////////////
class point
{
public:
	enum TYPE
	{
		TYPE_SPACE,
		TYPE_SCREEN,
	};
	
	//��λ����������ƽ��
	float x; float y; float z;
	float d;
	TYPE type;

	inline point();
	inline point(const float valueX, const float valueY, const float valueZ);
	inline void transform(const matrix& m);
};


////////////////////////////////////
//	
//	pointʵ��	
//	
////////////////////////////////////
inline point::point()
{ 
	x = 0; 
	y = 0; 
	z = 0; 
	d = 1;
	type = TYPE_SPACE; 
};

inline point::point(const float valueX, const float valueY, const float valueZ)
{ 
	x = valueX; 
	y = valueY; 
	z = valueZ; 
	d = 1;
	type = TYPE_SPACE; 
};

inline void point::transform(const matrix& m)
{
	float newX = (x * m.x1) + (y * m.x2) + (z * m.x3) + (d * m.x4);
	float newY = (x * m.y1) + (y * m.y2) + (z * m.y3) + (d * m.y4);
	float newZ = (x * m.z1) + (y * m.z2) + (z * m.z3) + (d * m.z4);
	float newD = (x * m.d1) + (y * m.d2) + (z * m.d3) + (d * m.d4);

	x = newX;
	y = newY;
	z = newZ;
	d = newD;
}

} //namespace scl

