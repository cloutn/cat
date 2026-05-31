
#include "cat/keyFrame.h"
#include "cat/names.h"

#include "scl/stringdef.h"
#include "scl/string.h"

namespace cat {

using scl::vector3;

KeyFrame::KeyFrame(const uint time) : 
	m_time			(time)
{
	// union 只 init 第一个成员；m_move / m_scale 在被赋值时自动激活，避免依次 clear 非活跃成员的 UB
	m_rotate.clear();
}


KeyFrame::~KeyFrame()
{
}

void KeyFrame::clear(const KEY_FRAME_TYPE type)
{
	switch (type)
	{
	case KEY_FRAME_TYPE_ROTATE				: m_rotate.set(0, 0, 0, 1); break;
	case KEY_FRAME_TYPE_MOVE				: m_move.clear(); break;
	case KEY_FRAME_TYPE_SCALE				: m_scale.set(1.f, 1.f, 1.f); break;
	default: assert(false); break;
	};
}

} // namespace cat

