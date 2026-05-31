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

	m_macros.add(macro);
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

		// 任一文件加载失败则跳过编译，避免把 NULL 传进 createShader 触发底层 svkCreateShaderProgramFromCode 崩溃
		if (NULL != vs_code && NULL != ps_code)
		{
			m_deviceShader = render->createShader(vs_code, ps_code);
		}

		delete[] vs_code;
		delete[] ps_code;

		m_dirty = false;
	}
	return m_deviceShader;
}


char* Shader::_loadfile(const char* const filename, const char* const macros)
{
	// 契约：caller (Shader::shader) 已保证 macros != NULL（传 String::c_str()），不在此处再判 NULL。
	scl::file f;
	if (!f.open(filename, "rb"))
	{
		assert(false);
		return NULL;
	}

	const int macrolen	= ::strlen(macros);
	const int filesize	= static_cast<int>(f.size());

	// 先把整文件读到独立的 fileBuf，再向 out 拼装。
	// 这样可以避免原实现"原地重排 buf"导致：
	//   (1) #version 不在文件开头时丢前置注释 / 重复 #version 行；
	//   (2) #version 行扫描越界（最后一行无换行时 *p++ 走出 buf）。
	char* fileBuf = new char[filesize + 1];
	const int readlen = f.read(fileBuf, filesize);
	assert(readlen == filesize);
	fileBuf[filesize] = '\0';

	const int outlen	= filesize + macrolen + 1;
	char* out			= new char[outlen];

	const char* version = (filesize > 0) ? ::strstr(fileBuf, "#version") : NULL;
	if (0 == macrolen || NULL == version)
	{
		// 无 #version 或无 macros：layout = macros + 整文件
		if (macrolen > 0)
			::memcpy(out, macros, macrolen);
		::memcpy(out + macrolen, fileBuf, filesize);
	}
	else
	{
		// 找 #version 行末换行（带边界检查，修复原 while (*p++ != '\n') 越界）
		const char* p = version;
		const char* const fileEnd = fileBuf + filesize;
		while (p < fileEnd && *p != '\n')
			++p;
		if (p < fileEnd)
			++p; // 把换行包进 prefix

		const int prefixLen = static_cast<int>(p - fileBuf);	// [文件起始 .. #version 行结束含 \n]
		const int suffixLen = filesize - prefixLen;				// #version 行之后

		// layout = prefix(含 #version 行) + macros + suffix
		::memcpy(out,							fileBuf,	prefixLen);
		::memcpy(out + prefixLen,				macros,		macrolen);
		::memcpy(out + prefixLen + macrolen,	p,			suffixLen);
	}
	out[outlen - 1] = '\0';

	delete[] fileBuf;
	return out;
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

