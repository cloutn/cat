#pragma once

#include "gfx/base.h"


namespace gfx {

#if defined(GFX_ENABLE_PNG) && defined(GFX_ENABLE_JPEG) && defined(GFX_ENABLE_TGA) && defined(GFX_ENABLE_OPENGLES)
// load png or jpeg image file to opengles texture.
// return texture id
uint			load_img			(const char* filename);
uint			load_img			(const char* filename, int* width, int* height, int* pitch, int* pixel);
unsigned char*	load_img_to_buffer	(_iobuf* fp, unsigned char* out_rgba, int* _width, int* _height, int* _pitch, int* _pixel);
void			get_img_size		(const char* filename, int* width, int* height, int* pixel);
void			get_img_size		(_iobuf* fp, int* width, int* height, int* pixel);
#endif

#if defined(GFX_ENABLE_PNG) && defined(GFX_ENABLE_OPENGLES)
uint			load_png			(const char* filename);
#endif

#ifdef GFX_ENABLE_PNG
unsigned char*	load_png_data		(_iobuf* fp,			unsigned char* out_rgba, int* out_width, int* out_height, int* pitch, int* pixel);
unsigned char*	load_png_data		(const char* filename,	unsigned char* out_rgba, int* out_width, int* out_height, int* pitch, int* pixel);
void			get_png_size		(const char* filename,	int* out_width, int* out_height, int* out_pixel);
void			get_png_size		(_iobuf* fp,			int* out_width, int* out_height, int* out_pixel);
#endif

#ifdef GFX_ENABLE_JPEG
unsigned char*	load_jpg_data		(_iobuf* fp, unsigned char* out_rgba, int* out_width, int* out_height, int* pitch, int* pixel);
void			get_jpg_size		(_iobuf* fp, int* out_width, int* out_heigh, int* out_pixelt);
void			get_jpg_size		(_iobuf* fp, int* out_width, int* out_heigh, int* out_pixelt);
#endif

#ifdef GFX_ENABLE_TGA
unsigned char*	load_tga_data		(_iobuf* fp, unsigned char* out_rgba, int* out_width, int* out_height, int* pitch, int* pixel);
void			get_tga_size		(const char* filename, int* out_width, int* out_heigh, int* out_pixelt);
void			get_tga_size		(_iobuf* fp, int* out_width, int* out_heigh, int* out_pixelt);
#endif

#ifdef GFX_ENABLE_ETC2
uint			load_etc2_to_opengl	(_iobuf* fp, int* out_width, int* out_height, int* pitch, int* pixel);
#endif

}

