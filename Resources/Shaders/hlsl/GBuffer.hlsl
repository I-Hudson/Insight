#include "Common.hlsl"

struct VertexOutput
{
	float4 Pos 						: SV_POSITION;
	float4 Colour 					: COLOR0;
	float4 WorldPos 				: POSITION1;
	float4 WorldNormal 				: NORMAL0;

	float4 position_ss_current  	: SCREEN_POS;
    float4 position_ss_previous 	: SCREEN_POS_PREVIOUS;

	float4 TexturesSet				: POSITION2;
	float2 UV 						: TEXCOORD0;
};

VertexOutput VSMain(const GeoVertexInput input, uint vertexID : SV_VertexID)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos, 1);
	vsOut.Colour = GetVertexColour(input);
	vsOut.WorldNormal = GetVertexNormal(input);

	[branch]
	if (bpo_SkinnedMesh && !GPUSkinningEnabled())
	{
		SkinMesh(input, vsOut.Pos, vsOut.WorldNormal);
	}

	vsOut.WorldPos = mul(bpo_Transform, vsOut.Pos);
	vsOut.Pos = mul(bf_Camera_Proj_View, vsOut.WorldPos);
	
	vsOut.WorldNormal = normalize(mul(bpo_Transform, float4(vsOut.WorldNormal.xyz, 0.0)));
	vsOut.UV = GetUVsForAPI(GetVertexUVs(input));

	vsOut.position_ss_current = mul(bf_Camera_View, vsOut.WorldPos);

	float4 world_pos_previous = mul(bpo_Previous_Transform, float4(input.Pos.xyz, 1));
	vsOut.position_ss_previous = mul(bf_Camera_View, world_pos_previous);
	
	return vsOut;
}

struct PixelOutput
{
	float4 Colour 			: SV_TARGET0;
	float4 World_Normal 	: SV_TARGET1;
	float2 Velocity 		: SV_TARGET2;
};

[earlydepthstencil]
PixelOutput PSMain(VertexOutput input)
{	
	PixelOutput Out;
	Out.World_Normal = float4(input.WorldNormal.xyz, 1.0);
	if(bpo_Textures_Set[0] == 1)
	{
		Out.Colour = Diffuse_Texture.Sample(Reapt_Sampler, input.UV);
	}
	else
	{
		Out.Colour = input.Colour;
	}
    
	float2 position_ndc_current = (input.position_ss_current.xy / input.position_ss_current.w);
	float2 position_ndc_previous = (input.position_ss_previous.xy / input.position_ss_previous.w);

	position_ndc_current -= bf_TAA_Jitter_Current;
    position_ndc_previous -= bf_TAA_Jitter_Previous;

	Out.Velocity = ndc_to_uv(position_ndc_current) - ndc_to_uv(position_ndc_previous);

	return Out;
}