// Set 0 - per frame
// Set 1 - Per object

#include "Defines.hlsl"

SamplerComparisonState Shadow_Sampler : register(s0, SamplerSpace);
SamplerState Reapt_Sampler : register(s1, SamplerSpace);
SamplerState Clamp_Sampler : register(s2, SamplerSpace);
SamplerState MirrorRepeat_Sampler : register(s3, SamplerSpace);
SamplerState ClampToBoarder_Sampler : register(s4, SamplerSpace);
