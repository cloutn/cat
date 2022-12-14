
#include "cat/gltf_raw_render.h"

#include "cat/mesh.h"
#include "cat/IRender.h"
#include "cat/cgltf_util.h"

#include "scl/string.h"
#include "scl/file.h"
#include "scl/matrix.h"

#include "cgltf/cgltf.h"

////// DEBUG //////
#include "GLES3/gl3.h"
////// DEBUG //////

#include <map>

#define glcheck(x)	do\
{ \
	x; \
	int err = glGetError(); \
	if ( err != GL_NO_ERROR ) \
	{ \
		gfx_assert(false); \
	} \
} while (false)


#ifndef assert
#define gfx_assert(expr) do { if (!(expr)) { throw 1; } } while(0)
#else
#define gfx_assert assert
#endif

namespace cat {

//BufferMap vbo_map;

typedef std::map<cgltf_buffer_view*, void*>	BufferMap;


void _drawMesh	(cgltf_mesh* mesh, BufferMap* vbo_map);
void _drawNode	(cgltf_node* node, BufferMap* vbo_map);
void _draw		(cgltf_data* data, BufferMap* vbo_map);

cgltf_data* gltf_load_from_file(const char* const filename)
{
	cgltf_data*		data = NULL;
	cgltf_options	options	= { cgltf_file_type_invalid };
	cgltf_result	result	= cgltf_parse_file(&options, filename, &data);
	if (result != cgltf_result_success)
	{
		cgltf_free(data);
		return NULL;
	}

	scl::stringPath s = filename;
	scl::extract_path(s.pstring());

	result = cgltf_load_buffers(&options, data, s.c_str());
	if (result != cgltf_result_success)
	{
		cgltf_free(data);
		return NULL;
	}
	return data;
}

void* gltf_create_render_data()
{
	return new BufferMap();
}

void gltf_do_render(cgltf_data* data, void* gltfRenderData, void* m_shader, void* texture, const scl::matrix& mvp)
{
	glUseProgram(GLuint(reinterpret_cast<uintptr_t>(m_shader)));
	int m_uniform_mvp = glGetUniformLocation(GLuint(reinterpret_cast<uintptr_t>(m_shader)), "mvp");
	assert(m_uniform_mvp >= 0);
	glcheck( glUniformMatrix4fv(m_uniform_mvp, 1, GL_FALSE, &mvp.m[0][0]) );

	uint tex = static_cast<uint>(reinterpret_cast<uint64>(texture));
	glcheck(glBindTexture(GL_TEXTURE_2D, tex));

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	BufferMap* vbo_map = static_cast<BufferMap*>(gltfRenderData);
	_draw(data, vbo_map);

	//clear
	glcheck( glDisableVertexAttribArray(0) );
	glcheck( glDisableVertexAttribArray(1) );
	glcheck( glDisableVertexAttribArray(2) );
	glcheck( glBindVertexArray(0) );
	glcheck( glBindTexture(GL_TEXTURE_2D, 0) );
	glcheck( glBindBuffer(GL_ARRAY_BUFFER, 0) );
	glcheck( glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0) );
}

void gltf_release(cgltf_data* data, void* renderData)
{
	cgltf_free(data);
	BufferMap* vbo_map = static_cast<BufferMap*>(renderData);
	delete vbo_map;
}

//void gltf_mesh_to_render_mesh(cgltf_mesh* gltf_mesh, Mesh* engine_mesh)
//{
//
//}

void _draw(cgltf_data* data, BufferMap* vbo_map)
{
	if (data->scene == NULL && data->scenes_count <= 0)
		return;
	cgltf_scene* scene = data->scene;
	if (NULL == scene)
		scene = &data->scenes[0];
	for (size_t i = 0; i < scene->nodes_count; ++i)
	{
		_drawNode(scene->nodes[i], vbo_map);
	}
}

void _drawNode(cgltf_node* node, BufferMap* vbo_map)
{
	_drawMesh(node->mesh, vbo_map);
	for (size_t i = 0; i < node->children_count; ++i)
	{
		_drawNode(node->children[i], vbo_map);
	}
}

//cgltf_get_buffer_view_index(cgltf_data* data, cgltf_buffer_view* view)
//{
//
//}

