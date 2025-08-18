
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
	const VertexAttr* attrs = vertex_color::get_attr();
	const int attrCount = vertex_color::get_attr_count();
	//	{ 0, 3, ELEM_TYPE_FLOAT, 0, sizeof(vertex_color), 0 },
	//	{ 1, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	//};
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
	p->setAttrs			(attrs, attrCount, attrBuffers);
	p->setVertices		(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture		(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);
}



void collectBoneVertices(Object* root, scl::varray<vertex_color>& vertices, scl::varray<uint16>& indices, int level)
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
		collectBoneVertices(obj, vertices, indices, ++level);
	}
}

Primitive* createBone(Object* root, IRender* render, Env* env)
{
	if (NULL == root)
		return NULL;
	Primitive* p = new Primitive();
	// attr
	const VertexAttr* attrs = vertex_color::get_attr();
	const int attrCount = vertex_color::get_attr_count();
	//	{ 0, 3, ELEM_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
	//	{ 5, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	//};
	int attrBuffers[] = { 0, 0 };
	ShaderMacroArray macros;
	macros.add("COLOR");

	scl::varray<vertex_color> vertices;
	scl::varray<uint16> indices;
	collectBoneVertices(root, vertices, indices);
	if (indices.size() == 0)
	{
		delete p;
		return NULL;
	}

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_LINES);
	p->setIndices(indices.begin(), indices.size(), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, attrCount, attrBuffers);
	p->setVertices(vertices.begin(), vertices.size(), sizeof(vertex_color));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);
	return p;
}

cat::Primitive* createTestPrimitive_getVertexData(IRender* render, Env* env, vertex_color* vertices, int vertexCount, uint16* indices, int indexCount)
{
	Primitive* p = new Primitive();

	const VertexAttr* attrs = vertex_color::get_attr();
	const int attrCount = vertex_color::get_attr_count();

	int attrBuffers[] = { 0, 0 };
	ShaderMacroArray macros;
	macros.add("COLOR");

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, indexCount, ELEM_TYPE_UINT16);
	p->setAttrs(attrs, attrCount, attrBuffers);
	p->setVertices(vertices, vertexCount, sizeof(vertex_color));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);

	return p;
}

Primitive* createTestVulkanPrimitive(IRender* render, Env* env)
{
	Primitive* p = new Primitive();
	// attr
	const VertexAttr* attrs = vertex_color_uv::get_attr();
	const int attr_count = vertex_color_uv::get_attr_count();
	//{
	//	{ 0, 4, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), 0 },
	//	{ 1, 4, ELEM_TYPE_UINT8,	1, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, color) },
	//	{ 2, 2, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, u) }
	//};
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
	p->setAttrs(attrs, attr_count, attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color_uv));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);
	return p;
}


Primitive* createTestVulkanPrimitiveColor(IRender* render, Env* env)
{
	Primitive* p = new Primitive();

	const VertexAttr* attrs = vertex_color::get_attr();
	const int attrCount = vertex_color::get_attr_count();
	//	{ 0, 3, ELEM_TYPE_FLOAT,		0, sizeof(vertex_color), 0 },
	//	{ 1, 4, ELEM_TYPE_UINT8, 1, sizeof(vertex_color), OFFSET(vertex_color, color) }
	//};
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
	p->setAttrs(attrs, attrCount, attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);
	return p;
}



void testPrimitive_vertexPosition_edgeCase(IRender* render, Env* env)
{
	// 测试边界情况：没有位置属性的 primitive
	Primitive* p = new Primitive();
	
	struct vertex_no_pos
	{
		uint32	color;
		float	u, v;
	};
	// 创建一个没有位置属性的 primitive
	vertex_no_pos vertices[3] = 
	{
		0xFF0000FF, 0, 0,
		0x00FF00FF, 0, 1,
		0x0000FFFF, 1, 0,
	};

	uint16 indices[] = { 0, 1, 2 };
	const VertexAttr attrs[2] = {
			{ ATTR_LOC_COLOR0,		4, ELEM_TYPE_UINT8,	1, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, color) },
			{ ATTR_LOC_TEXCOORD0,	2, ELEM_TYPE_FLOAT,	0, sizeof(vertex_color_uv), OFFSET(vertex_color_uv, u) }
	};
	const int attr_count = 2;

	int attrBuffers[] = { 0, 0 };
	ShaderMacroArray macros; 
	macros.add("USE_COLOR");

	p->setRender(render);
	p->setEnv(env);
	p->setPrimitiveType(PRIMITIVE_TYPE_TRIANGLES);
	p->setIndices(indices, countof(indices), ELEM_TYPE_UINT16);
	p->setAttrs(attrs, attr_count, attrBuffers);
	p->setVertices(vertices, countof(vertices), sizeof(vertex_color_uv));
	p->setTexture(NULL);
	p->setShaderWithPick(env->getDefaultShader(macros), env);

	// 测试1：没有位置属性时的 allVertexPositions
	int vertexCount = 0;
	vector3 positions[3];
	int positionsCapacity = 3;
	
	p->vertexPositions(positions, vertexCount, positionsCapacity);
	assert(vertexCount == 0); // 顶点数量应该为0

	// 测试2：没有位置属性时的 allVertexPositionsArray
	scl::varray<vector3> positionsArray = p->vertexPositions();
	assert(positionsArray.size() == 0); // 应该返回空数组

	// 测试3：没有位置属性时的 vertexPosition
	vector3 singlePosition = p->vertexPosition(0);
	assert(singlePosition.x == 0.0f && singlePosition.y == 0.0f && singlePosition.z == 0.0f);

	delete p;
}


