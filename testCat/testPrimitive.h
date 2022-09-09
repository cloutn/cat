#pragma once

#include "scl/varray.h"
#include "scl/vector.h"

namespace cat {

class Primitive;
class IRender;
class Env;
class Object;
class vertex_color;

Primitive* _createGridPrimitive				(IRender* render, Env* env);
Primitive* _createTestVulkanPrimitive		(IRender* render, Env* env);
Primitive* _createTestVulkanPrimitiveColor	(IRender* render, Env* env);
Primitive* _createBone						(Object* root, IRender* render, Env* env);
void		CollectBoneVertices				(Object* root, scl::varray<vertex_color>& vertices, scl::varray<uint16>& indices, int level = 0);

}
