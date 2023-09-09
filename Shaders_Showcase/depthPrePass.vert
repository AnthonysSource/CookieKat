#version 460

//--------------------------------------------------------------------

struct ObjectData{
	mat4 model;
    mat4 normalMat;
    vec3 albedo;
    float roughness;
    float metalic;
};

layout(std140, set = 0, binding = 1) readonly buffer ObjectBuffer{
    ObjectData objects[];
} objectBuffer;


layout(set = 0, binding = 0) uniform ViewData{
    mat4 viewProj;
    mat4 view;
    mat4 proj;
    mat4 viewInv;
    mat4 projInv;
} u_ViewData;

// Vertex Input
//--------------------------------------------------------------------

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

//--------------------------------------------------------------------

void main() {
    mat4 model = objectBuffer.objects[gl_BaseInstance].model;
    gl_Position = u_ViewData.viewProj * model * vec4(inPosition, 1.0);
}