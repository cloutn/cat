
#include "./DescriptorDataKey.h"

#include "xxhash.h"

#include "cat/IRender.h"

#include "scl/assert.h"

#include <string.h>

namespace cat {

DescriptorDataKey::DescriptorDataKey() :
	m_descriptorSetLayouts		(NULL),
	m_descriptorSetLayoutCount	(0),
	m_descriptorDatas			(NULL),
	m_dataHandleIndex			(0)
{
	//memset(&m_frameUniformBuffer, 0, sizeof(m_frameUniformBuffer));
	memset(m_dataHandles, 0, sizeof(m_dataHandles));
}

void DescriptorDataKey::init(const VkDescriptorSetLayoutBinding* descriptorSetLayouts, const int descriptorSetLayoutCount, const svkDescriptorData* descriptorDatas, svkBuffer& frameUniformBuffer, uint32_t* dynamicOffsets, uint32_t dynamicOffsetCount) 
{
	m_descriptorSetLayouts		= descriptorSetLayouts;
	m_descriptorSetLayoutCount	= descriptorSetLayoutCount;
	m_descriptorDatas			= descriptorDatas;
	//m_frameUniformBuffer		= frameUniformBuffer;

	_streamToHandles();

	for (int i = 0 ; i < dynamicOffsetCount; ++i)
	{
		assert(m_dataHandleIndex < MAX_DESC_COUNT);
		m_dataHandles[m_dataHandleIndex++] = dynamicOffsets[i];
	}
}

int DescriptorDataKey::hash() const
{
	int hash = XXH32(m_dataHandles, sizeof(uint64_t) * m_dataHandleIndex, 0);
	return hash;
}

void DescriptorDataKey::_streamToHandles()
{
	for (int bindIndex = 0; bindIndex < m_descriptorSetLayoutCount; ++bindIndex)
	{
		const VkDescriptorSetLayoutBinding& layoutBind = m_descriptorSetLayouts[bindIndex];
		for (uint32_t descIndex = 0; descIndex < layoutBind.descriptorCount; ++descIndex)
		{
			if (layoutBind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				|| layoutBind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
			{
				const svkDescriptorData::BufferInfo& bufferInfo = m_descriptorDatas[bindIndex].data[descIndex].buffer;
				assert(m_dataHandleIndex < MAX_DESC_COUNT);
				m_dataHandles[m_dataHandleIndex++] = (uint64_t)(bufferInfo.buffer);
				assert(m_dataHandleIndex < MAX_DESC_COUNT);
				m_dataHandles[m_dataHandleIndex++] = (uint64_t)(m_descriptorDatas[bindIndex].data[descIndex].buffer.bufferSize);
			}
			else if (layoutBind.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				const svkDescriptorData::TextureInfo& textureInfo = m_descriptorDatas[bindIndex].data[descIndex].texture;
				assert(m_dataHandleIndex < MAX_DESC_COUNT);
				m_dataHandles[m_dataHandleIndex++] = (uint64_t)(textureInfo.sampler);
				assert(m_dataHandleIndex < MAX_DESC_COUNT);
				m_dataHandles[m_dataHandleIndex++] = (uint64_t)(textureInfo.imageView);
			}
			else
			{
				assert(false);
			}
		}
	}
}

bool DescriptorDataKey::operator==(const DescriptorDataKey& other) const
{
	if (other.m_dataHandleIndex != m_dataHandleIndex)
		return false;
	return 0 == memcmp(m_dataHandles, other.m_dataHandles, sizeof(void*) * m_dataHandleIndex);
}

} // namespace cat
