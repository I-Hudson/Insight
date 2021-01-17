#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outPos;

layout (set = 1, binding = 1) uniform sampler2D texture_diffuse;

layout (set = 1, binding = 2) uniform TextureLookup
{
	int diffuse;
}textureLookup;

void main() 
{
	if(textureLookup.diffuse == 1)
	{
		outColor = vec4(texture(texture_diffuse, inUV).xyz, 1.0);
	}
	else
	{
		outColor = vec4(inColor, 1.0);
	}
	outNormal = vec4(1.0, 0.0, 0.0, 1.0);
	outPos = vec4(0.0, 1.0, 0.0, 1.0);
}