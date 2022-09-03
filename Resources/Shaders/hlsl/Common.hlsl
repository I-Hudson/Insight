[[vk::binding(0, 0)]]
cbuffer Main_Camera : register(b0)
{
	float4x4 Main_Camera_Proj_View;
	float4x4 Main_Camera_Projection;
	float4x4 Main_Camera_View;
	float4x4 Main_Camera_Projection_View_Inverted;
}

struct Shadow_Camera
{
	float4x4 Shadow_Camera_ProjView;
	float4x4 Shadow_Camera_Projection;
	float4x4 Shadow_Camera_View;
	float3 Shadow_Light_Direction;
	float Shadow_CameraSplit_Depth;
	float2 Shadow_Resolution;
	float pad1;
	float pad2;
};
[[vk::binding(1, 0)]] 
cbuffer Cascade_Shadow_Camera : register(b1)
{
	Shadow_Camera shadow_cameras[4]; 
}

static float4x4 biasMat = float4x4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

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
    return x.xy * float2(0.5f, 0.5f) + 0.5f;
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