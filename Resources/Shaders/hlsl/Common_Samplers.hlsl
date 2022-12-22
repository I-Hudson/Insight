// Set 0 - per frame
// Set 1 - Per object

[[vk::binding(4, 0)]]
SamplerComparisonState Shadow_Sampler : register(s0);
[[vk::binding(5, 0)]]
SamplerState Reapt_Sampler : register(s1);
[[vk::binding(6, 0)]]
SamplerState Clamp_Sampler : register(s2);
[[vk::binding(7, 0)]]
SamplerState MirrorRepeat_Sampler : register(s3);