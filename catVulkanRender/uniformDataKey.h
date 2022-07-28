#pragma once

#include "simplevulkan.h"

struct VkDescriptorSetLayoutBinding;

namespace cat {

class UniformDataKey
{
public:
	UniformDataKey				();
	int		hash				() const; // int GetHash() const
	void	_streamToHandles	(); // void _streamToHandles()
	bool	operator==			(const UniformDataKey& other) const;
	void	init				(
		const VkDescriptorSetLayoutBinding*	binds, 
		const int							bindCount, 
		const svkDescriptorData*			datas, 
		svkBuffer&							frameUniformBuffer);
	//UniformDataKey& operator=	(const UniformDataKey& other);

private:
	static const int						MAX_DESC_COUNT = 1024;
	const VkDescriptorSetLayoutBinding*		m_binds;
	int										m_bindCount;
	const svkDescriptorData*				m_datas;
	svkBuffer								m_frameUniformBuffer;
	uint64_t								m_dataHandles[MAX_DESC_COUNT];
	int										m_dataHandleIndex;

}; // class UniformDataKey

} // namespace cat


