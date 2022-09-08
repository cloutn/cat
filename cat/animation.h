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
	Animation	();
	~Animation	();

	void load	(cgltf_animation& animation, Env* env);
	void update	(double diff);

private:
	scl::varray<AnimationChannel*>	m_channels;
	double							m_time;

}; // class Animation


} // namespace cat



