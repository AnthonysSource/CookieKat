#version 460

// Uniforms
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_Bloom;
layout(set = 0, binding = 1) uniform sampler2D u_BaseColor;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_TexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

//--------------------------------------------------------------------

void main()
{
    float filterRadius = 0.0020;
    float x = filterRadius;
    float y = filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    vec3 a = texture(u_Bloom, vec2(in_TexCoord.x - x, in_TexCoord.y + y)).rgb;
    vec3 b = texture(u_Bloom, vec2(in_TexCoord.x,     in_TexCoord.y + y)).rgb;
    vec3 c = texture(u_Bloom, vec2(in_TexCoord.x + x, in_TexCoord.y + y)).rgb;

    vec3 d = texture(u_Bloom, vec2(in_TexCoord.x - x, in_TexCoord.y)).rgb;
    vec3 e = texture(u_Bloom, vec2(in_TexCoord.x,     in_TexCoord.y)).rgb;
    vec3 f = texture(u_Bloom, vec2(in_TexCoord.x + x, in_TexCoord.y)).rgb;

    vec3 g = texture(u_Bloom, vec2(in_TexCoord.x - x, in_TexCoord.y - y)).rgb;
    vec3 h = texture(u_Bloom, vec2(in_TexCoord.x,     in_TexCoord.y - y)).rgb;
    vec3 i = texture(u_Bloom, vec2(in_TexCoord.x + x, in_TexCoord.y - y)).rgb;

    // Apply weighted distribution using a 3x3 tent filter:
    vec3 upsample = e*4.0;
    upsample += (b+d+f+h)*2.0;
    upsample += (a+c+g+i);
    upsample *= 1.0 / 16.0;
    out_Color = vec4(texture(u_BaseColor, in_TexCoord).xyz + upsample, 1.0);
}