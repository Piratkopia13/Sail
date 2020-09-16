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
    PointLight pointLights[64];
} VSSystemCBuffer;

layout(set = 0, binding = 1, std140) uniform type_VSPSMaterials
{
    PhongMaterial sys_materials[10];
} VSPSMaterials;

layout(push_constant, scalar) uniform type_PushConstant_
{
    mat4 sys_mWorld;
    uint sys_materialIndex;
    vec3 padding;
} VSPSConsts;

uniform sampler2D SPIRV_Cross_CombinedtexArrPSss[];

layout(location = 0) in vec3 varying_NORMAL0;
layout(location = 1) in vec2 varying_TEXCOORD0;
layout(location = 2) in vec3 varying_WORLDPOS;
layout(location = 3) in vec3 varying_TOCAM;
layout(location = 4) in mat3 varying_TBN;
layout(location = 0) out vec4 out_var_SV_Target0;

void main()
{
    PointLight _81[64] = PointLight[](PointLight(VSSystemCBuffer.pointLights[0].color, VSSystemCBuffer.pointLights[0].attRadius, VSSystemCBuffer.pointLights[0].fragToLight, VSSystemCBuffer.pointLights[0].intensity), PointLight(VSSystemCBuffer.pointLights[1].color, VSSystemCBuffer.pointLights[1].attRadius, VSSystemCBuffer.pointLights[1].fragToLight, VSSystemCBuffer.pointLights[1].intensity), PointLight(VSSystemCBuffer.pointLights[2].color, VSSystemCBuffer.pointLights[2].attRadius, VSSystemCBuffer.pointLights[2].fragToLight, VSSystemCBuffer.pointLights[2].intensity), PointLight(VSSystemCBuffer.pointLights[3].color, VSSystemCBuffer.pointLights[3].attRadius, VSSystemCBuffer.pointLights[3].fragToLight, VSSystemCBuffer.pointLights[3].intensity), PointLight(VSSystemCBuffer.pointLights[4].color, VSSystemCBuffer.pointLights[4].attRadius, VSSystemCBuffer.pointLights[4].fragToLight, VSSystemCBuffer.pointLights[4].intensity), PointLight(VSSystemCBuffer.pointLights[5].color, VSSystemCBuffer.pointLights[5].attRadius, VSSystemCBuffer.pointLights[5].fragToLight, VSSystemCBuffer.pointLights[5].intensity), PointLight(VSSystemCBuffer.pointLights[6].color, VSSystemCBuffer.pointLights[6].attRadius, VSSystemCBuffer.pointLights[6].fragToLight, VSSystemCBuffer.pointLights[6].intensity), PointLight(VSSystemCBuffer.pointLights[7].color, VSSystemCBuffer.pointLights[7].attRadius, VSSystemCBuffer.pointLights[7].fragToLight, VSSystemCBuffer.pointLights[7].intensity), PointLight(VSSystemCBuffer.pointLights[8].color, VSSystemCBuffer.pointLights[8].attRadius, VSSystemCBuffer.pointLights[8].fragToLight, VSSystemCBuffer.pointLights[8].intensity), PointLight(VSSystemCBuffer.pointLights[9].color, VSSystemCBuffer.pointLights[9].attRadius, VSSystemCBuffer.pointLights[9].fragToLight, VSSystemCBuffer.pointLights[9].intensity), PointLight(VSSystemCBuffer.pointLights[10].color, VSSystemCBuffer.pointLights[10].attRadius, VSSystemCBuffer.pointLights[10].fragToLight, VSSystemCBuffer.pointLights[10].intensity), PointLight(VSSystemCBuffer.pointLights[11].color, VSSystemCBuffer.pointLights[11].attRadius, VSSystemCBuffer.pointLights[11].fragToLight, VSSystemCBuffer.pointLights[11].intensity), PointLight(VSSystemCBuffer.pointLights[12].color, VSSystemCBuffer.pointLights[12].attRadius, VSSystemCBuffer.pointLights[12].fragToLight, VSSystemCBuffer.pointLights[12].intensity), PointLight(VSSystemCBuffer.pointLights[13].color, VSSystemCBuffer.pointLights[13].attRadius, VSSystemCBuffer.pointLights[13].fragToLight, VSSystemCBuffer.pointLights[13].intensity), PointLight(VSSystemCBuffer.pointLights[14].color, VSSystemCBuffer.pointLights[14].attRadius, VSSystemCBuffer.pointLights[14].fragToLight, VSSystemCBuffer.pointLights[14].intensity), PointLight(VSSystemCBuffer.pointLights[15].color, VSSystemCBuffer.pointLights[15].attRadius, VSSystemCBuffer.pointLights[15].fragToLight, VSSystemCBuffer.pointLights[15].intensity), PointLight(VSSystemCBuffer.pointLights[16].color, VSSystemCBuffer.pointLights[16].attRadius, VSSystemCBuffer.pointLights[16].fragToLight, VSSystemCBuffer.pointLights[16].intensity), PointLight(VSSystemCBuffer.pointLights[17].color, VSSystemCBuffer.pointLights[17].attRadius, VSSystemCBuffer.pointLights[17].fragToLight, VSSystemCBuffer.pointLights[17].intensity), PointLight(VSSystemCBuffer.pointLights[18].color, VSSystemCBuffer.pointLights[18].attRadius, VSSystemCBuffer.pointLights[18].fragToLight, VSSystemCBuffer.pointLights[18].intensity), PointLight(VSSystemCBuffer.pointLights[19].color, VSSystemCBuffer.pointLights[19].attRadius, VSSystemCBuffer.pointLights[19].fragToLight, VSSystemCBuffer.pointLights[19].intensity), PointLight(VSSystemCBuffer.pointLights[20].color, VSSystemCBuffer.pointLights[20].attRadius, VSSystemCBuffer.pointLights[20].fragToLight, VSSystemCBuffer.pointLights[20].intensity), PointLight(VSSystemCBuffer.pointLights[21].color, VSSystemCBuffer.pointLights[21].attRadius, VSSystemCBuffer.pointLights[21].fragToLight, VSSystemCBuffer.pointLights[21].intensity), PointLight(VSSystemCBuffer.pointLights[22].color, VSSystemCBuffer.pointLights[22].attRadius, VSSystemCBuffer.pointLights[22].fragToLight, VSSystemCBuffer.pointLights[22].intensity), PointLight(VSSystemCBuffer.pointLights[23].color, VSSystemCBuffer.pointLights[23].attRadius, VSSystemCBuffer.pointLights[23].fragToLight, VSSystemCBuffer.pointLights[23].intensity), PointLight(VSSystemCBuffer.pointLights[24].color, VSSystemCBuffer.pointLights[24].attRadius, VSSystemCBuffer.pointLights[24].fragToLight, VSSystemCBuffer.pointLights[24].intensity), PointLight(VSSystemCBuffer.pointLights[25].color, VSSystemCBuffer.pointLights[25].attRadius, VSSystemCBuffer.pointLights[25].fragToLight, VSSystemCBuffer.pointLights[25].intensity), PointLight(VSSystemCBuffer.pointLights[26].color, VSSystemCBuffer.pointLights[26].attRadius, VSSystemCBuffer.pointLights[26].fragToLight, VSSystemCBuffer.pointLights[26].intensity), PointLight(VSSystemCBuffer.pointLights[27].color, VSSystemCBuffer.pointLights[27].attRadius, VSSystemCBuffer.pointLights[27].fragToLight, VSSystemCBuffer.pointLights[27].intensity), PointLight(VSSystemCBuffer.pointLights[28].color, VSSystemCBuffer.pointLights[28].attRadius, VSSystemCBuffer.pointLights[28].fragToLight, VSSystemCBuffer.pointLights[28].intensity), PointLight(VSSystemCBuffer.pointLights[29].color, VSSystemCBuffer.pointLights[29].attRadius, VSSystemCBuffer.pointLights[29].fragToLight, VSSystemCBuffer.pointLights[29].intensity), PointLight(VSSystemCBuffer.pointLights[30].color, VSSystemCBuffer.pointLights[30].attRadius, VSSystemCBuffer.pointLights[30].fragToLight, VSSystemCBuffer.pointLights[30].intensity), PointLight(VSSystemCBuffer.pointLights[31].color, VSSystemCBuffer.pointLights[31].attRadius, VSSystemCBuffer.pointLights[31].fragToLight, VSSystemCBuffer.pointLights[31].intensity), PointLight(VSSystemCBuffer.pointLights[32].color, VSSystemCBuffer.pointLights[32].attRadius, VSSystemCBuffer.pointLights[32].fragToLight, VSSystemCBuffer.pointLights[32].intensity), PointLight(VSSystemCBuffer.pointLights[33].color, VSSystemCBuffer.pointLights[33].attRadius, VSSystemCBuffer.pointLights[33].fragToLight, VSSystemCBuffer.pointLights[33].intensity), PointLight(VSSystemCBuffer.pointLights[34].color, VSSystemCBuffer.pointLights[34].attRadius, VSSystemCBuffer.pointLights[34].fragToLight, VSSystemCBuffer.pointLights[34].intensity), PointLight(VSSystemCBuffer.pointLights[35].color, VSSystemCBuffer.pointLights[35].attRadius, VSSystemCBuffer.pointLights[35].fragToLight, VSSystemCBuffer.pointLights[35].intensity), PointLight(VSSystemCBuffer.pointLights[36].color, VSSystemCBuffer.pointLights[36].attRadius, VSSystemCBuffer.pointLights[36].fragToLight, VSSystemCBuffer.pointLights[36].intensity), PointLight(VSSystemCBuffer.pointLights[37].color, VSSystemCBuffer.pointLights[37].attRadius, VSSystemCBuffer.pointLights[37].fragToLight, VSSystemCBuffer.pointLights[37].intensity), PointLight(VSSystemCBuffer.pointLights[38].color, VSSystemCBuffer.pointLights[38].attRadius, VSSystemCBuffer.pointLights[38].fragToLight, VSSystemCBuffer.pointLights[38].intensity), PointLight(VSSystemCBuffer.pointLights[39].color, VSSystemCBuffer.pointLights[39].attRadius, VSSystemCBuffer.pointLights[39].fragToLight, VSSystemCBuffer.pointLights[39].intensity), PointLight(VSSystemCBuffer.pointLights[40].color, VSSystemCBuffer.pointLights[40].attRadius, VSSystemCBuffer.pointLights[40].fragToLight, VSSystemCBuffer.pointLights[40].intensity), PointLight(VSSystemCBuffer.pointLights[41].color, VSSystemCBuffer.pointLights[41].attRadius, VSSystemCBuffer.pointLights[41].fragToLight, VSSystemCBuffer.pointLights[41].intensity), PointLight(VSSystemCBuffer.pointLights[42].color, VSSystemCBuffer.pointLights[42].attRadius, VSSystemCBuffer.pointLights[42].fragToLight, VSSystemCBuffer.pointLights[42].intensity), PointLight(VSSystemCBuffer.pointLights[43].color, VSSystemCBuffer.pointLights[43].attRadius, VSSystemCBuffer.pointLights[43].fragToLight, VSSystemCBuffer.pointLights[43].intensity), PointLight(VSSystemCBuffer.pointLights[44].color, VSSystemCBuffer.pointLights[44].attRadius, VSSystemCBuffer.pointLights[44].fragToLight, VSSystemCBuffer.pointLights[44].intensity), PointLight(VSSystemCBuffer.pointLights[45].color, VSSystemCBuffer.pointLights[45].attRadius, VSSystemCBuffer.pointLights[45].fragToLight, VSSystemCBuffer.pointLights[45].intensity), PointLight(VSSystemCBuffer.pointLights[46].color, VSSystemCBuffer.pointLights[46].attRadius, VSSystemCBuffer.pointLights[46].fragToLight, VSSystemCBuffer.pointLights[46].intensity), PointLight(VSSystemCBuffer.pointLights[47].color, VSSystemCBuffer.pointLights[47].attRadius, VSSystemCBuffer.pointLights[47].fragToLight, VSSystemCBuffer.pointLights[47].intensity), PointLight(VSSystemCBuffer.pointLights[48].color, VSSystemCBuffer.pointLights[48].attRadius, VSSystemCBuffer.pointLights[48].fragToLight, VSSystemCBuffer.pointLights[48].intensity), PointLight(VSSystemCBuffer.pointLights[49].color, VSSystemCBuffer.pointLights[49].attRadius, VSSystemCBuffer.pointLights[49].fragToLight, VSSystemCBuffer.pointLights[49].intensity), PointLight(VSSystemCBuffer.pointLights[50].color, VSSystemCBuffer.pointLights[50].attRadius, VSSystemCBuffer.pointLights[50].fragToLight, VSSystemCBuffer.pointLights[50].intensity), PointLight(VSSystemCBuffer.pointLights[51].color, VSSystemCBuffer.pointLights[51].attRadius, VSSystemCBuffer.pointLights[51].fragToLight, VSSystemCBuffer.pointLights[51].intensity), PointLight(VSSystemCBuffer.pointLights[52].color, VSSystemCBuffer.pointLights[52].attRadius, VSSystemCBuffer.pointLights[52].fragToLight, VSSystemCBuffer.pointLights[52].intensity), PointLight(VSSystemCBuffer.pointLights[53].color, VSSystemCBuffer.pointLights[53].attRadius, VSSystemCBuffer.pointLights[53].fragToLight, VSSystemCBuffer.pointLights[53].intensity), PointLight(VSSystemCBuffer.pointLights[54].color, VSSystemCBuffer.pointLights[54].attRadius, VSSystemCBuffer.pointLights[54].fragToLight, VSSystemCBuffer.pointLights[54].intensity), PointLight(VSSystemCBuffer.pointLights[55].color, VSSystemCBuffer.pointLights[55].attRadius, VSSystemCBuffer.pointLights[55].fragToLight, VSSystemCBuffer.pointLights[55].intensity), PointLight(VSSystemCBuffer.pointLights[56].color, VSSystemCBuffer.pointLights[56].attRadius, VSSystemCBuffer.pointLights[56].fragToLight, VSSystemCBuffer.pointLights[56].intensity), PointLight(VSSystemCBuffer.pointLights[57].color, VSSystemCBuffer.pointLights[57].attRadius, VSSystemCBuffer.pointLights[57].fragToLight, VSSystemCBuffer.pointLights[57].intensity), PointLight(VSSystemCBuffer.pointLights[58].color, VSSystemCBuffer.pointLights[58].attRadius, VSSystemCBuffer.pointLights[58].fragToLight, VSSystemCBuffer.pointLights[58].intensity), PointLight(VSSystemCBuffer.pointLights[59].color, VSSystemCBuffer.pointLights[59].attRadius, VSSystemCBuffer.pointLights[59].fragToLight, VSSystemCBuffer.pointLights[59].intensity), PointLight(VSSystemCBuffer.pointLights[60].color, VSSystemCBuffer.pointLights[60].attRadius, VSSystemCBuffer.pointLights[60].fragToLight, VSSystemCBuffer.pointLights[60].intensity), PointLight(VSSystemCBuffer.pointLights[61].color, VSSystemCBuffer.pointLights[61].attRadius, VSSystemCBuffer.pointLights[61].fragToLight, VSSystemCBuffer.pointLights[61].intensity), PointLight(VSSystemCBuffer.pointLights[62].color, VSSystemCBuffer.pointLights[62].attRadius, VSSystemCBuffer.pointLights[62].fragToLight, VSSystemCBuffer.pointLights[62].intensity), PointLight(VSSystemCBuffer.pointLights[63].color, VSSystemCBuffer.pointLights[63].attRadius, VSSystemCBuffer.pointLights[63].fragToLight, VSSystemCBuffer.pointLights[63].intensity));
    for (int _487 = 0; _487 < 64; )
    {
        _81[_487].fragToLight -= varying_WORLDPOS;
        _487++;
        continue;
    }
    vec4 _510;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].diffuseTexIndex != (-1))
    {
        _510 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor * texture(SPIRV_Cross_CombinedtexArrPSss[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].diffuseTexIndex)], varying_TEXCOORD0);
    }
    else
    {
        _510 = VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].modelColor;
    }
    vec3 _528;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex != (-1))
    {
        vec4 _519 = texture(SPIRV_Cross_CombinedtexArrPSss[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex)], varying_TEXCOORD0);
        vec3 _523 = _519.xyz;
        _523.x = 1.0 - _519.x;
        _528 = varying_TBN * normalize((_523 * 2.0) - vec3(1.0));
    }
    else
    {
        _528 = varying_NORMAL0;
    }
    vec3 _539;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].specularTexIndex != (-1))
    {
        _539 = texture(SPIRV_Cross_CombinedtexArrPSss[uint(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].specularTexIndex)], varying_TEXCOORD0).xyz;
    }
    else
    {
        _539 = vec3(1.0);
    }
    PointLight _80[64] = _81;
    vec3 _540 = normalize(varying_TOCAM);
    vec3 _541 = normalize(_528);
    vec3 _542 = normalize(VSSystemCBuffer.dirLight.direction);
    float _545 = clamp(dot(_541, -_542), 0.0, 1.0);
    vec3 _555;
    if (_545 > 0.0)
    {
        _555 = _539 * pow(clamp(dot(_540, normalize(reflect(_542, _541))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
    }
    else
    {
        _555 = vec3(0.0);
    }
    vec3 _564;
    _564 = ((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _545) + (_555 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _510.xyz) * VSSystemCBuffer.dirLight.color;
    vec3 _565;
    for (int _567 = 0; _567 < 64; _564 = _565, _567++)
    {
        vec3 _578 = normalize(_80[_567].fragToLight);
        float _580 = clamp(dot(_541, _578), 0.0, 1.0);
        vec3 _591;
        if (_580 > 0.0)
        {
            _591 = _539 * pow(clamp(dot(_540, normalize(reflect(-_578, _541))), 0.0, 1.0), VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].shininess);
        }
        else
        {
            _591 = vec3(0.0);
        }
        float _592 = length(_80[_567].fragToLight);
        _565 = _564 + ((((vec3(VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].kd * _580) + (_591 * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ks)) * _510.xyz) * _80[_567].color) * ((pow(clamp(1.0 - pow(_592 / _80[_567].attRadius, 4.0), 0.0, 1.0), 2.0) / ((_592 * _592) + 1.0)) * _80[_567].intensity));
    }
    out_var_SV_Target0 = vec4(clamp(((vec3(0.300000011920928955078125) * VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].ka) * _510.xyz) + _564, vec3(0.0), vec3(1.0)), 1.0);
}

