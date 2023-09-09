#version 460

// Uniforms
//--------------------------------------------------------------------

#define KERNEL_SIZE 8
#define NOISE_SIZE 16
#define NOISE_WIDTH 4
#define SSAO_RADIUS 0.1
#define SSAO_BIAS 0.01

layout(set = 0, binding = 0) uniform sampler2D u_PositionTex;
layout(set = 0, binding = 1) uniform sampler2D u_NormalTex;
layout(set = 0, binding = 2) uniform sampler2D u_DepthTex;

layout(set = 0, binding = 3) uniform ViewData{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
} u_ViewData;

layout(set = 0, binding = 4) uniform SamplingData{
    vec4 m_Samples[KERNEL_SIZE]; // Defined in Tangent Space
    vec4 m_Noise[NOISE_SIZE];
} u_SamplingData;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_AO;

//--------------------------------------------------------------------

vec4 ViewPosFromDepth(float depth, vec2 uv) {
    vec4 clipSpacePos = vec4(uv * 2.0 - 1.0, depth, 1.0);
    vec4 viewSpacePos = u_ViewData.projInv * clipSpacePos;
    viewSpacePos /= viewSpacePos.w;
    return vec4(viewSpacePos.xyz, 1.0);
}

void main()
{
    // Sample Data for GBuffer textures
    //vec4 position = u_ViewData.view * texture(u_PositionTex, in_UV);
    vec4 position = ViewPosFromDepth(texture(u_DepthTex, in_UV).x, in_UV);
    vec3 normal = normalize(texture(u_NormalTex, in_UV).xyz);

    // Get noise for this fragment
    // TODO: A small repeating texture might be better
    int noiseX = int(in_UV.x - 0.5) % NOISE_WIDTH;
    int noiseY = int(in_UV.y - 0.5) % NOISE_WIDTH;
    vec3 noiseVec = u_SamplingData.m_Noise[noiseX + (noiseY * NOISE_WIDTH)].xyz;

    // Setup TBN
    vec3 tangent = normalize(noiseVec - normal * dot(noiseVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0;
    for (int i = 0; i < KERNEL_SIZE; ++i) {
        // Convert sample from tanget space to view space
        vec3 samplePos = TBN * u_SamplingData.m_Samples[i].xyz;
        samplePos = samplePos * SSAO_RADIUS + position.xyz;

        vec4 sampleUV = u_ViewData.proj * vec4(samplePos, 1.0); // Convert from view to clip space
        sampleUV.xy /= sampleUV.w; // Get xy coords in range [-1,1][-1,1] with perspective division
        sampleUV.xy = sampleUV.xy * 0.5 + 0.5; // Convert to range [0,1][0,1] to sample texture

        // Get the sample z position in view space
        float sampleDepth = ViewPosFromDepth(texture(u_DepthTex, sampleUV.xy).x, sampleUV.xy).z;
        // Check that sampled position is within the sampling semisphere radius
        float rangeCheck = smoothstep( 0.0, 1.0, SSAO_RADIUS/abs(position.z - sampleDepth));
        occlusion += (sampleDepth >= samplePos.z + SSAO_BIAS ? 1.0 : 0.0) * rangeCheck;    
    }
    occlusion = max(1.0 - (occlusion / KERNEL_SIZE), 0.0);

    out_AO = vec4(vec3(occlusion * occlusion * occlusion), 1.0);
}