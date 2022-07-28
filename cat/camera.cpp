#include "cat/camera.h"

namespace cat {

Camera::Camera() : 
	m_position			{0, 0, -1},
	m_target			{0, 0, 0},
	m_up				{0, 1, 0},
	m_fov				(45.f),
	m_aspect			(1.f),
	m_near				(0.1f),
	m_far				(100.f),
	m_viewDirty			(true),
	m_projectionDirty	(true),
	m_dirty				(true)
{
	m_matrix = scl::matrix::identity();
}


void Camera::set(const scl::vector3& position, const scl::vector3& lookat, const scl::vector3 up, float fov, float aspect, float near, float far)
{
	setView(position, lookat, up);
	setProjection(fov, aspect, near, far);
}

void Camera::setView(const scl::vector3& position, const scl::vector3& lookat, const scl::vector3 up)
{
	m_viewDirty			= true;
	m_dirty				= true;
	m_position			= position;
	m_target			= lookat;
	m_up				= up;
}

void Camera::setProjection(float fov, float aspect, float near, float far)
{
	m_projectionDirty	= true;
	m_dirty				= true;
	m_fov				= fov;
	m_aspect			= aspect;
	m_near				= near;
	m_far				= far;
}

void Camera::move(scl::vector3 d)
{
	scl::vector3 lookat = m_target - m_position;
	m_position	+= d;
	m_target	= m_position + lookat;
	m_viewDirty	= true;
	m_dirty		= true;
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
	scl::matrix::perspective(m_projectionMatrix, m_fov, m_aspect, m_near, m_far);
	m_projectionDirty = false;
}

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

