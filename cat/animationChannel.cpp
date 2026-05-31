#include "cat/animationChannel.h"

#include "cat/object.h"
#include "cat/env.h"

#include "scl/quaternion.h"
#include "scl/vector.h"

namespace cat {

using scl::vector4;
using scl::vector3;
using scl::quaternion;

AnimationChannel::AnimationChannel() : 
	m_target(-1), 
	m_type(KEY_FRAME_TYPE_INVALID)
{
	m_rotate.clear();
	m_scale.clear();
	m_move.clear();
}

AnimationChannel::~AnimationChannel()
{
	for (int i = 0; i < m_frames.size(); ++i)
		safe_delete(m_frames[i]);
}

bool KeyFrame_compare_less2(KeyFrame* const& f1, KeyFrame* const& f2)
{
	// 不变量：addKeyFrame 已拒收 NULL，m_frames 中不会出现 NULL
	return f1->time() < f2->time();
}

void AnimationChannel::update(const uint _timeConst)
{
	scl::varray<KeyFrame*>& frames = m_frames;
	if (frames.size() <= 0)
		return;

	uint time = _timeConst;
	KeyFrame* lastFrame = frames[frames.size() - 1];
	// if time is larger than last frame, set it to last frame's time
	if (time > lastFrame->time())
		time = lastFrame->time() > 0 ? (time % lastFrame->time()) : 0;

	//找到位于哪两帧之间
	KeyFrame temp(time);
	int mid = frames.binary_search2(&temp, KeyFrame_compare_less2);

	KeyFrame zero;
	zero.clear(m_type);		// 临时帧也按当前 type 初始化，避免 _lerp 读未初始化成员
	if (mid == -1) // m_frames is empty
		_resetTransform();
	else if (time != frames[mid]->time())
	{
		KeyFrame* pBefore	= NULL;
		KeyFrame* pAfter	= NULL;
		if (time < frames[mid]->time())		// frame[mid - 1] < current_time < frame[mid] 
		{
			pBefore = mid > 0 ? frames[mid - 1] : &zero;
			pAfter	= frames[mid];
		}
		else if (time > frames[mid]->time())	// frame[mid] < current_time < frame[mid + 1] 
		{
			// Release 下原 assert 会失效，binary_search2 返回末尾时 frames[mid+1] 越界写
			if (mid + 1 >= frames.size())
			{
				_set(*frames[mid]);
				return;
			}
			pBefore = frames[mid];
			pAfter	= frames[mid + 1];
		}
		KeyFrame& before	= *pBefore; 
		KeyFrame& after		= *pAfter;

		// 两帧时间戳重复会让 delta 除零产生 inf/nan，污染 _lerp/_slerp 输出
		if (after.time() == before.time())
		{
			_set(after);
			return;
		}

		float delta			= (time - before.time()) / static_cast<float>(after.time() - before.time());

		_lerp(before, after, delta);
	}
	else if (time == frames[mid]->time())
	{
		KeyFrame& f = *frames[mid];
		_set(f);
	}
}
void AnimationChannel::_lerp(const KeyFrame& before, const KeyFrame& after, const float delta)
{
	switch (m_type)
	{
	case KEY_FRAME_TYPE_ROTATE				: quaternion::slerp	(before.rotate(),		after.rotate(),		delta, m_rotate			); break;
	case KEY_FRAME_TYPE_MOVE				: vector3::lerp		(before.move(),			after.move(),		delta, m_move			); break;
	case KEY_FRAME_TYPE_SCALE				: vector3::lerp		(before.scale(),		after.scale(),		delta, m_scale			); break;
	default: assert(false); break;
	};
}

void AnimationChannel::_set(const KeyFrame& f)
{
	switch (m_type)
	{
	case KEY_FRAME_TYPE_ROTATE				: m_rotate	= f.rotate();	break;
	case KEY_FRAME_TYPE_MOVE				: m_move	= f.move();		break;
	case KEY_FRAME_TYPE_SCALE				: m_scale	= f.scale();	break;
	default: assert(false); break;
	};
}

void AnimationChannel::_resetTransform()
{
	switch (m_type)
	{
	case KEY_FRAME_TYPE_ROTATE				: m_rotate.set(0, 0, 0, 1); break;
	case KEY_FRAME_TYPE_MOVE				: m_move.clear();			break;
	case KEY_FRAME_TYPE_SCALE				: m_scale.set(1, 1, 1);		break;
	default:break;
	};
}

void AnimationChannel::apply()
{
	Object* target = Object::objectByID(m_target);
	if (NULL == target)
		return;

	if (!target->isEnableAnimation())
		return;

	switch (m_type)
	{
	case KEY_FRAME_TYPE_ROTATE				: target->setRotate(m_rotate); break;
	case KEY_FRAME_TYPE_MOVE				: target->setMove(m_move); break;
	case KEY_FRAME_TYPE_SCALE				: target->setScale(m_scale); break;
	default:break;
	};
}

} // namespace cat



