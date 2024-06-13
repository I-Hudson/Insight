cbuffer VertexShaderConstantBuffer : register(b0)
{
    float4x4 ProjectionView;
    float4x4 Projection;		// projection matrix
    float4x4 View;			    // view matrix
	float4x4 LightView;		    // view matrix of the light
	float4x4 LightProjection;	// projection matrix of the light
};

struct VertexInput
{
	float4 Pos 		: POSITION;
	float4 Colour 	: COLOR0;
	float2 Tex 		: TEXCOORD0;
};

struct VertexOutput
{
	float4 Pos		: SV_POSITION;
	float4 Colour	: COLOR0;
	float2 Tex 		: TEXCOORD0;
};