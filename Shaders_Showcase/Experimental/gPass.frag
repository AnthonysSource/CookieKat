#version 460

//--------------------------------------------------------------------

layout(set = 1, binding = 0) uniform sampler2D u_AlbedoSampler;
layout(set = 1, binding = 1) uniform sampler2D u_NormalSampler;
layout(set = 1, binding = 2) uniform sampler2D u_RoughnessSampler;
layout(set = 1, binding = 3) uniform sampler2D u_MetalicSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in mat4 fragModel;

layout(location = 8) flat in vec3 albedo;
layout(location = 9) flat in float roughness;
layout(location = 10) flat in float metalic;
layout(location = 11) flat in int objectIdx;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outAlbedo;
layout(location = 1) out vec4 outPosition;
layout(location = 2) out vec4 outNormals;
layout(location = 3) out vec4 outRoughnessMetalness;
layout(location = 4) out vec4 outObjectIdx;

void main()
{
    // Sample textures
    vec3 albedo = texture(u_AlbedoSampler, fragTexCoord).xyz * albedo;
    vec3 normalSampled = texture(u_NormalSampler, fragTexCoord).xyz;
    float roughness = texture(u_RoughnessSampler, fragTexCoord).x * roughness;
    float metalness = texture(u_MetalicSampler, fragTexCoord).x * metalic;

    vec3 bitangent = cross(fragNormal, fragTangent);    
    vec3 T = normalize(vec3(fragModel * vec4(fragTangent, 0.0)));
    vec3 B = normalize(vec3(fragModel * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(fragModel * vec4(fragNormal, 0.0)));
    mat3 TBN = mat3(T, B, N);
    vec3 normal = normalize(TBN * (normalSampled * 2.0 - 1.0)); // Convert sampled normal to world space

    outAlbedo = vec4(albedo, 1.0);
    outPosition = vec4(fragPos, 1.0);
    outNormals = vec4(normal, 1.0); 
    outRoughnessMetalness = vec4(roughness, metalness, 0.0, 1.0);
    outObjectIdx = vec4(objectIdx, 0.0, 0.0, 1.0);
}