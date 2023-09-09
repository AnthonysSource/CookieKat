#version 450

//--------------------------------------------------------------------

layout(set = 0, binding = 0) uniform sampler2D u_SceneColorSampler;

// Vertex Stage Input
//--------------------------------------------------------------------

layout(location = 0) in vec2 fragTexCoord;

// Framebuffer Output
//--------------------------------------------------------------------

layout(location = 0) out vec4 outColor;

//--------------------------------------------------------------------

vec3 ACESFilm(vec3 x)
{
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), vec3(0.0), vec3(1.0));
}

vec3 Reinhard(vec3 x){
    return x / (x + vec3(1.0));
}

vec3 AcesComplex(vec3 color){
	mat3 m1 = mat3(
        0.59719, 0.07600, 0.02840,
        0.35458, 0.90834, 0.13383,
        0.04823, 0.01566, 0.83777
	);
	mat3 m2 = mat3(
        1.60475, -0.10208, -0.00327,
        -0.53108,  1.10813, -0.07276,
        -0.07367, -0.00605,  1.07602
	);
	vec3 v = m1 * color;    
	vec3 a = v * (v + 0.0245786) - 0.000090537;
	vec3 b = v * (0.983729 * v + 0.4329510) + 0.238081;
	return clamp(m2 * (a / b), 0.0, 1.0);	
}

void main()
{
    vec3 sceneColor = texture(u_SceneColorSampler, fragTexCoord).xyz;
    sceneColor = AcesComplex(sceneColor);
    outColor = vec4(sceneColor, 1.0);
}