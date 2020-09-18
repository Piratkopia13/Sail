#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

struct DirectionalLight
{
    vec3 color;
    float intensity;
    vec3 direction;
    float padding;
};

struct PointLight
{
    vec3 color;
    float attRadius;
    vec3 fragToLight;
    float intensity;
};

struct PBRMaterial
{
    vec4 modelColor;
    float metalnessScale;
    float roughnessScale;
    float aoIntensity;
    int albedoTexIndex;
    int normalTexIndex;
    int mraoTexIndex;
    int radianceMapTexIndex;
    int irradianceMapTexIndex;
    int brdfLutTexIndex;
    vec3 padding;
};

layout(set = 0, binding = 0, std140) uniform type_VSPSSystemCBuffer
{
    mat4 sys_mVP;
    vec3 sys_cameraPos;
    float padding;
    vec4 sys_clippingPlane;
    DirectionalLight dirLight;
    PointLight pointLights[8];
} VSPSSystemCBuffer;

layout(set = 0, binding = 1, scalar) uniform type_VSPSMaterials
{
    PBRMaterial sys_materials[1024];
} VSPSMaterials;

layout(push_constant, std430) uniform type_PushConstant_
{
    mat4 sys_mWorld;
    uint sys_materialIndex;
} VSPSConsts;

layout(set = 0, binding = 5) uniform sampler PSLinearSampler;
layout(set = 0, binding = 6) uniform sampler PSss;
layout(set = 0, binding = 7) uniform texture2D texArr[];
layout(set = 0, binding = 8) uniform textureCube texCubeArr[];

layout(location = 0) in vec3 varying_NORMAL0;
layout(location = 1) in vec2 varying_TEXCOORD0;
layout(location = 2) in vec3 varying_worldPos;
layout(location = 3) in mat3 varying_TBN;
layout(location = 0) out vec4 out_var_SV_Target0;

uint _91;
bool _92;

