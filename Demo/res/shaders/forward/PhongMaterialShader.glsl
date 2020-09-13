#version 450

struct PointLight
{
    vec3 color;
    float attRadius;
    vec3 fragToLight;
    float intensity;
};

uniform sampler2D SPIRV_Cross_Combinedsys_texDiffusePSss;

layout(location = 0) in vec3 in_var_NORMAL0;
layout(location = 1) in vec2 varying_TEXCOORD0;
layout(location = 2) in vec3 in_var_TOCAM;
layout(location = 3) in vec3 in_var_LIGHTS;
layout(location = 4) in float in_var_LIGHTS1;
layout(location = 5) in vec3 in_var_LIGHTS2;
layout(location = 6) in float in_var_LIGHTS3;
layout(location = 7) in PointLight in_var_LIGHTS4[2];
layout(location = 0) out vec4 out_var_SV_Target0;

void main()
{
    out_var_SV_Target0 = texture(SPIRV_Cross_Combinedsys_texDiffusePSss, varying_TEXCOORD0);
}

