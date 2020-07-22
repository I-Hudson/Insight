#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 2) in vec4 inNormal;
layout(location = 3) in vec2 InUV1;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 FragUV;

void main() {
    gl_Position = inPosition;
    fragColor = inColor.xyz;
    FragUV = InUV1;
}