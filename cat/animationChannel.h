#pragma once

#include "cat/keyFrame.h"
#include "cat/def.h"

#include "scl/varray.h"

struct cgltf_animation_channel;

namespace cat {

class Env;

class AnimationChannel
{
public:
	AnimationChannel();
	~AnimationChannel();

	void load				(const cgltf_animation_channel& channel, Env* env);
	void update				(const uint timeConst);
	void apply				();

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


