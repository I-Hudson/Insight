#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 FragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D offscreenTexture;

void main() 
{
    outColor = texture(offscreenTexture, FragUV);
}