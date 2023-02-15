#include "cat/shader.h"

#include "cat/IRender.h"

#include "scl/file.h"

namespace cat {

void Shader::load(const char* const vs_filename, const char* const ps_filename)
{
	m_vsFilename = vs_filename;	
	m_psFilename = ps_filename;
	invalidate();
}

void Shader::addMacro(const ShaderMacro& macro)
{
	if (macro.name.empty())
		return;

	m_macros.push_back(macro);
	invalidate();
}

void* Shader::shader(IRender* render)
{
	if (NULL == m_deviceShader || m_dirty)
	{
		m_render = render;

		if (m_deviceShader != NULL)
			render->releaseShader(m_deviceShader);

		String macros;
		_allmacros(macros);
	
		char* vs_code = _loadfile(m_vsFilename.c_str(), macros.c_str());
		char* ps_code = _loadfile(m_psFilename.c_str(), macros.c_str());

		m_deviceShader = render->createShader(vs_code, ps_code);

		delete[] vs_code;
		delete[] ps_code;

		m_dirty = false;
	}
	return m_deviceShader;
}


char* Shader::_loadfile(const char* const filename, const char* const macros)
{
	scl::file f;
	if (!f.open(filename, "rb"))
	{
		assert(false);
		return false;
	}
	const int	macrolen	= ::strlen(macros);
	const int	filesize	= static_cast<int>(f.size());
	const int	buflen		= macrolen + filesize + 1; // total buffer length = filesize + macros' strlen.
	char*		buf			= new char[buflen];
	memset(buf, 0, buflen);
	const int readlen = f.read(buf + macrolen, filesize);
	assert(readlen == filesize);
	if (macrolen == 0)
		return buf;

	char* text = buf + macrolen;
	char* version = ::strstr(text, "#version");
	if (NULL == version) // no #version found.
	{
		scl::strncpy_unsafe(buf, macros, macrolen);		
	}
	else // insert macros between #version and other code.
	{
		char* p = version + 1;
		while (*p++ != '\n') 
		{ }
		int versionlen = (p - version);
		scl::strncpy_unsafe(buf, version, versionlen);
		scl::strncpy_unsafe(buf + versionlen, macros, macrolen);
	}

	return buf;
}

void Shader::_allmacros(String& output)
{
	for (int i = 0; i < m_macros.size(); ++i)
	{
		ShaderMacro& macro = m_macros[i];

		output += "\n#define ";

		output += macro.name.c_str();

		if (!macro.value.empty())
		{
			output += " ";
			output += macro.value.c_str();
		}

		output += "\n";
	}
}

Shader::~Shader()
{
	if (NULL != m_deviceShader)
	{
		m_render->releaseShader(m_deviceShader);
		m_deviceShader = NULL;
	}
}

} // namespace cat

