#include "cat/Primitive.h"

#include "cat/IRender.h"
#include "cat/material.h"
#include "cat/cgltf_util.h"
#include "cat/shader.h"
#include "cat/shaderCache.h"
#include "cat/mesh.h"
#include "cat/object.h"
#include "cat/env.h"
#include "cat/def.h"

#include "scl/assert.h"

#include "cgltf/cgltf.h"

#include <string.h>

#include "gltf_raw_render.h"

namespace cat {
	
int _attrNameToIndex(const char* const name);

Primitive::Primitive() : 
	m_render					(NULL),
	m_env						(NULL),
	m_deviceIndexBuffer			(NULL),
	m_indexCount				(0),
	m_indexComponentType		(ELEM_TYPE_INVALID),
	m_indexOffset				(0),
	m_deviceVertexBuffers		(NULL),
	m_attrCount					(0),
	m_attrs						(NULL),
	m_attrBufferIndices			(NULL),
	m_primitiveType				(cat::PRIMITIVE_TYPE_POINTS),
	m_material					(NULL),
	m_shader					(NULL),
	//m_shaderMacros				(NULL),
	m_pickShader				(NULL),
	m_parent					(NULL)
{
}

Primitive::~Primitive()
{
	release();
}

int _attrNameToIndex(const char* const name)
{
	int attrIndex = -1;
	if		(0 == _stricmp(name, "POSITION"		))	attrIndex = 0;
	else if (0 == _stricmp(name, "NORMAL"		))	attrIndex = 1;
	else if (0 == _stricmp(name, "TANGENT"		))	attrIndex = 2;
	else if (0 == _stricmp(name, "TEXCOORD_0"	))	attrIndex = 3;
	else if (0 == _stricmp(name, "TEXCOORD_1"	))	attrIndex = 4;
	else if (0 == _stricmp(name, "COLOR_0"		))	attrIndex = 5;
	else if (0 == _stricmp(name, "JOINTS_0"		))	attrIndex = 6;
	else if (0 == _stricmp(name, "WEIGHTS_0"	))	attrIndex = 7;
	return attrIndex;
}



void Primitive::_loadVertex(const cgltf_primitive&	primitive, IRender* render)
{
	if (primitive.attributes_count <= 0)
		return;
	m_attrCount					= primitive.attributes_count;
	m_deviceVertexBuffers		= new void*[m_attrCount];
	memset(m_deviceVertexBuffers, 0, sizeof(m_deviceVertexBuffers[0]) * m_attrCount);
	m_attrs						= new VertexAttr[m_attrCount];
	memset(m_attrs, 0, sizeof(m_attrs[0]) * m_attrCount);
	int		attrOffsets[1024]	= { 0 };

	// calculate sizeof(attr), calculate offset
	int		sizeofVertex		= 0;
	int		vertexCount			= primitive.attributes[0].data->count;
	for (int i = 0; i < m_attrCount; ++i)
	{
		cgltf_attribute&	attr		= primitive.attributes[i];
		cgltf_accessor*		accessor	= attr.data;		
		int					size		= cgltf_calc_size(accessor->type, accessor->component_type);
		sizeofVertex += size;
		attrOffsets[i + 1] = attrOffsets[i] + size;
	}

	int bufferSize = sizeofVertex * vertexCount;

	//TODO don't new a big buffer!!!
	byte* buffer = new byte[bufferSize];
	memset(buffer, 0, bufferSize);

	for (int i = 0; i < m_attrCount; ++i)
	{
		cgltf_attribute&	attr		= primitive.attributes[i];
		cgltf_accessor*		accessor	= attr.data;	

		m_attrs[i].index = _attrNameToIndex(attr.name);
		if (m_attrs[i].index < 0)
		{
			printf("attr index = %s\n", attr.name);
			continue;
		}
		m_attrs[i].size			= cgltf_num_components(accessor->type);
		m_attrs[i].dataType		= static_cast<ELEM_TYPE>(gltf_type_to_attr_type(accessor->component_type));
		m_attrs[i].normalize	= accessor->normalized;
		m_attrs[i].stride		= sizeofVertex;
		m_attrs[i].offset		= reinterpret_cast<void*>(static_cast<uintptr_t>(attrOffsets[i]));

		int	elementSize = accessor->stride; 
		if (elementSize == 0)
			elementSize = cgltf_calc_size(accessor->type, accessor->component_type);;;

		cgltf_buffer_view*		view		= accessor->buffer_view;
		const byte*				viewBuffer	= cgltf_get_accessor_buffer(accessor); //(byte*)view->buffer->data + view->offset + accessor->offset;
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			assert(vi * sizeofVertex + attrOffsets[i] + elementSize <= bufferSize);
			byte*		dst = buffer + vi * sizeofVertex + attrOffsets[i];
			const byte* src = viewBuffer + vi * elementSize;
			memcpy(dst, src, elementSize);
		}
	}

