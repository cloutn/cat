#pragma once

#include "scl/varray.h"
#include "scl/vector.h"

namespace cat {

class Primitive;
class IRender;
class Env;
class Object;
class vertex_color;

Primitive*	createTestVulkanPrimitive				(IRender* render, Env* env);
Primitive*	createTestVulkanPrimitiveColor			(IRender* render, Env* env);
Primitive*	createBone								(Object* root, IRender* render, Env* env);
void		collectBoneVertices						(Object* root, scl::varray<vertex_color>& vertices, scl::varray<uint16>& indices, int level = 0);

Primitive*	createTestPrimitive_getVertexData		(IRender* render, Env* env, vertex_color* vertices, int vertexCount, uint16* indices, int indexCount);
void		testPrimitive_vertexAttr				(IRender* render, Env* env);
void		testPrimitive_vertexPosition			(IRender* render, Env* env);
void		testPrimitive_vertexPosition_edgeCase	(IRender* render, Env* env);

//void		testPrimitiveGetVertexData			(IRender* render, Env* env);
//void		testPrimitiveGetAllVertexAttrs		(IRender* render, Env* env);
//void		testPrimitiveVertexPositions		(IRender* render, Env* env);

}
