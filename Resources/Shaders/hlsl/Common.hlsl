#include "Defines.hlsl"
#include "Common_Buffers.hlsl"
#include "Common_Structs.hlsl"
#include "Common_Textures.hlsl"
#include "Common_Samplers.hlsl"

static float4x4 biasMat = float4x4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

// Return the correct UV format for if the shader is DX12 or Vulkan
float2 GetUVsForAPI(float2 uv)
{
#ifdef DX12
	return uv;
#elif defined(VULKAN)
    return float2(uv.x, 1.0 - uv.y);
#endif
}

float LineariseFloat(const float v, const float min, const float max)
{
	return min * max / (max + v * (min - max));
}

uint DirectionToCubeFaceIndex(float3 direction)
{
    direction = normalize(direction);
    // find the absolute values of the direction components
    float3 abs_direction = abs(direction);

    // identify which component is the greatest
    float max_component = max(max(abs_direction.x, abs_direction.y), abs_direction.z);

    // determine the cube face index based on the greatest component and its sign
    if (max_component == abs_direction.x)
        return (direction.x > 0.0f) ? 0 : 1;

    if (max_component == abs_direction.y)
        return (direction.y > 0.0f) ? 2 : 3;

    if (max_component == abs_direction.z)
        return (direction.z > 0.0f) ? 4 : 5;

    return 0;
}

//================================================================
// is_saturated
//================================================================
bool is_saturated(float value)  { return value == saturate(value); }
bool is_saturated(float2 value) { return is_saturated(value.x) && is_saturated(value.y); }

//================================================================
// ndc_to_uv
//================================================================
float2 ndc_to_uv(float2 x)
{
    return x * float2(0.5f, -0.5f) + 0.5f;
}
float2 ndc_to_uv(float3 x)
{
    float2 uv = x.xy * float2(0.5f, 0.5f) + 0.5f;
	#ifdef VULKAN
    uv.y = 1.0 - uv.y;
	#endif
	return uv;
}

//================================================================
// world_to_ndc
//================================================================
float3 world_to_ndc(float3 x, float4x4 transform) // shadow mapping
{
    float4 ndc = mul(transform, float4(x, 1.0f));
    return ndc.xyz / ndc.w;
}
//================================================================
// remap_to_0_to_1
//================================================================
float2 remap_to_0_to_1(float2 v)
{
	return (v + 1) * 0.5;
}
float3 remap_to_0_to_1(float3 v)
{
	return (v + 1) * 0.5;
}

//================================================================
// remap_to_neg_1_to_1
//================================================================
float2 remap_to_neg_1_to_1(float2 v)
{
	return (v * 2) - 1;
}
float3 remap_to_neg_1_to_1(float3 v)
{
	return (v * 2) - 1;
}

// Reconstructs world-space position from depth buffer
//	uv		: screen space coordinate in [0, 1] range
//	z		: depth value at current pixel
//	InvVP	: Inverse of the View-Projection matrix that was used to generate the depth value
inline float3 reconstruct_position(in float2 uv, in float z, in float4x4 inverse_view_projection)
{
	float x = uv.x * 2 - 1;
	float y = (1 - uv.y) * 2 - 1;
	float4 position_s = float4(x, y, z, 1);
	float4 position_v = mul(inverse_view_projection, position_s);
	return position_v.xyz / position_v.w;
}