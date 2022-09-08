#pragma once


//#ifndef assert
#define gfx_assert(expr) do { if (!(expr)) { throw 1; } } while(0)
//#endif


#define glcheck(x)	do\
{ \
	x; \
	int err = glGetError(); \
	if ( err != GL_NO_ERROR ) \
	{ \
		gfx_assert(false); \
	} \
} while (false)


#ifndef NULL
#define NULL 0
#endif

struct _iobuf;

namespace img { 

typedef unsigned int uint;
typedef unsigned int uint32;
typedef int int32;
typedef wchar_t wchar;
	

#ifdef GFX_ENABLE_IMAGE
#define GFX_ENABLE_PNG
#define GFX_ENABLE_JPEG
#define GFX_ENABLE_TGA
#define GFX_ENABLE_ETC2
#define GFX_ENABLE_OPENGLES
#endif


_iobuf* my_fopen(const char* const filename);

} // namespace gfx
