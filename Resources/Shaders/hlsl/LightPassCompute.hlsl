#include "Common.hlsl"

RWTexture2D<float4> OutputTex  : register(u0);
Texture2D DepthTex : register(t0);
Texture2D ColourTex : register(t1);

[numthreads(8, 8, 1)]
void CSMain(uint3 thread_id : SV_DispatchThreadID)
{
    float2 resolution_out;
    OutputTex.GetDimensions(resolution_out.x, resolution_out.y);
    if (any(int2(thread_id.xy) >= resolution_out))
    {
        return;
    }
    
	const float DepthValue = DepthTex[thread_id.xy].r;
    const float3 worldPosition = reconstruct_position(thread_id.xy, DepthValue, bf_Camera_Projection_View_Inverted);
	const float3 albedo = ColourTex[thread_id.xy].rgb;
    const float3 ambientAlbedo = albedo * 0.35;

    OutputTex[thread_id.xy] = float4(ambientAlbedo, 1);
}