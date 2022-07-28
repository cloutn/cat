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
	//for (int type = 0; type < KEY_FRAME_TYPE_COUNT; ++type)
	//{
	//	scl::varray<KeyFrame*>& frames = m_frames[type];
	//	for (int i = 0; i < frames.size(); ++i)
	//		delete frames[i];
	//}
	
	for (int i = 0; i < m_channels.size(); ++i)
	{
		delete m_channels[i];
	}
}

//KEY_FRAME_TYPE _cgltfType2KeyFrameType2222(cgltf_animation_path_type ctype)
//{
//	switch (ctype)
//	{
//	case cgltf_animation_path_type_rotation		: return KEY_FRAME_TYPE_ROTATE;
//	case cgltf_animation_path_type_scale		: return KEY_FRAME_TYPE_SCALE;
//	case cgltf_animation_path_type_translation	: return KEY_FRAME_TYPE_MOVE;
//	default: assert(false); break;
//	};
//	return KEY_FRAME_TYPE_INVALID;
//}

void Animation::load(cgltf_animation& animation, Env* env)
{
	for (cgltf_size i = 0; i < animation.channels_count; ++i)	
	{
		AnimationChannel* channel = new AnimationChannel();
		channel->load(animation.channels[i], env);
		m_channels.push_back(channel);
	} 

} // Animation::load


//void Animation::load2(cgltf_animation& animation, Env* env)
//{
//	//const int MAX_FRAME_COUNT = 1024;
//	for (cgltf_size i = 0; i < animation.channels_count; ++i)	
//	{
//		const cgltf_animation_channel&		channel			= animation.channels[i];
//		const KEY_FRAME_TYPE				type			= _cgltfType2KeyFrameType2222(channel.target_path);
//
//		m_target = env->getObjectIDByGltfNode(channel.target_node);
//
//		const cgltf_animation_sampler*		sampler			= channel.sampler;
//		const cgltf_accessor*				timeAccessor	= sampler->input;
//		const cgltf_accessor*				frameAccessor	= sampler->output;
//
//		const int	frameCount = timeAccessor->count;
//
//		// time must be a float array.
//		if (timeAccessor->component_type != cgltf_component_type_r_32f || timeAccessor->type != cgltf_type_scalar)
//			continue;
//
//		// frame must be a vec4 array.
//		if (frameAccessor->component_type != cgltf_component_type_r_32f || (frameAccessor->type != cgltf_type_vec4 && frameAccessor->type != cgltf_type_vec3) || frameCount != frameAccessor->count)
//			continue;
//
//		const float*		times		= reinterpret_cast<const float*>	(cgltf_get_accessor_buffer(timeAccessor));
//		const vector4*		frameDatas	= reinterpret_cast<const vector4*>	(cgltf_get_accessor_buffer(frameAccessor));
//
//		scl::varray<KeyFrame*>&		frames			= m_frames[type];
//		for (int i = 0; i < frameCount; ++i)
//		{
//			KeyFrame*		f		= new KeyFrame(static_cast<uint>(times[i] * 1000));
//			const vector4&	data	= frameDatas[i];
//			switch (type)
//			{
//			case KEY_FRAME_TYPE_ROTATE :
//				f->setRotate(quaternion { data.x, data.y, data.z, data.d });
//				break;
//			case KEY_FRAME_TYPE_SCALE:
//				f->setScale(vector3 { data.x, data.y, data.z });
//				break;
//			case KEY_FRAME_TYPE_MOVE:
//				f->setMove(vector3 { data.x, data.y, data.z });
//				break;
//			default:
//				assert(false);
//				break;
//			}
//			frames.push_back(f);
//		}
//	}
//}
//
//bool KeyFrame_compare_less(KeyFrame* const& f1, KeyFrame* const& f2)
//{
//	return f1->time() < f2->time();
//}

//void Animation::_clearTransform(const KEY_FRAME_TYPE type)
//{
//	switch (type)
//	{
//	case KEY_FRAME_TYPE_ROTATE				: m_rotate.set(0, 0, 0, 1); break;
//	case KEY_FRAME_TYPE_MOVE				: m_move.clear();			break;
//	case KEY_FRAME_TYPE_SCALE				: m_scale.set(1, 1, 1);		break;
//	default:break;
//	};
//}


