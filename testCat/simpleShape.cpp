#include "./simpleShape.h"

#include "cat/primitive.h"
#include "cat/object.h"
#include "cat/mesh.h"
#include "cat/shaderMacro.h"
#include "cat/vertex.h"
#include "cat/shader.h"
#include "cat/env.h"
#include "cat/def.h"

#include "scl/vector.h"

namespace cat {

using scl::vector3;

Object* _createGrid(IRender* render, Env* env)
{
	Primitive* p = new Primitive();

	// attr
	VertexAttr	attrs[16] = { 0 };
	int			attrCount = vertex_color::getAttr(attrs, countof(attrs));

	ShaderMacroArray macros;
	macros.add("COLOR");

	// vertex 
	const int ROW		= 3;
	const int COLUMN	= 3;
	const int VERTEX_COUNT = (ROW + COLUMN) * 2;
	uint32 color = 0xFFFF0000;
	vertex_color vertices[VERTEX_COUNT];
	memset(vertices, 0, sizeof(vertices));
	for (int i = 0; i < ROW; ++i)
	{
		float z = static_cast<float>(i - (ROW - 1) / 2);	
		vertices[2 * i		] = { vector3{-10.0f,	0, z}, color };
		vertices[2 * i + 1	] = { vector3{10.0f,	0, z}, color };
	}
	for (int i = 0; i < COLUMN; ++i)
	{
		float x = static_cast<float>(i - (COLUMN- 1) / 2);	
		vertices[ROW * 2 + 2 * i		] = { vector3{x,	0, 10}, color };
		vertices[ROW * 2 + 2 * i + 1	] = { vector3{x,	0, -10}, color };

	}

	// index
	uint16 indices[VERTEX_COUNT];
	for (int i = 0; i < VERTEX_COUNT; ++i)
		indices[i] = i;

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_LINES);
	p->setIndices(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, attrCount);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);

	Object* obj = new Object();

	Mesh* mesh = new Mesh();
	mesh->setParent(obj);
	mesh->addPrimitive(p);

	obj->setMesh(mesh);

	return obj;
}


cat::Object* _createCube(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);

	//p->setVertices();
	//p->setIndices();
	return NULL;
}

} // namespcae cat



