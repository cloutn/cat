#pragma once

#include "cat/keyFrame.h"
#include "cat/animationChannel.h"

#include "scl/varray.h"

namespace cat {

class Env;

class Animation
{
public:
	Animation	();
	~Animation	();

	void update		(double diff);
	void addChannel	(AnimationChannel* c) { m_channels.push_back(c); }

private:
	scl::varray<AnimationChannel*>	m_channels;
	double							m_time;

}; // class Animation


} // namespace cat



