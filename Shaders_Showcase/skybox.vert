#version 450

layout(set = 0, binding = 0) uniform ViewData{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
} u_ViewData;

// Vertex Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 in_Position;

// Output
//--------------------------------------------------------------------

layout(location = 0) out vec3 out_UV;

//--------------------------------------------------------------------

void main() {
    out_UV = in_Position;
    mat4 view = mat4(mat3(u_ViewData.view));
    vec4 position = u_ViewData.proj * view * vec4(in_Position, 1.0);
    gl_Position = position.xyww; // Trick to have maximum depth value
}