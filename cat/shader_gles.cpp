#include "cat/shader_gles.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>


#ifdef GFX_ENABLE_OPENGLES

#ifndef OPENGL_ES
#define GLEW_STATIC 
#include "GL/glew.h"
#include "GL/glut.h"
#else
#ifdef __APPLE__
#include <OpenGLES/ES3/gl.h>
#else
#include <GLES3/gl3.h>
#endif
#endif

#endif

//#define SCL_ENABLE
#ifdef SCL_ENABLE
#include "scl/log.h"
#define myprint log_debug_unsafe
#else
#define myprint printf
#endif

namespace cat {

inline FILE* my_fopen(const char* const filename)
{
#ifdef _WIN32
	FILE* f = NULL;
	fopen_s(&f, filename, "rb");
#else
	FILE* f = fopen(filename, "rb");
#endif
	return f;
}

char* open_file(const char* const filename)
{
	FILE* f = my_fopen(filename);
	if (NULL == f)
		return NULL;

	static const int FILE_LEN = 1024 * 1024;
	char* buf = static_cast<char*>(malloc(FILE_LEN));
	memset(buf, 0, FILE_LEN);
	fread(buf, 1, FILE_LEN, f);
	fclose(f);
	return buf;
}


#ifdef GFX_ENABLE_OPENGLES
uint load_shader(const char* const vs_source, const char* const ps_source)
{
	if (NULL == vs_source || 0 == vs_source[0])
		return 0;
	if (NULL == ps_source || 0 == ps_source[0])
		return 0;

	//create
	const uint vs = glCreateShader(GL_VERTEX_SHADER);
	const uint ps = glCreateShader(GL_FRAGMENT_SHADER);
	if (vs == 0 || ps == 0)
		return 0;

	const char* ppsource[1] = { 0 };
	ppsource[0] = vs_source;
	glShaderSource(vs, 1, ppsource, NULL);

	ppsource[0] = ps_source;
	glShaderSource(ps, 1, ppsource, NULL);

	//compile
	int r = 0;
	glCompileShader(vs);
	glGetShaderiv(vs, GL_COMPILE_STATUS, &r);
	if (!r)
	{
		char log[1024] = { 0 };
		glGetShaderInfoLog(vs, 1024, NULL, log);
		myprint("%s\n", log);
		glDeleteShader(vs);
		glDeleteShader(ps);
		return 0;
	}

	glCompileShader(ps);
	glGetShaderiv(ps, GL_COMPILE_STATUS, &r);
	if (!r)
	{
		char log[1024] = { 0 };
		glGetShaderInfoLog(ps, 1024, NULL, log);
		myprint("%s\n", log);
		glDeleteShader(vs);
		glDeleteShader(ps);
		return 0;
	}

	//attach
	uint prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, ps);

	//bind attributes
	//glBindAttribLocation(prog, 0, "vertex");
	//glBindAttribLocation(prog, 1, "color");

	//link
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &r);
	if (!r)
	{
		char log[1024] = { 0 };
		glGetProgramInfoLog(prog, 1024, NULL, log);
		myprint("%s\n", log);
		glDeleteShader(vs);
		glDeleteShader(ps);
		glDeleteProgram(prog);
		return 0;
	}

	//clear
	glDeleteShader(vs);
	glDeleteShader(ps);

	return prog;
}
#endif


#ifdef GFX_ENABLE_OPENGLES
uint load_shader_file(const char* const vs_filename, const char* const ps_filename)
{
	uint g_prog = 0;
	char* vs = open_file(vs_filename);
	char* ps = open_file(ps_filename);
	g_prog = load_shader(vs, ps);
	free(vs);
	free(ps);
	if (g_prog <= 0)
	{
		myprint("load shader failed! errno = %d", glGetError());
		return 0;
	}
	return g_prog;
}
#endif


} // namespace cat



