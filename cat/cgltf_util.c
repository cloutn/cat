
#include "cat/cgltf_util.h"

const unsigned char* cgltf_get_accessor_buffer(const cgltf_accessor* accessor)
{
	const unsigned char* buf = (const unsigned char*)accessor->buffer_view->buffer->data + accessor->buffer_view->offset + accessor->offset;
	return buf;
}

size_t cgltf_num_components(int type) 
{
	switch (type)
	{
	case cgltf_type_vec2:
		return 2;
	case cgltf_type_vec3:
		return 3;
	case cgltf_type_vec4:
		return 4;
	case cgltf_type_mat2:
		return 4;
	case cgltf_type_mat3:
		return 9;
	case cgltf_type_mat4:
		return 16;
	case cgltf_type_invalid:
	case cgltf_type_scalar:
	default:
		return 1;
	}
}

size_t cgltf_component_size(int component_type) 
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

size_t cgltf_calc_size(int type, int component_type)
{
	cgltf_size component_size = cgltf_component_size(component_type);
	if (type == cgltf_type_mat2 && component_size == 1)
	{
		return 8 * component_size;
	}
	else if (type == cgltf_type_mat3 && (component_size == 1 || component_size == 2))
	{
		return 12 * component_size;
	}
	return component_size * cgltf_num_components(type);
}

bool cgltf_primitive_has_attr(const cgltf_primitive* primitive, const char* const attrName)
{
	int len = strlen(attrName);
	for (unsigned int i = 0; i < primitive->attributes_count; ++i)
	{
		if (0 == _strnicmp(attrName, primitive->attributes[i].name, len))
			return true;
	}
	return false;
}


