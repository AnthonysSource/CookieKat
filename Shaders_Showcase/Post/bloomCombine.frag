#version 460

// Uniforms
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_SceneColorSampler;
layout(set = 0, binding = 1) uniform sampler2D u_BloomSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 fragTexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

//--------------------------------------------------------------------

vec3 computeBloomMix()
{
    vec3 hdr = texture(u_SceneColorSampler, fragTexCoord).rgb;
    vec3 bloom = texture(u_BloomSampler, fragTexCoord).rgb;
    vec3 col = mix(hdr, bloom, vec3(0.04));
    return col;
}

void main()
{
    // vec4 sceneColor = texture(u_SceneColorSampler, fragTexCoord);
    // vec4 bloom = texture(u_BloomSampler, fragTexCoord);
    // outColor = vec4(sceneColor.rgb + bloom.rgb, sceneColor.a);
    outColor = vec4(computeBloomMix(), 1.0);
}