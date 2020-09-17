#version 450
#extension GL_EXT_nonuniform_qualifier : require

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

struct PhongMaterial
{
    vec4 modelColor;
    float ka;
    float kd;
    float ks;
    float shininess;
    int diffuseTexIndex;
    int normalTexIndex;
    int specularTexIndex;
    float padding;
};

layout(set = 0, binding = 0, std140) uniform type_VSSystemCBuffer
{
    mat4 sys_mVP;
    vec4 sys_clippingPlane;
    vec3 sys_cameraPos;
    float padding;
    DirectionalLight dirLight;
    PointLight pointLights[8];
} VSSystemCBuffer;

layout(set = 0, binding = 1, std140) uniform type_VSPSMaterials
{
    PhongMaterial sys_materials[1024];
} VSPSMaterials;

layout(push_constant, std430) uniform type_PushConstant_
{
    mat4 sys_mWorld;
    uint sys_materialIndex;
} VSPSConsts;

layout(set = 0, binding = 5) uniform sampler PSss;
layout(set = 0, binding = 5) uniform texture2D texArr[];

layout(location = 0) in vec3 varying_NORMAL0;
layout(location = 1) in vec2 varying_TEXCOORD0;
layout(location = 2) in vec3 varying_WORLDPOS;
layout(location = 3) in vec3 varying_TOCAM;
layout(location = 4) in mat3 varying_TBN;
layout(location = 0) out vec4 out_var_SV_Target0;

