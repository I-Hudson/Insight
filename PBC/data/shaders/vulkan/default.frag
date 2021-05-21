#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec2 inUV;
layout (location = 4) in vec3 inViewVec;
layout (location = 5) in vec3 inLightVec;
layout (location = 6) in vec4 inShadowCoord;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outPos;


layout (set = 1, binding = 0) uniform sampler2D texture_diffuse;
layout (set = 1, binding = 1) uniform TextureLookup
{
	int diffuse;
}textureLookup;

layout (set = 0, binding = 2) uniform sampler2D texture_shadowpass;

float LinearizeDepth(float depth)
{
  float n = 0.1; // camera z near
  float f = 128; // camera z far
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

#define ambient 0.1
float textureProj(vec4 shadowCoord, vec2 off)
{
	float shadow = 1.0;
	if ( shadowCoord.z > -1.0 && shadowCoord.z < 1.0 ) 
	{
		float dist = texture(texture_shadowpass, shadowCoord.xy + off ).x;
		if (dist < shadowCoord.z) 
		{
			shadow = ambient;
		}
	}
	return shadow;
}

float filterPCF(vec4 sc)
{
	ivec2 texDim = textureSize(texture_shadowpass, 0);
	float scale = 1.5;
	float dx = scale * 1.0 / float(texDim.x);
	float dy = scale * 1.0 / float(texDim.y);

	float shadowFactor = 0.0;
	int count = 0;
	int range = 1;
	
	for (int x = -range; x <= range; x++)
	{
		for (int y = -range; y <= range; y++)
		{
			shadowFactor += textureProj(sc, vec2(dx*x, dy*y));
			count++;
		}
	
	}
	return shadowFactor / count;
}

void main() 
{
	//float shadow = (enablePCF == 1) ? filterPCF(inShadowCoord / inShadowCoord.w) : textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));
	//float shadow = filterPCF(inShadowCoord / inShadowCoord.w);
	float shadow = textureProj(inShadowCoord / inShadowCoord.w, vec2(0.0));

	if(textureLookup.diffuse == 1)
	{
		outColor = vec4(texture(texture_diffuse, inUV).xyz, 1.0);
	}
	else
	{
		outColor = vec4(1, 1, 1, 1);
	}

	outNormal = vec4(inNormal, 1);
	outPos = vec4(inColor, 1);

	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = normalize(-reflect(L, N));
	vec3 diffuse = max(dot(N, L), ambient) * inColor;

	outColor = vec4(diffuse * shadow, 1.0);
}