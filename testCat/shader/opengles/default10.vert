
uniform mat4 mvp;									
attribute vec4 position;						
attribute vec4 color;						
attribute vec2 coord;					


#ifdef SKIN_MESH
attribute vec4 joints;
attribute vec4 weights;
uniform mat4 joint_matrices[50];
#endif


varying vec4 vertex_color;										
varying vec2 vertex_coord;									


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

	vertex_color.rgba 	= color.bgra;				
	vertex_coord		= coord;						
}															

