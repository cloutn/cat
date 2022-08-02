//#version 300 es											

uniform mat4 mvp;									
layout(location = 0) in vec4 position;			
layout(location = 1) in vec4 color;			
layout(location = 2) in vec2 coord;		

#ifdef SKIN_MESH
layout(location = 3) in vec4 joints;
layout(location = 4) in vec4 weights;
uniform mat4 joint_matrices[50];
#endif


out vec4 vertex_color;				
out vec2 vertex_coord;			
void main()					
{						
#ifdef SKIN_MESH
	mat4 skinMatrix = weights.x * joint_matrices[int(joints.x)] +
		weights.y * joint_matrices[int(joints.y)] +
		weights.z * joint_matrices[int(joints.z)] +
		weights.w * joint_matrices[int(joints.w)];

	gl_Position	= mvp * skinMatrix * vec4(position.xyz, 1.0);
#else
	gl_Position	= mvp * vec4(position.xyz, 1.0);
#endif


   //gl_Position 			= mvp * position;				
   vertex_color.rgba 		= vec4(1.0, 1.0, 1.0, 1.0);
   vertex_coord			= coord;					
}												


