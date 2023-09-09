#version 460

//--------------------------------------------------------------------

layout(set = 1, binding = 0) uniform sampler2D u_AlbedoSampler;
layout(set = 1, binding = 1) uniform sampler2D u_NormalSampler;
layout(set = 1, binding = 2) uniform sampler2D u_RoughnessSampler;
layout(set = 1, binding = 3) uniform sampler2D u_MetalicSampler;

//--------------------------------------------------------------------

layout(location = 0) in vec3 in_Pos;
layout(location = 1) in vec2 in_UV;
layout(location = 2) in vec3 in_Normal;
layout(location = 3) in vec3 in_Tangent;
layout(location = 4) flat in mat4 in_Model;

layout(location = 8) flat in vec3 in_Albedo;
layout(location = 9) flat in float in_Roughness;
layout(location = 10) flat in float in_Metallic;
layout(location = 11) flat in float in_Reflectance;
layout(location = 12) flat in float in_ObjIdx;

//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Albedo;
layout(location = 1) out vec4 out_Position;
layout(location = 2) out vec4 out_Normals;
layout(location = 3) out vec4 out_RMR;
layout(location = 4) out vec4 out_ObjIdx;

void main()
{
    // Sample textures
    vec3 albedo = texture(u_AlbedoSampler, in_UV).xyz * in_Albedo;
    vec3 normalSampled = texture(u_NormalSampler, in_UV).xyz;
    float roughness = texture(u_RoughnessSampler, in_UV).x * in_Roughness;
    float metalness = texture(u_MetalicSampler, in_UV).x * in_Metallic;
    float reflectance = in_Reflectance;

    vec3 bitangent = cross(in_Normal, in_Tangent);    
    vec3 T = normalize(in_Tangent);
    vec3 B = normalize(bitangent);
    vec3 N = normalize(in_Normal);
    mat3 TBN = mat3(T, B, N);
    vec3 normal = normalize(TBN * (normalSampled * 2.0 - 1.0)); // Convert sampled normal to view space

    out_Albedo = vec4(albedo, 1.0);
    out_Position = vec4(in_Pos, 1.0);
    out_Normals =  vec4(normal, 1.0); 
    out_RMR = vec4(roughness, metalness, reflectance, 1.0);
    out_ObjIdx = vec4(vec3(in_ObjIdx), 1.0);
}