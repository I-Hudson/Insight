#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 fColor;

layout(set = 0, binding =  0) uniform sampler2D sTexture;

layout(location = 0) in vec4 inColour;
layout(location = 1) in vec2 inUV;

void main()
{
    fColor = vec4(1.0, 0.0, 0.0, 1.0); //inColour;// * texture(sTexture, inUV.st);
}