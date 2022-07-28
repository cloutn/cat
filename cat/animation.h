#pragma once

#include "cat/keyFrame.h"
#include "cat/animationChannel.h"

#include "scl/varray.h"

struct cgltf_animation;

namespace cat {

class Env;

class Animation
{
public:
	Animation();
	~Animation();

	//void load2(cgltf_animation& animation, Env* env);
	void load(cgltf_animation& animation, Env* env);
	void update(double diff);

private:
	//void _update(const uint _timeConst, KEY_FRAME_TYPE type);
	//void _update(const uint _timeConst, AnimationChannel& channel);
	//void _clearTransform(const KEY_FRAME_TYPE type);
	//void _lerp(const KEY_FRAME_TYPE type, const KeyFrame& before, const KeyFrame& after, const float delta);
	//void _set(const KEY_FRAME_TYPE type, const KeyFrame& f);

private:
	//scl::varray<KeyFrame*> m_frames[KEY_FRAME_TYPE_COUNT];

	scl::varray<AnimationChannel*>	m_channels;
	//int	m_target;
	double m_time;
	//scl::quaternion m_rotate;
	//scl::vector3	m_move;
	//scl::vector3	m_scale;

}; // class Animation


} // namespace cat



