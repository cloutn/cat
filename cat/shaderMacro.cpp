#include "cat/shaderMacro.h"

namespace cat {

void ShaderMacroArray::add(const char* const name)
{
	add(name, "");
}

void ShaderMacroArray::add(const ShaderMacro& macro)
{
	add(macro.name.c_str(), macro.value.c_str());
}

void ShaderMacroArray::add(const char* const name, const int value)
{
	if (contains(name))
		return;

	ShaderMacro& m = m_macros.push_back_fast();
	m.name = name;
	m.value.from_int(value);
}

void ShaderMacroArray::add(const char* const name, const char* const value)
{
	if (contains(name))
		return;

	ShaderMacro& m = m_macros.push_back_fast();
	m.name = name;
	m.value = value;
}

void ShaderMacroArray::remove(const char* const name)
{
	//for (int i = 0; i < m_macros.size(); ++i)
	for (int i = m_macros.size() - 1; i >= 0; --i)
	{
		ShaderMacro m = m_macros[i];
		if (m.name != name)
			continue;
		m_macros.erase_fast(i);
	}
}

bool ShaderMacroArray::contains(const char* const name)
{
	for (int i = 0; i < m_macros.size(); ++i)
	{
		ShaderMacro m = m_macros[i];
		if (m.name != name)
			continue;
		return true;
	}
	return false;
}

void ShaderMacroArray::assign(const ShaderMacroArray& other)
{
	if (this == &other)
		return;
	if (m_macros.c_array() == other.m_macros.c_array())
		return;

	m_macros.clear();

	for (int i = 0; i < other.size(); ++i)
		add(other.m_macros[i]);
}

void ShaderMacroArray::assign(const ShaderMacro* macros, const int macroCount)
{
	if (m_macros.c_array() == macros)
		return;	

	m_macros.clear();

	for (int i = 0; i < macroCount; ++i)
		add(m_macros[i]);
}

void ShaderMacroArray::clear()
{
	m_macros.clear();
}

} // namespace cat




