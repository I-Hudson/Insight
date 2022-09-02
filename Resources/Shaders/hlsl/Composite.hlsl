#include "Common.hlsl"

// Define all our textures needed

[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
Texture2D<float4> GBuffer_Colour : register(t0);
[[vk::combinedImageSampler]][[vk::binding(0, 1)]]
SamplerState GBuffer_Colour_Sampler : register(s0);

[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
Texture2D<float4> GBuffer_World_Normal: register(t1);
[[vk::combinedImageSampler]][[vk::binding(1, 1)]]
SamplerState GBuffer_World_Normal_Sampler : register(s1);

[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
Texture2D<float4> GBuffer_World_Position : register(t2);
[[vk::combinedImageSampler]][[vk::binding(2, 1)]]
SamplerState GBuffer_World_Position_Sampler : register(s2);

[[vk::binding(3, 1)]]
Texture2DArray<float> Cascade_Shadow : register(t3);
[[vk::binding(4, 1)]]
SamplerState Cascade_Shadow_Sampler : register(s3);



struct PushConstant
{
	int Output_Texture;
};
[[vk::push_constant]] PushConstant push_constant;

struct VertexOutput
{
	float4 Position : SV_POSITION;
	float2 UV : TEXCOORD0;
};

VertexOutput VSMain(uint id : SV_VertexID)
{
	VertexOutput o;
	o.UV = float2((id << 1) & 2, id & 2);
	o.Position = float4(o.UV * float2(2, -2) + float2(-1, 1), 0, 1);
	return o;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
	float4 colour 			= GBuffer_Colour.Sample(GBuffer_Colour_Sampler, input.UV);
	float4 world_normal 	= GBuffer_World_Normal.Sample(GBuffer_World_Normal_Sampler, input.UV);
	float4 world_position 	= GBuffer_World_Position.Sample(GBuffer_World_Position_Sampler, input.UV);
	float3 world_pos = world_position.xyz / world_position.w; 
	
	float4 position_view_space = float4(world_to_ndc(world_pos, Main_Camera_View), 1.0);
	float4 shadow = 0;
	
    for (uint cascade = 0; cascade < 1; cascade++)
    {
		// Project into light space
		float4x4 shadow_space_matrix = shadow_camera[cascade].Shadow_Camera_ProjView;
		float3 shadow_pos_ndc = world_to_ndc(world_pos, shadow_space_matrix);
		float2 shadow_uv = ndc_to_uv(shadow_pos_ndc);

		float shadow_sample = Cascade_Shadow.Sample(Cascade_Shadow_Sampler, float3(shadow_uv.x, shadow_uv.y, cascade)).r;
		shadow = float4(shadow_sample, shadow_sample, shadow_sample, 1.0);
		shadow = float4(shadow_uv, 0.0, 1.0);

	}
	float4 result;

	if(push_constant.Output_Texture == 0)
	{
		result = colour;
	}
	else if(push_constant.Output_Texture == 1)
	{
		world_normal = (world_normal + 1) / 2; //Remap normal to 0-1.
		result = world_normal;
	}
	else if(push_constant.Output_Texture == 2)
	{
		result = world_position;
	}
	else if(push_constant.Output_Texture == 3)
	{
		result = shadow;
	}
	else if(push_constant.Output_Texture == 3)
	{
		result = float4(0, 0, position_view_space.z, 1);
	}
	return result;
}