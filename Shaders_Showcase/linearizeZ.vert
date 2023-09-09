#version 450

// Vertex Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

// Output
//--------------------------------------------------------------------

layout(location = 0) out vec2 fragTexCoord;

//--------------------------------------------------------------------

void main() {
    fragTexCoord = inTexCoord;
    gl_Position = vec4(inPosition, 1.0);
}