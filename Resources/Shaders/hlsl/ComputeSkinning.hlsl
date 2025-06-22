#include "Common_Structs.hlsl"

RWStructuredBuffer<GeoVertexInput> InputVertices  : register(u0);
RWStructuredBuffer<GeoVertexInput> OutputVertices  : register(u1);
RWStructuredBuffer<float4x4> SkeletonBones  : register(u2);

cbuffer MeshInfoBuffer : register(b0)
{
    uint MeshInfoVertexCount;
}

#define ZERO_IDENTITY float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
#define ZERO_MATRIX float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

float4x4 SkinnedBoneMatrix(const in GeoVertexInput input)
{
	float4x4 BoneTransform = ZERO_MATRIX; 
	BoneTransform += SkeletonBones[GetVertexBondId(input.BoneIds, 0)] * GetVertexBoneWeight(input.BoneWeights, 0);
    BoneTransform += SkeletonBones[GetVertexBondId(input.BoneIds, 1)] * GetVertexBoneWeight(input.BoneWeights, 1);
    BoneTransform += SkeletonBones[GetVertexBondId(input.BoneIds, 2)] * GetVertexBoneWeight(input.BoneWeights, 2);
    BoneTransform += SkeletonBones[GetVertexBondId(input.BoneIds, 3)] * GetVertexBoneWeight(input.BoneWeights, 3);
	return BoneTransform;
}

[numthreads(64, 1, 1)]
void CSMain(uint3 thread_id : SV_DispatchThreadID)
{
    const uint vertexId = thread_id.x;

    [branch]
    if (vertexId >= MeshInfoVertexCount)
    {
        return;
    }

    GeoVertexInput inputVertex = InputVertices[vertexId];
    const float4x4 boneTransform = SkinnedBoneMatrix(inputVertex);

	inputVertex.Position = mul(boneTransform, float4(inputVertex.Position.xyz, 1)).xyz;

    //float4x4 transform = ZERO_IDENTITY;
    //transform[3] = float4(0, 10, 0, 1);
    //inputVertex.Position = inputVertex.Position + float3(0, 5, 0);

    OutputVertices[vertexId] = inputVertex;
}