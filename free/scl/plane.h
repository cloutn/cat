#pragma once

#include "scl/vector.h"

namespace scl {

class plane 
{
public:
	vector3	normal;
	float	w;

	//
	//	plane : Ax + By + Cz + D = 0;
	//
	//	plane	(float a, float b, float c, float d) { normal.x = a; normal.y = b; normal.z = c; w = d; } 
	//
	//	float	a() const	{ return normal.x; }
	//	float	b() const	{ return normal.y; }
	//	float	c() const	{ return normal.z; }
	//	float	d() const	{ return w; }
	//

	void			from_normal_point		(const scl::vector3& normal, const scl::vector3& point, bool make_normalize = true);
	void			from_point				(const scl::vector3& pt1, const scl::vector3& pt2, const scl::vector3& pt3, bool make_normalize = true);
	void			from_intercept			(float x_intercept, float y_intercept, float z_intercept, bool make_normalize = true);

	static plane	create_from_normal_point(const scl::vector3& normal, const scl::vector3& point, bool make_normalize = true);
	static plane	create_from_point		(const scl::vector3& pt1, const scl::vector3& pt2, const scl::vector3& pt3, bool make_normalize = true);
	static plane	create_from_intercept	(float x_intercept, float y_intercept, float z_intercept, bool make_normalize = true);


	bool			operator==				(const plane& other) { return normal == other.normal && float_equal(w, other.w); }

	int				side					(const vector3& pt) const;
	bool			same_side				(const vector3& pt1, const vector3& pt2) const;
	float			dot						(const vector3& pt) const;
	float			point_distance			(const vector3& pt) const;

	bool			normalize				();

	const float*	value_ptr				() const { return normal.value_ptr(); }
	float*			value_ptr				() { return normal.value_ptr(); }
};

void plane::from_normal_point(const scl::vector3& _normal, const scl::vector3& _point, bool make_normalize)
{
	normal = _normal;
	w = -scl::vector3::dot(_normal, _point);

	if (make_normalize)
		normalize();
}

plane plane::create_from_normal_point(const scl::vector3& _normal, const scl::vector3& _point, bool make_normalize)
{
	plane _plane;
	_plane.from_normal_point(_normal, _point, make_normalize);
	return _plane;
}

void plane::from_point(const scl::vector3& pt1, const scl::vector3& pt2, const scl::vector3& pt3, bool make_normalize)
{
	scl::vector3 v1 = pt2 - pt1;
	scl::vector3 v2 = pt3 - pt1;
	scl::vector3 n = scl::vector3::cross(v1, v2);

	from_normal_point(n, pt1, make_normalize);
}

void plane::from_intercept(float x_intercept, float y_intercept, float z_intercept, bool make_normalize)
{
	normal = { 1 / x_intercept, 1 / y_intercept, 1 / z_intercept };
	w = -1;

	if (make_normalize)
		normalize();
}

plane plane::create_from_point(const scl::vector3& pt1, const scl::vector3& pt2, const scl::vector3& pt3, bool make_normalize)
{
	plane _plane;
	_plane.from_point(pt1, pt2, pt3, make_normalize);
	return _plane;	
}

scl::plane plane::create_from_intercept(float x_intercept, float y_intercept, float z_intercept, bool make_normalize)
{
	plane _plane;
	_plane.from_intercept(x_intercept, y_intercept, z_intercept, make_normalize);
	return _plane;	
}

int plane::side(const vector3& pt) const
{
	float d = dot(pt);
	if (float_is_zero(d))
		return 0;
	else
		return d > 0 ? 1 : -1;
}

bool plane::same_side(const vector3& pt1, const vector3& pt2) const
{
	int side1 = side(pt1);
	int side2 = side(pt2);
	if (side1 == 0 || side2 == 0)
		return true;
	return side1 == side2;
}

inline float plane::dot(const vector3& pt) const
{
	return normal.dot(pt) + w;
}

float plane::point_distance(const vector3& pt) const
{
	float d = dot(pt);
	d = scl::absolute(d / normal.length());
	return d;
}

inline bool	plane::normalize()
{
	const float len_sqr = normal.length_sqr();
	if(float_is_zero(len_sqr))
		return false;

	const float scale = scl::inv_sqrt(len_sqr);
	normal *= scale;
	w *= scale;
	return true;
}

} // namespace scl

