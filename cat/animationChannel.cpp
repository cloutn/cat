#include "cat/animationChannel.h"

#include "cat/object.h"
#include "cat/env.h"
#include "cat/cgltf_util.h"

#include "scl/quaternion.h"
#include "scl/vector.h"

#include "cgltf/cgltf.h"

namespace cat {

using scl::vector4;
using scl::vector3;
using scl::quaternion;

KEY_FRAME_TYPE _cgltfType2KeyFrameType(cgltf_animation_path_type ctype)
{
	switch (ctype)
	{
	case cgltf_animation_path_type_rotation		: return KEY_FRAME_TYPE_ROTATE;
	case cgltf_animation_path_type_scale		: return KEY_FRAME_TYPE_SCALE;
	case cgltf_animation_path_type_translation	: return KEY_FRAME_TYPE_MOVE;
	default: assert(false); break;
	};
	return KEY_FRAME_TYPE_INVALID;
}

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
		delete m_frames[i];
}

void AnimationChannel::load(const cgltf_animation_channel& channel, Env* env)
{
	m_target	= env->getObjectIDByGltfNode(channel.target_node);
	m_type		= _cgltfType2KeyFrameType(channel.target_path);

	const cgltf_animation_sampler*		sampler			= channel.sampler;
	const cgltf_accessor*				timeAccessor	= sampler->input;
	const cgltf_accessor*				frameAccessor	= sampler->output;
	const int							frameCount		= timeAccessor->count;

	// time must be a float array.
	if (timeAccessor->component_type != cgltf_component_type_r_32f || timeAccessor->type != cgltf_type_scalar)
	{
		assert(false);
		return;
	}

	// frame must be a vec4 array.
	if (frameAccessor->component_type != cgltf_component_type_r_32f || (frameAccessor->type != cgltf_type_vec4 && frameAccessor->type != cgltf_type_vec3) || frameCount != frameAccessor->count)
	{
		assert(false);
		return;
	}

	const float*		times			= reinterpret_cast<const float*>	(cgltf_get_accessor_buffer(timeAccessor));
	const float*		frameDatas		= reinterpret_cast<const float*>	(cgltf_get_accessor_buffer(frameAccessor));
	const cgltf_type	type			= frameAccessor->type;
	const int			componentCount	= cgltf_num_components(type);


	scl::varray<KeyFrame*>&		frames			= m_frames;
	for (int i = 0; i < frameCount; ++i)
	{
		KeyFrame*		frame	= new KeyFrame(static_cast<uint>(times[i] * 1000));
		const float*	f		= &frameDatas[i * componentCount];

		switch (m_type)
		{
		case KEY_FRAME_TYPE_ROTATE:
			{
				assert(componentCount >= 4);
				frame->setRotate(quaternion{ f[0], f[1], f[2], f[3] });
			}
			break;
		case KEY_FRAME_TYPE_SCALE:
			frame->setScale(vector3 { f[0], f[1], f[2] });
			break;
		case KEY_FRAME_TYPE_MOVE:
			frame->setMove(vector3 { f[0], f[1], f[2] });
			break;
		default:
			assert(false);
			break;
		}
		frames.push_back(frame);
	}
}

bool KeyFrame_compare_less2(KeyFrame* const& f1, KeyFrame* const& f2)
{
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
			assert(mid < frames.size() - 1);
			pBefore = frames[mid];
			pAfter	= frames[mid + 1];
		}
		KeyFrame& before	= *pBefore; 
		KeyFrame& after		= *pAfter;

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
	switch (m_type)
	{
	case KEY_FRAME_TYPE_ROTATE				: target->setRotate(m_rotate); break;
	case KEY_FRAME_TYPE_MOVE				: target->setMove(m_move); break;
	case KEY_FRAME_TYPE_SCALE				: target->setScale(m_scale); break;
	default:break;
	};
}

} // namespace cat



