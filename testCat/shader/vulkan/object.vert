#version 460


////////////////////////////////////
// uniforms
////////////////////////////////////
layout(set = 0, binding = 0, std140) uniform MvpMatrix 
{
    mat4 mat;
} mvp;

#ifdef SKIN
layout(set = 0, binding = 2, std140) uniform JointMatrices
{
	mat4 mats[JOINT_MATRIX_COUNT];
} joint_matrices;
#endif


////////////////////////////////////
// inputs
// location number defined in primitive.cpp function _attrNameToIndex
////////////////////////////////////

layout(location = 0) in vec4 i_position;

#ifdef NORMAL
layout(location = 1) in vec3 i_normal;
#endif

#ifdef TANGENT
layout(location = 2) in vec3 i_tangent;
#endif

#ifdef COLOR
layout(location = 5) in vec4 i_color;
#endif

#ifdef TEXTURE
layout(location = 3) in vec2 i_uv;
#endif

#ifdef SKIN
layout(location = 6) in uvec4 i_joints;
layout(location = 7) in vec4 i_weights;
#endif



////////////////////////////////////
// outputs
////////////////////////////////////

#ifdef TEXTURE
layout(location = 0) out vec4 o_texcoord;
#endif

layout(location = 1) out vec3 o_position;

#ifdef COLOR
layout(location = 2) out vec4 o_color;
#endif

void main()
{
#ifdef COLOR
	o_color.rgba = i_color.bgra;
#endif

#ifdef TEXTURE
    o_texcoord.xy	= i_uv.xy;
#endif

#ifdef SKIN
	mat4 skinMatrix = 
		i_weights.x * joint_matrices.mats[i_joints.x] +
		i_weights.y * joint_matrices.mats[i_joints.y] +
		i_weights.z * joint_matrices.mats[i_joints.z] +
		i_weights.w * joint_matrices.mats[i_joints.w];

	gl_Position	= mvp.mat * skinMatrix * vec4(i_position.xyz, 1.0);
#else
    gl_Position = mvp.mat * vec4(i_position.xyz, 1.0);
#endif

    o_position		= gl_Position.xyz;
}


