
#include "cat/material.h"
#include "cat/IRender.h"
#include "cat/env.h"

#include "scl/string.h"

namespace cat {

//Material* Material::s_default = NULL;

Material::Material() : m_render(NULL), m_env(NULL), m_textureFile(NULL)
{

}

Material::~Material()
{
	release();
}

void Material::init(IRender* render, const char* const textureFilename, Env* env)
{
	m_render		= render;
	m_env			= env;
	m_textureFile	= env->getTextureFile(textureFilename);

	//int			textureWidth	= 0;
	//int			textureHeight	= 0;
	//int			pitch			= 0;
	//gfx::PIXEL	pixel			= gfx::PIXEL_INVALID;
	//m_texture	= render->createTexture(textureFilename, &textureWidth, &textureHeight, &pitch, &pixel);
}

void Material::release()
{
	// 双 NULL 守卫：避免 init 失败 / 未调用 init 时 m_env 仍为 NULL 还去解引用
	if (NULL != m_env && NULL != m_textureFile)
		m_env->releaseTextureFile(m_textureFile);
	m_textureFile	= NULL;
	m_env			= NULL;
	m_render		= NULL;
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

