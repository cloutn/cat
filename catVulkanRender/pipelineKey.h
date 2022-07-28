#pragma once

#include "scl/type.h"

namespace cat {

struct PipelineKey
{
public:
	PipelineKey();
	PipelineKey(const void* const vertexAttr, const void* const shaderID, const int topology);

	uint32	hash		() const;
	bool	operator==	(const PipelineKey& other) const;

private:
	const void*		m_vertexAttrs;	// TODO ����Ӧ�ú���Ĺ������ʽ��
	const void*		m_shader;		// TODO shaderӦ��֧���ҳ��ظ� shader ����������Ҫ�ظ�������ȫ��ͬ��shader���������������
	int				m_topology;

}; // class PipelineKey


} // namespace cat {


