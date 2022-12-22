// Set 0 - per frame
// Set 1 - Per object

[[vk::binding(9, 0)]]
Texture2DArray<float> Cascade_Shadow : register(t0);

[[vk::binding(1, 1)]]
Texture2D<float4> DiffuseTexture : register(t1);

[[vk::binding(2, 1)]]
Texture2D<float4> NormalTexture : register(t2);

[[vk::binding(2, 1)]]
Texture2D<float4> SpecularTexture : register(t2);

[[vk::binding(3, 1)]]
Texture2D<float4> ShadowTexture : register(t3);

