
#include "gfx/image.h"

#ifdef GFX_ENABLE_PNG
#include <libpng/png.h>  
#endif

#ifdef GFX_ENABLE_JPEG
#include <jpeglib.h>
#include <jerror.h>
#endif  
 
#ifdef GFX_ENABLE_TGA
#include <libtga/src/tga.h> 
#endif

#ifdef GFX_ENABLE_ETC2
#include <libktx/ktx.h>
#endif

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
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

namespace gfx {

static const int PIXEL_RGBA		= 4;
static const int PIXEL_RGB		= 3;

FILE* my_fopen(const char* const filename)
{
#ifdef _WIN32
	FILE* f = NULL;
	fopen_s(&f, filename, "rb");
#else
	FILE* f = fopen(filename, "rb");
#endif
	return f;
}


#if defined(GFX_ENABLE_PNG) && defined(GFX_ENABLE_OPENGLES)
uint load_png(const char* filename)  
{  
	int				width		= 0;
	int				height		= 0;
	unsigned char*	rgba		= load_png_data(filename, NULL, &width, &height, NULL, NULL);
	
#ifndef OPENGL_ES
	//开启纹理贴图特效  
	glcheck( glEnable(GL_TEXTURE_2D) );  
#endif
	
	//创建纹理   
	GLuint textureID;
	glcheck( glGenTextures(1, &textureID) );  
	glcheck( glBindTexture(GL_TEXTURE_2D, textureID) ); //惟麓艗鈭喡棵屸垶脹鈭偮滴┾垰藲鈼娒? 
	
#ifndef OPENGL_ES
	//设置贴图和纹理的混合效果
	glcheck( glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL) );  
#endif
	
	//设置纹理所用到图片数据 
	glcheck( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba) );  

	free(rgba);
	return textureID;  
}  
#endif


#ifdef GFX_ENABLE_PNG
unsigned char* load_png_data(const char* filename, unsigned char* out_rgba, int* out_width, int* out_height, int* pitch, int* pixel)
{
	const int HEADER_SIZE = 8;

	FILE* fp = my_fopen(filename);
	if(NULL == fp)
		return 0;
	
	unsigned char* rgba = load_png_data(fp, out_rgba, out_width, out_height, pitch, pixel);

	fclose(fp);

	return rgba;  
}
#endif

#if defined(GFX_ENABLE_PNG) && defined(GFX_ENABLE_JPEG) && defined(GFX_ENABLE_OPENGLES)
uint load_img(const char* filename)
{
	return load_img(filename, NULL, NULL, NULL, NULL);
}


void get_img_size(const char* filename, int* width, int* height, int* pixel)
{
	FILE* fp = my_fopen(filename);
	if (NULL == fp)
		return;
	get_img_size(fp, width, height, pixel);
}

void get_img_size(_iobuf* fp, int* width, int* height, int* pixel)
{
	get_png_size(fp, width, height, pixel);
	if (*width == 0 && *height == 0)
	{
		fseek(fp, 0, SEEK_SET);
		get_jpg_size(fp, width, height, pixel);
		if (*width == 0 && *height == 0)
		{
			fseek(fp, 0, SEEK_SET);
			get_tga_size(fp, width, height, pixel);
		}
	}
}

unsigned char* load_img_to_buffer(_iobuf* fp, unsigned char* out_rgba, int* _width, int* _height, int* _pitch, int* _pixel)
{
	int				width		= 0;
	int				height		= 0;
	unsigned char*	rgba		= NULL;
	int				pixel		= -1;
	//FILE*			fp			= NULL;

	//fp = my_fopen(filename);
	//if (NULL == fp)
	//	return NULL;

	rgba = load_png_data(fp, out_rgba, &width, &height, _pitch, &pixel);
	if (NULL == rgba)
	{
		fseek(fp, 0, SEEK_SET);
		rgba = load_jpg_data(fp, out_rgba, &width, &height, _pitch, &pixel);
		if (NULL == rgba)
		{
			fseek(fp, 0, SEEK_SET);
			rgba = load_tga_data(fp, out_rgba, &width, &height, _pitch, &pixel);
			if (NULL == rgba)
			{
				//fseek(fp, 0, SEEK_SET);
				//return load_etc2_to_opengl(fp, &width, &height, _pitch, &pixel);
				return NULL;
			}
		}
	}

	if (NULL != _width)
		*_width = width;
	if (NULL != _height)
		*_height = height;
	if (NULL != _pixel)
		*_pixel = pixel;

	fclose(fp);
	return rgba;
}

