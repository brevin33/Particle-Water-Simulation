#version 450

layout(binding = 0) uniform sampler2D inputColor;
layout(binding = 1) uniform sampler2D inputDepth;

layout (location = 0) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() 
{
    vec4 color = texture(inputColor,inUV);
    outColor = vec4(color.rgb, 1.0f);
}