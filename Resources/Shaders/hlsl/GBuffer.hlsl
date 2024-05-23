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

VertexOutput VSMain(const GeoVertexInput input)
{
	VertexOutput vsOut;
	vsOut.Pos = float4(input.Pos, 1);
	vsOut.Colour = float4(input.Colour, 1.0);

/*
	float4x4 BoneTransform = IDENTITY_MATRIX; 
	BoneTransform += bpo_BoneMatrices[input.BoneIds[0]] * input.BoneWeights[0];
    BoneTransform += bpo_BoneMatrices[input.BoneIds[1]] * input.BoneWeights[1];
    BoneTransform += bpo_BoneMatrices[input.BoneIds[2]] * input.BoneWeights[2];
    BoneTransform += bpo_BoneMatrices[input.BoneIds[3]] * input.BoneWeights[3];

*/
	vsOut.WorldNormal[0] = input.BoneIds[0];
	vsOut.WorldNormal[1] = input.BoneIds[1];
	vsOut.WorldNormal[2] = input.BoneIds[2];
	vsOut.WorldNormal[3] = input.BoneIds[3];

	float4 totalPosition = vsOut.Pos;
	if (bpo_SkinnedMesh == 1)
	{
		totalPosition = float4(0,0,0,0);
		for(int i = 0 ; i < 4 ; ++i)
    	{
        	if(input.BoneIds[i] == -1)
			{
            	continue;
			}

        	if(input.BoneIds[i] >= 100) 
        	{
            	totalPosition = float4(input.Pos, 1.0f);
            	break;
        	}
        	float4 localPosition = mul(bpo_BoneMatrices[input.BoneIds[i]], float4(input.Pos, 1));
			totalPosition += localPosition * input.BoneWeights[i];
        	//float4 localNormal = mul(bpo_BoneMatrices[boneId], float4(input.Normal, 0));
   		}
	}
	//vsOut.Pos = mul(BoneTransform, float4(vsOut.Pos.xyz, 1));
	vsOut.Pos = totalPosition;
	vsOut.WorldPos = mul(bpo_Transform, vsOut.Pos);
	vsOut.Pos = mul(bf_Camera_Proj_View, vsOut.WorldPos);
	
	//vsOut.WorldNormal = normalize(mul(bpo_Transform, float4(input.Normal.xyz, 0.0)));
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
	if(bpo_Textures_Set[0] == 1)
	{
		Out.Colour = Diffuse_Texture.Sample(Reapt_Sampler, input.UV);
	}
	else
	{
		Out.Colour = input.Colour;
	}
	Out.World_Normal = float4(input.WorldNormal.xyz, 1.0);
    
	float2 position_ndc_current = (input.position_ss_current.xy / input.position_ss_current.w);
	float2 position_ndc_previous = (input.position_ss_previous.xy / input.position_ss_previous.w);

	position_ndc_current -= bf_TAA_Jitter_Current;
    position_ndc_previous -= bf_TAA_Jitter_Previous;

	Out.Velocity = ndc_to_uv(position_ndc_current) - ndc_to_uv(position_ndc_previous);

	return Out;
}