#pragma once

struct cgltf_material;

namespace cat {

class IRender;
class Shader;
class Env;
class TextureFile;

class Material
{
public:
	Material();
	virtual ~Material();

	void		load			(cgltf_material*, const char* const currentPath, IRender* render, Env* env);
	void		init			(IRender* render, const char* const textureFilename, Env* env);
	void*		texture			();
	void		release			();

private:
	IRender*					m_render;
	Env*						m_env;
	const TextureFile*			m_textureFile;
};

} // namespace cat 

