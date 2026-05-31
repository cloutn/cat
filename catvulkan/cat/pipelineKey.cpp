#include "cat/pipelineKey.h"

#include "cat/xxhash.h"

#include <string.h>

namespace cat {

uint32 PipelineKey::hash() const
{
	return XXH32(this, sizeof(PipelineKey), 0);	
}

// hash() / operator== 都对整个结构体内存做摘要/比较，包含字段间 padding 字节。
// ctor 必须用 memset 把 padding 强制清零，否则 padding 是栈/堆 garbage，
// 两个语义相同的 PipelineKey 会因 padding 不同而 hash 不等、memcmp 不等，
// 导致 m_pipelines cache 每帧 miss，重复创建相同 pipeline → 每帧泄漏。
// 必须先 memset 后赋值，反过来会把字段也清掉。
// TODO 这是止血修法。更稳健的做法是逐字段 hash + 逐字段 operator==，
//      避免 padding 参与；并加 static_assert(sizeof(PipelineKey)) 在加字段时提醒同步 hash/eq。
PipelineKey::PipelineKey()
{
	memset(this, 0, sizeof(*this));
	m_vertexAttrs	= NULL;
	m_shader		= NULL;
	m_topology		= 0;
	m_renderPass	= NULL;
}

PipelineKey::PipelineKey(const void* const vertexAttrs, const void* const shader, const int topology, void* renderPass)
{
	memset(this, 0, sizeof(*this));
	m_vertexAttrs	= vertexAttrs;
	m_shader		= shader;
	m_topology		= topology;
	m_renderPass	= renderPass;
}

bool PipelineKey::operator==(const PipelineKey& other) const
{
	return 0 == memcmp(this, &other, sizeof(*this));
}

} // namespace cat


