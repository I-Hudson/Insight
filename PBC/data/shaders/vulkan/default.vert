#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 PVMatrix;
	mat4 lightSpace;
	vec4 lightPos;
} ubo;

layout (set = 0, binding = 1) uniform MODELUBO //#dynamic
{
	mat4 model;
} modelUBO;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

layout (location = 0) out vec4 outPos;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outColor;
layout (location = 3) out vec2 outUV;
layout (location = 4) out vec4 outShadowCoord;

void main() 
{
	gl_Position = ubo.PVMatrix * modelUBO.model * vec4(inPos.xyz, 1.0);
	outPos = ubo.PVMatrix * modelUBO.model * vec4(inPos.xyz, 1.0);
	outNormal = vec4(inNormal, 1.0);
	outColor = inColor;
	outUV = inUV;
	outShadowCoord = biasMat * ubo.lightSpace * modelUBO.model * vec4(inPos.xyz, 1.0);
}