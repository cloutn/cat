#pragma once

#include "scl/type.h"

namespace scl {

class frame_lock
{
public:
	frame_lock() : m_start(0) {}

	void	start	();
	void	wait	(const int locktime = 20); //��֡ʱ�䣬��λ�����룬Ĭ��ֵ20���룬��50֡

private:
	uint64	m_start;
};

} //namespace scl 
