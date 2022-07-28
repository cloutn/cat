
#include "descriptorAllocator.h"

#include "xxhash.h"

namespace cat {

DescriptorAllocator::DescriptorAllocator() : m_layout(NULL), m_pageCount(0)
{
	memset(&m_device, 0, sizeof(m_device));
	memset(m_countPerType, 0, sizeof(m_countPerType));
	memset(&m_pages, 0, sizeof(m_pages));
}

DescriptorAllocator::~DescriptorAllocator()
{
	release();
}

void DescriptorAllocator::release()
{
	if (NULL == m_layout)
		return;
	for (int i = 0; i < MAX_PAGE_COUNT; ++i)
		m_pages[i].release(m_device);
	vkDestroyDescriptorSetLayout(m_device.device, m_layout, NULL);
	m_layout = NULL;
}

void DescriptorAllocator::init(svkDevice& device, const VkDescriptorSetLayoutBinding* binds, const int bindCount)
{
	m_device = device;
	m_layout = svkCreateDescriptorLayoutEx(device, binds, bindCount);
	for (int i = 0; i < bindCount; ++i)
	{
		int type = static_cast<int>(binds[i].descriptorType);
		if (type >= 0 && type < VK_DESCRIPTOR_TYPE_RANGE_SIZE)
			m_countPerType[type] += binds[i].descriptorCount;
	}
}

DescriptorSet DescriptorAllocator::alloc()
{
	DescriptorSet s = _allocFromPages();
	if (s.set != NULL)
		return s;

	// not found, create a new page
	if (m_pageCount >= MAX_PAGE_COUNT)
	{
		assertf(false, "All descriptor pages are full!");
		return DescriptorSet { NULL, 0 };
	}

	DescriptorPage&	newPage = m_pages[m_pageCount];
	newPage.init(m_device, m_layout, m_countPerType);
	s = newPage.alloc(m_pageCount);
	++m_pageCount;
	return s;
}

cat::DescriptorSet DescriptorAllocator::_allocFromPages()
{
	DescriptorSet set { NULL, 0 };
	for (int i = m_pageCount -1; i >= 0; --i)	
	{
		DescriptorPage& page = m_pages[i];
		assert(NULL != page.memory);

		if (page.freeIndices.size() == 0) // current page is full
			continue;

		set = page.alloc(i);

		break;
	}
	return set;
}

void DescriptorAllocator::free(DescriptorSet& set)
{
	int pageIndex	= ((set.allocIndex & 0xFFFF0000) >> 16);
	int setIndex	= (set.allocIndex & 0x0000FFFF);
	m_pages[pageIndex].freeIndices.push(setIndex);
}

DescriptorSet DescriptorPage::alloc(const int pageIndex)
{
	assert(freeIndices.size() > 0 || allocCount > 0);

	int index = -1;
	if (allocCount > 0)
	{
		index = allocCount - 1;
		--allocCount;
	}
	else
		index = freeIndices.pop();		

	assert(index >= 0);

	VkDescriptorSet	set			= sets[index];
	uint32			fullIndex	= ((pageIndex << 16) | index);
	return DescriptorSet { set, fullIndex };
}

void DescriptorPage::init(svkDevice& device, VkDescriptorSetLayout& layout, const int* countPerType)
{
	memory					= svkCreateDescriptorPoolEx2(device, PAGE_SIZE, countPerType);
	allocCount				= PAGE_SIZE;

	VkDescriptorSetLayout layouts[PAGE_SIZE] = { NULL };
	for (int i = 0; i < PAGE_SIZE; ++i)
		layouts[i] = layout;

	VkDescriptorSetAllocateInfo allocInfo;
	memset(&allocInfo, 0, sizeof(allocInfo));
	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.pNext					= NULL;
	allocInfo.descriptorPool		= memory;
	allocInfo.descriptorSetCount	= DescriptorPage::PAGE_SIZE;
	allocInfo.pSetLayouts			= layouts;

	VkResult err = vkAllocateDescriptorSets(device.device, &allocInfo, sets);
	assert(err == VK_SUCCESS);
}

void DescriptorPage::release(svkDevice& device)
{
	if (NULL == memory)
		return;
	//vkFreeDescriptorSets(device.device, memory, DescriptorPage::PAGE_SIZE, sets);
	vkDestroyDescriptorPool(device.device, memory, NULL);
}

//int DescriptorData::GetHash()
//{
//	int h = XXH32(elems, sizeof(Elem) * elemCount, 0);	
//	return h;
//}

}	// namespace cat


