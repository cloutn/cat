#include "./color.h"

#include "scl/math.h"

namespace cat {

void argb_to_float(uint color, float& a, float& r, float& g, float& b)
{
	uint ia = (color & 0xFF000000) >> 24;
	uint ir = (color & 0x00FF0000) >> 16;
	uint ig = (color & 0x0000FF00) >> 8;
	uint ib = (color & 0x000000FF);
	a = ia / 255.f;
	r = ir / 255.f;
	g = ig / 255.f;
	b = ib / 255.f;
}

uint32 color_lerp(uint32 color1, uint32 color2, const float t)
{
	int a1 = COLOR_GET_A(color1);
	int r1 = COLOR_GET_R(color1);
	int g1 = COLOR_GET_G(color1);
	int b1 = COLOR_GET_B(color1);

	int a2 = COLOR_GET_A(color2);
	int r2 = COLOR_GET_R(color2);
	int g2 = COLOR_GET_G(color2);
	int b2 = COLOR_GET_B(color2);

	int a = scl::lerp(a1, a2, t);
	int r = scl::lerp(r1, r2, t);
	int g = scl::lerp(g1, g2, t);
	int b = scl::lerp(b1, b2, t);

	uint32 lerp = COLOR_ARGB(a, r, g, b);
	return lerp;
}


} // namespace cat

