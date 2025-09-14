#include "cat/resources.h"

#include "cat/material.h"

#include "cgltf/cgltf.h"

#include "scl/string.h"

namespace cat {


//Resources::~Resources()
//{
//	for (int i = 0; i < m_materials.size(); ++i)
//	{
//		delete m_materials[i];
//	}
//	m_materials.clear();
//}
//
//void Resources::LoadMaterials(cgltf_material* materials, int materials_count, const char* const currentPath, int shader, IRender* render)
//{
//	for (int i = 0; i < materials_count; ++i)
//	{
//		cgltf_material&					gltf_mat	= materials[i];
//
//		if (!gltf_mat.has_pbr_metallic_roughness)
//			continue;
//
//		cgltf_pbr_metallic_roughness&	pbr_mr		= gltf_mat.pbr_metallic_roughness;
//		cgltf_texture_view&				textureView = pbr_mr.base_color_texture;
//
//		Material* mat = new Material();
//
//		if (NULL == textureView.texture->image || NULL == textureView.texture->image->uri)
//			continue;
//
//		string256 fullPath = currentPath;
//		fullPath += textureView.texture->image->uri;
//		mat->init(render, shader, fullPath.c_str());
//
//		m_materials.push_back(mat);
//	}
//}

} // namespace cat
