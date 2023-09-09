#version 460

// Structs and Defines
//--------------------------------------------------------------------

#define PI 3.141592653589793

struct PointLightData{
    vec4 viewPos;
    vec4 radiance;
};

struct SHCoeffs9{
    vec4 value[9];
};

//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform LightData{
    vec4 size;
    PointLightData light[1000];
} u_Lights;

layout(set = 0, binding = 1) uniform sampler2D u_AlbedoSampler;
layout(set = 0, binding = 2) uniform sampler2D u_NormalSampler;
layout(set = 0, binding = 3) uniform sampler2D u_RMRSampler;
layout(set = 0, binding = 4) uniform sampler2D u_Positions;
layout(set = 0, binding = 5) uniform sampler2D u_SSAOSampler;

layout(set = 0, binding = 6) uniform EnvData{
    SHCoeffs9 env;
    SHCoeffs9 test[9];
} u_EnvData;

layout(set = 0, binding = 7) uniform ViewData{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
} u_ViewData;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

// PBR Functions
//--------------------------------------------------------------------

float D_GGX(vec3 N, vec3 H, float roughness)
{
    // We define alpha as the roughness^2
    float a = roughness * roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = NdotH2 * (a2 - 1.0) + 1.0;
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGXX(float cosTheta, float roughness)
{
    float r = roughness + 1.0;
    float k = (r*r) / 8.0; // Direct
    
    return cosTheta / ( cosTheta * (1 - k) + k );
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGXX(NdotV, roughness); // Geometry Obstruction
    float ggx2 = GeometrySchlickGXX(NdotL, roughness); // Geometry Shadowing
    return ggx1 * ggx2;
}

vec3 F_Schlick(float HdotV, vec3 F0)
{
    float p = clamp(1.0 - HdotV, 0.0, 1.0);
    float p5 = p * p * p * p * p;
    return F0 + (1.0 - F0) * p5;
}

vec3 F_SchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}   

float Fd_Lambert() {
    return 1.0 / PI;
}

float F_SchlickBurley(float u, float f0, float f90) {
    return f0 + (f90 - f0) * pow(1.0 - u, 5.0);
}

float Fd_Burley(float NdotV, float NdotL, float LdotH, float roughness) {
    float f90 = 0.5 + 2.0 * roughness * LdotH * LdotH;
    float lightScatter = F_SchlickBurley(NdotL, 1.0, f90).x;
    float viewScatter = F_SchlickBurley(NdotV, 1.0, f90).x;
    return lightScatter * viewScatter * (1.0 / PI);
}

//--------------------------------------------------------------------

 // Evaluates the irradiance perceived in the provided direction
 // Analytic method from http://www1.cs.columbia.edu/~ravir/papers/envmap/envmap.pdf eq. 13
 // Implementation from https://patapom.com/blog/SHPortal/#using-the-result-for-real-time-diffuse-irradiance-estimation
vec3 EvalSH_EnvMap(const vec4 shCoeffs[9], const in vec3 normal) {
    const float c1 = 0.42904276540489171563379376569857; // 4 * Â2.Y22 = 1/4 * sqrt(15.PI)
    const float c2 = 0.51166335397324424423977581244463; // 0.5 * Â1.Y10 = 1/2 * sqrt(PI/3)
    const float c3 = 0.24770795610037568833406429782001; // Â2.Y20 = 1/16 * sqrt(5.PI)
    const float c4 = 0.88622692545275801364908374167057; // Â0.Y00 = 1/2 * sqrt(PI)

    float x = normal.x;
    float y = normal.y;
    float z = normal.z;

    return max(vec3(0.0),
            (c1*(x*x - y*y)) * shCoeffs[8].xyz                       // c1.L22.(x²-y²)
            + (c3*(3.0*z*z - 1)) * shCoeffs[6].xyz                   // c3.L20.(3.z² - 1)
            + c4 * shCoeffs[0].xyz                                   // c4.L00 
            + 2.0*c1*(shCoeffs[4].xyz*x*y + shCoeffs[7].xyz*x*z + shCoeffs[5].xyz*y*z) // 2.c1.(L2-2.xy + L21.xz + L2-1.yz)
            + 2.0*c2*(shCoeffs[3].xyz*x + shCoeffs[1].xyz*y + shCoeffs[2].xyz*z) );    // 2.c2.(L11.x + L1-1.y + L10.z)
};

//--------------------------------------------------------------------

void main()
{
    // Sample textures
    vec3 albedo = texture(u_AlbedoSampler, in_UV).xyz;
    vec3 normal = texture(u_NormalSampler, in_UV).xyz;
    vec3 pos = texture(u_Positions, in_UV).xyz; // viewSpace
    vec3 rmr = texture(u_RMRSampler, in_UV).xyz;
    float roughness = max(0.08, rmr.x);
    float metalness = rmr.y;
    float reflectiveness = rmr.z;
    float ao = texture(u_SSAOSampler, in_UV).x;

    //--------------------------------------------------------------------

    vec3 N = normalize(normal);
    vec3 V = normalize(-pos);

    vec3 F0 = vec3(reflectiveness);
    // If the material is a metal we use the albedo
    F0 = mix(F0, albedo, metalness);
    vec3 correctedAlbedo = albedo * (1.0 - metalness);

    // Iterate over all of the lights and calculate their contribution
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < u_Lights.size.x; i++){
        vec3 lightPos = u_Lights.light[i].viewPos.xyz;
        float lightDist = length(lightPos - pos);
        vec3 L = normalize(lightPos - pos);
        vec3 H = normalize(V + L); // In theory this could blow up if V and L are perfectly co-linear

        float attenuation = min(1.0 / (lightDist * lightDist), 50000);
        vec3 Li = u_Lights.light[i].radiance.xyz * attenuation;

        // Specular term
        float D = D_GGX(N, H, roughness);
        float G = G_Smith(N, V, L, roughness);
        vec3 F = F_Schlick(max(dot(H, V), 0.0), F0);
        vec3 num = D * G * F;
        float denom = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
        vec3 Fr = num / denom;

        // Diffuse term
        vec3 Fd = correctedAlbedo * Fd_Burley(dot(N, V), dot(N, L), dot(L, H), roughness * roughness);

        // Conservation of energy
        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;

        float NdotL = max(dot(N, L), 0.0);
        Lo += (kD * Fd + Fr) * Li * NdotL;
    }

    //--------------------------------------------------------------------
    // Enviorement Contribution
    vec3 envMapDiffuseIrradiance = EvalSH_EnvMap(u_EnvData.env.value, (u_ViewData.viewInv * vec4(N, 0.0)).xyz) / PI;
    vec3 kS = F_SchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 kD = 1.0 - kS;
    vec3 Fd = envMapDiffuseIrradiance * albedo;
    // NOTE: This just a hack to not have completelly black env reflections because
    // we didn't implement the specular contribution of the env.lighting
    vec3 Fr = envMapDiffuseIrradiance * 1.25;
    vec3 ambient = (kD * Fd + kS * Fr) * ao;

    vec3 color = ambient + Lo;
    out_Color = vec4(color, 1.0);
}