#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;
//layout (location = 4) in int  inVindex;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 depthMVP;
} ubo;

layout (location = 0) out vec4 outPos;

void main() 
{
	gl_Position = ubo.depthMVP * vec4(inPos.xyz, 1.0);
}