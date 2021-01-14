#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;
layout (location = 3) in vec2 inUV;

layout (set = 0, binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 view;
	mat4 model;
	vec4 lightPos;
} ubo;

layout (set = 1, binding = 0) uniform DEBUGINFO 
{
	vec3 debugColorOptions; 
	int debugIsOn;
} debugInfo;

layout (set = 2, binding = 0) uniform DYNAMIC_UBO //#dynamic
{
	vec4 Color;
} dynamicUbo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 outUV;
layout (location = 3) out vec3 outViewVec;
layout (location = 4) out vec3 outLightVec;
layout (location = 5) out vec3 outCameraVec;

void main() 
{
	if (debugInfo.debugColorOptions.x == 1)
	{
		outColor = inNormal;
	}
	else if (debugInfo.debugColorOptions.y == 1)
	{
		outColor = inColor.xyz;
	}	
	else if (debugInfo.debugColorOptions.z == 1)
	{
		outColor = dynamicUbo.Color.xyz;
	}
	else
	{
		outColor = vec3(0.85, 0.25, 0.35);//inColor.xyz;
	}

	outNormal = inNormal;
	gl_Position = ubo.projection * inverse(ubo.view) * ubo.model * vec4(inPos.xyz, 1.0);
	
	vec3 cameraPos = ubo.view[3].xyz;

	vec4 pos = ubo.model * vec4(inPos, 1.0);
	outNormal = mat3(ubo.model) * inNormal;
	vec3 lPos = mat3(ubo.model) * ubo.lightPos.xyz;
	outLightVec = lPos - pos.xyz;
	outViewVec = -pos.xyz;	

	outUV = inUV;
	outCameraVec = cameraPos - pos.xyz;
}