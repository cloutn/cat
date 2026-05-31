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
	m_frames.clear();
}

bool KeyFrame_compare_less2(KeyFrame* const& f1, KeyFrame* const& f2)
{
	// 双保险：addKeyFrame 已挡 NULL，但 binary_search 路径里再防一层，
	// 把 NULL 视为时间 0，保证排序总有定义、不解引用空指针
	const uint t1 = (NULL != f1) ? f1->time() : 0;
	const uint t2 = (NULL != f2) ? f2->time() : 0;
	return t1 < t2;
}

void AnimationChannel::update(const uint _timeConst)
{
	scl::varray<KeyFrame*>& frames = m_frames;
	if (frames.size() <= 0)
		return;

	KeyFrame* lastFrame = frames[frames.size() - 1];
	if (NULL == lastFrame)
		return;

	uint time = _timeConst;
	// if time is larger than last frame, set it to last frame's time
	if (time > lastFrame->time())
		time = lastFrame->time() > 0 ? (time % lastFrame->time()) : 0;

	//找到位于哪两帧之间
	KeyFrame temp(time);
	int mid = frames.binary_search2(&temp, KeyFrame_compare_less2);

	// 未命中或越界：reset 到默认姿态，避免后续解引用越界
	if (mid < 0 || mid >= frames.size())
	{
		_resetTransform();
		return;
	}

	KeyFrame* midFrame = frames[mid];
	if (NULL == midFrame)
	{
		_resetTransform();
		return;
	}

	// 精确命中
	if (time == midFrame->time())
	{
		_set(*midFrame);
		return;
	}

	// 落在区间内，定位 before / after
	KeyFrame zero;
	zero.clear(m_type);

	KeyFrame* pBefore	= NULL;
	KeyFrame* pAfter	= NULL;
	if (time < midFrame->time())		// frame[mid - 1] < current_time < frame[mid]
	{
		pBefore = mid > 0 ? frames[mid - 1] : &zero;
		pAfter	= midFrame;
	}
	else								// frame[mid] < current_time，需要 frame[mid + 1]
	{
		// Release 下原 assert 失效，binary_search2 返回末尾时 frames[mid+1] 会越界写
		if (mid + 1 >= frames.size())
		{
			_set(*midFrame);
			return;
		}
		pBefore = midFrame;
		pAfter	= frames[mid + 1];
	}

	if (NULL == pBefore || NULL == pAfter)
	{
		_resetTransform();
		return;
	}

	// 防止两帧时间戳重复造成除零 → inf/nan 污染 lerp/slerp 输出
	if (pAfter->time() == pBefore->time())
	{
		_set(*pAfter);
		return;
	}

	float delta = (time - pBefore->time()) / static_cast<float>(pAfter->time() - pBefore->time());
	_lerp(*pBefore, *pAfter, delta);
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



