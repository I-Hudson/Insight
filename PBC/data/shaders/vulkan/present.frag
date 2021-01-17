#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 0) uniform sampler2D fullScreenQuadColorTex;
layout (set = 0, binding = 1) uniform sampler2D fullScreenQuadNormalTex;
layout (set = 0, binding = 2) uniform sampler2D fullScreenQuadPositionTex;

void main() 
{
	outFragColor = vec4(0.0, 1.0, 0.0, 1.0);
}