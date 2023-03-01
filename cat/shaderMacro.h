#pragma once

#include "cat/string.h"

#include "scl/varray.h"

namespace cat {

class ShaderMacro
{
public:
	String name;
	String value;
};

class ShaderMacroArray
{
public:
	void				add			(const char* const name);
	void				add			(const ShaderMacro& macro);
	void				add			(const char* const name, const int value);
	void				add			(const char* const name, const char* const value);
	void				remove		(const char* const name);
	bool				contains	(const char* const name);
	void				assign		(const ShaderMacroArray& other);
	void				assign		(const ShaderMacro* macros, const int macroCount);
	void				clear		();
	const ShaderMacro*	data		() const { return m_macros.begin(); }
	int					size		() const { return m_macros.size(); }
	ShaderMacro&		operator[]	(const int index) { return m_macros[index]; }

private:
	scl::varray<ShaderMacro>	m_macros;

}; // class ShaderMacro

} // namespace cat


