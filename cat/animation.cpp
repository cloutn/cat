#include "cat/animation.h"

#include "cat/env.h"
#include "cat/object.h"

#include "scl/vector.h"

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
		safe_delete(m_channels[i]);
	}
}

// 约定单线程调用：update 通过 channel->apply 写到 Object 的 transform，
// 调用方需保证同一时刻只有一个线程驱动 update，且不在另一线程 delete 目标 Object。
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

		// m_time 是 double 累计毫秒；强转 uint 在 ~49.7 天后会 wrap，
		// 当前编辑器/游戏场景不会持续运行这么久，channel->update 内还会 mod 一次最后帧时间。
		channel->update(static_cast<uint>(m_time));
		channel->apply();
	}
}

} // namespace cat