void main()
{
    vec3 _222;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].albedoTexIndex != (-1))
    {
        _222 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor.xyz * texture(sampler2D(texArr[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].albedoTexIndex)], PSss), varying_TEXCOORD0).xyz;
    }
    else
    {
        _222 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor.xyz;
    }
    vec3 _243;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex != (-1))
    {
        vec4 _231 = texture(sampler2D(texArr[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex)], PSss), varying_TEXCOORD0);
        vec3 _235 = _231.xyz;
        _235.y = 1.0 - _231.y;
        vec3 _238 = _235;
        _238.x = 1.0 - _231.x;
        _243 = varying_TBN * normalize((_238 * 2.0) - vec3(1.0));
    }
    else
    {
        _243 = varying_NORMAL0;
    }
    float _260;
    float _261;
    float _262;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].mraoTexIndex != (-1))
    {
        vec4 _252 = texture(sampler2D(texArr[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].mraoTexIndex)], PSss), varying_TEXCOORD0);
        _260 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].metalnessScale * _252.x;
        _261 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].roughnessScale * (1.0 - _252.y);
        _262 = _252.z + VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].aoIntensity;
    }
    else
    {
        _260 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].metalnessScale;
        _261 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].roughnessScale;
        _262 = 1.0;
    }
    PointLight _104[8] = PointLight[](PointLight(VSPSSystemCBuffer.pointLights[0u].color, VSPSSystemCBuffer.pointLights[0u].attRadius, VSPSSystemCBuffer.pointLights[0u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[0u].intensity), PointLight(VSPSSystemCBuffer.pointLights[1u].color, VSPSSystemCBuffer.pointLights[1u].attRadius, VSPSSystemCBuffer.pointLights[1u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[1u].intensity), PointLight(VSPSSystemCBuffer.pointLights[2u].color, VSPSSystemCBuffer.pointLights[2u].attRadius, VSPSSystemCBuffer.pointLights[2u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[2u].intensity), PointLight(VSPSSystemCBuffer.pointLights[3u].color, VSPSSystemCBuffer.pointLights[3u].attRadius, VSPSSystemCBuffer.pointLights[3u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[3u].intensity), PointLight(VSPSSystemCBuffer.pointLights[4u].color, VSPSSystemCBuffer.pointLights[4u].attRadius, VSPSSystemCBuffer.pointLights[4u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[4u].intensity), PointLight(VSPSSystemCBuffer.pointLights[5u].color, VSPSSystemCBuffer.pointLights[5u].attRadius, VSPSSystemCBuffer.pointLights[5u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[5u].intensity), PointLight(VSPSSystemCBuffer.pointLights[6u].color, VSPSSystemCBuffer.pointLights[6u].attRadius, VSPSSystemCBuffer.pointLights[6u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[6u].intensity), PointLight(VSPSSystemCBuffer.pointLights[7u].color, VSPSSystemCBuffer.pointLights[7u].attRadius, VSPSSystemCBuffer.pointLights[7u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[7u].intensity));
    vec3 _272 = normalize(_243);
    vec3 _274 = normalize(VSPSSystemCBuffer.sys_cameraPos - varying_worldPos);
    vec3 _276 = mix(vec3(0.039999999105930328369140625), _222, vec3(_260));
    vec3 _347;
    if (!(all(equal(VSPSSystemCBuffer.dirLight.color, vec3(0.0))) || _92))
    {
        vec3 _283 = -VSPSSystemCBuffer.dirLight.direction;
        vec3 _284 = normalize(_283);
        vec3 _286 = normalize(_274 + _284);
        float _287 = length(_283);
        float _296 = (pow(clamp(1.0 - pow(_287 * 1.0000000133514319600180897396058e-10, 4.0), 0.0, 1.0), 2.0) / ((_287 * _287) + 1.0)) * VSPSSystemCBuffer.dirLight.intensity;
        vec3 _304 = _276 + ((vec3(1.0) - _276) * pow(1.0 - max(dot(_286, _274), 0.0), 5.0));
        float _305 = _261 * _261;
        float _306 = _305 * _305;
        float _308 = max(dot(_272, _286), 0.0);
        float _312 = ((_308 * _308) * (_306 - 1.0)) + 1.0;
        float _317 = max(dot(_272, _274), 0.0);
        float _319 = max(dot(_272, _284), 0.0);
        float _320 = _261 + 1.0;
        float _322 = (_320 * _320) * 0.125;
        float _323 = 1.0 - _322;
        _347 = ((((((vec3(1.0) - _304) * (1.0 - _260)) * _222) * vec3(0.3183098733425140380859375)) + (((_304 * ((_306 / ((3.1415927410125732421875 * _312) * _312)) * ((_319 / ((_319 * _323) + _322)) * (_317 / ((_317 * _323) + _322))))) / vec3(max((4.0 * _317) * _319, 0.001000000047497451305389404296875))) * _296)) * (VSPSSystemCBuffer.dirLight.color * _296)) * _319;
    }
    else
    {
        _347 = vec3(0.0);
    }
    vec3 _349;
    _349 = _347;
    vec3 _350;
    for (uint _352 = 0u; _352 < _91; _349 = _350, _352++)
    {
        if ((_104[_352].intensity == 0.0) || all(equal(_104[_352].color, vec3(0.0))))
        {
            _350 = _349;
            continue;
        }
        float _369 = length(_104[_352].fragToLight);
        if (_369 > _104[_352].attRadius)
        {
            _350 = _349;
            continue;
        }
        vec3 _373 = normalize(_104[_352].fragToLight);
        vec3 _375 = normalize(_274 + _373);
        float _384 = (pow(clamp(1.0 - pow(_369 / _104[_352].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_369 * _369) + 1.0)) * _104[_352].intensity;
        vec3 _392 = _276 + ((vec3(1.0) - _276) * pow(1.0 - max(dot(_375, _274), 0.0), 5.0));
        float _393 = _261 * _261;
        float _394 = _393 * _393;
        float _396 = max(dot(_272, _375), 0.0);
        float _400 = ((_396 * _396) * (_394 - 1.0)) + 1.0;
        float _405 = max(dot(_272, _274), 0.0);
        float _407 = max(dot(_272, _373), 0.0);
        float _408 = _261 + 1.0;
        float _410 = (_408 * _408) * 0.125;
        float _411 = 1.0 - _410;
        _350 = _349 + (((((((vec3(1.0) - _392) * (1.0 - _260)) * _222) * vec3(0.3183098733425140380859375)) + (((_392 * ((_394 / ((3.1415927410125732421875 * _400) * _400)) * ((_407 / ((_407 * _411) + _410)) * (_405 / ((_405 * _411) + _410))))) / vec3(max((4.0 * _405) * _407, 0.001000000047497451305389404296875))) * _384)) * (_104[_352].color * _384)) * _407);
    }
    float _437 = max(dot(_272, _274), 0.0);
    vec3 _445 = _276 + ((max(vec3(1.0 - _261), _276) - _276) * pow(1.0 - _437, 5.0));
    vec4 _461 = textureLod(sampler2D(texArr[VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].brdfLutTexIndex], PSLinearSampler), vec2(_437, _261), 0.0);
    vec3 _471 = (((((vec3(1.0) - _445) * (1.0 - _260)) * (textureLod(samplerCube(texCubeArr[VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].irradianceMapTexIndex], PSLinearSampler), _272, 0.0).xyz * _222)) + (textureLod(samplerCube(texCubeArr[VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].radianceMapTexIndex], PSLinearSampler), reflect(-_274, _272), _261 * 4.0).xyz * ((_445 * _461.x) + vec3(_461.y)))) * _262) + _349;
    out_var_SV_Target0 = vec4(pow(_471 / (_471 + vec3(1.0)), vec3(0.454545438289642333984375)), 1.0);
}

