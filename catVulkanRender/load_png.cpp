
#include "./load_png.h"

#include <libpng/png.h>  
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

static const int PIXEL_RGBA		= 4;
static const int PIXEL_RGB		= 3;

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

unsigned char* load_png_data_to_memory(FILE* fp, unsigned char* rgba, int* out_width, int* out_height, int* pitch, int* pixel)
{
	fseek(fp, 0, SEEK_SET);

	png_structp png_ptr;
	png_infop info_ptr;
	if (my_png_create(fp, png_ptr, info_ptr) == false)
	{
		return NULL;
	}

	int width			= png_get_image_width	(png_ptr, info_ptr);  
	int height			= png_get_image_height	(png_ptr, info_ptr);  
	*out_width = width;
	*out_height = height;
	if (NULL == rgba)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		return NULL;
	}

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

	//unsigned char*	rgba			= (unsigned char*	)malloc(width * height * 4);  
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
