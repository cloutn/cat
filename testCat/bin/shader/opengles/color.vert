#version 300 es											

uniform mat4 mvp;									
layout(location = 0) in vec4 position;			
layout(location = 1) in vec4 color;			

out vec4 vertex_color;				

void main()					
{						
    gl_Position	= mvp * vec4(position.xyz, 1.0);
    vertex_color.rgba = color.bgra;											
}


