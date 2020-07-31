#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 FragUV;

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform sampler2D offscreenTexture;
//layout(set = 0, binding = 1) uniform sampler2D uiTexture;

void main() 
{
    if (texture(offscreenTexture, FragUV).a > 0)
    {
        outColor = vec4(1.0, 0.0, 0.0, 1.0);
    }
    else
    {
        outColor = vec4(0.0, 0.0, 0.0, 1.0);
    }

    //outColor = texture(offscreenTexture, FragUV);// + texture(uiTexture, FragUV);
}