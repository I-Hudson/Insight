#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;
layout (location = 4) in ivec4 inJointIndices;
layout (location = 5) in vec4 inJointWeights;
//layout (location = 4) in int  inVindex;

layout (location = 0) out vec4 outPos;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 depthMVP;
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
	gl_Position = ubo.depthMVP * skinMat * vec4(inPos.xyz, 1.0);
}