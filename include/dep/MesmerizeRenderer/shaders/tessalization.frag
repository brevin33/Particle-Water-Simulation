#version 450

layout (location = 0) in float Height;

layout (location = 0) out vec4 FragColor;

void main()
{
    float h = (Height + 16)/64.0f;
    FragColor = vec4(h, h, h, 1.0);
}