void main()
{
    PointLight _90[8] = PointLight[](PointLight(VSSystemCBuffer.pointLights[0].color, VSSystemCBuffer.pointLights[0].attRadius, VSSystemCBuffer.pointLights[0].fragToLight, VSSystemCBuffer.pointLights[0].intensity), PointLight(VSSystemCBuffer.pointLights[1].color, VSSystemCBuffer.pointLights[1].attRadius, VSSystemCBuffer.pointLights[1].fragToLight, VSSystemCBuffer.pointLights[1].intensity), PointLight(VSSystemCBuffer.pointLights[2].color, VSSystemCBuffer.pointLights[2].attRadius, VSSystemCBuffer.pointLights[2].fragToLight, VSSystemCBuffer.pointLights[2].intensity), PointLight(VSSystemCBuffer.pointLights[3].color, VSSystemCBuffer.pointLights[3].attRadius, VSSystemCBuffer.pointLights[3].fragToLight, VSSystemCBuffer.pointLights[3].intensity), PointLight(VSSystemCBuffer.pointLights[4].color, VSSystemCBuffer.pointLights[4].attRadius, VSSystemCBuffer.pointLights[4].fragToLight, VSSystemCBuffer.pointLights[4].intensity), PointLight(VSSystemCBuffer.pointLights[5].color, VSSystemCBuffer.pointLights[5].attRadius, VSSystemCBuffer.pointLights[5].fragToLight, VSSystemCBuffer.pointLights[5].intensity), PointLight(VSSystemCBuffer.pointLights[6].color, VSSystemCBuffer.pointLights[6].attRadius, VSSystemCBuffer.pointLights[6].fragToLight, VSSystemCBuffer.pointLights[6].intensity), PointLight(VSSystemCBuffer.pointLights[7].color, VSSystemCBuffer.pointLights[7].attRadius, VSSystemCBuffer.pointLights[7].fragToLight, VSSystemCBuffer.pointLights[7].intensity));
    for (int _167 = 0; _167 < 8; )
    {
        _90[_167].fragToLight -= varying_WORLDPOS;
        _167++;
        continue;
    }
    vec4 _191;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].diffuseTexIndex != (-1))
    {
        _191 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor * texture(sampler2D(texArr[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].diffuseTexIndex)], PSss), varying_TEXCOORD0);
    }
    else
    {
        _191 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor;
    }
    vec3 _209;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex != (-1))
    {
        vec4 _200 = texture(sampler2D(texArr[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex)], PSss), varying_TEXCOORD0);
        vec3 _204 = _200.xyz;
        _204.x = 1.0 - _200.x;
        _209 = varying_TBN * normalize((_204 * 2.0) - vec3(1.0));
    }
    else
    {
        _209 = varying_NORMAL0;
    }
    vec3 _220;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].specularTexIndex != (-1))
    {
        _220 = texture(sampler2D(texArr[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].specularTexIndex)], PSss), varying_TEXCOORD0).xyz;
    }
    else
    {
        _220 = vec3(1.0);
    }
    vec3 _229 = normalize(varying_TOCAM);
    vec3 _230 = normalize(_209);
    vec3 _231 = normalize(VSSystemCBuffer.dirLight.direction);
    float _234 = clamp(dot(_230, -_231), 0.0, 1.0);
    vec3 _244;
    if (_234 > 0.0)
    {
        _244 = _220 * pow(clamp(dot(_229, normalize(reflect(_231, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _244 = vec3(0.0);
    }
    vec3 _256 = normalize(_90[0].fragToLight);
    float _258 = clamp(dot(_230, _256), 0.0, 1.0);
    vec3 _269;
    if (_258 > 0.0)
    {
        _269 = _220 * pow(clamp(dot(_229, normalize(reflect(-_256, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _269 = vec3(0.0);
    }
    float _270 = length(_90[0].fragToLight);
    vec3 _292 = normalize(_90[1].fragToLight);
    float _294 = clamp(dot(_230, _292), 0.0, 1.0);
    vec3 _305;
    if (_294 > 0.0)
    {
        _305 = _220 * pow(clamp(dot(_229, normalize(reflect(-_292, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _305 = vec3(0.0);
    }
    float _306 = length(_90[1].fragToLight);
    vec3 _328 = normalize(_90[2].fragToLight);
    float _330 = clamp(dot(_230, _328), 0.0, 1.0);
    vec3 _341;
    if (_330 > 0.0)
    {
        _341 = _220 * pow(clamp(dot(_229, normalize(reflect(-_328, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _341 = vec3(0.0);
    }
    float _342 = length(_90[2].fragToLight);
    vec3 _364 = normalize(_90[3].fragToLight);
    float _366 = clamp(dot(_230, _364), 0.0, 1.0);
    vec3 _377;
    if (_366 > 0.0)
    {
        _377 = _220 * pow(clamp(dot(_229, normalize(reflect(-_364, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _377 = vec3(0.0);
    }
    float _378 = length(_90[3].fragToLight);
    vec3 _400 = normalize(_90[4].fragToLight);
    float _402 = clamp(dot(_230, _400), 0.0, 1.0);
    vec3 _413;
    if (_402 > 0.0)
    {
        _413 = _220 * pow(clamp(dot(_229, normalize(reflect(-_400, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _413 = vec3(0.0);
    }
    float _414 = length(_90[4].fragToLight);
    vec3 _436 = normalize(_90[5].fragToLight);
    float _438 = clamp(dot(_230, _436), 0.0, 1.0);
    vec3 _449;
    if (_438 > 0.0)
    {
        _449 = _220 * pow(clamp(dot(_229, normalize(reflect(-_436, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _449 = vec3(0.0);
    }
    float _450 = length(_90[5].fragToLight);
    vec3 _472 = normalize(_90[6].fragToLight);
    float _474 = clamp(dot(_230, _472), 0.0, 1.0);
    vec3 _485;
    if (_474 > 0.0)
    {
        _485 = _220 * pow(clamp(dot(_229, normalize(reflect(-_472, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _485 = vec3(0.0);
    }
    float _486 = length(_90[6].fragToLight);
    vec3 _508 = normalize(_90[7].fragToLight);
    float _510 = clamp(dot(_230, _508), 0.0, 1.0);
    vec3 _521;
    if (_510 > 0.0)
    {
        _521 = _220 * pow(clamp(dot(_229, normalize(reflect(-_508, _230))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _521 = vec3(0.0);
    }
    float _522 = length(_90[7].fragToLight);
    out_var_SV_Target0 = vec4(clamp(((vec3(0.300000011920928955078125) * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ka) * _191.xyz) + (((((((((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _234) + (_244 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * VSSystemCBuffer.dirLight.color) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _258) + (_269 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[0].color) * ((pow(clamp(1.0 - pow(_270 / _90[0].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_270 * _270) + 1.0)) * _90[0].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _294) + (_305 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[1].color) * ((pow(clamp(1.0 - pow(_306 / _90[1].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_306 * _306) + 1.0)) * _90[1].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _330) + (_341 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[2].color) * ((pow(clamp(1.0 - pow(_342 / _90[2].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_342 * _342) + 1.0)) * _90[2].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _366) + (_377 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[3].color) * ((pow(clamp(1.0 - pow(_378 / _90[3].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_378 * _378) + 1.0)) * _90[3].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _402) + (_413 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[4].color) * ((pow(clamp(1.0 - pow(_414 / _90[4].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_414 * _414) + 1.0)) * _90[4].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _438) + (_449 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[5].color) * ((pow(clamp(1.0 - pow(_450 / _90[5].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_450 * _450) + 1.0)) * _90[5].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _474) + (_485 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[6].color) * ((pow(clamp(1.0 - pow(_486 / _90[6].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_486 * _486) + 1.0)) * _90[6].intensity))) + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _510) + (_521 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _191.xyz) * _90[7].color) * ((pow(clamp(1.0 - pow(_522 / _90[7].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_522 * _522) + 1.0)) * _90[7].intensity))), vec3(0.0), vec3(1.0)), 1.0);
}

