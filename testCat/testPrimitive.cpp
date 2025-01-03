
#include "testPrimitive.h"

#include "scl/vector.h"

#include "cat/IRender.h"
#include "cat/Primitive.h"
#include "cat/Object.h"
#include "cat/shaderMacro.h"
#include "cat/shader.h"
#include "cat/vertex.h"
#include "cat/env.h"


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
	ShaderMacroArray macros;
	macros.add("COLOR");
	//ShaderMacro macros[1] = { {"COLOR", ""} };
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
	p->setShaderWithPick(env->getDefaultShader(macros), env);
}



void CollectBoneVertices(Object* root, scl::varray<vertex_color>& vertices, scl::varray<uint16>& indices, int level)
{
	if (root->childCount() <= 0)
		return;

	matrix	matRoot		= root->globalMatrix();
	vector3	posRoot		{ 0, 0, 0};
	posRoot.mul_matrix(matRoot);
	uint	fromColor	= 0xFFFFFFFF;
	uint	toColor		= 0xFF00FF00;
	vertices.push_back({ posRoot, fromColor});
	int		rootIndex	= vertices.size() - 1;
	for (int i = 0; i < root->childCount(); ++i)
	{
		Object* obj = root->child(i);
		scl::matrix m = obj->globalMatrix();
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
	ShaderMacroArray macros;
	macros.add("COLOR");

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
	p->setShaderWithPick(env->getDefaultShader(macros), env);
	return p;
}

Primitive* _createTestVulkanPrimitive(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	// attr
	VertexAttr attrs[] = {
		{ 0, 4, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), 0 },
		{ 1, 4, ELEM_TYPE_UINT8,	1, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, color) },
		{ 2, 2, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, u) }
	};
	int attrBuffers[] = { 0, 0, 0 };
	//ShaderMacro macros[1] = { {"USE_COLOR", ""} };
	ShaderMacroArray macros; 
	macros.add("USE_COLOR");

	// vertex 
	vertex_color_uv vertices[3] = 
	{
		0,		0,		-1, 0xFF0000FF, 0, 0,
		0.5,	0,		-1, 0x00FF00FF, 0, 1,
		0,		0.5,	-1, 0x0000FFFF, 1, 0,
	};

	// index
	uint16 indices[] = { 0, 1, 2 };

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, countof(attrs), attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color_uv));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);
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
	ShaderMacroArray macros;
	macros.add("COLOR");

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
	p->setShaderWithPick(env->getDefaultShader(macros), env);
	return p;
}


} // namespace cat


