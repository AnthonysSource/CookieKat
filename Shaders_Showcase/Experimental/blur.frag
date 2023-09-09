#version 460

//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_SSAOSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 fragTexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outSSAOBlurred;

//--------------------------------------------------------------------

void main()
{
    vec2 texelSize = 1.0/ vec2(textureSize(u_SSAOSampler, 0));
    float result = 0.0;
    for(int x = -2; x < 2; ++x){
        for(int y = -2; y < 2; ++y){
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_SSAOSampler, fragTexCoord + offset).x;
        }
    }
    outSSAOBlurred = vec4(result, 0.0, 0.0, 0.0) / 16.0;
}