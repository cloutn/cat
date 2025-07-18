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
	// attr
	const VertexAttr*	attrs		= vertex_color::get_attr();
	const int			attrCount	= vertex_color::get_attr_count();

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

	Primitive* p = new Primitive();
	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_LINES);
	p->setIndices(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, attrCount);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setShaderWithPick(env->getDefaultShader(macros), env);

	Object* obj = new Object();

	Mesh* mesh = new Mesh();
	mesh->setParent(obj);
	mesh->addPrimitive(p);

	obj->setMesh(mesh);

	return obj;
}

//int generate_sphere_vertices(float radius, int latitude_steps, int longitude_steps, vertex_color*& vertices)
//{
//    int vertex_count = (latitude_steps + 1) * (longitude_steps + 1);
//    vertices = new vertex_color[vertex_count];
//    for (int i = 0; i <= latitude_steps; ++i) 
//    {
//        float theta = i * scl::PI / latitude_steps;
//        float sin_theta = sin(theta);
//        float cos_theta = cos(theta);
//        for (int j = 0; j <= longitude_steps; ++j) 
//        {
//            float phi = j * 2.0f * scl::PI / longitude_steps;
//            float sin_phi = sin(phi);
//            float cos_phi = cos(phi);
//			int idx = i * (longitude_steps + 1) + j;
//			assert(idx < vertex_count);
//            vertices[idx].position.x = radius * sin_theta * cos_phi;
//            vertices[idx].position.y = radius * sin_theta * sin_phi;
//            vertices[idx].position.z = radius * cos_theta;
//        }
//    }
//	return vertex_count;
//}


cat::Object* _createCube(IRender* render, Env* env)
{
	// TODO: 实现立方体创建
	// 
	//vertex_color* sphere_vertices = NULL;
	//int vertex_count = generate_sphere_vertices(1, 10, 10, sphere_vertices);

	vertex_color vertices[8] = 
	{
		{ 0.5,	0.5,	0.5,	0xFFFFFFFF },
		{ 0.5,	0.5,	-0.5,	0xFFFFFFFF },
		{ -0.5,	0.5,	0.5,	0xFFFFFFFF },
		{ -0.5,	0.5,	-0.5,	0xFFFFFFFF },
		{ 0.5,	-0.5,	0.5,	0xFFFFFFFF },
		{ 0.5,	-0.5,	-0.5,	0xFFFFFFFF },
		{ -0.5,	-0.5,	0.5,	0xFFFFFFFF },
		{ -0.5,	-0.5,	-0.5,	0xFFFFFFFF },
	};
	//uint16 indices[]

	//Primitive* p = new Primitive();
	//p->setRender(render);
	//p->setEnv(env);
	//p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);

	//p->setVertices();
	//p->setIndices();
	return NULL;
}

} // namespcae cat



