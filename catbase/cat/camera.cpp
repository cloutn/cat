#include "cat/camera.h"

#include "scl/quaternion.h"

namespace cat {

using scl::vector3;
using scl::quaternion;
using scl::matrix;
using scl::z_range;

Camera::Camera() : 
	m_position					{0, 0, -1},
	m_target					{0, 0, 0},
	m_up						{0, 1, 0},
	m_fov						(45.f),
	m_aspect					(1.f),
	m_near						(0.1f),
	m_far						(100.f),
	m_viewDirty					(true),
	m_projectionDirty			(true),
	m_dirty						(true),
	m_ortho						(false),
	m_zRange					(z_range::negative_one_to_one),
	m_orthoRefPosition	{0, 0, 0}
{
	m_matrix					= matrix::identity();
	m_projectionMatrix			= matrix::identity();
	m_viewMatrix				= matrix::identity();
}


void Camera::set(const scl::vector3& position, const scl::vector3& lookat, const scl::vector3 up, float fov, float aspect, float near, float far, z_range zRange)
{
	setView(position, lookat, up);
	setProjection(fov, aspect, near, far);
	setZRange(zRange);
}

void Camera::setView(const scl::vector3& position, const scl::vector3& lookat, const scl::vector3 up)
{
	_invalidateView();
	m_position			= position;
	m_target			= lookat;
	m_up				= up;
}

void Camera::setProjection(float fov, float aspect, float near, float far)
{
	_invalidateProjection();
	m_fov				= fov;
	m_aspect			= aspect;
	m_near				= near;
	m_far				= far;
}

void Camera::setAspect(float aspect)
{
	_invalidateProjection();
	m_aspect			= aspect;
}

void Camera::setViewByMatrix(const scl::matrix& m)
{
	_invalidateView();
	scl::matrix::decompose_lookat(m, m_position.x, m_position.y, m_position.z, m_target.x, m_target.y, m_target.z, m_up.x, m_up.y, m_up.z);
}

void Camera::setOrtho(const bool b)
{
	if (m_ortho == b)
		return;
	m_ortho = b;
	_invalidateProjection();
}

void Camera::setZRange(const scl::z_range r)
{
	if (m_zRange == r)
		return;

	m_zRange = r;
	_invalidateProjection();
}

void Camera::move(scl::vector3 d)
{
	vector3 lookat	= _front();
	m_position		+= d;
	m_target		= m_position + lookat;
	_invalidateView();
}


void Camera::rotate(float x, float y, float z)
{
	vector3 front	= _front();
	vector3 right	= vector3::cross(front, m_up);
	right.normalize();

	quaternion q;
	q.from_euler_angle(x, y, z);
	scl::matrix transform;
	q.to_matrix(transform);
	front.mul_matrix(transform);
	
	m_target = m_position + front;
	m_up = vector3::cross(right, front);
	m_up.normalize();

	_invalidateView();
}

// let the camrea rotate around the right vector, the fowrad(front) will turn up and down.
void Camera::orbit_right(float angle)
{
	vector3 front = _front();
	vector3 right = _right();

	quaternion q;
	q.from_pivot_radian(right, scl::radian(angle));
	scl::matrix mat;
	q.to_matrix(mat);

	front.mul_matrix(mat);

	m_target = m_position + front;
	m_up = vector3::cross(right, front);

	_invalidateView();
}

// let the camera rotate around the up vector, the fowrad(front) will turn left or right.
void Camera::orbit_up(float angle)
{
	vector3 front = _front();
	vector3 right = _right();
	vector3 up = { 0, 1, 0 };

	quaternion q;
	q.from_pivot_radian(up, scl::radian(angle));
	scl::matrix mat;
	q.to_matrix(mat);

	front.mul_matrix(mat);
	right.mul_matrix(mat);

	m_target = m_position + front;
	m_up = vector3::cross(right, front);

	_invalidateView();
}

const scl::matrix& Camera::matrix() const
{
	if (m_viewDirty)
		_updateView();
	if (m_projectionDirty)
		_updateProjection();
	if (m_dirty)
	{
		m_matrix = m_viewMatrix;
		m_matrix.mul(m_projectionMatrix);
		m_dirty = false;
	}
	return m_matrix;
}

const	scl::matrix& Camera::viewMatrix() const
{
	_updateView();
	return m_viewMatrix;
}

const	scl::matrix& Camera::projectionMatrix() const
{
	_updateProjection();
	return m_projectionMatrix;
}

void Camera::_updateView() const
{
	if (!m_viewDirty)
		return;
	scl::matrix::lookat(m_viewMatrix, m_position.x, m_position.y, m_position.z, m_target.x, m_target.y, m_target.z, m_up.x, m_up.y, m_up.z);
	m_viewDirty = false;
}

void Camera::_updateProjection() const
{
	if (!m_projectionDirty)
		return;
	if (m_ortho)
		scl::matrix::ortho(m_projectionMatrix, m_fov, m_aspect, m_near, m_far, _orthoRefPositionToZ(), m_zRange);
	else
		scl::matrix::perspective(m_projectionMatrix, m_fov, m_aspect, m_near, m_far, m_zRange);
	m_projectionDirty = false;
}

void Camera::_invalidateView() const
{
	m_viewDirty = true;
	m_dirty = true;
}

void Camera::_invalidateProjection() const
{
	m_projectionDirty = true;
	m_dirty = true;
}

float Camera::_orthoRefPositionToZ() const
{
	vector3 vecToRef = m_orthoRefPosition - m_position;
	float projectionOnFront = vector3::dot(vecToRef, _front());
	return projectionOnFront;
}

void Camera::move_front(float d)
{
	vector3 m_front = _front();
	move(m_front * d);
}

void Camera::move_side(float d)
{
	vector3 side = vector3::cross(_front(), m_up);
	move(side * d);
}


scl::vector3 Camera::_front() const
{
	vector3 front = m_target - m_position;
	front.normalize();
	return front;
}

scl::vector3 Camera::_right() const
{
	vector3 right = vector3::cross(_front(), m_up);
	right.normalize();
	return right;
}


//scl::vector3 Camera::_right(const scl::vector3& front) const
//{
//	vector3 right = vector3::cross(front, m_up);
//	right.normalize();
//	return right;
//}

//void Camera::_updateMatrix() const
//{
//	if (m_viewDirty)
//		_updateView();
//	if (m_projectionDirty)
//		_updateProjection();
//	if (m_dirty)
//	{
//		m_matrix = m_viewMatrix;
//		m_matrix.mul(m_projectionMatrix);
//		m_dirty = false;
//	}
//}

} // namespace cat 

