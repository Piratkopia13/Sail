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
    PointLight _71[2];
    for (uint _84 = 0u; _84 < 2u; )
    {
        _71[_84].attRadius = VSLights.pointLights[_84].attRadius;
        _71[_84].color = VSLights.pointLights[_84].color;
        _71[_84].intensity = VSLights.pointLights[_84].intensity;
        _84++;
        continue;
    }
    vec4 _98 = in_var_POSITION0;
    _98.w = 1.0;
    vec4 _101 = _98 * VSPSSystemCBuffer.sys_mWorld;
    vec3 _107 = _101.xyz;
    vec3 _108 = VSPSSystemCBuffer.sys_cameraPos - _107;
    for (uint _110 = 0u; _110 < 2u; )
    {
        _71[_110].fragToLight = VSLights.pointLights[_110].position - _107;
        _110++;
        continue;
    }
    vec3 _153;
    vec3 _154;
    if (VSPSSystemCBuffer.sys_material.hasNormalTexture != 0u)
    {
        mat3 _133 = mat3(VSPSSystemCBuffer.sys_mWorld[0].xyz, VSPSSystemCBuffer.sys_mWorld[1].xyz, VSPSSystemCBuffer.sys_mWorld[2].xyz);
        mat3 _141 = transpose(mat3(normalize(in_var_TANGENT0) * _133, normalize(in_var_BINORMAL0) * _133, normalize(in_var_NORMAL0) * _133));
        for (int _145 = 0; _145 < 2; )
        {
            _71[_145].fragToLight = _141 * _71[_145].fragToLight;
            _145++;
            continue;
        }
        _153 = _141 * _108;
        _154 = _141 * VSLights.dirLight.direction;
    }
    else
    {
        _153 = _108;
        _154 = VSLights.dirLight.direction;
    }
    gl_Position = _101 * VSPSSystemCBuffer.sys_mVP;
    varying_NORMAL0 = normalize(in_var_NORMAL0 * mat3(VSPSSystemCBuffer.sys_mWorld[0].xyz, VSPSSystemCBuffer.sys_mWorld[1].xyz, VSPSSystemCBuffer.sys_mWorld[2].xyz));
    varying_TEXCOORD0 = in_var_TEXCOORD0;
    gl_ClipDistance[0u] = dot(_101, VSPSSystemCBuffer.sys_clippingPlane);
    varying_TOCAM = _153;
    varying_LIGHTS = VSLights.dirLight.color;
    varying_LIGHTS1 = VSLights.dirLight.intensity;
    varying_LIGHTS2 = _154;
    varying_LIGHTS3 = VSLights.dirLight.padding;
    varying_LIGHTS4 = _71;
}

