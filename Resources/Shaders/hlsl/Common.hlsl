[[vk::binding(0, 0)]]
cbuffer Main_Camera : register(b0)
{
	float4x4 Main_Camera_Proj_View;
	float4x4 Main_Camera_Projection;
	float4x4 Main_Camera_View;
}

struct Shadow_Camera
{
	float4x4 Shadow_Camera_ProjView;
	float4x4 Shadow_Camera_Projection;
	float4x4 Shadow_Camera_View;
	float Shadow_CameraSplit_Depth;
	// Make sure this struct is on a 4 byte boundary.
	float pad1;
	float pad2;
	float pad3;
};
[[vk::binding(1, 0)]] 
cbuffer Cascade_Shadow_Camera : register(b1)
{
	Shadow_Camera shadow_camera[4]; 
}

static float4x4 biasMat = float4x4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

bool is_saturated(float value)  { return value == saturate(value); }
bool is_saturated(float2 value) { return is_saturated(value.x) && is_saturated(value.y); }

float2 ndc_to_uv(float2 x)
{
    return x * float2(0.5f, -0.5f) + 0.5f;
}
float2 ndc_to_uv(float3 x)
{
    return x.xy * float2(0.5f, 0.5f) + 0.5f;
}

float3 world_to_ndc(float3 x, float4x4 transform) // shadow mapping
{
    float4 ndc = mul(float4(x, 1.0f), transform);
    return ndc.xyz / ndc.w;
}