struct VertexInput
{
    float2 aPos : POSITION;
    float2 aUV : TEXCOORD0;
    float4 aColor : COLOR0;
};

struct VertexOutput
{
	float4 Position : SV_POSITION;
    float4 Color : COLOR0;
    float2 UV : TEXCOORD0;
};

#ifdef VULKAN
cbuffer UBO : register(b0, space0)
{
    float2 uScale;
    float2 uTranslate;
};
#elif DX12
 cbuffer UBO : register(b0)
 {
     float4x4 ProjectionMatrix;
 }
 #endif

Texture2D<float4> sTexture : register(t0, space1);
SamplerState sSampler : register(s0, space2);

VertexOutput VSMain(const VertexInput input)
{
	VertexOutput Out;
    Out.Color = input.aColor;
    Out.UV = input.aUV;
    #ifdef VULKAN
    Out.Position = float4(input.aPos * uScale + uTranslate, 0, 1);
    #elif DX12
    Out.Position = mul(ProjectionMatrix, float4(input.aPos.xy, 0.f, 1.f));
    #endif
	return Out;
}

float4 PSMain(VertexOutput input) : SV_TARGET
{	
    float4 r = float4(0,0,0,1);
    r = input.Color * sTexture.Sample(sSampler, input.UV);
    return r;
}