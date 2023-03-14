#pragma once


#include "cat/IRender.h"
#include "cat/string.h"

#include "scl/vector.h"

struct cgltf_primitive;
struct cgltf_buffer_view;

namespace cat {

class IRender;
class VertexAttr;
class Material;
class Shader;
class Mesh;
class Env;
class ShaderMacro;
class ShaderMacroArray;
class Object;

class Primitive
{
public:
	Primitive();
	virtual ~Primitive();

	void				load				(cgltf_primitive* data, const char* const path, int skinJointCount, Mesh* parent, IRender* render, Env* env);
	void				draw				(const scl::matrix& mvp, const scl::matrix* jointMatrices, const int jointMatrixCount, bool isPick, IRender* render);
	void				release				();
	Shader*				shader				() { return m_shader; }
	void				setShader			(Shader* shader) { m_shader = shader; }
	void				setShaderWithPick	(Shader* shader, Env* env);
	void				setPickShader		(Shader* pickShader) { m_pickShader = pickShader; }
	void				setTexture			(const char* const filename);
	//void				loadMemory			(
	//	void*			indices, 
	//	const int		indexCount, 
	//	const ELEM_TYPE	indexComponentType, 
	//	void**			verticesList,
	//	int*			vertexCountList,
	//	int*			sizeOfVertex,
	//	int				attrCount,
	//	VertexAttr*		attrs,
	//	int*			attrVertexBuffer,
	//	PRIMITIVE_TYPE	primitiveType,
	//	Material*		material,
	//	Shader*			shader,
	//	IRender*		render
	//	);
	void				setRender			(IRender* render) { m_render = render; } 
	void				setEnv				(Env* env) { m_env = env; } 
	void**				vertexBuffers		() { return m_deviceVertexBuffers;	}
	void*				indexBuffer			() { return m_deviceIndexBuffer;	}
	int					attrCount			() const { return m_attrCount;		}
	const VertexAttr*	attrs				() const { return m_attrs;			}

	// 当顶点属性位于不同的 buffer 的时候，需要用参数 attrBufferIndices 指定每个 buffer 所在的 index
	// index 的具体对应关系参见函数 setVertices 的多 verticesList 版本。
	void				setAttrs			(const VertexAttr* attrs, const int attrCount, const int* attrBufferIndices = NULL);
	void				setIndices			(const void* indices, const int indexCount, const ELEM_TYPE indexComponentType);
	void				setVertices			(void** verticesList, int* vertexCountList, int* sizeOfVertex);
	void				setVertices			(void* vertices, int vertexCount, int sizeOfVertex);
	void				setPrimitiveType	(PRIMITIVE_TYPE t) { m_primitiveType = t; }
	void				updateVertices		(void* vertices, int vertexCount, int sizeOfVertex);
	Mesh*				parent				() { return m_parent; }
	Object*				parentObject		();

private:
	void				_loadVertex			(const cgltf_primitive&	primitive, IRender* render);

private:
	IRender*			m_render;
	Env*				m_env;

	// indices data
	void*				m_deviceIndexBuffer;
	int					m_indexCount;
	ELEM_TYPE			m_indexComponentType;
	int					m_indexOffset;

	// vertices data
	void**				m_deviceVertexBuffers;
	int					m_attrCount;
	VertexAttr*			m_attrs;
	int*				m_attrBufferIndices;

	// other data
	PRIMITIVE_TYPE		m_primitiveType;
	Material*			m_material;
	Shader*				m_shader;
	Shader*				m_pickShader;

	//TODO parent is for debug
	Mesh*				m_parent;
};

} // namespace cat {

