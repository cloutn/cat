#include "cat/Primitive.h"

#include "cat/IRender.h"
#include "cat/material.h"
#include "cat/cgltf_util.h"
#include "cat/shader.h"
#include "cat/mesh.h"
#include "cat/object.h"
#include "cat/env.h"
#include "cat/def.h"

#include "scl/assert.h"

#include "cgltf/cgltf.h"

#include <string.h>

namespace cat {
	
int _attrNameToIndex(const char* const name);

Primitive::Primitive() : 
	m_render					(NULL),
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
	m_pickShader				(NULL),
	m_parent					(NULL)
{

}

Primitive::~Primitive()
{
	release();
}



//void Primitive::initFromBuffer(
//		const void*		vertexBuffer, 
//		const int		vertexBufferSize, 
//		const int		vertexCount, 
//		const void*		indexBuffer,
//		const int		indexBufferSize, 
//		const int		indexCount,
//		const int		attrCount,
//		const VertexAttr* attrs,
//		IRender*		render)
//{
//	if (NULL == render || NULL == vertexBuffer || NULL == indexBuffer)
//		return;
//	if (0 == vertexBufferSize || 0 == indexBufferSize || 0 == vertexCount || 0 == indexCount)
//		return;
//	if (0 == attrCount || NULL == attrs)
//		return;
//
//	release();
//
//	m_render = render;
//
//	m_deviceVertexBuffer	= render->createVertexBuffer(vertexCount);
//	m_deviceIndexBuffer		= render->createIndexBuffer	(indexCount);
//	render->copyVertexBuffer(vertexBuffer,	m_deviceVertexBuffer,	vertexBufferSize);
//	render->copyIndexBuffer	(indexBuffer,	m_deviceIndexBuffer,	indexBufferSize);
//	m_vertexCount = vertexCount;
//	m_indexCount = indexCount;
//
//	assert(m_attrCount == 0);
//	assert(m_attrs == NULL);
//
//	m_attrCount = attrCount;
//
//	m_attrs = new VertexAttr[m_attrCount];
//	memcpy(m_attrs, attrs, m_attrCount * sizeof(attrs[0]));
//}

//void* createBuffer(const cgltf_accessor* accessor, scl::tree<cgltf_buffer_view*, void*>& bufferMap, IRender* render)
//{
//	cgltf_buffer_view*	view		= accessor->buffer_view;
//	auto				it			= bufferMap.find(view);
//	if (it != bufferMap.end())
//	{
//		//G.refCounter.AddRef(it->second);
//		return (*it).second;
//	}
//
//	//elementCount = view->size / cgltf_calc_size(accessor->type, accessor->component_type);
//	const byte*	pBuffer			= (byte*)view->buffer->data + view->offset;
//	void*		deviceBuffer	= render->createIndexBuffer(-1);
//	if (view->type == cgltf_buffer_view_type_indices)
//		render->copyIndexBuffer(pBuffer, deviceBuffer, view->size);
//	//else if (view->type == cgltf_buffer_view_type_vertices)
//	else 
//		render->copyVertexBuffer(pBuffer, deviceBuffer, view->size);
//	//bufferMap.insert(std::make_pair(view, deviceBuffer));
//	bufferMap.add(view, deviceBuffer);
//	//G.refCounter.AddRef(deviceBuffer);
//	return deviceBuffer;
//}

//void Primitive::_loadVertexOriginal(const cgltf_primitive&	primitive, Env* env, IRender* render)
//{
//	// vertex
//	if (primitive.attributes_count <= 0)
//		return;
//	m_attrCount					= primitive.attributes_count;
//	m_deviceVertexBuffers		= new void*[m_attrCount];
//	memset(m_deviceVertexBuffers, 0, sizeof(m_deviceVertexBuffers[0]) * m_attrCount);
//	m_attrs						= new VertexAttr[m_attrCount];
//	memset(m_attrs, 0, sizeof(m_attrs[0]) * m_attrCount);
//
//	for (size_t i = 0; i < primitive.attributes_count; ++i)
//	{
//		cgltf_attribute&	attr			= primitive.attributes[i];
//		cgltf_accessor*		accessor		= attr.data;
//
//		m_attrs[i].index		= _attrNameToIndex(attr.name);
//		if (m_attrs[i].index < 0)
//		{
//			printf("attr index = %s\n", attr.name);
//			continue;
//		}
//
//		m_deviceVertexBuffers[i] = createBuffer(attr.data, env->bufferMap(), render);
//
//		m_attrs[i].size			= cgltf_num_components(accessor->type);
//		m_attrs[i].dataType		= static_cast<ELEM_TYPE>(gltf_type_to_attr_type(accessor->component_type));
//		m_attrs[i].normalize	= accessor->normalized;
//		m_attrs[i].stride		= (accessor->buffer_view->stride == 0) ?  accessor->stride : accessor->buffer_view->stride;
//		m_attrs[i].offset		= (void*)accessor->offset;
//	}
//}

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

bool _hasAttr(const cgltf_primitive&	primitive, const char* const attrName)
{
	int len = strlen(attrName);
	for (uint i = 0; i < primitive.attributes_count; ++i)
	{
		if (0 == _strnicmp(attrName, primitive.attributes[i].name, len))
			return true;	
	}
	return false;
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

	// calculate sizeof(attr)
	// calculate offset
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

	//ShaderMacro macros[128];
	//int			macroCount = 0;
	ShaderMacroArray macros;

	assert(NULL == m_shader);

	bool hasJoints	= _hasAttr(primitive, "joints");
	bool hasWeights	= _hasAttr(primitive, "weights");
	if (skinJointCount > 0 && hasJoints && hasWeights)
	{
		macros.add("SKIN");
		macros.add("JOINT_MATRIX_COUNT", skinJointCount);
	}
	if (_hasAttr(primitive, "NORMAL"))
	{
		macros.add("NORMAL");
	}
	if (_hasAttr(primitive, "TANGENT"))
	{
		macros.add("TANGENT");
	}
	if (_hasAttr(primitive, "TEXCOORD"))
	{
		if (NULL != m_material && NULL != m_material->texture())
		{
			macros.add("TEXTURE");
		}
		else
		{
			const char* objectName = (NULL == m_parent || NULL == m_parent->parent()) ? "" : m_parent->parent()->name().c_str();
			const char* meshName	= NULL == m_parent  ? "" : m_parent->name().c_str();
			printf("warning : object [%s] mesh [%s]\n\tprimitive attribute has TEXCOORD, but material has NO texture.\n", objectName, meshName);
		}
	}
	if (_hasAttr(primitive, "COLOR"))
	{
		macros.add("COLOR");
	}

	m_shader = m_env->getShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros.data(), macros.size());


