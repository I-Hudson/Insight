#include "Defines.hlsl"

cbuffer BufferFrame : register(b0, FrameSpace)
{
	float4x4 bf_Camera_Proj_View;
	float4x4 bf_Camera_Projection;
	float4x4 bf_Camera_View;
	float4x4 bf_Camera_View_Inverted;
	float4x4 bf_Camera_Projection_View_Inverted;

    int2 Render_Resolution;
    int2 Output_Resolution;

    float2 bf_TAA_Jitter_Current;
    float2 bf_TAA_Jitter_Previous;

    float Delta_Time;
    int bf_RenderOptions; // bit0 - GPUSkinning Enabled
    float bf__pad0;
    float bf__pad1;
}

bool GPUSkinningEnabled()
{
    return (bf_RenderOptions & 0x1) == 1;
}

#define s_Cascade_Count 4

cbuffer BufferLight : register(b0, PassSpace)
{
	float4x4 bl_Camera_Proj_View[s_Cascade_Count];
	float4x4 bl_Camera_Projection[s_Cascade_Count];
	float4x4 bl_Camera_View[s_Cascade_Count];
    float4 bl_Light_Split_Depth;
    float4 bl_Light_Direction;
    float4 bl_Light_Colour;
    float2 bl_Shadow_Resolution;
}

cbuffer BufferPerObject : register(b0, PerObjectUniform)
{
    float4x4 bpo_Transform;
    float4x4 bpo_Previous_Transform;

    float4 bpo_Textures_Set;
    int bpo_SkinnedMesh;
}

#define s_MAX_BONE_COUNT 72
cbuffer BufferPerObjectBoneMatrices : register(b2, PerObjectUniform)
{
    float4x4 bpo_BoneMatrices[s_MAX_BONE_COUNT];
}
