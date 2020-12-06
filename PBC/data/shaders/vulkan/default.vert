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
	vec2 debugColorOptions; 
	int debugIsOn;
} debugInfo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outViewVec;
layout (location = 3) out vec3 outLightVec;
layout (location = 4) out vec3 outCameraVec;

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

	outCameraVec = cameraPos - pos.xyz;
}