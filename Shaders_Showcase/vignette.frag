#version 450
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_Image;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

//--------------------------------------------------------------------

void main()
{
    vec2 uv = in_UV;
    uv = uv * (1.0 - uv.xy);
    float vignette = uv.x * uv.y * 15.0;
    vignette = pow(vignette, 0.1);
    vec3 finalColor = texture(u_Image, in_UV).rgb * vignette;
    outColor = vec4(finalColor, 1.0);
}
