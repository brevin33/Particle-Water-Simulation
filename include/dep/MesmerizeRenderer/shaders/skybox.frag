#version 450
layout(location = 0) out vec4 FragColor;

layout(location = 0) in vec3 TexCoords;

layout(binding = 1) uniform samplerCube skybox;

void main()
{    
    FragColor = texture(skybox, TexCoords);
}