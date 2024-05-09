
#include "cat/terrain.h"
#include "cat/primitive.h"
#include "cat/env.h"
#include "cat/shaderMacro.h"

namespace cat {

void Terrain::init(IRender* render, Env* env)
{
	m_render = render;

	m_vertices = new vertex_color_uv[VERTEX_COUNT];
	m_indices = new uint[INDEX_COUNT];

	memset(m_vertices, 0, sizeof(vertex_color_uv) * VERTEX_COUNT);
	memset(m_indices, 0, sizeof(uint) * INDEX_COUNT);

	for (int row = 0; row < TERRAIN_SIZE; ++row)
	{
		for (int col = 0; col < TERRAIN_SIZE; ++col)
		{
			int		idx	= row * TERRAIN_SIZE + col;
			float	x	= row - TERRAIN_SIZE / 2;
			float	y	= -col + TERRAIN_SIZE / 2;
			float	u	= float(row) / TERRAIN_SIZE;
			float	v	= float(col) / TERRAIN_SIZE;
			vertex_color_uv& vertex = m_vertices[idx]; 		
			vertex.set_xyz(x, 0, y);
			vertex.set_uv(u, v);
			vertex.set_color(0xFFFFFFFF);

		}
	}


	int indexCount = 0;
	for (int row = 0; row < TERRAIN_SIZE - 1; ++row)
	{
		for (int col = 0; col < TERRAIN_SIZE - 1; ++col)
		{

			// single quad index
			//
			//   i0   ___  i1
			//		 |   |
			//   i2  |___| i3
			//		

			int		i0	= row * TERRAIN_SIZE + col;
			int		i1 = i0 + 1;
			int		i2 = (row + 1) * TERRAIN_SIZE + col;
			int		i3 = i2 + 1;
			//assert(idx * 6 + 5 < INDEX_COUNT);
			assert(indexCount < INDEX_COUNT);

			m_indices[indexCount++] = i0;
			m_indices[indexCount++] = i1;
			m_indices[indexCount++] = i2;

			m_indices[indexCount++] = i1;
			m_indices[indexCount++] = i3;
			m_indices[indexCount++] = i2;
		}
	}

	m_primitive = new Primitive();
	m_primitive->setRender			(render);
	m_primitive->setEnv				(env);
	m_primitive->setPrimitiveType	(PRIMITIVE_TYPE_TRIANGLES);
	//jm_primitive->setIndices			(m_indices, INDEX_COUNT, ELEM_TYPE_UINT32);
	m_primitive->setIndices			(m_indices, 3, ELEM_TYPE_UINT32);
	m_primitive->setAttrs			(vertex_color_uv::get_attr(), vertex_color_uv::get_attr_count());
	m_primitive->setVertices		(m_vertices, VERTEX_COUNT, sizeof(vertex_color_uv));

	ShaderMacroArray macros;
	macros.add("COLOR");
	m_primitive->setShaderWithPick(env->getDefaultShader(macros), env);

}

void Terrain::draw(const scl::matrix& mvp, bool isPick, IRender* render)
{
	if (NULL == m_primitive)
		return;
	m_primitive->draw(mvp, NULL, 0, isPick, render);
}

Terrain::Terrain()
{
	m_vertices		= NULL;
	m_indices		= NULL;
	m_primitive		= NULL;
	m_render		= NULL;;
}

Terrain::~Terrain()
{
	safe_delete(m_primitive);
	safe_delete_array(m_vertices);
	safe_delete_array(m_indices);
}

} // namespace cat



