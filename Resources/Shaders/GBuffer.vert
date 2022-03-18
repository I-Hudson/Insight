#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec4 inColour;

layout(location = 0) out vec4 outColour;

void main() 
{
	gl_Position = vec4(inPos.xyz, 1.0);
	outColour = inColour;
}