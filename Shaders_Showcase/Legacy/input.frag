#version 450

// Structs and Defines
//--------------------------------------------------------------------

#define PI 3.141592653589793

struct PointLightData{
    vec3 pos;
    vec3 color;
};

// View Uniforms
//--------------------------------------------------------------------

layout(set = 0, binding = 1) uniform Cam{
    vec3 pos;
} u_Cam;

layout(set = 0, binding = 2) uniform LightData{
    PointLightData light;
} u_Light;

// Per Object Uniforms
//--------------------------------------------------------------------

layout(set = 1, binding = 0) uniform sampler2D u_AlbedoSampler;
layout(set = 1, binding = 1) uniform sampler2D u_NormalSampler;
layout(set = 1, binding = 2) uniform sampler2D u_RoughnessSampler;
layout(set = 1, binding = 3) uniform sampler2D u_MetalicSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec2 fragTexCoord;
layout(location = 2) in mat3 TBN;
layout(location = 5) in vec3 fragNormal;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

// PBR Functions
//--------------------------------------------------------------------

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    // Squaring the roughness to 
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGXX(float NdotV, float roughness)
{
    float r = roughness + 1.0;
    float k = (r*r) / 8.0; // Direct
    
    return NdotV / ( NdotV * (1 - k) + k );
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGXX(NdotV, roughness); // Geometry Obstruction
    float ggx2 = GeometrySchlickGXX(NdotL, roughness); // Geometry Shadowing
    return ggx1 * ggx2;
}

// CosTheta is NdotH or NdotV
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    float p = clamp(1.0 - cosTheta, 0.0, 1.0);
    float p5 = p * p * p * p * p;
    return F0 + (1.0 - F0) * p5;
}

void main()
{
    // Sample textures
    vec3 albedo = texture(u_AlbedoSampler, fragTexCoord).xyz;
    vec3 normalSampled = texture(u_NormalSampler, fragTexCoord).xyz;
    float roughness = texture(u_RoughnessSampler, fragTexCoord).x;
    float metalness = texture(u_MetalicSampler, fragTexCoord).x;

    //--------------------------------------------------------------------

    vec3 N = normalize(TBN * (normalSampled * 2.0 - 1.0)); // Convert sampled normal to world space
    vec3 V = normalize(u_Cam.pos - fragPos);

    // Blinn-Phong
    //--------------------------------------------------------------------

    // float lightDist = length(u_Light.light.pos - fragPos);
    // vec3 L = normalize(u_Light.light.pos - fragPos);
    // vec3 H = normalize(L + V);

    // // Ambient
    // vec3 ambient = albedo * 0.01;
    // vec3 lighting = ambient;

    // vec3 lightColor = u_Light.light.color * 0.01;

    // // Diffuse
    // vec3 diffuse = max(dot(N, L), 0.0) * albedo * lightColor;
    // lighting += diffuse;
    
    // // Specular
    // vec3 specular = pow(max(dot(N, H), 0.0), 32) * metalness * lightColor;
    // lighting += specular;

    // // Attenuation
    // float attenuation = 1.0 / (1.0 + 0.5 * lightDist + 0.1 * lightDist * lightDist);
    // lighting *= attenuation;

    // // Tone-Mapping with Reinhard operator
    // vec3 finalColor = lighting / (lighting + vec3(1.0));
    // outColor = vec4(finalColor, 1.0);

    // PBR
    //--------------------------------------------------------------------

    vec3 F0 = vec3(0.04); // Aprox. for dielectric materials

    // If the material is a metal we use the albedo
    F0 = mix(F0, albedo, metalness);

    vec3 Lo = vec3(0.0);
    
    // FOR EACH LIGHT
    float lightDist = length(u_Light.light.pos - fragPos);
    vec3 L = normalize(u_Light.light.pos - fragPos);
    vec3 H = normalize(V + L);

    float attenuation = 1.0 / (lightDist * lightDist);
    vec3 radiance = u_Light.light.color * attenuation; 
    
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);

    vec3 num = NDF * G * F;
    float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = num / denom;

    // Conservation of energy
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    // Remove refractions in the case of metalic materials
    kD *= 1.0 - metalness;

    float NdotL = max(dot(N, L), 0.0);
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;

    // Ambient
    vec3 ambient = albedo * vec3(0.01);
    vec3 color = ambient + Lo;

    // Reinhard Tone-Mapping 
    color = color / (color + vec3(1.0));

    outColor = vec4(color, 1.0);
}