#version 450
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_SceneColor;
layout(set = 0, binding = 1) uniform sampler2D u_LastSceneColor;

layout(set = 0, binding = 2) uniform sampler2D u_ObjIdx;
layout(set = 0, binding = 3) uniform sampler2D u_LastObjIdx;

// struct CooldownData{
//     float m_T;
// };

// layout(std140, set = 0, binding = 4) readonly buffer CooldownData{
//     CooldownData cd[];
// } cooldownData;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_UV;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

//--------------------------------------------------------------------

void main()
{
    vec3 color = texture(u_SceneColor, in_UV).rgb;
    vec3 lastColor = texture(u_LastSceneColor, in_UV).rgb;
    // int objIdx = int(texture(u_ObjIdx, in_UV).x);
    // int lastObjIdx = int(texture(u_LastObjIdx, in_UV).x);
    // int tempC1 = int(texture(u_ObjIdx, in_UV).a);
    // int tempC2 = int(texture(u_LastObjIdx, in_UV).a);

    float lum1 = 0.299*lastColor.r + 0.587*lastColor.g + 0.114*lastColor.b;
    float lum2 = 0.299*color.r + 0.587*color.g + 0.114*color.b;

    vec3 c = vec3(0.0, 0.0, 0.0);
    // && (objIdx != lastObjIdx || tempC1 != tempC2)
    float threshold = 0.025;
    float delta = min(abs(lum1 - lum2), 1.0);
    if(lum1 - lum2 > threshold){
        c = vec3(delta,0.0,0.0);
    }else if(lum1 - lum2 < -threshold){
        c= vec3(0.0,0.0,delta);
    }

    outColor = vec4(c, 1.0);
}
