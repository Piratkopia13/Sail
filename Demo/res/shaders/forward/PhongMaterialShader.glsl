#version 450

struct PhongMaterial
{
    vec4 modelColor;
    float ka;
    float kd;
    float ks;
    float shininess;
    uint hasDiffuseTexture;
    uint hasNormalTexture;
    uint hasSpecularTexture;
};

struct PointLight
{
    vec3 color;
    float attRadius;
    vec3 fragToLight;
    float intensity;
};

layout(binding = 0, std140) uniform type_VSPSSystemCBuffer
{
    mat4 sys_mWorld;
    mat4 sys_mVP;
    PhongMaterial sys_material;
    vec4 sys_clippingPlane;
    vec3 sys_cameraPos;
} VSPSSystemCBuffer;

uniform sampler2D SPIRV_Cross_Combinedsys_texDiffusePSss;
uniform sampler2D SPIRV_Cross_Combinedsys_texNormalPSss;
uniform sampler2D SPIRV_Cross_Combinedsys_texSpecularPSss;

layout(location = 0) in vec3 varying_NORMAL0;
layout(location = 1) in vec2 varying_TEXCOORD0;
layout(location = 2) in vec3 varying_TOCAM;
layout(location = 3) in vec3 varying_LIGHTS;
layout(location = 4) in float in_var_LIGHTS1;
layout(location = 5) in vec3 varying_LIGHTS2;
layout(location = 6) in float in_var_LIGHTS3;
layout(location = 7) in PointLight varying_LIGHTS4[2];
layout(location = 0) out vec4 out_var_SV_Target0;

void main()
{
    vec4 _95;
    if (VSPSSystemCBuffer.sys_material.hasDiffuseTexture != 0u)
    {
        _95 = VSPSSystemCBuffer.sys_material.modelColor * texture(SPIRV_Cross_Combinedsys_texDiffusePSss, varying_TEXCOORD0);
    }
    else
    {
        _95 = VSPSSystemCBuffer.sys_material.modelColor;
    }
    vec3 _108;
    if (VSPSSystemCBuffer.sys_material.hasNormalTexture != 0u)
    {
        _108 = (texture(SPIRV_Cross_Combinedsys_texNormalPSss, varying_TEXCOORD0).xyz * 2.0) - vec3(1.0);
    }
    else
    {
        _108 = varying_NORMAL0;
    }
    vec3 _119;
    if (VSPSSystemCBuffer.sys_material.hasSpecularTexture != 0u)
    {
        _119 = texture(SPIRV_Cross_Combinedsys_texSpecularPSss, varying_TEXCOORD0).xyz;
    }
    else
    {
        _119 = vec3(1.0);
    }
    PointLight _70[2] = varying_LIGHTS4;
    vec3 _120 = normalize(varying_TOCAM);
    vec3 _121 = normalize(_108);
    vec3 _122 = normalize(varying_LIGHTS2);
    float _125 = clamp(dot(_121, -_122), 0.0, 1.0);
    vec3 _135;
    if (_125 > 0.0)
    {
        _135 = _119 * pow(clamp(dot(_120, normalize(reflect(_122, _121))), 0.0, 1.0), VSPSSystemCBuffer.sys_material.shininess);
    }
    else
    {
        _135 = vec3(0.0);
    }
    vec3 _144;
    _144 = ((vec3(VSPSSystemCBuffer.sys_material.kd * _125) + (_135 * VSPSSystemCBuffer.sys_material.ks)) * _95.xyz) * varying_LIGHTS;
    vec3 _145;
    for (int _147 = 0; _147 < 2; _144 = _145, _147++)
    {
        vec3 _158 = normalize(_70[_147].fragToLight);
        float _160 = clamp(dot(_121, _158), 0.0, 1.0);
        vec3 _171;
        if (_160 > 0.0)
        {
            _171 = _119 * pow(clamp(dot(_120, normalize(reflect(-_158, _121))), 0.0, 1.0), VSPSSystemCBuffer.sys_material.shininess);
        }
        else
        {
            _171 = vec3(0.0);
        }
        float _172 = length(_158);
        _145 = _144 + ((((vec3(VSPSSystemCBuffer.sys_material.kd * _160) + (_171 * VSPSSystemCBuffer.sys_material.ks)) * _95.xyz) * _70[_147].color) * ((pow(clamp(1.0 - pow(_172 / _70[_147].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_172 * _172) + 1.0)) * _70[_147].intensity));
    }
    out_var_SV_Target0 = vec4(clamp(((vec3(0.300000011920928955078125) * VSPSSystemCBuffer.sys_material.ka) * _95.xyz) + _144, vec3(0.0), vec3(1.0)), 1.0);
}

