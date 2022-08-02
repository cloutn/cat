#version 460

#ifdef TEXTURE
layout(set = 0, binding = 1) uniform sampler2D tex;
layout(location = 0) in vec4 i_texcoord;
#endif

layout(location = 1) in vec3 i_position;

#ifdef COLOR
layout(location = 2) in vec4 i_color;
#endif

//////////////////////////////
// outputs
//////////////////////////////
layout(location = 0) out vec4 o_color;

void main()
{

#ifdef COLOR
	vec4 color = i_color;
#else
	vec4 color = vec4(1);
#endif

#ifdef TEXTURE
	o_color = texture(tex, i_texcoord.xy) * color;
#else
	o_color = color;
#endif
}

