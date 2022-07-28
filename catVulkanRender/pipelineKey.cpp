#include "pipelineKey.h"

#include "xxhash.h"

#include <string.h>

namespace cat {

uint32 PipelineKey::hash() const
{
	return XXH32(this, sizeof(PipelineKey), 0);	
}

PipelineKey::PipelineKey() : 
	m_vertexAttrs	(NULL),
	m_shader		(NULL),
	m_topology		(0)
{

}

PipelineKey::PipelineKey(const void* const vertexAttrs, const void* const shader, const int topology) :
	m_vertexAttrs	(vertexAttrs),
	m_shader		(shader),
	m_topology		(topology)
{

}

bool PipelineKey::operator==(const PipelineKey& other) const
{
	return 0 == memcmp(this, &other, sizeof(*this));
}

} // namespace cat