void testPrimitive_vertexAttr(IRender* render, Env* env)
{
	vertex_color vertices[3] = 
	{
		0.1,		-0.2,			33,						0x12345678,
		111,		222,			-33.333,				0x00998877,
		-45678,		0.45678,		-98765.45678,			0xAABBCCDD,
	};

	uint16 indices[] = { 0, 1, 2 };

	Primitive* p = createTestPrimitive_getVertexData(render, env, vertices, countof(vertices), indices, countof(indices));

	float pos[3] = { 0 };
	uint32 color = 0;

	// 检查 vertices
	for (int i = 0; i < countof(vertices); ++i)
	{
		p->vertexAttr(i, 0 , pos, sizeof(pos));
		assert(pos[0] == vertices[i].position.x);
		assert(pos[1] == vertices[i].position.y);
		assert(pos[2] == vertices[i].position.z);


		p->vertexAttr(i, 1 , &color, sizeof(color));
		assert(color == vertices[i].color);
	}

	// 测试获取所有顶点的位置属性
	float allPositions[3][3] = { 0 };
	p->vertexAttrs(0, allPositions, sizeof(allPositions));

	// 测试获取所有顶点的颜色属性
	uint32 allColors[3] = { 0 }; 
	p->vertexAttrs(1, allColors, sizeof(allColors));

	for (int i = 0; i < countof(vertices); ++i)
	{
		assert(allPositions[i][0] == vertices[i].position.x);
		assert(allPositions[i][1] == vertices[i].position.y);
		assert(allPositions[i][2] == vertices[i].position.z);

		assert(allColors[i] == vertices[i].color);
	}

	delete p;
}

void testPrimitive_vertexPosition(IRender* render, Env* env)
{
	// vertex 
	vertex_color vertices[3] = 
	{
		0.1,		-0.2,			33,						0x12345678,
		111,		222,			-33.333,				0x00998877,
		-45678,		0.45678,		-98765.45678,			0xAABBCCDD,
	};

	uint16 indices[] = { 0, 1, 2 };

	Primitive* p = createTestPrimitive_getVertexData(render, env, vertices, countof(vertices), indices, countof(indices));

	// 测试1：通过参数传递数组的方式
	int vertexCount = 0;
	vector3 positions[3]; 
	int positionsCapacity = 3;
	
	p->vertexPositions(positions, vertexCount, positionsCapacity);

	// 验证返回的数据
	assert(vertexCount == countof(vertices));

	// 验证每个顶点的位置数据
	for (int i = 0; i < vertexCount; ++i)
	{
		assert(positions[i].x == vertices[i].position.x);
		assert(positions[i].y == vertices[i].position.y);
		assert(positions[i].z == vertices[i].position.z);
	}

	// 测试2：容量不足的情况
	int smallVertexCount = 0;
	vector3 smallPositions[2]; // 容量只有2，但需要3个
	int smallPositionsCapacity = 2;
	
	p->vertexPositions(smallPositions, smallVertexCount, smallPositionsCapacity);
	
	// 验证容量不足时返回正确的顶点数量
	assert(smallVertexCount == countof(vertices)); // 应该返回正确的顶点数量

	// 测试3：空指针的情况
	int nullVertexCount = 0;
	p->vertexPositions(nullptr, nullVertexCount, 0);
	assert(nullVertexCount == countof(vertices)); // 应该返回正确的顶点数量

	// 测试4：对比与逐个获取的结果
	for (int i = 0; i < vertexCount; ++i)
	{
		vector3 singlePosition = p->vertexPosition(i);
		assert(positions[i].x == singlePosition.x);
		assert(positions[i].y == singlePosition.y);
		assert(positions[i].z == singlePosition.z);
	}

	// 测试1：使用 vertexPositions 获取所有顶点位置
	scl::varray<vector3> positionsArray = p->vertexPositions();
	
	// 验证返回的数组大小
	assert(positionsArray.size() == countof(vertices));

	// 验证每个顶点的位置数据
	for (int i = 0; i < positionsArray.size(); ++i)
	{
		assert(positionsArray[i].x == vertices[i].position.x);
		assert(positionsArray[i].y == vertices[i].position.y);
		assert(positionsArray[i].z == vertices[i].position.z);
	}

	// 测试2：对比与逐个获取的结果
	for (int i = 0; i < positionsArray.size(); ++i)
	{
		vector3 singlePosition = p->vertexPosition(i);
		assert(positionsArray[i].x == singlePosition.x);
		assert(positionsArray[i].y == singlePosition.y);
		assert(positionsArray[i].z == singlePosition.z);
	}

	// 测试3：对比与参数传递方式的结果
	//p->vertexPositions(positions, vertexCount, positionsCapacity);
	assert(vertexCount == positionsArray.size());

	for (int i = 0; i < vertexCount; ++i)
	{
		assert(positions[i].x == positionsArray[i].x);
		assert(positions[i].y == positionsArray[i].y);
		assert(positions[i].z == positionsArray[i].z);
	}

	// 测试4：验证 varray 的移动语义
	scl::varray<vector3> movedArray = std::move(positionsArray);
	assert(movedArray.size() == countof(vertices));
	assert(positionsArray.size() == 0); // 移动后原数组应该为空

	// 验证移动后的数据仍然正确
	for (int i = 0; i < movedArray.size(); ++i)
	{
		assert(movedArray[i].x == vertices[i].position.x);
		assert(movedArray[i].y == vertices[i].position.y);
		assert(movedArray[i].z == vertices[i].position.z);
	}
	delete p;
}


} // namespace cat


