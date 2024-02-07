#version 450

layout(binding = 0) uniform c{ mat4 view; mat4 projection;} camera;

layout(location = 0) in vec3 aPos;

layout(location = 0) out vec3 TexCoords;


void main()
{
    TexCoords = aPos;
    vec4 pos = camera.projection * mat4(mat3(camera.view)) * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}  