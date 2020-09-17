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

uniform sampler2D SPIRV_Cross_CombinedtexArrPSss[];
uniform samplerCube SPIRV_Cross_CombinedtexCubeArrPSss[];

layout(location = 0) in vec3 varying_NORMAL0;
layout(location = 1) in vec2 varying_TEXCOORD0;
layout(location = 2) in vec3 varying_worldPos;
layout(location = 3) in mat3 varying_TBN;
layout(location = 0) out vec4 out_var_SV_Target0;

uint _90;
bool _91;

void main()
{
    vec3 _220;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].albedoTexIndex != (-1))
    {
        _220 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor.xyz * texture(SPIRV_Cross_CombinedtexArrPSss[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].albedoTexIndex)], varying_TEXCOORD0).xyz;
    }
    else
    {
        _220 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor.xyz;
    }
    vec3 _240;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex != (-1))
    {
        vec4 _228 = texture(SPIRV_Cross_CombinedtexArrPSss[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex)], varying_TEXCOORD0);
        vec3 _232 = _228.xyz;
        _232.y = 1.0 - _228.y;
        vec3 _235 = _232;
        _235.x = 1.0 - _228.x;
        _240 = varying_TBN * normalize((_235 * 2.0) - vec3(1.0));
    }
    else
    {
        _240 = varying_NORMAL0;
    }
    float _256;
    float _257;
    float _258;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].mraoTexIndex != (-1))
    {
        vec4 _248 = texture(SPIRV_Cross_CombinedtexArrPSss[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].mraoTexIndex)], varying_TEXCOORD0);
        _256 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].metalnessScale * _248.x;
        _257 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].roughnessScale * (1.0 - _248.y);
        _258 = _248.z + VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].aoIntensity;
    }
    else
    {
        _256 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].metalnessScale;
        _257 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].roughnessScale;
        _258 = 1.0;
    }
    PointLight _103[8] = PointLight[](PointLight(VSPSSystemCBuffer.pointLights[0u].color, VSPSSystemCBuffer.pointLights[0u].attRadius, VSPSSystemCBuffer.pointLights[0u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[0u].intensity), PointLight(VSPSSystemCBuffer.pointLights[1u].color, VSPSSystemCBuffer.pointLights[1u].attRadius, VSPSSystemCBuffer.pointLights[1u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[1u].intensity), PointLight(VSPSSystemCBuffer.pointLights[2u].color, VSPSSystemCBuffer.pointLights[2u].attRadius, VSPSSystemCBuffer.pointLights[2u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[2u].intensity), PointLight(VSPSSystemCBuffer.pointLights[3u].color, VSPSSystemCBuffer.pointLights[3u].attRadius, VSPSSystemCBuffer.pointLights[3u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[3u].intensity), PointLight(VSPSSystemCBuffer.pointLights[4u].color, VSPSSystemCBuffer.pointLights[4u].attRadius, VSPSSystemCBuffer.pointLights[4u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[4u].intensity), PointLight(VSPSSystemCBuffer.pointLights[5u].color, VSPSSystemCBuffer.pointLights[5u].attRadius, VSPSSystemCBuffer.pointLights[5u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[5u].intensity), PointLight(VSPSSystemCBuffer.pointLights[6u].color, VSPSSystemCBuffer.pointLights[6u].attRadius, VSPSSystemCBuffer.pointLights[6u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[6u].intensity), PointLight(VSPSSystemCBuffer.pointLights[7u].color, VSPSSystemCBuffer.pointLights[7u].attRadius, VSPSSystemCBuffer.pointLights[7u].fragToLight - varying_worldPos, VSPSSystemCBuffer.pointLights[7u].intensity));
    vec3 _268 = normalize(_240);
    vec3 _270 = normalize(VSPSSystemCBuffer.sys_cameraPos - varying_worldPos);
    vec3 _272 = mix(vec3(0.039999999105930328369140625), _220, vec3(_256));
    vec3 _343;
    if (!(all(equal(VSPSSystemCBuffer.dirLight.color, vec3(0.0))) || _91))
    {
        vec3 _279 = -VSPSSystemCBuffer.dirLight.direction;
        vec3 _280 = normalize(_279);
        vec3 _282 = normalize(_270 + _280);
        float _283 = length(_279);
        float _292 = (pow(clamp(1.0 - pow(_283 * 1.0000000133514319600180897396058e-10, 4.0), 0.0, 1.0), 2.0) / ((_283 * _283) + 1.0)) * VSPSSystemCBuffer.dirLight.intensity;
        vec3 _300 = _272 + ((vec3(1.0) - _272) * pow(1.0 - max(dot(_282, _270), 0.0), 5.0));
        float _301 = _257 * _257;
        float _302 = _301 * _301;
        float _304 = max(dot(_268, _282), 0.0);
        float _308 = ((_304 * _304) * (_302 - 1.0)) + 1.0;
        float _313 = max(dot(_268, _270), 0.0);
        float _315 = max(dot(_268, _280), 0.0);
        float _316 = _257 + 1.0;
        float _318 = (_316 * _316) * 0.125;
        float _319 = 1.0 - _318;
        _343 = ((((((vec3(1.0) - _300) * (1.0 - _256)) * _220) * vec3(0.3183098733425140380859375)) + (((_300 * ((_302 / ((3.1415927410125732421875 * _308) * _308)) * ((_315 / ((_315 * _319) + _318)) * (_313 / ((_313 * _319) + _318))))) / vec3(max((4.0 * _313) * _315, 0.001000000047497451305389404296875))) * _292)) * (VSPSSystemCBuffer.dirLight.color * _292)) * _315;
    }
    else
    {
        _343 = vec3(0.0);
    }
    vec3 _345;
    _345 = _343;
    vec3 _346;
    for (uint _348 = 0u; _348 < _90; _345 = _346, _348++)
    {
        if ((_103[_348].intensity == 0.0) || all(equal(_103[_348].color, vec3(0.0))))
        {
            _346 = _345;
            continue;
        }
        float _365 = length(_103[_348].fragToLight);
        if (_365 > _103[_348].attRadius)
        {
            _346 = _345;
            continue;
        }
        vec3 _369 = normalize(_103[_348].fragToLight);
        vec3 _371 = normalize(_270 + _369);
        float _380 = (pow(clamp(1.0 - pow(_365 / _103[_348].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_365 * _365) + 1.0)) * _103[_348].intensity;
        vec3 _388 = _272 + ((vec3(1.0) - _272) * pow(1.0 - max(dot(_371, _270), 0.0), 5.0));
        float _389 = _257 * _257;
        float _390 = _389 * _389;
        float _392 = max(dot(_268, _371), 0.0);
        float _396 = ((_392 * _392) * (_390 - 1.0)) + 1.0;
        float _401 = max(dot(_268, _270), 0.0);
        float _403 = max(dot(_268, _369), 0.0);
        float _404 = _257 + 1.0;
        float _406 = (_404 * _404) * 0.125;
        float _407 = 1.0 - _406;
        _346 = _345 + (((((((vec3(1.0) - _388) * (1.0 - _256)) * _220) * vec3(0.3183098733425140380859375)) + (((_388 * ((_390 / ((3.1415927410125732421875 * _396) * _396)) * ((_403 / ((_403 * _407) + _406)) * (_401 / ((_401 * _407) + _406))))) / vec3(max((4.0 * _401) * _403, 0.001000000047497451305389404296875))) * _380)) * (_103[_348].color * _380)) * _403);
    }
    float _433 = max(dot(_268, _270), 0.0);
    vec3 _441 = _272 + ((max(vec3(1.0 - _257), _272) - _272) * pow(1.0 - _433, 5.0));
    vec4 _457 = textureLod(SPIRV_Cross_CombinedtexArrPSss[VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].brdfLutTexIndex], vec2(_433, _257), 0.0);
    vec3 _467 = (((((vec3(1.0) - _441) * (1.0 - _256)) * (textureLod(SPIRV_Cross_CombinedtexCubeArrPSss[VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].irradianceMapTexIndex], _268, 0.0).xyz * _220)) + (textureLod(SPIRV_Cross_CombinedtexCubeArrPSss[VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].radianceMapTexIndex], reflect(-_270, _268), _257 * 4.0).xyz * ((_441 * _457.x) + vec3(_457.y)))) * _258) + _345;
    out_var_SV_Target0 = vec4(pow(_467 / (_467 + vec3(1.0)), vec3(0.454545438289642333984375)), 1.0);
}

