#version 460

// SSBO
//--------------------------------------------------------------------

struct ObjectData{
	mat4 model;
    vec3 albedo;
    float roughness;
    float metalic;
};

layout(std140, set = 0, binding = 1) readonly buffer ObjectBuffer{
    ObjectData objects[];
} objectBuffer;

// Descriptors
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform ViewData{
    mat4 viewProj;
} viewData;

// Vertex Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

// Output
//--------------------------------------------------------------------

layout(location = 0) out vec3 fragPos; // World Space
layout(location = 1) out vec2 fragTexCoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out mat4 fragModel;

layout(location = 8) out vec3 albedo;
layout(location = 9) out float roughness;
layout(location = 10) out float metalic;
layout(location = 11) out int objectIdx;

//--------------------------------------------------------------------

void main() {
    fragModel = objectBuffer.objects[gl_BaseInstance].model;
    fragPos = (fragModel * vec4(inPosition, 1.0)).xyz;
    fragTexCoord = inTexCoord;
    fragNormal = inNormal;
    fragTangent = inTangent;
    objectIdx = gl_BaseInstance;

    albedo = objectBuffer.objects[gl_BaseInstance].albedo;
    roughness = objectBuffer.objects[gl_BaseInstance].roughness;
    metalic = objectBuffer.objects[gl_BaseInstance].metalic;

    gl_Position = viewData.viewProj * fragModel * vec4(inPosition, 1.0);
}