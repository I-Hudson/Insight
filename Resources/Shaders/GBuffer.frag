#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec4 inColour;

layout (location = 0) out vec4 outFragColor;

void main() 
{
	outFragColor = inColour;
}