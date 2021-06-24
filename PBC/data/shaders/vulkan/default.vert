#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;
layout (location = 4) in ivec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;
//layout (location = 4) in int  inVindex;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 PVMatrix;
	mat4 lightSpace;
	vec3 lightDir;
} ubo;

layout(push_constant) uniform PER_OBJECT
{
	mat4 modelMatrix;
	int SkinnedMesh;
}perObject;

layout(set = 1, binding = 0) readonly buffer JointMatrices 
{
	mat4 jointMatrices[];
};

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec2 outUV;
layout (location = 4) out vec3 outViewVec;
layout (location = 5) out vec3 outLightVec;
layout (location = 6) out vec4 outShadowCoord;

mat4 Animate()
{
	// Calculate skinned matrix from weights and joint indices of the current vertex
	mat4 boneTransform = jointMatrices[inJointIndices.x] * inJointWeights.x;
	boneTransform += jointMatrices[inJointIndices.y] * inJointWeights.y;
	boneTransform += jointMatrices[inJointIndices.z] * inJointWeights.z;
	boneTransform += jointMatrices[inJointIndices.w] * inJointWeights.w;
	return boneTransform;
}

void main()
{
	mat4 skinMat = mat4(1);
	if (perObject.SkinnedMesh == 1)
	{
		 skinMat = Animate();
	}

	gl_Position = ubo.PVMatrix * perObject.modelMatrix * skinMat * vec4(inPos.xyz, 1.0);
	outPos = mat3(perObject.modelMatrix * skinMat) * inPos.xyz;
	outNormal = mat3(perObject.modelMatrix * skinMat) * inNormal;
	outColor = inColor.xyz;
	outUV = inUV;

	outLightVec = ubo.lightDir;
    outViewVec = -outPos.xyz;	
	outShadowCoord = biasMat * ubo.lightSpace * perObject.modelMatrix * skinMat * vec4(inPos.xyz, 1);
}