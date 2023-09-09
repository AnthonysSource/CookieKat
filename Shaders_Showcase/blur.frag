#version 460

//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_Source;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 fragTexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Blurred;

//--------------------------------------------------------------------

void main()
{
    vec2 texelSize = 1.0/ vec2(textureSize(u_Source, 0));
    float result = 0.0;
    for(int x = -2; x < 2; ++x){
        for(int y = -2; y < 2; ++y){
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(u_Source, fragTexCoord + offset).x;
        }
    }
    result /= 16.0;
    out_Blurred = vec4(vec3(result), 1.0);
}