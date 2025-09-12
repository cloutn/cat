#pragma once

#include "scl/type.h"

#include "simplevulkan.h"

struct VkDescriptorSetLayoutBinding;

namespace cat {

class DescriptorDataKey
{
public:
	DescriptorDataKey			();
	int		hash				() const; // int GetHash() const
	void	_streamToHandles	(); // void _streamToHandles()
	bool	operator==			(const DescriptorDataKey& other) const;
	void	init				(
		const VkDescriptorSetLayoutBinding*	descriptorSetLayouts, 
		const int							descriptorSetLayoutCount, 
		const svkDescriptorData*			descriptorDatas, 
		svkBuffer&							frameUniformBuffer,
		uint32_t*							dynamicOffsets,
		uint32_t							dynamicOffsetCount);
	//UniformDataKey& operator=	(const UniformDataKey& other);

private:
	static const int						MAX_DESC_COUNT = 1024;
	const VkDescriptorSetLayoutBinding*		m_descriptorSetLayouts;
	int										m_descriptorSetLayoutCount;
	const svkDescriptorData*				m_descriptorDatas;
	//svkBuffer								m_frameUniformBuffer;
	uint64_t								m_dataHandles[MAX_DESC_COUNT];
	int										m_dataHandleIndex;

}; // class UniformDataKey

} // namespace cat

namespace scl {

inline uint hash_function(const cat::DescriptorDataKey& s)
{
	return s.hash();
}

}