uint load_img(const char* filename, int* _width, int* _height, int* _pitch, int* _pixel)  
{  
	int				width		= 0;
	int				height		= 0;
	unsigned char*	rgba		= NULL;
	int				pixel		= -1;
	FILE*			fp			= NULL;

	fp = my_fopen(filename);
	if (NULL == fp)
		return -1;

	rgba = load_png_data(fp, NULL, &width, &height, _pitch, &pixel);
	if (NULL == rgba)
	{
		fseek(fp, 0, SEEK_SET);
		rgba = load_jpg_data(fp, NULL, &width, &height, _pitch, &pixel);
		if (NULL == rgba)
		{
			fseek(fp, 0, SEEK_SET);
			rgba = load_tga_data(fp, NULL, &width, &height, _pitch, &pixel);
			if (NULL == rgba)
			{
				fseek(fp, 0, SEEK_SET);
				//return load_etc2_to_opengl(fp, &width, &height, _pitch, &pixel);
				return load_etc2_to_opengl(fp, NULL, NULL, NULL, NULL); // not implemented feature: etc2 read size support 
			}
		}
	}

	if (NULL != _width)
		*_width = width;
	if (NULL != _height)
		*_height = height;
	if (NULL != _pixel)
		*_pixel = pixel;

	fclose(fp);

	if (NULL == rgba)
		return -1;
	
#ifndef OPENGL_ES
	//开启纹理贴图特效  
	glcheck( glEnable(GL_TEXTURE_2D) );  
#endif
	
	//创建纹理   
	GLuint textureID;
	glcheck( glGenTextures(1, &textureID) );  
	glcheck( glBindTexture(GL_TEXTURE_2D, textureID) );
	
#ifndef OPENGL_ES
	//设置贴图和纹理的混合效果
	glcheck( glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL) );  
    glcheck( glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR) );       //线性滤波
    glcheck( glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR) );       //线性滤波
#endif
	
	int align = 0;
	glcheck( glGetIntegerv(GL_UNPACK_ALIGNMENT, &align) );

	int srcPixel = -1;
	if (pixel == PIXEL_RGBA)
		srcPixel = GL_RGBA;
	else if (pixel == PIXEL_RGB)
	{
		srcPixel = GL_RGB;
		glcheck( glPixelStorei(GL_UNPACK_ALIGNMENT, 1) );
	}


	//设置纹理所用到图片数据 
#ifdef OPENGL_ES
	glcheck( glTexImage2D(GL_TEXTURE_2D, 0, srcPixel, width, height, 0, srcPixel, GL_UNSIGNED_BYTE, rgba) );  
#else
	glcheck( glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, srcPixel, GL_UNSIGNED_BYTE, rgba) );  
#endif

	glcheck( glPixelStorei(GL_UNPACK_ALIGNMENT, align) );

	free(rgba);
	return textureID;  
}  
#endif


#ifdef GFX_ENABLE_PNG
bool my_png_create(FILE* fp, png_structp& png_ptr, png_infop& info_ptr)
{
	const int HEADER_SIZE = 8;
	
	//check header
	fseek(fp, 0, SEEK_SET);
	unsigned char header[8] = { 0 };
	fread(header, 1, HEADER_SIZE, fp);  
	if(0 != png_sig_cmp(header, 0, HEADER_SIZE))  
		return false;
	
	//create png pic and info
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL); 
	if(NULL == png_ptr)
		return false;  

	info_ptr = png_create_info_struct(png_ptr);  
	if(NULL == info_ptr)  
	{  
		png_destroy_read_struct(&png_ptr, NULL, NULL);  
		return false;  
	}  
	
	//error handler
	if (setjmp(png_jmpbuf(png_ptr)))  
	{  
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);  
		return false;  
	}  
	
	//read info
	png_init_io			(png_ptr, fp);  
	png_set_sig_bytes	(png_ptr, HEADER_SIZE);  
	png_read_info		(png_ptr, info_ptr);  
	return true;
}
#endif

