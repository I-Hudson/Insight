#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outColor1;

void main() {
    outColor = vec4(fragColor, 1.0);
    outColor1 = vec4(1.0, 0.5, 0.25, 1.0);
}