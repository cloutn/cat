#pragma once

#include "cat/keyFrame.h"
#include "cat/def.h"

#include "scl/varray.h"

namespace cat {

class Env;

class AnimationChannel
{
public:
	AnimationChannel();
	~AnimationChannel();

	void update				(const uint timeConst);
	void apply				();
	void setTarget			(int objectID) { m_target = objectID; }
	void setType			(KEY_FRAME_TYPE t) { m_type = t; }
	void addKeyFrame		(KeyFrame* f) { m_frames.push_back(f); }

private:
	void _lerp				(const KeyFrame& before, const KeyFrame& after, const float delta);
	void _set				(const KeyFrame& f);
	void _resetTransform	();

private:
	int						m_target;
	KEY_FRAME_TYPE			m_type;
	scl::varray<KeyFrame*>	m_frames;
	union
	{
		scl::quaternion		m_rotate;
		scl::vector3		m_move;
		scl::vector3		m_scale;
	};
};

} // namespace cat