#ifdef GFX_ENABLE_PNG
unsigned char* load_png_data(FILE* fp, unsigned char* out_rgba, int* out_width, int* out_height, int* pitch, int* pixel)
{
	fseek(fp, 0, SEEK_SET);

	png_structp png_ptr;
	png_infop info_ptr;
	if (!my_png_create(fp, png_ptr, info_ptr))
		return NULL;

	int width			= png_get_image_width	(png_ptr, info_ptr);  
	int height			= png_get_image_height	(png_ptr, info_ptr);  
	png_byte color_type = png_get_color_type	(png_ptr, info_ptr);  // if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)  
	switch (color_type)
	{
		case PNG_COLOR_TYPE_RGB_ALPHA: 
		{
		}
			break;
		case PNG_COLOR_TYPE_RGB: 
		{
			png_set_add_alpha(png_ptr, 0xFF, PNG_FILLER_AFTER);
		}
			break;
		default:
		{
			png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
			return 0;
		}
	};
	
	//png_byte bit_depth	= png_get_bit_depth		(png_ptr, info_ptr);
	
	//int number_of_passes = png_set_interlace_handling(png_ptr);  
	
	png_read_update_info(png_ptr, info_ptr);  
	if (setjmp(png_jmpbuf(png_ptr)))
	{  
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);  
		return NULL;  
	}  
	

	unsigned char*	rgba			= out_rgba;  
	if (NULL == rgba)
		rgba						= (unsigned char*	)malloc(width * height * 4);  
	png_bytep*		row_pointers	= (png_bytep*		)malloc(sizeof(png_bytep) * height);  
	
	for (int i = 0; i < height; i++)  
		row_pointers[i] = NULL;  
	
	//通过扫描流里面的每一行将得到的数据赋值给动态数组         
	for (int i = 0; i < height; i++)  
		row_pointers[i] = (png_bytep)png_malloc(png_ptr, png_get_rowbytes(png_ptr, info_ptr));  
	
	//read image data
	png_read_image(png_ptr, row_pointers);  
	
	//flip image
	//由于png他的像素是由 左-右-从顶到底 而贴图需要的像素都是从左-右-底到顶的所以在这里需要把像素内容进行一个从新排列 
	for(int row = 0; row < height; row++)  
	{  
//#ifdef __APPLE__
		//memcpy(&rgba[row * width * 4], row_pointers[height - row - 1], width * 4);
//#else
		memcpy(&rgba[row * width * 4], row_pointers[row], width * 4);
//#endif
	}  
	
	for (int i = 0; i < height; i++)
		png_free(png_ptr, row_pointers[i]); 
	free(row_pointers);   
	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

	if (NULL != out_width)
		*out_width = width;
	if (NULL != out_height)
		*out_height = height;
	if (NULL != pitch)
		*pitch = width * 4;
	if (NULL != pixel)
		*pixel = PIXEL_RGBA;

	return rgba;  
}
#endif

#ifdef GFX_ENABLE_JPEG
struct my_error_mgr
{
	struct jpeg_error_mgr pub;
	jmp_buf setjmp_buffer;
};
typedef struct my_error_mgr * my_error_ptr;
//char	jpegLastErrorMsg[JMSG_LENGTH_MAX] = {};
int		jpegLastErrorCode = 0;
void my_error_exit(j_common_ptr cinfo)
{
    my_error_ptr myerr = (my_error_ptr) cinfo->err;

	//(*(cinfo->err->format_message))(cinfo, jpegLastErrorMsg);

	jpegLastErrorCode = cinfo->err->msg_code;
	if (jpegLastErrorCode != JERR_TOO_LITTLE_DATA)
		(*cinfo->err->output_message)(cinfo);

    longjmp(myerr->setjmp_buffer, 1);
}
#endif

#ifdef GFX_ENABLE_JPEG
unsigned char* load_jpg_data(FILE* fp, unsigned char* out_rgba, int* width, int* height, int* pitch, int* pixel)
{
	struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    JSAMPARRAY		buffer;
    int				row_stride	= 0;
    unsigned char*	tmp_buffer	= NULL;
    int				rgb_size	= 0;

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    if (setjmp(jerr.setjmp_buffer))
    {
        jpeg_destroy_decompress(&cinfo);
        return NULL;
    }

    jpeg_create_decompress(&cinfo);

	fseek(fp, 0, SEEK_SET);
    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, TRUE);

	//cinfo.jpeg_color_space;
    cinfo.out_color_space = JCS_EXT_RGBX; //JCS_YCbCr;  // 设置输出格式

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
	if (NULL != pitch)
		*pitch = row_stride;
	if (NULL != pixel)
		*pixel = PIXEL_RGBA;
	if (NULL != width)
		*width = cinfo.output_width;
	if (NULL != height)
		*height = cinfo.output_height;

    rgb_size = row_stride * cinfo.output_height; // 总大小

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
    
    GLubyte* rgb_buffer = out_rgba;
	if (NULL == rgb_buffer)
		rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);    // 分配总内存
    
    //myprint("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size, cinfo.image_width*cinfo.image_height*3, cinfo.image_width, cinfo.image_height, row_stride);

    tmp_buffer = rgb_buffer;
    while (cinfo.output_scanline < cinfo.output_height) // 解压每一行
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    return rgb_buffer;
}
#endif

