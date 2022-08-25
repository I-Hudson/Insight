struct VertexInput
{
	[[vk::location(0)]] float2 aPos : POSITION;
	[[vk::location(1)]] float2 aUV : NORMAL0;
	[[vk::location(2)]] float4 aColor : COLOR0;
};

struct VertexOutput
{
	float4 Pos : SV_POSITION;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

struct PushConsts
{
    float2 uScale;
    float2 uTranslate;
};
[[vk::push_constant]] PushConsts pushConstants;

[[vk::combinedImageSampler]][[vk::binding(0, 0)]]
Texture2D<float4> sTexture : register(t0);
[[vk::combinedImageSampler]][[vk::binding(0, 0)]]
SamplerState sSampler : register(s0);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput Out;
    Out.Color = input.aColor;
    Out.UV = input.aUV;
    Out.Pos = float4(input.aPos * pushConstants.uScale + pushConstants.uTranslate, 0, 1);
	return Out;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
    float4 r = float4(0,0,0,1);
    r = input.Color * sTexture.Sample(sSampler, input.UV);
    return r;
}