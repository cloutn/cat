
precision mediump float;							
varying vec4 vertex_color;						
varying vec2 vertex_coord;					
uniform sampler2D tex;					

void main()							
{															
	gl_FragColor = texture2D(tex, vertex_coord);	
}								

