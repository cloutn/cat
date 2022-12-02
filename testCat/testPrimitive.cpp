
#include "testPrimitive.h"

#include "scl/vector.h"

#include "cat/IRender.h"
#include "cat/Primitive.h"
#include "cat/Object.h"
#include "cat/shaderMacro.h"
#include "cat/shader.h"
#include "cat/vertex.h"


namespace cat {

using scl::matrix;
using scl::vector2;
using scl::vector3;
using scl::vector4;



void _loadPrimivteFromMemory2(Primitive* p, IRender* render, Env* env)
{
	// index
	uint16 indices[] = { 0, 1, 2, 3 };
	// attr
	VertexAttr attrs[] = {
		{ 0, 3, ELEM_TYPE_FLOAT, 0, sizeof(vertex_color), 0 },
		{ 1, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1] = { {"COLOR", ""} };
	// vertex 
	uint32 color = 0xCCCCCCFF;
	vertex_color vertices[] = {
		{ vector3{ -100, 0, 0 }, color },
		{ vector3{ 100, 0, 0 }, color },
		{ vector3{ -100, 1, 0 }, color },
		{ vector3{ 100, 1, 0 }, color }
	};

	p->setEnv			(env);
	p->setRender		(render);
	p->setPrimitiveType	(PRIMITIVE_TYPE_LINES);
	p->setIndices		(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs			(attrs, countof(attrs), attrBuffers);
	p->setVertices		(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture		(NULL);
	p->loadShader		(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
}


Primitive* _createGridPrimitive(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 3, ELEM_TYPE_FLOAT, 0, sizeof(vertex_color), 0 },
		{ 5, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1];
	macros[0].name = "COLOR";

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
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}

void CollectBoneVertices(Object* root, scl::varray<vertex_color>& vertices, scl::varray<uint16>& indices, int level)
{
	if (root->childCount() <= 0)
		return;

	matrix	matRoot		= root->globalMatrixWithAnimation();
	vector3	posRoot		{ 0, 0, 0};
	posRoot.mul_matrix(matRoot);
	uint	fromColor	= 0xFFFFFFFF;
	uint	toColor		= 0xFF00FF00;
	vertices.push_back({ posRoot, fromColor});
	int		rootIndex	= vertices.size() - 1;
	for (int i = 0; i < root->childCount(); ++i)
	{
		Object* obj = root->child(i);
		scl::matrix m = obj->globalMatrixWithAnimation();
		vector3 pos = { 0, 0, 0 };
		pos.mul_matrix(m);
		vertices.push_back({ pos, toColor });
		indices.push_back(rootIndex);
		indices.push_back(vertices.size() - 1);
	}

	for (int i = 0; i < root->childCount(); ++i)
	{
		Object* obj = root->child(i);
		CollectBoneVertices(obj, vertices, indices, ++level);
	}
}

Primitive* _createBone(Object* root, IRender* render, Env* env)
{
	if (NULL == root)
		return NULL;
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 3, ELEM_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 5, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1];
	macros[0].name = "COLOR";

	scl::varray<vertex_color> vertices;
	scl::varray<uint16> indices;
	CollectBoneVertices(root, vertices, indices);
	if (indices.size() == 0)
	{
		delete p;
		return NULL;
	}

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_LINES);
	p->setIndices(indices.begin(), indices.size(), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices.begin(), vertices.size(), sizeof(vertex_color));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}

Primitive* _createTestVulkanPrimitive(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 4, ELEM_TYPE_FLOAT,	0, sizeof(vertex_coord), 0 },
		{ 1, 4, ELEM_TYPE_UINT8,	1, sizeof(vertex_coord), OFFSET(vertex_coord, color) },
		{ 2, 2, ELEM_TYPE_FLOAT,	0, sizeof(vertex_coord), OFFSET(vertex_coord, texcoord) }
	};
	int attrBuffers[] = { 0, 0, 0 };
	ShaderMacro macros[1] = { {"USE_COLOR", ""} };

	// vertex 
	vertex_coord vertices[3] = 
	{
		0,		0,		-1, 1,	0xFF0000FF, 0, 0,
		0.5,	0,		-1, 1,	0x00FF00FF, 0, 1,
		0,		0.5,	-1, 1,	0x0000FFFF, 1, 0,
	};

	// index
	uint16 indices[] = { 0, 1, 2 };

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_coord));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}


Primitive* _createTestVulkanPrimitiveColor(IRender* render, Env* env)
{
	Primitive* p = new Primitive();

	VertexAttr attrs[] = {
		{ 0, 3, ELEM_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
		{ 1, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	};
	int attrBuffers[] = { 0, 0 };
	ShaderMacro macros[1] = { {"COLOR", ""} };

	// vertex 
	vertex_color vertices[3] = 
	{
		0,		0,		-1, 0xFF0000FF,
		1,		0,		-1,	0x00FF00FF,
		0,		1,		-1, 0x0000FFFF,
	};

	// index
	uint16 indices[] = { 0, 1, 2 };

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture(NULL);
	p->loadShader(SHADER_PATH "object.vert", SHADER_PATH "object.frag", macros, 1);
	return p;
}


} // namespace cat


