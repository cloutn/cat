
#include "CommandAllocator.h"

#include <scl/assert.h>

#include <memory>

namespace cat {

CommandAllocator::CommandAllocator() : 
	m_pool		(NULL), 
	m_allocIndex(-1)
{
	memset(m_commandBuffers, 0, sizeof(m_commandBuffers));
}

void CommandAllocator::init(svkDevice& device)
{
	m_pool = svkCreateCommandPool(device, false);
	svkCreateCommandBuffer(device, m_pool, false, MAX_COUNT, m_commandBuffers);
}

void CommandAllocator::release(svkDevice& device)
{
	//vkFreeCommandBuffers(device.device, m_pool, MAX_COUNT, m_commandBuffers);
	vkDestroyCommandPool(device.device, m_pool, NULL);
}

void CommandAllocator::reset(svkDevice& device)
{
	vkResetCommandPool(device.device, m_pool, 0);
	m_allocIndex = -1;
}

VkCommandBuffer CommandAllocator::alloc()
{
	if (m_allocIndex + 1 >= MAX_COUNT)
	{
		assert(false);
		return NULL;
	}

	m_allocIndex++;
	return m_commandBuffers[m_allocIndex];
}


VkCommandBuffer* CommandAllocator::getAllocArray()
{
	return m_commandBuffers;
}

} // namespace cat