#ifdef GFX_ENABLE_JPEG
void get_jpg_size(const char* filename, int* out_width, int* out_height, int* out_pixel)
{
	FILE* f = my_fopen(filename);
	if (NULL == f)
		return;
	get_jpg_size(f, out_width, out_height, out_pixel);
	fclose(f);
}
#endif

#ifdef GFX_ENABLE_JPEG
void get_jpg_size(_iobuf* fp, int* out_width, int* out_height, int* out_pixel)
{
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	//JSAMPARRAY		buffer;
	//int				row_stride	= 0;
	//unsigned char*	tmp_buffer	= NULL;
	int				rgb_size	= 0;

	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer))
	{
		jpeg_destroy_decompress(&cinfo);
		return;
	}

	jpeg_create_decompress(&cinfo);

	fseek(fp, 0, SEEK_SET);
	jpeg_stdio_src(&cinfo, fp);

	jpeg_read_header(&cinfo, TRUE);

	//cinfo.jpeg_color_space;
	//cinfo.out_color_space = JCS_EXT_RGBX; //JCS_YCbCr;  // 设置输出格式

	jpeg_start_decompress(&cinfo);

	//row_stride = cinfo.output_width * cinfo.output_components;
	//if (NULL != pitch)
	//	*pitch = row_stride;
	//if (NULL != pixel)
	//	*pixel = PIXEL_RGB;
	if (NULL != out_width)
		*out_width = cinfo.output_width;
	if (NULL != out_height)
		*out_height = cinfo.output_height;
	if (NULL != out_pixel)
		*out_pixel = PIXEL_RGB; // TODO: now we only loaded jpg as rgb.

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
}
#endif

#ifdef GFX_ENABLE_TGA

void my_tga_error_exit(TGA *tga, int ErrorType)
{
	if (tga->jmp_buffer)
	{
		longjmp(tga->jmp_buffer, 1);
	}
}
unsigned char* load_tga_data(FILE* fp, unsigned char* out_rgba, int* width, int* height, int* pitch, int* pixel)
{
	//create TGA
	int				row_stride = 0;
	int				rgb_size = 0;
	unsigned char*	tmp_buffer = NULL;
	unsigned char*	rgb_buffer = NULL;
	unsigned char*	buffer	   = NULL;

	TGA *tga = TGAOpenFd(fp);
	if (NULL == tga)
		return NULL;

	tga->error = my_tga_error_exit;
	if (setjmp(tga->jmp_buffer))
	{
		if (tga) {
			free(tga);
		}
		if (tmp_buffer)
			free(tmp_buffer);
		if (rgb_buffer)
			free(rgb_buffer);
		return NULL;
	}

	//读取 头文件信息
	int headerindex = TGAReadHeader(tga);
	if (headerindex != TGA_OK)
	{
		// close TGA
		if (tga) {
			//fclose(tga->fd);
			free(tga);
		}
		return NULL;
	}

	//只支持 24 或者 32 位的图片格式
	if (tga && tga->hdr.depth != 24 && tga->hdr.depth != 32)
	{
		free(tga);
		return NULL;
	}

	row_stride = TGA_SCANLINE_SIZE(tga);
	if (NULL != pitch)
		*pitch = row_stride;
	if (NULL != pixel)
		*pixel = tga->hdr.depth/8;
	if (NULL != width)
		*width = tga->hdr.width;
	if (NULL != height)
		*height = tga->hdr.height;


	rgb_size		= tga->hdr.height * row_stride;
	rgb_buffer		= out_rgba;
	if (NULL == rgb_buffer)
		rgb_buffer	= (unsigned char *)malloc(sizeof(char) * rgb_size);    // 分配总内存
	tmp_buffer		= (unsigned char *)malloc(sizeof(char) * rgb_size);    // 分配总内存
	if (NULL == rgb_buffer)
	{
		free(tga);
		return NULL;
	}
	if (NULL == tmp_buffer)
	{
		free(tga);
		free(rgb_buffer);
		return NULL;
	}
	//读取像素数据

	if (TGAReadScanlines(tga, tmp_buffer, 0, tga->hdr.height, TGA_RGB) != tga->hdr.height || tga->last != TGA_OK) {
		if (tga) {
			//fclose(tga->fd);
			free(tga);
		}
		free(rgb_buffer);
		free(tmp_buffer);
		return NULL;
	}

	//flip image
	//由于tga他的像素是由 左-右-从顶到底 而贴图需要的像素都是从左-右-底到顶的所以在这里需要把像素内容进行一个从新排列 
	for (int row = 0; row < static_cast<int>(tga->hdr.height); row++)
	{
		//#ifdef __APPLE__
		//memcpy(&rgba[row * width * 4], row_pointers[height - row - 1], width * 4);
		//#else
		memcpy(&rgb_buffer[row * row_stride], &tmp_buffer[rgb_size - row_stride*(row+1)], row_stride);
		//#endif
	}

	free(tmp_buffer);
	if (tga) {
		free(tga);
	}
	return rgb_buffer;
}
#endif

