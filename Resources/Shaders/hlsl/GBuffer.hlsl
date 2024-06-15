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

#define IDENTITY_MATRIX float4x4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1)
#define ZERO_MATRIX float4x4(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0)

float4x4 SkinnedBoneMatrix(const in GeoVertexInput input)
{
	float4x4 BoneTransform = ZERO_MATRIX; 
	BoneTransform += bpo_BoneMatrices[GetVertexBondId(input.BoneIds, 0)] * GetVertexBoneWeight(input.BoneWeights, 0);
    BoneTransform += bpo_BoneMatrices[GetVertexBondId(input.BoneIds, 1)] * GetVertexBoneWeight(input.BoneWeights, 1);
    BoneTransform += bpo_BoneMatrices[GetVertexBondId(input.BoneIds, 2)] * GetVertexBoneWeight(input.BoneWeights, 2);
    BoneTransform += bpo_BoneMatrices[GetVertexBondId(input.BoneIds, 3)] * GetVertexBoneWeight(input.BoneWeights, 3);
	return BoneTransform;
}

VertexOutput VSMain(const GeoVertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos, 1);
	vsOut.Colour = GetVertexColour(input);
	vsOut.WorldNormal = GetVertexNormal(input);

	if (bpo_SkinnedMesh == 1)
	{
		const float4x4 BoneTransform = SkinnedBoneMatrix(input);
		
		vsOut.Pos = mul(BoneTransform, float4(vsOut.Pos.xyz, 1));
		vsOut.WorldNormal = mul(BoneTransform, float4(vsOut.WorldNormal.xyz, 1));
	}
	vsOut.WorldPos = mul(bpo_Transform, vsOut.Pos);
	vsOut.Pos = mul(bf_Camera_Proj_View, vsOut.WorldPos);
	
	vsOut.WorldNormal = normalize(mul(bpo_Transform, float4(vsOut.WorldNormal.xyz, 0.0)));
	vsOut.UV = GetUVsForAPI(input.UV);

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