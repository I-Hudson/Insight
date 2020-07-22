#version 330 core
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inNormal;
layout(location = 3) in vec2 InUV1;

layout(location = 0) out vec3 fragColor;

uniform MVPUniformBuffer
{
    mat4 u_model;
    mat4 u_view;
    mat4 u_proj;
} mvp;


void main() 
{
    gl_Position = mvp.u_proj * mvp.u_view * mvp.u_model * inPosition;
    fragColor = inColor.xyz;
}