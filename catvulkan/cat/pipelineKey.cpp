#include "cat/pipelineKey.h"

#include "cat/xxhash.h"

#include <string.h>

namespace cat {

uint32 PipelineKey::hash() const
{
	return XXH32(this, sizeof(PipelineKey), 0);	
}

PipelineKey::PipelineKey() : 
	m_vertexAttrs	(NULL),
	m_shader		(NULL),
	m_topology		(0),
	m_renderPass	(NULL)
{

}

PipelineKey::PipelineKey(const void* const vertexAttrs, const void* const shader, const int topology, void* renderPass) :
	m_vertexAttrs	(vertexAttrs),
	m_shader		(shader),
	m_topology		(topology),
	m_renderPass	(renderPass)
{

}

bool PipelineKey::operator==(const PipelineKey& other) const
{
	return 0 == memcmp(this, &other, sizeof(*this));
}

} // namespace cat


