#pragma once

#include "scl/type.h"

//	pixel format
//	0	= alpha, 8 bits
//	1	= luminance, 8 bits
//	2	= luminance & alpha, 16 bits
//	3	= 24 bits
//	4	= 32 bits

namespace cat {

#ifdef GFX_ENABLE_OPENGLES
uint			load_shader			(const char* const vs_source, const char* const ps_source);
uint			load_shader_file	(const char* const vs_filename, const char* const ps_filename);
#endif

	
#ifdef USE_OPENGL_300_ES

//static const char* const shader_vs = 
//"#version 300 es											\n"
//"uniform mat4 mvp;											\n"
//"layout(location = 0) in vec4 position;						\n"
//"layout(location = 1) in vec4 color;						\n"
//"layout(location = 2) in vec2 coord;						\n"
//"out vec4 vertex_color;										\n"
//"out vec2 vertex_coord;										\n"
//"void main()												\n"
//"{															\n"
//"   gl_Position 			= mvp * position;				\n"
//"   vertex_color.rgba 		= vec4(1.0, 1.0, 1.0, 1.0);					\n"
//"   vertex_coord			= coord;						\n"
//"}															\n";
//
//
//static const char* const shader_ps = 
//"#version 300 es											\n"
//"precision mediump float;									\n"
//"in vec4 vertex_color;										\n"
//"in vec2 vertex_coord;										\n"
//"out vec4 out_color;										\n"
//"uniform sampler2D tex;										\n"
//"void main()												\n"
//"{															\n"
//"	out_color = texture(tex, vertex_coord);	\n"
//"}															\n";
//
//static const char* const shader_add_color_ps = 
//"#version 300 es											\n"
//"precision mediump float;									\n"
//"in vec4 vertex_color;										\n"
//"in vec2 vertex_coord;										\n"
//"out vec4 out_color;										\n"
//"uniform sampler2D tex;										\n"
//"void main()												\n"
//"{															\n"
//"	vec4 tex_color	= texture(tex, vertex_coord);			\n"
//"	out_color.a		= vertex_color.a * tex_color.a;			\n"
//"	out_color.rgb	= vertex_color.rgb + tex_color.rgb;		\n"
//"}															\n";
//
//static const char* const shader_color_ps = 
//"#version 300 es											\n"
//"precision mediump float;									\n"
//"in vec4 vertex_color;										\n"
//"in vec2 vertex_coord;										\n"
//"out vec4 out_color;										\n"
//"uniform sampler2D tex;										\n"
//"void main()												\n"
//"{															\n"
//"	out_color = vertex_color;								\n"
//"}															\n";
//
//static const char* const shader_font_ps =
//"#version 300 es										\n"
//"precision mediump float;								\n"
//"in vec4 vertex_color;									\n"
//"in vec2 vertex_coord;									\n"
//"out vec4 out_color;									\n"
//"uniform sampler2D tex;									\n"
//"void main()											\n"
//"{														\n"
//"	vec4 tex_color = texture(tex, vertex_coord);		\n"
//"	out_color = vec4(vertex_color.rgb , tex_color.a);	\n"
//"}														\n";
//
//static const char* const shader_font_outline_ps =
//"#version 300 es																\n"
//"precision mediump float;														\n"
//"in vec4 vertex_color;															\n"
//"in vec2 vertex_coord;															\n"
//"out vec4 out_color;															\n"
//"uniform sampler2D tex;															\n"
//"uniform vec4 outline_color;													\n"
//"void main()																	\n"
//"{																				\n"
//"	vec4 tex_color		= texture(tex, vertex_coord);							\n"
//"	float fontAlpha		= tex_color.a; 											\n"
//"	float outlineAlpha	= tex_color.r; 											\n"
//"	vec4 color = vertex_color * fontAlpha + outline_color * (1.0 - fontAlpha);	\n"
//"	out_color = vec4(color.rgb , max(fontAlpha, outlineAlpha) * color.a);		\n"
//"}																				\n";
//
//
//static const char* const shader_gray_ps = 
//"#version 300 es												\n"
//"precision mediump float;										\n"
//"in vec4 vertex_color;											\n"
//"in vec2 vertex_coord;											\n"
//"out vec4 out_color;											\n"
//"uniform sampler2D tex;											\n"
//"void main()													\n"
//"{																\n"
//"	out_color = vertex_color * texture(tex, vertex_coord);		\n"
//"	float gray = dot(out_color.rgb, vec3(0.299, 0.587, 0.114));	\n"
//"	out_color.rgb = vec3(gray, gray, gray);						\n"
//"}";		

#else  // end of USE_OPENGL_300_ES

static const char* const shader_vs = R"(
#version 100 											
uniform mat4 mvp;									
attribute vec4 position;						
attribute vec4 color;						
attribute vec2 coord;					
varying vec4 vertex_color;										
varying vec2 vertex_coord;									
void main()											
{													
   gl_Position 			= mvp * vec4(position.xyz, 1.0);
   vertex_color.rgba 	= color.bgra;				
   //vertex_color.rgba 	= vec4(color.r * 0.0001, 1.0, 0, 1.0);				
   //if (gl_Position.z / gl_Position.w > 0.991) vertex_color.g = 0;			
   vertex_coord			= coord;						
}															
)";

