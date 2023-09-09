#version 460

// Uniforms
//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_SceneColorSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 in_TexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 out_Color;

// Auxiliary Functions
//--------------------------------------------------------------------

vec3 PowVec3(vec3 v, float p){
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

vec3 LinearToSRGB(vec3 v) {
    return PowVec3(v, 1.0 / 2.2); 
}

float RGBToLuminance(vec3 col){
    return dot(col, vec3(0.2126, 0.7152, 0.0722));
}

// https://learnopengl.com/Guest-Articles/2022/Phys.-Based-Bloom
float KarisAverage(vec3 col){
    float luma = RGBToLuminance(LinearToSRGB(col)) * 0.25;
    return 1.0 / (1.0 + luma);
}

void main() {
    vec2 texSize = textureSize(u_SceneColorSampler, 0);
    vec2 srcTexelSize = 1.0 / texSize;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(u_SceneColorSampler, vec2(in_TexCoord.x - 2*x, in_TexCoord.y + 2*y)).rgb;
    vec3 b = texture(u_SceneColorSampler, vec2(in_TexCoord.x,       in_TexCoord.y + 2*y)).rgb;
    vec3 c = texture(u_SceneColorSampler, vec2(in_TexCoord.x + 2*x, in_TexCoord.y + 2*y)).rgb;

    vec3 d = texture(u_SceneColorSampler, vec2(in_TexCoord.x - 2*x, in_TexCoord.y)).rgb;
    vec3 e = texture(u_SceneColorSampler, vec2(in_TexCoord.x,       in_TexCoord.y)).rgb;
    vec3 f = texture(u_SceneColorSampler, vec2(in_TexCoord.x + 2*x, in_TexCoord.y)).rgb;

    vec3 g = texture(u_SceneColorSampler, vec2(in_TexCoord.x - 2*x, in_TexCoord.y - 2*y)).rgb;
    vec3 h = texture(u_SceneColorSampler, vec2(in_TexCoord.x,       in_TexCoord.y - 2*y)).rgb;
    vec3 i = texture(u_SceneColorSampler, vec2(in_TexCoord.x + 2*x, in_TexCoord.y - 2*y)).rgb;

    vec3 j = texture(u_SceneColorSampler, vec2(in_TexCoord.x - x, in_TexCoord.y + y)).rgb;
    vec3 k = texture(u_SceneColorSampler, vec2(in_TexCoord.x + x, in_TexCoord.y + y)).rgb;
    vec3 l = texture(u_SceneColorSampler, vec2(in_TexCoord.x - x, in_TexCoord.y - y)).rgb;
    vec3 m = texture(u_SceneColorSampler, vec2(in_TexCoord.x + x, in_TexCoord.y - y)).rgb;

    // Apply weighted distribution
    vec3 downsample = vec3(0.0);
    vec3 groups[5];

    // Horrible way to check if its the first downsample
    if(texSize.x >= 900){
        groups[0] = (a+b+d+e) * (0.125/4.0);
        groups[1] = (b+c+e+f) * (0.125/4.0);
        groups[2] = (d+e+g+h) * (0.125/4.0);
        groups[3] = (e+f+h+i) * (0.125/4.0);
        groups[4] = (j+k+l+m) * (0.5/4.0);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        downsample = groups[0]+groups[1]+groups[2]+groups[3]+groups[4];
    }else{
        downsample += e*0.125;
        downsample += (a+c+g+i)*0.03125;
        downsample += (b+d+f+h)*0.0625;
        downsample += (j+k+l+m)*0.125;
    }
    out_Color = vec4(max(downsample, 0.0001), 1.0);
}