	void* deviceBuffer = render->createVertexBuffer(-1);
	render->copyVertexBuffer(buffer, deviceBuffer, bufferSize);
	m_deviceVertexBuffers[0] = deviceBuffer; // if all attrs share one buffer, we only need to set the first element in array, the vulkanRender will try to use first element when meet a NULL ptr in array.

	delete[] buffer;
}


void Primitive::load(cgltf_primitive* data, const char* const path, int skinJointCount, Mesh* parent, IRender* render, Env* env)
{
	if (NULL == data)
		return;

	release();

	m_env		= env;
	m_render	= render;
	m_parent	= parent;

	const cgltf_primitive&	primitive		= *data;
	const cgltf_accessor*	indices			= primitive.indices;

	m_primitiveType			= static_cast<PRIMITIVE_TYPE>(primitive.type);

	// index
	const byte*	pBuffer		= cgltf_get_accessor_buffer(indices);	//(byte*)indices->buffer_view->buffer->data + indices->buffer_view->offset + indices->offset;
	setIndices(pBuffer, indices->count, gltf_type_to_attr_type(indices->component_type));

	//_loadVertexOriginal(primitive, bufferMap, render);
	_loadVertex(primitive, render);

	// material
	if (NULL != primitive.material)
	{
		assert(NULL == m_material);
		m_material = new Material();
		m_material->load(primitive.material, path, render, env);
	}
	else
	{
		m_material	= m_env->getDefaultMaterial();
	}
}

void Primitive::draw(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, bool isPick, IRender* render)
{
	void* texture = NULL;
	if (NULL == m_material || NULL == m_material->texture())
		texture = NULL;		//m_env->getDefaultMaterial()->texture();
	else
		texture = m_material->texture();

	void**				_vertexBuffers	= vertexBuffers();
	void*				_indexBuffer	= indexBuffer();
	int					_attrCount		= attrCount();
	const VertexAttr*	_attrs			= attrs();
	void*				_shader			= isPick ? m_pickShader->shader(render) : m_shader->shader(m_render);
	scl::vector4		pickColor		= isPick ? m_env->registerPickPrimitive(this) : scl::vector4();
	void*				_pushConst		= isPick ? &pickColor : NULL;
	int					_pushConstSize	= isPick ? sizeof(scl::vector4) : 0;

	render->draw2(
		texture,
		_vertexBuffers,
		m_primitiveType,
		_indexBuffer,
		m_indexCount,
		m_indexComponentType,
		m_indexOffset,
		_attrCount,
		_attrs,
		_shader,
		mvp,
		jointMatrices,
		jointMatrixCount,
		_pushConst, 
		_pushConstSize);
}

template <typename T>
bool _isInArray(T* array, int lastIndex, T buf)
{
	for (int i = 0; i <= lastIndex; ++i)
	{
		if (array[i] == buf)
			return true;
	}
	return false;
}

void Primitive::release()
{
	//safe_delete(m_shaderMacros);

	if (NULL != m_deviceIndexBuffer)
	{
		//if (0 == G.refCounter.DecRef(m_deviceIndexBuffer))
		m_render->releaseIndexBuffer(m_deviceIndexBuffer);

		m_deviceIndexBuffer = NULL;
	}

	safe_delete_array(m_attrs);
	safe_delete_array(m_attrBufferIndices);
	if (NULL != m_deviceVertexBuffers)
	{
		for (int i = 0; i < m_attrCount; ++i)
		{
			void* buf = m_deviceVertexBuffers[i];
			if (NULL == buf)
				continue;
			if (_isInArray(m_deviceVertexBuffers, i - 1, buf))
				continue;
			//if (0 == G.refCounter.DecRef(buf))
			m_render->releaseVertexBuffer(buf);
			//m_deviceVertexBuffers[i] = NULL;
		}
		delete[] m_deviceVertexBuffers;
	}
	safe_delete(m_material);
	m_attrCount = 0;
}


//void Primitive::loadMemory(
//	void*				indices,
//	const int			indexCount,
//	const ELEM_TYPE		indexComponentType,
//	void**				verticesList,
//	int*				vertexCountList,
//	int*				sizeOfVertex,
//	int					attrCount,
//	VertexAttr*			attrs,
//	int*				attrVertexBuffer,
//	PRIMITIVE_TYPE		primitiveType,
//	Material*			material,
//	Shader*				shader,
//	IRender*			render)
//{
//	release();
//
//	m_render = render;
//
//	m_primitiveType = primitiveType;
//
//	// index
//	m_indexCount = indexCount;
//	m_indexOffset = 0;
//	m_indexComponentType = indexComponentType;
//	m_deviceIndexBuffer = render->createIndexBuffer(-1);
//	const int indexBytes = m_indexCount * cgltf_component_size(m_indexComponentType);
//	render->copyIndexBuffer(indices, m_deviceIndexBuffer, indexBytes);
//
//	// vertex
//	m_attrCount = attrCount;
//	m_deviceVertexBuffers = new void*[m_attrCount];
//	memset(m_deviceVertexBuffers, 0, sizeof(m_deviceVertexBuffers[0]) * m_attrCount);
//	m_attrs = new VertexAttr[m_attrCount];
//	memset(m_attrs, 0, sizeof(m_attrs[0]) * m_attrCount);
//	memcpy(m_attrs, attrs, sizeof(m_attrs[0]) * m_attrCount);
//	scl::tree<int, void*> bufferMap;
//	for (int i = 0; i < attrCount; ++i)
//	{
//		const int bufferIndex = attrVertexBuffer[i];
//		scl::tree<int, void*>::iterator it = bufferMap.find(bufferIndex);
//		if (it == bufferMap.end())
//		{
//			VertexAttr& attr = attrs[i];
//			m_deviceVertexBuffers[i] = render->createVertexBuffer(-1);
//			//G.refCounter.AddRef(m_deviceVertexBuffers[i]);
//			const int vertexBytes = vertexCountList[bufferIndex] * sizeOfVertex[bufferIndex];
//			render->copyVertexBuffer(verticesList[bufferIndex], m_deviceVertexBuffers[i], vertexBytes);
//			bufferMap.add(bufferIndex, m_deviceVertexBuffers[i]);
//		}
//		else
//			m_deviceVertexBuffers[i] = (*it).second;
//	}
//
//	// material
//	m_material = material;
//	m_shader = shader;
//}

