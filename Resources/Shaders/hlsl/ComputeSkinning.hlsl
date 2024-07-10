#include "Common_Structs.hlsl"

RWStructuredBuffer<GeoVertexInput> InputVertices  : register(u0);
RWStructuredBuffer<GeoVertexInput> OutputVertices  : register(u1);
RWStructuredBuffer<float4x4> SkeletonBones  : register(u2);

cbuffer MeshInfoBuffer : register(b0)
{
    uint MeshInfoVertexCount;
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

    const uint vertexOffset = sizeof(GeoVertexInput) * vertexId;

    const GeoVertexInput inputVertex = InputVertices[vertexId];
    OutputVertices[vertexId] = inputVertex;
}