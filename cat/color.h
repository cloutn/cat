////////////////////////////////////////////////////////////////////////////////
//	颜色相关
//	
//	2010.09.05 caolei
////////////////////////////////////////////////////////////////////////////////
#pragma once

#include "scl/type.h"

namespace cat {

// The "ARGB" in catui is in the word-order scheme. A is the most significant bit.
//		in little-endian:
//		addr:	0 1	2 3	
//				B G R A	
//		word:	0xARGB 

// In OpenGL shader, the "RGBA" is in the byte-order scheme.
//		in little-endian:
//		addr:	0 1 2 3
//				R G B A
//		word:	0xABGR

#define COLOR_RGB(r,g,b)          ((uint32)(((byte)(r)|((uint16)((byte)(g))<<8))|(((uint32)(byte)(b))<<16)))

#define COLOR_ARGB(a, r, g, b) \
	( \
		byte(b) | uint16(g << 8) | uint32( r << 16) | uint32( a << 24 ) \
	)

#define COLOR_GET_A(c) byte((c & 0xFF000000) >> 24)
#define COLOR_GET_R(c) byte((c & 0x00FF0000) >> 16)
#define COLOR_GET_G(c) byte((c & 0x0000FF00) >> 8)
#define COLOR_GET_B(c) byte(c & 0x000000FF)
#define COLOR_SET_A(c, a) (c = ((c & 0x00FFFFFF) | (((uint)a) << 24)))

//纹理的alpha的混合模式
enum ALPHA_MODE
{
	ALPHA_MODE_INVALID	= 0,	//默认的混合方式, SRCBLEND = SRCALPHA	DESTBLEND = INVSRCALPHA
	ALPHA_MODE_ADD		= 1,	//SRCBLEND = DESTALPHA, DESTBLEND = ONE 
};

inline uint blendColor(uint32 back, uint32 front, byte alpha)
{
	uint backR = COLOR_GET_R(back);
	uint backG = COLOR_GET_G(back);
	uint backB = COLOR_GET_B(back);

	uint frontR = COLOR_GET_R(front);
	uint frontG = COLOR_GET_G(front);
	uint frontB = COLOR_GET_B(front);

	uint blendR = backR * (0xFF - alpha) / 0xFF + frontR * alpha / 0xFF;
	uint blendG = backG * (0xFF - alpha) / 0xFF + frontG * alpha / 0xFF;
	uint blendB = backB * (0xFF - alpha) / 0xFF + frontB * alpha / 0xFF;

	return COLOR_ARGB(0x00, blendR, blendG, blendB);
}

inline uint32 color_mixAlpha(uint32 color, uint8 alpha)
{
	uint8	oldAlpha	= COLOR_GET_A(color);
	uint8	newAlpha	= static_cast<uint8>((oldAlpha * static_cast<uint32>(alpha)) / 0xFF);
	uint32	newColor	= (color & 0x00FFFFFF) | (static_cast<uint32>(newAlpha) << 24);
	return newColor;
}

inline uint32 color_setRGB(uint32 color, uint32 rgb)
{
	uint8 a = COLOR_GET_A(color);
	uint8 r = COLOR_GET_R(rgb);
	uint8 g = COLOR_GET_G(rgb);
	uint8 b = COLOR_GET_B(rgb);
	uint32 result =  COLOR_ARGB(a, r, g, b);
	return result;
}

inline uint32 color_setAlpha(uint32 color, uint8 alpha)
{
	uint8 r = COLOR_GET_R(color);
	uint8 g = COLOR_GET_G(color);
	uint8 b = COLOR_GET_B(color);
	uint32 result =  COLOR_ARGB(alpha, r, g, b);
	return result;
}

////////////////////////////////////////////////////////////////////////////////
//	class Color
////////////////////////////////////////////////////////////////////////////////
class Color
{
public:
	Color();
	~Color();

	uint32	getColor() const				{ return m_color; }
	void	setColor(const uint32 color) { m_color = color; }

 	static inline uint32 fromRGB(int r, int g, int b)
 	{
 		return COLOR_RGB(r, g, b);
 	}

private:
	uint32	m_color;
};


enum PIXEL
{
	PIXEL_INVALID	= -1,

	PIXEL_A			= 0,		//alpha, 8 bits
	PIXEL_L			= 1,		//luminance, 8 bits
	PIXEL_LA		= 2,		//luminance & alpha, 16 bits
	PIXEL_RGB		= 3,		//24 bits
	PIXEL_RGBA		= 4,		//32 bits

	PIXEL_COUNT,	
};

static const int PixelByte	[PIXEL_COUNT] = { 1, 1, 2,	3,	4	};
inline int pixel_to_byte(PIXEL p) 
{
	if (p <= PIXEL_INVALID || p >= PIXEL_COUNT)
		return 0;
	return PixelByte[p];
}

void argb_to_float(uint color, float& a, float& r, float& g, float& b);


uint32 color_lerp(const uint32 color1, const uint32 color2, const float t);

}	// namespace cat

