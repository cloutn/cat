#version 300 es		

precision mediump float;								
in vec4 vertex_color;								
in vec2 vertex_coord;								
out vec4 out_color;									
uniform sampler2D tex;								
void main()											
{													
	out_color = texture(tex, vertex_coord);	
}													
