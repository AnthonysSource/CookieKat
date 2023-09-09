#version 460

// Uniforms
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_SceneColorSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_TexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

//--------------------------------------------------------------------

void main()
{
    vec4 o = texture(u_SceneColorSampler, in_TexCoord);
    out_Color = o;
}