#ifdef GFX_ENABLE_PNG
void get_png_size(const char* filename, int* out_width, int* out_height, int* out_pixel)
{
	FILE* f = my_fopen(filename);
	if (NULL == f)
		return;
	get_png_size(f, out_width, out_height, out_pixel);
	fclose(f);
}
#endif

#ifdef GFX_ENABLE_PNG
void get_png_size(_iobuf* fp, int* out_width, int* out_height, int* out_pixel)
{
	png_structp png_ptr = NULL;
	png_infop info_ptr = NULL;
	my_png_create(fp, png_ptr, info_ptr);

	if (NULL != out_width && NULL != png_ptr && NULL != info_ptr)
		*out_width	= png_get_image_width(png_ptr, info_ptr);  
	if (NULL != out_height && NULL != png_ptr && NULL != info_ptr)
		*out_height = png_get_image_height(png_ptr, info_ptr);  
	if (NULL != out_pixel)
		*out_pixel = PIXEL_RGBA; // rgb png will be load as rgba, so we only need to return rgba

	png_destroy_read_struct(&png_ptr, &info_ptr, NULL);  
}
#endif

#ifdef GFX_ENABLE_TGA

void get_tga_size(const char* filename, int* out_width, int* out_height, int* out_pixel)
{
	FILE* f = my_fopen(filename);
	if (NULL == f)
		return;
	get_tga_size(f, out_width, out_height, out_pixel);
	fclose(f);
}
#endif

#ifdef GFX_ENABLE_TGA
void get_tga_size(_iobuf* fp, int* out_width, int* out_height, int* out_pixel)
{
	TGA *tga = TGAOpenFd(fp);
	if (NULL == tga)
		return;

	tga->error = my_tga_error_exit;
	if (setjmp(tga->jmp_buffer))
	{
		if (tga) {
			free(tga);
		}
		return;
	}

	//读取 头文件信息
	int headerindex = TGAReadHeader(tga);
	if (headerindex != TGA_OK)
	{
		// close TGA
		if (tga) {
			//fclose(tga->fd);
			free(tga);
		}
		return;
	}
	if (NULL != out_width)
		*out_width = tga->hdr.width;
	if (NULL != out_height)
		*out_height = tga->hdr.height;
	if (NULL != out_pixel)
		*out_pixel = tga->hdr.depth / 8;

	if (tga) {
		free(tga);
	}
}
#endif // end of GFX_ENABLE_TGA

#ifdef GFX_ENABLE_ETC2
uint load_etc2_to_opengl(_iobuf* fp, int* out_width, int* out_height, int* pitch, int* pixel)
{
	GLuint			texture		= 0;
	GLenum			target		= GL_TEXTURE_2D;
	GLboolean		isMipmapped = GL_FALSE;
	GLenum			glError		= 0;
	KTX_dimensions	dims;
	fseek(fp, 0, SEEK_SET);
	memset(&dims, 0, sizeof(dims));
	KTX_error_code err = ktxLoadTextureF(fp, &texture, &target, &dims, &isMipmapped, &glError, NULL, NULL);
	if (err != KTX_SUCCESS)
	{
		printf("ktx error = %d\n", err);
		fclose(fp);
		return -1;
	}
	fclose(fp);
	return texture;
}


#endif // end of GFX_ENABLE_ETC2

}
