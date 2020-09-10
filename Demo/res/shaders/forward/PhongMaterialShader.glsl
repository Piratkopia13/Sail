#version 450

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_ClipDistance[1];
};

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

struct DirectionalLight
{
    vec3 color;
    float intensity;
    vec3 direction;
    float padding;
};

struct PointLightInput
{
    vec3 color;
    float attRadius;
    vec3 position;
    float intensity;
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

layout(binding = 1, std140) uniform type_VSLights
{
    DirectionalLight dirLight;
    PointLightInput pointLights[2];
} VSLights;

struct type_PushConstant_
{
    vec3 test;
};

uniform type_PushConstant_ pc;

layout(location = 0) in vec4 in_var_POSITION0;
layout(location = 1) in vec2 in_var_TEXCOORD0;
layout(location = 2) in vec3 in_var_NORMAL0;
layout(location = 3) in vec3 in_var_TANGENT0;
layout(location = 4) in vec3 in_var_BINORMAL0;
layout(location = 0) out vec3 varying_NORMAL0;
layout(location = 1) out vec2 varying_TEXCOORD0;
layout(location = 2) out vec3 varying_TOCAM;
layout(location = 3) out vec3 varying_LIGHTS;
layout(location = 4) out float varying_LIGHTS1;
layout(location = 5) out vec3 varying_LIGHTS2;
layout(location = 6) out float varying_LIGHTS3;
layout(location = 7) out PointLight varying_LIGHTS4[2];

void main()
{
    PointLight _75[2];
    for (uint _88 = 0u; _88 < 2u; )
    {
        _75[_88].attRadius = VSLights.pointLights[_88].attRadius;
        _75[_88].color = VSLights.pointLights[_88].color;
        _75[_88].intensity = VSLights.pointLights[_88].intensity;
        _88++;
        continue;
    }
    vec4 _102 = in_var_POSITION0;
    _102.w = 1.0;
    vec4 _105 = _102 * VSPSSystemCBuffer.sys_mWorld;
    vec3 _111 = _105.xyz;
    vec3 _112 = VSPSSystemCBuffer.sys_cameraPos - _111;
    for (uint _114 = 0u; _114 < 2u; )
    {
        _75[_114].fragToLight = VSLights.pointLights[_114].position - _111;
        _114++;
        continue;
    }
    vec4 _125 = _105 * VSPSSystemCBuffer.sys_mVP;
    vec3 _157;
    vec3 _158;
    if (VSPSSystemCBuffer.sys_material.hasNormalTexture != 0u)
    {
        mat3 _137 = mat3(VSPSSystemCBuffer.sys_mWorld[0].xyz, VSPSSystemCBuffer.sys_mWorld[1].xyz, VSPSSystemCBuffer.sys_mWorld[2].xyz);
        mat3 _145 = transpose(mat3(normalize(in_var_TANGENT0) * _137, normalize(in_var_BINORMAL0) * _137, normalize(in_var_NORMAL0) * _137));
        for (int _149 = 0; _149 < 2; )
        {
            _75[_149].fragToLight = _145 * _75[_149].fragToLight;
            _149++;
            continue;
        }
        _157 = _145 * _112;
        _158 = _145 * VSLights.dirLight.direction;
    }
    else
    {
        _157 = _112;
        _158 = VSLights.dirLight.direction;
    }
    vec4 _172 = _125;
    _172.w = _125.w + pc.test.x;
    gl_Position = _172;
    varying_NORMAL0 = normalize(in_var_NORMAL0 * mat3(VSPSSystemCBuffer.sys_mWorld[0].xyz, VSPSSystemCBuffer.sys_mWorld[1].xyz, VSPSSystemCBuffer.sys_mWorld[2].xyz));
    varying_TEXCOORD0 = in_var_TEXCOORD0;
    gl_ClipDistance[0u] = dot(_105, VSPSSystemCBuffer.sys_clippingPlane);
    varying_TOCAM = _157;
    varying_LIGHTS = VSLights.dirLight.color;
    varying_LIGHTS1 = VSLights.dirLight.intensity;
    varying_LIGHTS2 = _158;
    varying_LIGHTS3 = VSLights.dirLight.padding;
    varying_LIGHTS4 = _75;
}

