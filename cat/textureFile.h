#pragma once

#include "gfx/color.h"

#include <stddef.h>

namespace cat {

class TextureFile 
{
public:
	TextureFile() : name(""), texture(NULL), counter(0) {}
	TextureFile(const char* const _name, void* _handle, int _counter) : name(_name), texture(_handle), counter(_counter) {}
	virtual ~TextureFile() {}

	static const TextureFile&	empty()
	{
		static TextureFile s_empty;
		return s_empty;
	}

	const char*			name;
	void*				texture;
	int					counter;
	int					width;
	int					height;
	int					pitch;
	gfx::PIXEL			pixel;

}; // class TextureFile

} // namespace cat


