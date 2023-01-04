struct VertexInput
{
    #ifdef VULKAN
	[[vk::location(0)]] 
    #endif
    float2 aPos : POSITION;
	#ifdef VULKAN
    [[vk::location(1)]] 
    #endif
    float2 aUV : NORMAL0;
	#ifdef VULKAN
    [[vk::location(2)]] 
    #endif
    float4 aColor : COLOR0;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

#ifdef VULKAN
struct PushConsts
{
    float2 uScale;
    float2 uTranslate;
};
[[vk::push_constant]]
PushConsts pushConstants;
#elif DX12
cbuffer UBO : register(b0)
{
    float4x4 ProjectionMatrix;
}
#endif
#ifdef VULKAN
[[vk::binding(0, 0)]]
#endif
Texture2D<float4> sTexture : register(t0);
#ifdef VULKAN
[[vk::binding(1, 0)]]
#endif
SamplerState sSampler : register(s0);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput Out;
    Out.Color = input.aColor;
    Out.UV = input.aUV;
    #ifdef VULKAN
    Out.Pos = float4(input.aPos * pushConstants.uScale + pushConstants.uTranslate, 0, 1);
    #elif DX12
    Out.Pos = mul(ProjectionMatrix, float4(input.aPos.xy, 0.f, 1.f));
    #endif
	return Out;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
    float4 r = float4(0,0,0,1);
    r = input.Color * sTexture.Sample(sSampler, input.UV);
    return r;
}