#pragma once

#include "cat/simplevulkan.h"

namespace cat {

class CommandAllocator
{
public:
	CommandAllocator();

	void				init				(svkDevice& device);
	void				release				(svkDevice& device);
	VkCommandBuffer		alloc				();
	void				reset				(svkDevice& device);
	VkCommandBuffer*	getAllocArray		();
	int					getAllocCount		() const { return m_allocIndex + 1; }

private:
	static const int	MAX_COUNT = 256;

	VkCommandPool		m_pool;
	VkCommandBuffer		m_commandBuffers[MAX_COUNT];
	int					m_allocIndex;

}; // class CommandBufferAllocator

} // namespace cat

