#pragma once

#include "cat/string.h"

#include "scl/varray.h"

namespace cat {

class ShaderMacro
{
public:
	String name;
	String value;

}; // class ShaderMacro


class ShaderMacroArray
{
public:
	void add(const char* const name)
	{
		add(name, "");
	}

	void add(const ShaderMacro& macro)
	{
		add(macro.name.c_str(), macro.value.c_str());
	}

	void add(const char* const name, const int value)
	{
		ShaderMacro& m = m_macros.push_back_fast();
		m.name = name;
		m.value.from_int(value);
	}

	void add(const char* const name, const char* const value)
	{
		ShaderMacro& m = m_macros.push_back_fast();
		m.name = name;
		m.value = value;
	}

	void remove(const char* const name)
	{
		for (int i = 0; i < m_macros.size(); ++i)
		{
			ShaderMacro m = m_macros[i];
			if (m.name != name)
				continue;
			m_macros.erase_fast(i);
			break;
		}
	}

	ShaderMacro* data() { return m_macros.begin(); }

	int size() const { return m_macros.size(); }

private:
	scl::varray<ShaderMacro> m_macros;
}; // class ShaderMacro

} // namespace cat