//void Animation::_update(const uint _timeConst, KEY_FRAME_TYPE type)
//{
//	scl::varray<KeyFrame*>& frames = m_frames[type];
//	if (frames.size() <= 0)
//		return;
//
//	uint time = _timeConst;
//	KeyFrame* lastFrame = frames[frames.size() - 1];
//
//	// if time is larger than last frame, set it to last frame's time
//	if (time > lastFrame->time())
//		time = time % lastFrame->time();
//
//	//找到位于哪两帧之间
//	KeyFrame temp(time);
//	int mid = frames.binary_search2(&temp, KeyFrame_compare_less);
//
//	KeyFrame zero;
//	if (mid == -1) // m_frames is empty
//		_clearTransform(type);
//	else if (time != frames[mid]->time())
//	{
//		KeyFrame* pBefore	= NULL;
//		KeyFrame* pAfter	= NULL;
//		if (time < frames[mid]->time())		// frame[mid - 1] < current_time < frame[mid] 
//		{
//			pBefore = mid > 0 ? frames[mid - 1] : &zero;
//			pAfter	= frames[mid];
//		}
//		else if (time > frames[mid]->time())	// frame[mid] < current_time < frame[mid + 1] 
//		{
//			assert(mid < frames.size() - 1);
//			pBefore = frames[mid];
//			pAfter	= frames[mid + 1];
//		}
//		KeyFrame& before	= *pBefore; 
//		KeyFrame& after		= *pAfter;
//
//		float delta			= (time - before.time()) / static_cast<float>(after.time() - before.time());
//		//if (before.accelerate() == KEY_FRAME_ACCELERATE_UP)
//		//	delta = delta * delta;  // d(x^2)/dx = 2x, monotonically increasing.
//		//else if (before.accelerate() == KEY_FRAME_ACCELERATE_DOWN)
//		//	delta = ::sqrtf(delta); // d(sqrt(x))/dx = (1/2) * x^(-1/2), monotonically decreasing.
//
//		_lerp(type, before, after, delta);
//	}
//	else if (time == frames[mid]->time())
//	{
//		KeyFrame& f = *frames[mid];
//		_set(type, f);
//	}
//	//if (type == KEY_FRAME_TYPE_ROTATE)
//	//	printf("time[%d], rotate = [%.3f, %.3f, %.3f, %.3f]\n", time, m_rotate.x, m_rotate.y, m_rotate.z, m_rotate.w);
//}

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

	//for (int i = 0; i < KEY_FRAME_TYPE_COUNT; ++i)
	//{
	//	_update(m_time, static_cast<KEY_FRAME_TYPE>(i));
	//}
	//Object* target = Object::objectByID(m_target);
	//if (NULL != target)
	//{
	//	target->setAnimationMove(m_move);
	//	target->setAnimationScale(m_scale);
	//	target->setAnimationRotate(m_rotate);
	//}
}

//void Animation::_lerp(const KEY_FRAME_TYPE type, const KeyFrame& before, const KeyFrame& after, const float delta)
//{
//	switch (type)
//	{
//	case KEY_FRAME_TYPE_ROTATE				: quaternion::slerp	(before.rotate(),		after.rotate(),		delta, m_rotate			); break;
//	case KEY_FRAME_TYPE_MOVE				: vector3::lerp		(before.move(),			after.move(),		delta, m_move			); break;
//	case KEY_FRAME_TYPE_SCALE				: vector3::lerp		(before.scale(),		after.scale(),		delta, m_scale			); break;
//	default: assert(false); break;
//	};
//}
//
//void Animation::_set(const KEY_FRAME_TYPE type, const KeyFrame& f)
//{
//	switch (type)
//	{
//	case KEY_FRAME_TYPE_ROTATE				: m_rotate			= f.rotate();	break;
//	case KEY_FRAME_TYPE_MOVE				: m_move			= f.move();		break;
//	case KEY_FRAME_TYPE_SCALE				: m_scale			= f.scale();	break;
//	default: assert(false); break;
//	};
//}

} // namespace cat


