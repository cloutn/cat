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
	const void*		m_vertexAttrs;	// TODO 引擎应该合理的管理顶点格式。
	const void*		m_shader;		// TODO shader应该支持找出重复 shader 的能力，不要重复创建完全相同的shader。这是引擎的责任
	int				m_topology;

}; // class PipelineKey


} // namespace cat {


