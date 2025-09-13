#pragma once

#include "cat/simplevulkan.h"

#include "scl/stack.h"

#include <vulkan/vulkan_core.h>

namespace cat {

class DescriptorSet
{
public:
	VkDescriptorSet		set;
	uint32				allocIndex; // used for free descriptorset quickly
};

// a page is a VkDescriptorPool with 64 pre-allocated VkDescriptorSets.
class DescriptorPage
{
public:
	static const int			PAGE_SIZE = 64;

	VkDescriptorPool			memory;
	VkDescriptorSet				sets[PAGE_SIZE];
	int							allocCount;
	scl::stack<int, PAGE_SIZE>	freeIndices;

	void						init	(svkDevice& device, VkDescriptorSetLayout& layout, const int* countPerType);
	DescriptorSet				alloc	(const int pageIndex);
	void						release	(svkDevice& device);
};

// an allocator is a collection of pages. 
// it has a respective descriptor set layout, you can't changed it after initialized. if the layout changed, you MUST create a new allocator
class DescriptorAllocator
{
public:
	DescriptorAllocator();
	~DescriptorAllocator();

	void							init(svkDevice& device, const VkDescriptorSetLayoutBinding* binds, const int bindCount);
	DescriptorSet					alloc();
	void							free(DescriptorSet&);
	void							release();
	VkDescriptorSetLayout			getLayout() { return m_layout; }


private:
	DescriptorSet					_allocFromPages();

private:
	static const int				MAX_PAGE_COUNT		= 256;

	svkDevice						m_device;
	VkDescriptorSetLayout			m_layout; 
	DescriptorPage					m_pages[MAX_PAGE_COUNT];
	int								m_pageCount;
	int								m_countPerType[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
};



} // namespace cat



