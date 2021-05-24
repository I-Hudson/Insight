#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 PVMatrix;
} ubo;

layout (location = 0) out vec4 outColor;
void main() 
{
	gl_Position = ubo.PVMatrix * vec4(inPos.xyz, 1.0);
	outColor = inColor;
}