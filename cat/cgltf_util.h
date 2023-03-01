
#include "cgltf/cgltf.h"

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


const unsigned char*	cgltf_get_accessor_buffer	(const cgltf_accessor* accessor);
size_t					cgltf_num_components		(int type); 
size_t					cgltf_component_size		(int component_type);
size_t					cgltf_calc_size				(int type, int component_type);
bool					cgltf_primitive_has_attr	(const cgltf_primitive*	primitive, const char* const attrName);


#ifdef __cplusplus
}
#endif