GLuint _getVbo(cgltf_buffer_view* bufferView, cgltf_type type, BufferMap* vbo_map)
{
	GLuint vbo = -1;
	BufferMap::const_iterator it = vbo_map->find(bufferView);
	if (it == vbo_map->end())
	{
		glGenBuffers(1, &vbo);
		glBindBuffer(bufferView->target, vbo);

		if (bufferView->type == cgltf_buffer_view_type_indices)
		{
			//printf("loading indices : size = %d\n", bufferView->size);
			uint16* indices = (uint16*)((byte*)bufferView->buffer->data + bufferView->offset);
			//for (size_t i = 0; i < bufferView->size / sizeof(uint16); ++i)
			//{
			//	printf("%d, ", indices[i]);
			//}
			//printf("\n");
		}
		else if (bufferView->type == cgltf_buffer_view_type_vertices)
		{
			//printf("loading vertex : size = %d\n", bufferView->size);
			float* indices = (float*)((byte*)bufferView->buffer->data + bufferView->offset);
			for (size_t i = 0; i < bufferView->size / sizeof(float); ++i)
			{
				//if (((i + 1) % 3) == 0 && bufferView->size > 400)
				//	indices[i] = 0;
				//if (type == cgltf_type_vec2 && indices[i] < 0)
				//	indices[i] = 0;
				//printf("%.3f, ", indices[i]);
				//if (type == cgltf_type_vec2 && ((i + 1) % 2 == 0))
				//	printf("\n");
				//if (type == cgltf_type_vec3 && ((i + 1) % 3 == 0))
				//	printf("\n");
			}
			//printf("\n");
		}

		glBufferData(
			bufferView->target,
			bufferView->size,
			(byte*)bufferView->buffer->data + bufferView->offset,
			GL_STATIC_DRAW);

		vbo_map->insert(std::make_pair(bufferView, reinterpret_cast<void*>(static_cast<uintptr_t>(vbo))));
		printf("loading buffer [%p]: file = %s, type = %d, target = %d\n", bufferView, bufferView->buffer->uri, bufferView->type, bufferView->target);
	}
	else
		vbo = GLuint(reinterpret_cast<uintptr_t>(it->second));
	return vbo;
}

void _drawMesh(cgltf_mesh* mesh, BufferMap* vbo_map)
{
	if (NULL == mesh)
		return;
	for (size_t i = 0; i < mesh->primitives_count; ++i) 
	{
		const cgltf_primitive&	primitive	= mesh->primitives[i];
		const cgltf_accessor*	indices		= primitive.indices;

		GLuint indices_vbo = _getVbo(indices->buffer_view, indices->type, vbo_map);
		
		//// buffer
		glcheck(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices_vbo));

		for (size_t j = 0; j < primitive.attributes_count; ++j)
		{
			cgltf_attribute& attr = primitive.attributes[j];

			//"attributes" : 
			//{
			//   "NORMAL" : 2,
			//   "POSITION" : 1,
			//   "TANGENT" : 3,
			//   "TEXCOORD_0" : 4
			//}

			int attrIndex = -1;
			if (0 == _stricmp(attr.name, "position"))
				attrIndex = 0;
			if (0 == _stricmp(attr.name, "texcoord_0"))
				attrIndex = 2;
			//if (attrIndex >= 0)
			if (attrIndex >= 0)
			{
				cgltf_accessor* accessor = attr.data;
				GLuint attr_vbo = _getVbo(accessor->buffer_view, accessor->type, vbo_map);
				glcheck(glBindBuffer(GL_ARRAY_BUFFER, attr_vbo));

				glcheck(glEnableVertexAttribArray(attrIndex));
				glcheck(glVertexAttribPointer(
					attrIndex, 
					cgltf_num_components(accessor->type), 
					accessor->gl_component_type, 
					accessor->normalized, 
					accessor->stride, 
					(void*)accessor->offset));
				//printf("set vertext attr : name = %s, index = %d, size = %d, type = %d, normalized = %d, stride = %d offset = %x\n", 
				//	attr.name, attrIndex, cgltf_num_components(accessor->type), accessor->gl_component_type, accessor->normalized, accessor->stride, accessor->offset);
			}
			else
			{
				//printf("attr index failed : %s\n", attr.name);
			}
		}

		glcheck(glDrawElements(
			primitive.type,
			indices->count,
			indices->gl_component_type,
			reinterpret_cast<void*>(indices->offset)));
	}
}


ELEM_TYPE gltf_type_to_attr_type(int gltf_component_type)
{
	switch (gltf_component_type)
	{
	case cgltf_component_type_r_8	: return ELEM_TYPE_INT8;
	case cgltf_component_type_r_8u	: return ELEM_TYPE_UINT8;
	case cgltf_component_type_r_16	: return ELEM_TYPE_INT16;
	case cgltf_component_type_r_16u	: return ELEM_TYPE_UINT16;
	case cgltf_component_type_r_32u	: return ELEM_TYPE_UINT32;
	case cgltf_component_type_r_32f	: return ELEM_TYPE_FLOAT;
	default : assert(false); break;
	};
	return ELEM_TYPE_INVALID;
}

static cgltf_size component_size(cgltf_component_type component_type) 
{
	switch (component_type)
	{
	case cgltf_component_type_r_8:
	case cgltf_component_type_r_8u:
		return 1;
	case cgltf_component_type_r_16:
	case cgltf_component_type_r_16u:
		return 2;
	case cgltf_component_type_r_32u:
	case cgltf_component_type_r_32f:
		return 4;
	case cgltf_component_type_invalid:
	default:
		return 0;
	}
}

} // namespace cat