void Primitive::setShaderWithPick(Shader* shader, Env* env)
{
	setShader(shader);
	m_pickShader = env->shaderCache()->getPickShader(shader);
}


void Primitive::setTexture(const char* const filename)
{
	if (NULL != m_material)
	{
		assert(false);
		return;
	}
	m_material = new Material();
}

void Primitive::setAttrs(const VertexAttr* attrs, const int attrCount, const int* attrBufferIndices)
{
	if (attrCount > m_attrCount)
	{
		if (NULL != m_attrs)
		{
			safe_delete_array(m_attrs);
			m_attrCount = 0;
		}
		m_attrs = new VertexAttr[attrCount];
		m_attrBufferIndices = new int[attrCount];
	}
	m_attrCount = attrCount;
	memcpy(m_attrs, attrs, m_attrCount * sizeof(attrs[0]));
	if (NULL == attrBufferIndices)
	{
		memset(m_attrBufferIndices, 0, sizeof(m_attrBufferIndices[0]) * attrCount);
	}
	else
	{
		memcpy(m_attrBufferIndices, attrBufferIndices, m_attrCount * sizeof(attrBufferIndices[0]));
	}
}

void Primitive::setIndices(const void* indices, const int indexCount, const ELEM_TYPE indexComponentType)
{
	if (NULL == m_render)
		return;

	// index
	m_indexCount			= indexCount;
	m_indexComponentType	= indexComponentType;
	m_deviceIndexBuffer		= m_render->createIndexBuffer(-1);
	const int indexBytes	= m_indexCount * elem_type_byte(m_indexComponentType);
	m_render->copyIndexBuffer(indices, m_deviceIndexBuffer, indexBytes);
	m_indexOffset = 0;
}

void Primitive::setVertices(void** verticesList, int* vertexCountList, int* sizeOfVertex)
{
	if (NULL == m_render)
		return;

	if (m_attrCount <= 0)
	{
		assertf(false, "set vertex attr first!");
		return;
	}

	const int attrCount = m_attrCount;
	m_deviceVertexBuffers = new void*[m_attrCount];
	memset(m_deviceVertexBuffers, 0, sizeof(m_deviceVertexBuffers[0]) * m_attrCount);
	scl::tree<int, void*> bufferMap;
	for (int i = 0; i < attrCount; ++i)
	{
		const int bufferIndex = m_attrBufferIndices[i];
		scl::tree<int, void*>::iterator it = bufferMap.find(bufferIndex);
		if (it == bufferMap.end())
		{
			VertexAttr& attr = m_attrs[i];
			m_deviceVertexBuffers[i] = m_render->createVertexBuffer(-1);
			//G.refCounter.AddRef(m_deviceVertexBuffers[i]);
			const int vertexBytes = vertexCountList[bufferIndex] * sizeOfVertex[bufferIndex];
			m_render->copyVertexBuffer(verticesList[bufferIndex], m_deviceVertexBuffers[i], vertexBytes);
			bufferMap.add(bufferIndex, m_deviceVertexBuffers[i]);
		}
		else
			m_deviceVertexBuffers[i] = (*it).second;
	}
}

void Primitive::setVertices(void* vertices, int vertexCount, int sizeOfVertex)
{
	void* verticesList[1] = { vertices };
	int vertexCountList[1] = { vertexCount };
	int sizeOfVertexList[1] = { sizeOfVertex };
	setVertices(verticesList, vertexCountList, sizeOfVertexList);
}

void Primitive::updateVertices(void* vertices, int vertexCount, int sizeOfVertex)
{
	if (m_deviceVertexBuffers[0] <= 0)
		return;
	m_render->copyVertexBuffer(vertices, m_deviceVertexBuffers[0], sizeOfVertex * vertexCount);
}

cat::Object* Primitive::parentObject()
{
	if (NULL == parent())
		return NULL;
	return parent()->parent();
}

} // namespace cat


