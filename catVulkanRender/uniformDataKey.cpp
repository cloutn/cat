
#include "./uniformDataKey.h"

#include "xxhash.h"

#include "cat/IRender.h"

#include "scl/assert.h"

#include <string.h>

namespace cat {

UniformDataKey::UniformDataKey() :
	m_binds		(NULL),
	m_bindCount	(0),
	m_datas		(NULL),
	m_dataHandleIndex (0)
{
	memset(&m_frameUniformBuffer, 0, sizeof(m_frameUniformBuffer));
	memset(m_dataHandles, 0, sizeof(m_dataHandles));
}

void UniformDataKey::init(const VkDescriptorSetLayoutBinding* binds, const int bindCount, const svkDescriptorData* datas, svkBuffer& frameUniformBuffer)
{
	m_binds = binds;
	m_bindCount = bindCount;
	m_datas = datas;
	m_frameUniformBuffer = frameUniformBuffer;
	_streamToHandles();
}

int UniformDataKey::hash() const
{
	int hash = XXH32(m_dataHandles, sizeof(uint64_t) * m_dataHandleIndex, 0);
	return hash;
}

void UniformDataKey::_streamToHandles()
{
	for (int bindIndex = 0; bindIndex < m_bindCount; ++bindIndex)
	{
		const VkDescriptorSetLayoutBinding& bind = m_binds[bindIndex];
		for (uint32_t descIndex = 0; descIndex < bind.descriptorCount; ++descIndex)
		{
			if (bind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
				|| bind.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC)
			{
				//const scl::matrix* matrices = static_cast<const scl::matrix*>(m_datas[bindIndex].data[descIndex]);
				assert(m_dataHandleIndex < MAX_DESC_COUNT);
				
				//VkBuffer vb = m_frameUniformBuffer.buffer;
				//void* cast = reinterpret_cast<void*>(vb);
				m_dataHandles[m_dataHandleIndex++] = (uint64_t)(m_frameUniformBuffer.buffer);
			}
			else if (bind.descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
			{
				//const svkTexture* texture = static_cast<const svkTexture*>(.texture);
				const svkDescriptorData::TextureInfo& textureInfo = m_datas[bindIndex].data[descIndex].texture;
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

bool UniformDataKey::operator==(const UniformDataKey& other) const
{
	if (other.m_dataHandleIndex != m_dataHandleIndex)
		return false;
	return 0 == memcmp(m_dataHandles, other.m_dataHandles, sizeof(void*) * m_dataHandleIndex);
}

} // namespace cat
