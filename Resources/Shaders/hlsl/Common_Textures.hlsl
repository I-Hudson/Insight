// Set 0 - per frame
// Set 1 - Per object

#include "Defines.hlsl"

Texture2DArray<float> Cascade_Shadow : register(t0, PassSpace);
Texture2D<float4> Depth_Texture : register(t1, PassSpace);

Texture2D<float4> Diffuse_Texture : register(t0, PerObjectMaterial);
Texture2D<float4> Normal_Texture : register(t1, PerObjectMaterial);
Texture2D<float4> Specular_Texture : register(t2, PerObjectMaterial);


