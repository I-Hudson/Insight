// Set 0 - per frame
// Set 1 - Per object

[[vk::binding(6, 0)]]
Texture2DArray<float> Cascade_Shadow : register(t0);

[[vk::binding(7, 0)]]
Texture2D<float4> GBuffer_Colour : register(t1);

[[vk::binding(8, 0)]]
Texture2D<float4> GBuffer_Normal : register(t2);

[[vk::binding(9, 0)]]
Texture2D<float4> GBuffer_Shadow : register(t3);

