#version 450

// Camera
layout(binding = 0) uniform c{ mat4 view; mat4 projection;} camera;


// Vertex Data
layout(location = 0) in vec3 inPosition;


// Instance Data
layout(location = 1) in vec3 inModel;
layout(location = 2) in vec3 inVel;
layout(location = 3) in vec3 inAcc;


void main() {
    gl_Position = camera.projection * camera.view * vec4(inModel + inPosition, 1.0);
}