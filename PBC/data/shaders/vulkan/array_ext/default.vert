#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;
layout (location = 4) in int inSubMeshIndex;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 PVMatrix;
	mat4 lightSpace;
	vec3 lightDir;
} ubo;

layout (set = 0, binding = 1) uniform PER_OBJECT
{
	mat4 transforms[];
} pc;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec2 outUV;
layout (location = 4) out int outSubMeshIndex;
layout (location = 5) out vec3 outViewVec;
layout (location = 6) out vec3 outLightVec;
layout (location = 7) out vec4 outShadowCoord;

void main() 
{
	gl_Position = ubo.PVMatrix * pc.transforms[inSubMeshIndex] * vec4(inPos.xyz, 1.0);
	outPos = mat3(pc.transforms[inSubMeshIndex]) * inPos.xyz;
	outNormal = mat3(pc.transforms[inSubMeshIndex]l) * inNormal;
	outColor = inColor.xyz;
	outUV = inUV;

	outLightVec = ubo.lightDir;
    outViewVec = -outPos.xyz;	
	outShadowCoord = (biasMat * ubo.lightSpace * pc.transforms[inSubMeshIndex]) * vec4(inPos.xyz, 1);
}