static const char* const shader_vs_skin = R"(
#version 100 											
uniform mat4 mvp;									
attribute vec4 position;						
attribute vec4 color;						
attribute vec2 coord;					
attribute vec4 joints;
attribute vec4 weights;
uniform mat4 joint_matrices[50];
varying vec4 vertex_color;										
varying vec2 vertex_coord;									
void main()											
{													
	mat4 skinMatrix = weights.x * joint_matrices[int(joints.x)] +
		weights.y * joint_matrices[int(joints.y)] +
		weights.z * joint_matrices[int(joints.z)] +
		weights.w * joint_matrices[int(joints.w)];

	gl_Position 			= mvp * skinMatrix * vec4(position.xyz, 1.0);
	vertex_color.rgba 	= color.bgra;				
	//vertex_color.rgba 	= vec4(color.r * 0.0001, 1.0, 0, 1.0);				
	//if (gl_Position.z / gl_Position.w > 0.991) vertex_color.g = 0;			
	vertex_coord			= coord;						
}															
)";

//"#version 100 											\n"
//"uniform mat4 mvp;											\n"
//"attribute vec4 position;						\n"
//"attribute vec4 color;						\n"
//"attribute vec2 coord;						\n"
//"varying vec4 vertex_color;										\n"
//"varying vec2 vertex_coord;										\n"
//"void main()												\n"
//"{															\n"
//"   gl_Position 			= mvp * vec4(position.xyz, 1.0);				\n"
//"   vertex_color.rgba 		= color.bgra;					\n"
//"   //vertex_color.rgba 		= vec4(color.r * 0.0001, 1.0, 0, 1.0);					\n"
//"   //if (gl_Position.z / gl_Position.w > 0.991) vertex_color.g = 0;					\n"
//"   vertex_coord			= coord;						\n"
//"}															\n";


static const char* const shader_ps = 
"#version 100											\n"
"precision mediump float;									\n"
"varying vec4 vertex_color;										\n"
"varying vec2 vertex_coord;										\n"
"uniform sampler2D tex;										\n"
"void main()												\n"
"{															\n"
"	gl_FragColor = texture2D(tex, vertex_coord);	\n"
"	//gl_FragColor = vertex_color;	\n"
"}															\n";

static const char* const shader_add_color_ps = 
"#version 100											\n"
"precision mediump float;									\n"
"varying vec4 vertex_color;										\n"
"varying vec2 vertex_coord;										\n"
"uniform sampler2D tex;										\n"
"void main()												\n"
"{															\n"
"	vec4 tex_color	= texture2D(tex, vertex_coord);			\n"
"	gl_FragColor.a		= vertex_color.a * tex_color.a;			\n"
"	gl_FragColor.rgb	= vertex_color.rgb + tex_color.rgb;		\n"
"}															\n";

static const char* const shader_color_ps = 
"#version 100											\n"
"precision mediump float;									\n"
"varying vec4 vertex_color;										\n"
"varying vec2 vertex_coord;										\n"
"uniform sampler2D tex;										\n"
"void main()												\n"
"{															\n"
"	gl_FragColor = vertex_color;								\n"
"}															\n";

static const char* const shader_font_ps =
"#version 100										\n"
"precision mediump float;								\n"
"varying vec4 vertex_color;									\n"
"varying vec2 vertex_coord;									\n"
"uniform sampler2D tex;									\n"
"void main()											\n"
"{														\n"
"	vec4 tex_color = texture2D(tex, vertex_coord);		\n"
"	gl_FragColor = vec4(vertex_color.rgb , tex_color.a);	\n"
"}														\n";

static const char* const shader_font_outline_ps =
"#version 100																\n"
"precision mediump float;														\n"
"varying vec4 vertex_color;															\n"
"varying vec2 vertex_coord;															\n"
"uniform sampler2D tex;															\n"
"uniform vec4 outline_color;													\n"
"void main()																	\n"
"{																				\n"
"	vec4 tex_color		= texture2D(tex, vertex_coord);							\n"
"	float fontAlpha		= tex_color.a; 											\n"
"	float outlineAlpha	= tex_color.r; 											\n"
"	vec4 color = vertex_color * fontAlpha + outline_color * (1.0 - fontAlpha);	\n"
"	gl_FragColor = vec4(color.rgb , max(fontAlpha, outlineAlpha) * color.a);		\n"
"}																				\n";

static const char* const shader_gray_ps = 
"#version 100												\n"
"precision mediump float;										\n"
"varying vec4 vertex_color;											\n"
"varying vec2 vertex_coord;											\n"
"uniform sampler2D tex;											\n"
"void main()													\n"
"{																\n"
"	gl_FragColor = vertex_color * texture2D(tex, vertex_coord);		\n"
"	float gray = dot(gl_FragColor.rgb, vec3(0.299, 0.587, 0.114));	\n"
"	gl_FragColor.rgb = vec3(gray, gray, gray);						\n"
"}";		

#endif // end of NOT USE_OPENGL_300_ES


} // namespace cat 
