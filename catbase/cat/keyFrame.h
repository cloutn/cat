#pragma once

#include "cat/def.h"

#include "scl/vector.h"
#include "scl/quaternion.h"
#include "scl/bitset.h"

namespace cat {

class KeyFrame_Editor;
	
class KeyFrame
{
public:
	KeyFrame(const uint time = 0);
	~KeyFrame();

	uint					time			() const { return m_time;	}
	const scl::quaternion&	rotate			() const { return m_rotate;	}
	const scl::vector3&		move			() const { return m_move;	}
	const scl::vector3&		scale			() const { return m_scale;	}
	bool					operator<		(const KeyFrame& other) const { return m_time < other.m_time; }
	bool					operator==		(const KeyFrame& other) const { return m_time == other.m_time; }
	void					clear			(const KEY_FRAME_TYPE type);
	void					setTime			(const uint v) { m_time = v; }
	void					setRotate		(const scl::quaternion& v) { m_rotate = v; }
	void					setMove			(const scl::vector3& v) { m_move = v; }
	void					setScale		(const scl::vector3& v) { m_scale = v; }
	//scl::vector3&			rotateEuler		();
	//void					setRotateEuler	(const scl::vector3& v);
	//void					setRotateEuler	(const float x, const float y, const float z);

	//static KEY_FRAME_TYPE	stringToType	(const char* const s);
	//static const char*		typeToString	(KEY_FRAME_TYPE);
	//static int				compareFunc		(const void* k1, const void* k2);

private:
	uint					m_time;					//当前帧对应的时间，单位:毫秒
	// 原本是 union，但 C++ 同一时刻只能有一个 active member，
	// 而构造/clear 路径需要一次性把三个成员都置初值，依次操作非活跃成员严格 UB；
	// 改成 struct 多用 24 字节但消除 UB，且 m_type 切换不再需要 placement new。
	scl::quaternion 		m_rotate;
	scl::vector3			m_move;
	scl::vector3			m_scale;

}; //class KeyFrame


} // namespace cat


