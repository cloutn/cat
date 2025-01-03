#pragma once

#include "scl/matrix.h"
#include "scl/vector.h"

namespace cat {

// camera is a matrix ( view matrix multiply projection matrix)

class Camera
{
public:
	Camera();

	const	scl::vector3&	position		() const { return m_position;	}
	const	scl::vector3&	target			() const { return m_target;		}
	const	scl::vector3&	up				() const { return m_up;			}
	const	float			fov				() const { return m_fov;		}
	const	float			aspect			() const { return m_aspect;		}
	const	float			near			() const { return m_near;		}
	const	float			far				() const { return m_far;		}

	void					set				(const scl::vector3& position, const scl::vector3& lookat, const scl::vector3 up, float fov, float aspect, float near, float far);
	void					setView			(const scl::vector3& position, const scl::vector3& lookat, const scl::vector3 up);
	void					setProjection	(float fov, float aspect, float near, float far);
	void					setAspect		(float aspect);
	void					setViewByMatrix	(const scl::matrix& m);

	void					move			(scl::vector3 d);
	void					move			(float dx, float dy, float dz) { move({dx, dy, dz}); }
	void					move_front		(float d);
	void					move_side		(float d);
	void					rotate			(float x, float y, float z);
	void					orbit_right		(float angle);
	void					orbit_up		(float angle);

	const	scl::matrix&	matrix			() const;
	const	scl::matrix&	viewMatrix		() const;
	const	scl::matrix&	projectionMatrix() const;

private:
	scl::vector3			_front			() const;	// front vector = m_target - m_position
	scl::vector3			_right			() const;	// right vector = cross(front, up)
	//scl::vector3			_right			(const scl::vector3& front) const;	// right vector = cross(front, up)
	void					_updateView		() const;
	void					_updateProjection() const;

private:
	// view
	scl::vector3			m_position;
	scl::vector3			m_target;
	scl::vector3			m_up;

	// projection
	float					m_fov;			// field of view degree in angle. default 45 degree.
	float					m_aspect;		// width/height ratio. default 1.f
	float					m_near;			// near Z. MUST be positive number. default 0.1f
	float					m_far;			// far Z. MUST be positive number. default 100.f

	mutable scl::matrix		m_viewMatrix;
	mutable scl::matrix		m_projectionMatrix;
	mutable scl::matrix		m_matrix;
	mutable	bool			m_viewDirty;
	mutable	bool			m_projectionDirty;
	mutable bool			m_dirty;
};

} // namespace cat 


