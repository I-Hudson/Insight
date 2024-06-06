#include "Common.hlsl"

RWTexture2D<float4> OutputTex  : register(u0);
Texture2D DepthTex : register(u1);
Texture2D ColourTex : register(u2);

[numthreads(8, 8, 1)]
void CSMain(uint3 thread_id : SV_DispatchThreadID)
{
    float2 resolution_out;
    OutputTex.GetDimensions(resolution_out.x, resolution_out.y);
    if (any(int2(thread_id.xy) >= resolution_out))
    {
        return;
    }
    
	const float DepthValue = DepthTex.Sample(Clamp_Sampler, input.UV).r;
    const float3 worldPosition = reconstruct_position(input.UV, DepthValue, bf_Camera_Projection_View_Inverted);
	const float3 albedo = (ColourTex.Sample(Clamp_Sampler, input.UV).xyz);
    const float3 ambientAlbedo = albedo * 0.15;

    tex_uav[thread_id.xy] = saturate_16(color);
}