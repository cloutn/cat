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

	//static void			setDefault(IRender* render, const char* const textureFilename);
	//static Material*	_default();
	//static void			releaseDefault();

private:
	IRender*				m_render;
	Env*					m_env;
	//void*					m_texture;
	const TextureFile*		m_textureFile;

	//static Material*	s_default;
};

} // namespace cat 

