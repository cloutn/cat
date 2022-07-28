
#include "cat/material.h"
#include "cat/IRender.h"
#include "cat/env.h"

#include "scl/string.h"

#include "cgltf/cgltf.h"

namespace cat {

//Material* Material::s_default = NULL;

Material::Material() : m_render(NULL), m_textureFile(NULL)
{

}

Material::~Material()
{
	release();
}

void Material::load(cgltf_material* data, const char* const currentPath, IRender* render, Env* env)
{
	if (NULL == data)
		return;

	cgltf_texture* texture = NULL;
	if (data->has_pbr_metallic_roughness)
	{
		cgltf_texture_view& textureView	= data->pbr_metallic_roughness.base_color_texture;
		texture = textureView.texture;
	}
	else if (data->has_pbr_specular_glossiness)
	{
		cgltf_texture_view& textureView	= data->pbr_specular_glossiness.diffuse_texture;
		texture = textureView.texture;
	}
	else
	{
		texture = data->normal_texture.texture;
	}

	if (NULL == texture || NULL == texture->image || NULL == texture->image->uri)
		return;

	string256 fullPath = currentPath;
	fullPath += texture->image->uri;

	init(render, fullPath.c_str(), env);
}

void Material::init(IRender* render, const char* const textureFilename, Env* env)
{
	m_render	= render;
	m_env		= env;
	m_textureFile = env->getTextureFile(textureFilename);

	//int			textureWidth	= 0;
	//int			textureHeight	= 0;
	//int			pitch			= 0;
	//gfx::PIXEL	pixel			= gfx::PIXEL_INVALID;
	//m_texture	= render->createTexture(textureFilename, &textureWidth, &textureHeight, &pitch, &pixel);
}

void Material::release()
{
	if (NULL != m_textureFile)
	{
		m_env->releaseTextureFile(m_textureFile);
		m_textureFile = NULL;
	}
}

void* Material::texture()
{
	if (NULL == m_textureFile)
		return NULL;
	return m_textureFile->texture;
}

//Material* Material::_default()
//{
//	return s_default;
//}
//
//void Material::setDefault(IRender* render, const char* const textureFilename)
//{
//	if (NULL != s_default)
//	{
//		delete s_default;
//		s_default = NULL;
//	}
//	s_default = new Material();
//	s_default->init(render, textureFilename);
//}
//
//void Material::releaseDefault()
//{
//	delete s_default;
//	s_default = NULL;
//}

} // namespace cat {

