#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inNormal;
layout(location = 3) in vec2 InUV1;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform MVPUniformBuffer
{
    mat4 u_model;
    mat4 u_view;
    mat4 u_proj;
} mvp;

const mat4 tempView = 
{
    vec4(1.0, 0.0, 0.0, 0.0),
    vec4(0.0, 1.0, 0.0, 0.0),
    vec4(0.0, 0.0, 1.0, 0.0),
    vec4(0.0, 0.0, 0.0, 1.0)
};

void main() {
    gl_Position = mvp.u_proj * mvp.u_view * mvp.u_model * inPosition;
    fragColor = vec3(1,1,1);
}