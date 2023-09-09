#version 460

//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_DepthSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 fragTexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outDepth;

//--------------------------------------------------------------------

float LinearizeDepth(float d,float zNear,float zFar)
{
    return zNear * zFar / (zFar + d * (zNear - zFar));
}

void main()
{
    vec4 depthValue = texture(u_DepthSampler, fragTexCoord);
    depthValue.z = LinearizeDepth(depthValue.z, 0.005, 1000.0);
    outDepth = depthValue;
}