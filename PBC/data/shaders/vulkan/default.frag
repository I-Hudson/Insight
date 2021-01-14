#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;
layout (location = 5) in vec3 inCameraVec;

layout (location = 0) out vec4 outFragColor;

layout (set = 0, binding = 1) uniform sampler2D texture_diffuse;

void main() 
{
	// Desaturate color
    vec3 color = inColor;//vec3(mix(inColor, vec3(dot(vec3(0.2126,0.7152,0.0722), inColor)), 0.65));	

	// High ambient colors because mesh materials are pretty dark
	vec3 ambient = color * vec3(1.0);
	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, normalize(inCameraVec)), 0.0) * color;
	outFragColor = vec4(texture(texture_diffuse, inUV).xyz, 1.0);//vec4(diffuse, 1.0);
	//vec3 specular = pow(max(dot(R, V), 0.0), 32.0) * vec3(0.35);
	//outFragColor = vec4(ambient + diffuse * 1.75 + specular, 1.0);		
}