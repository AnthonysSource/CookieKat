#version 450

//--------------------------------------------------------------------

layout(set = 0, binding = 1) uniform samplerCube u_SkyBoxCubeMap;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 in_UV;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

//--------------------------------------------------------------------

void main()
{
    out_Color = texture(u_SkyBoxCubeMap, in_UV);
}