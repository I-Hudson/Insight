#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 lightPos;
} ubo;

layout (set = 0, binding = 1) uniform MODELUBO //#dynamic
{
	mat4 model;
} modelUBO;

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outColor;
layout (location = 3) out vec2 outUV;

void main() 
{
	gl_Position = ubo.projection * inverse(ubo.view) * vec4(inPos.xyz, 1.0);
	outPos = ubo.projection * inverse(ubo.view) * ubo.model * vec4(inPos.xyz, 1.0);
	outNormal = vec4(inNormal, 1.0);
	outColor = inColor;
	outUV = inUV;
}