	//macros.remove("TEXTURE");
	//macros.remove("COLOR");
	//macros.remove("TANGENT");
	//macros.remove("NORMAL");

	macros.add("PICK");
	m_pickShader = m_env->getShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag",  macros.data(), macros.size());

}

void Primitive::draw(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, IRender* render)
{
	//Material* material = m_material;
	////if (NULL == material || NULL == material->texture())
	////	material = Material::_default();
	//if (NULL == m_material)
	//	return;

	void* texture = NULL;
	if (NULL == m_material || NULL == m_material->texture())
		texture = NULL;//m_env->getDefaultMaterial()->texture();
	else
		texture = m_material->texture();

	render->draw2(
		texture,
		vertexBuffers(),
		m_primitiveType,
		indexBuffer(),
		m_indexCount,
		m_indexComponentType,
		m_indexOffset,
		attrCount(),
		attrs(),
		//material->deviceShader(),
		m_shader->shader(render),
		mvp,
		jointMatrices,
		jointMatrixCount);
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
	//safe_delete(m_shader); // shader is deleted in ShaderCache
	m_attrCount = 0;
}

void Primitive::loadMemory(
	void*				indices,
	const int			indexCount,
	const ELEM_TYPE		indexComponentType,
	void**				verticesList,
	int*				vertexCountList,
	int*				sizeOfVertex,
	int					attrCount,
	VertexAttr*			attrs,
	int*				attrVertexBuffer,
	PRIMITIVE_TYPE		primitiveType,
	Material*			material,
	Shader*				shader,
	IRender*			render)
{
	release();

	m_render = render;

	m_primitiveType = primitiveType;

	// index
	m_indexCount = indexCount;
	m_indexOffset = 0;
	m_indexComponentType = indexComponentType;
	m_deviceIndexBuffer = render->createIndexBuffer(-1);
	const int indexBytes = m_indexCount * cgltf_component_size(m_indexComponentType);
	render->copyIndexBuffer(indices, m_deviceIndexBuffer, indexBytes);

	// vertex
	m_attrCount = attrCount;
	m_deviceVertexBuffers = new void*[m_attrCount];
	memset(m_deviceVertexBuffers, 0, sizeof(m_deviceVertexBuffers[0]) * m_attrCount);
	m_attrs = new VertexAttr[m_attrCount];
	memset(m_attrs, 0, sizeof(m_attrs[0]) * m_attrCount);
	memcpy(m_attrs, attrs, sizeof(m_attrs[0]) * m_attrCount);
	scl::tree<int, void*> bufferMap;
	for (int i = 0; i < attrCount; ++i)
	{
		const int bufferIndex = attrVertexBuffer[i];
		scl::tree<int, void*>::iterator it = bufferMap.find(bufferIndex);
		if (it == bufferMap.end())
		{
			VertexAttr& attr = attrs[i];
			m_deviceVertexBuffers[i] = render->createVertexBuffer(-1);
			//G.refCounter.AddRef(m_deviceVertexBuffers[i]);
			const int vertexBytes = vertexCountList[bufferIndex] * sizeOfVertex[bufferIndex];
			render->copyVertexBuffer(verticesList[bufferIndex], m_deviceVertexBuffers[i], vertexBytes);
			bufferMap.add(bufferIndex, m_deviceVertexBuffers[i]);
		}
		else
			m_deviceVertexBuffers[i] = (*it).second;
	}

	// material
	m_material = material;
	m_shader = shader;
}

void Primitive::loadShader(const char* const vs_filename, const char* const ps_filename, ShaderMacro* macros, const int macroCount)
{
	if (NULL != m_shader)
	{
		assert(false);
		return;
	}
	m_shader = m_env->getShader(vs_filename, ps_filename, macros, macroCount);
	//m_shader = new Shader();
	//m_shader->load(vs_filename, ps_filename);
}

void Primitive::setTexture(const char* const filename)
{
	if (NULL != m_material)
	{
		assert(false);
		return;
	}
	m_material = new Material();
	//m_material->setTexture();
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
	memcpy(m_attrBufferIndices, attrBufferIndices, m_attrCount * sizeof(attrBufferIndices[0]));
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

} // namespace cat


