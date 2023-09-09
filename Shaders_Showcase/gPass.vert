#version 460

// SSBO
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform ViewData{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
} u_ViewData;

struct ObjectData{
	mat4 model;
    mat4 normalMat;
    vec3 albedo;
    float roughness;
    float metalic;
    float reflectance;
};

layout(std140, set = 0, binding = 1) readonly buffer ObjectBuffer{
    ObjectData objects[];
} u_ObjectBuffer;

// Vertex Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec3 in_Tangent;
layout(location = 3) in vec2 in_UV;

// Output
//--------------------------------------------------------------------

layout(location = 0) out vec3 out_Pos; // World Space
layout(location = 1) out vec2 out_UV;
layout(location = 2) out vec3 out_Normal;
layout(location = 3) out vec3 out_Tangent;
layout(location = 4) out mat4 out_Model;

layout(location = 8) out vec3 out_Albedo;
layout(location = 9) out float out_Roughness;
layout(location = 10) out float out_Metalic;
layout(location = 11) out float out_Reflectance;
layout(location = 12) out float out_ObjIdx;

//--------------------------------------------------------------------

void main() {
    mat4 model = u_ObjectBuffer.objects[gl_BaseInstance].model; 
    mat3 normalMat = mat3(u_ObjectBuffer.objects[gl_BaseInstance].normalMat);

    out_Model = model;
    out_Pos = (u_ViewData.view * model * vec4(in_Position, 1.0)).xyz;
    out_UV = in_UV;
    out_Normal = (u_ViewData.view * vec4(normalMat * in_Normal, 0.0)).xyz;
    out_Tangent = (u_ViewData.view * vec4(normalMat * in_Tangent, 0.0)).xyz;

    out_Albedo = u_ObjectBuffer.objects[gl_BaseInstance].albedo;
    out_Roughness = u_ObjectBuffer.objects[gl_BaseInstance].roughness;
    out_Metalic = u_ObjectBuffer.objects[gl_BaseInstance].metalic;
    out_Reflectance = u_ObjectBuffer.objects[gl_BaseInstance].reflectance;
    out_ObjIdx = gl_BaseInstance + 1;

    gl_Position = u_ViewData.viewProj * model * vec4(in_Position, 1.0);
}