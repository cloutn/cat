#pragma once

#include "cat/typedef.h"

#include "cat/def.h"

#include <map>

struct cgltf_data;
struct cgltf_node;
struct cgltf_mesh;
struct cgltf_buffer_view;

namespace scl { class matrix; }

namespace cat {

class Mesh;

cgltf_data*	gltf_load_from_file			(const char* const filename);
void*		gltf_create_render_data		();
void		gltf_do_render				(cgltf_data*, void* renderData, void* m_shader, void* texture, const scl::matrix& mvp);
void		gltf_release				(cgltf_data*, void* renderData);
void		gltf_mesh_to_render_mesh	(cgltf_mesh* gltf_mesh, Mesh* engine_mesh);

ELEM_TYPE	gltf_type_to_attr_type		(int gltf_component_type);

} // namespace cat 

