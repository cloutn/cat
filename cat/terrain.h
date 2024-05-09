#pragma once

#include "cat/vertex.h"

namespace cat {

class Primitive;
class IRender;
class Env;

class Terrain
{
public:
	static const int TERRAIN_SIZE = 4;
	static const int TERRAIN_AREA = TERRAIN_SIZE * TERRAIN_SIZE;
	static const int VERTEX_COUNT = TERRAIN_SIZE * TERRAIN_SIZE;
	static const int INDEX_COUNT = (TERRAIN_SIZE - 1) * (TERRAIN_SIZE - 1) * 2 * 3; // QUAD_COUNT = (TERRAIN_SIZE-1)*(TERRAIN_SIZE-1), TRIANGLE_COUNT = QUAD_COUNT * 2;  INDEX_COUNT = TRIANGLE_COUNT * 3;

	void init(IRender* render, Env* env);
	void draw(const scl::matrix& mvp, bool isPick, IRender* render);

	Terrain();
	virtual ~Terrain();


private:	
	vertex_color_uv*	m_vertices;
	uint*				m_indices;
	Primitive*			m_primitive;
	IRender*			m_render;


}; // class Terrain


} // namespace cat




