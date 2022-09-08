#include "cat/animation.h"

#include "cat/cgltf_util.h"
#include "cat/env.h"
#include "cat/object.h"

#include "scl/vector.h"

#include "cgltf/cgltf.h"

namespace cat
{

using scl::vector3;
using scl::vector4;
using scl::quaternion;


Animation::Animation() : m_time(0)
{
}

Animation::~Animation()
{
	for (int i = 0; i < m_channels.size(); ++i)
	{
		delete m_channels[i];
	}
}

void Animation::load(cgltf_animation& animation, Env* env)
{
	for (cgltf_size i = 0; i < animation.channels_count; ++i)	
	{
		AnimationChannel* channel = new AnimationChannel();
		channel->load(animation.channels[i], env);
		m_channels.push_back(channel);
	} 

} // Animation::load

void Animation::update(double diff)
{
	m_time += diff;
	for (int i = 0; i < m_channels.size(); ++i)
	{
		AnimationChannel* channel = m_channels[i];
		if (NULL == channel)
		{
			assert(false);
			continue;
		}

		channel->update(static_cast<uint>(m_time));
		channel->apply();
	}
}

} // namespace cat


