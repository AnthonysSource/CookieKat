#version 450

// Push Constants
//--------------------------------------------------------------------

layout(push_constant) uniform PushConstants{
    mat4 model;
} push;

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
layout(location = 2) out mat3 TBN;
layout(location = 5) out vec3 fragNormal;

//--------------------------------------------------------------------

void main() {
    fragPos = (push.model * vec4(inPosition, 1.0)).xyz;
    fragTexCoord = inTexCoord;
    fragNormal = mat3(transpose(inverse(push.model))) * inNormal;

    vec3 bitangent = cross(inNormal, inTangent);    
    vec3 T = normalize(vec3(push.model * vec4(inTangent, 0.0)));
    vec3 B = normalize(vec3(push.model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(push.model * vec4(inNormal, 0.0)));
    TBN = mat3(T, B, N);

    gl_Position = viewData.viewProj * push.model * vec4(inPosition, 1.0);
}