#include "cat/env.h"

#include "cat/material.h"
#include "cat/shaderCache.h"
#include "cat/shader.h"
#include "cat/object.h"
#include "cat/IRender.h"

#include "cgltf/cgltf.h"

namespace cat {

Env::Env() : 
	m_defaultShader		(NULL),
	m_render			(NULL)
{
	m_shaderCache = new ShaderCache();
}

Env::~Env()
{
	if (NULL != m_shaderCache)
		delete m_shaderCache;
}

Shader* Env::getShader(const char* const vsFilename, const char* const fsFilename, ShaderMacro* macros, const int macroCount)
{
	return m_shaderCache->getShader(vsFilename, fsFilename, macros, macroCount);
}

void Env::setDefaultShader(const char* const vsFilename, const char* const fsFilename)
{
	m_defaultShader = getShader(vsFilename, fsFilename, NULL, 0);
}

Shader* Env::getDefaultShader()
{
	return m_defaultShader;
}

void Env::setDefaultMaterial(const char* const textureFilename)
{
	m_defaultMaterialTextureName = textureFilename;
}

Material* Env::getDefaultMaterial()
{
	Material* mat = new Material();
	mat->init(render(), m_defaultMaterialTextureName.c_str(), this);
	return mat;
}

const TextureFile* Env::getTextureFile(const char* const filename)
{
	TextureFileMap::iterator it = m_textureFiles.find(filename);
	if (it != m_textureFiles.end())
	{
		TextureFile& textureFile = (*it).second;
		++textureFile.counter;
		return &textureFile;
	}

	int			width	= 0;
	int			height	= 0;
	int			pitch	= 0;
	PIXEL		pixel	= PIXEL_INVALID;
	void*		texture	= render()->createTexture(filename, &width, &height, &pitch, &pixel);
	if (NULL == texture)
	{
		assert(false);
		return &TextureFile::empty();
	}

	TextureFileMap::iterator newIter = m_textureFiles.add(filename, TextureFile(NULL, texture, 1));
	TextureFile&		newFile = (*newIter).second;
	newFile.name		= (*newIter).first.c_str();
	newFile.width		= width;
	newFile.height		= height;
	newFile.pitch		= pitch;
	newFile.pixel		= pixel;
	return &newFile;
}

void Env::releaseTextureFile(const TextureFile* pTextureFile)
{
	TextureFileMap::iterator it = m_textureFiles.find(pTextureFile->name);
	if (it == m_textureFiles.end())
		return;

	TextureFile& textureFile = (*it).second;
	int& counter = textureFile.counter;
	--counter;
	if (counter > 0)
		return;

	render()->releaseTexture(textureFile.texture);
	m_textureFiles.erase(it);
}

void Env::addToGltfNodeMap(cgltf_node* node, int objectID)
{
	scl::tree<cgltf_node*, int>::iterator it = m_gltfNodeMap.find(node);
	if (it != m_gltfNodeMap.end())
	{
		assert(false);
		return;
	}
	m_gltfNodeMap.add(node, objectID);
}

int Env::getObjectIDByGltfNode(cgltf_node* node)
{
	scl::tree<cgltf_node*, int>::iterator it = m_gltfNodeMap.find(node);
	if (it == m_gltfNodeMap.end())
		return -1;
	return (*it).second;
}

Object* Env::getObjectByGltfNode(cgltf_node* node)
{
	int id = getObjectIDByGltfNode(node);	
	return Object::objectByID(id);
}

void Env::clearGltfNodeMap()
{
	m_gltfNodeMap.clear();
}


void Env::clearPickPrimtives()
{
	m_pickPrimitives.clear();
}

scl::vector4 Env::registerPickPrimitive(Primitive* primitive)
{
	uint32			id = m_pickPrimitives.size();
	m_pickPrimitives.push_back(primitive);
	scl::vector4	color;
	argb_to_float(id, color.a, color.r, color.g, color.b);
	return color;
}

cat::Primitive* Env::getPickPrimitive(scl::vector4& color)
{
	uint32			id = float_to_argb(color.a, color.r, color.g, color.b);
	if (id >= m_pickPrimitives.size())
		return NULL;
	return m_pickPrimitives[id];
}

//void Env::addToBufferMap(cgltf_buffer_view* bufferView, void* buffer)
//{
//	if (m_bufferMap.count(bufferView))
//	{
//		assert(false);
//		return;
//	}
//	m_bufferMap.add(bufferView, buffer);
//}
//
//void Env::clearBufferMap()
//{
//	m_bufferMap.clear();
//}
//
//void* Env::getBufferByBufferView(cgltf_buffer_view* bufferView)
//{
//	scl::tree<cgltf_buffer_view*, void*>::iterator it = m_bufferMap.find(bufferView);
//	if (it == m_bufferMap.end())
//		return NULL;
//	return (*it).second;
//}

} // namespace cat

