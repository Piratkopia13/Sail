#version 450
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require

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
    int diffuseTexIndex;
    int normalTexIndex;
    int specularTexIndex;
    float padding;
};

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

layout(set = 0, binding = 1, std140) uniform type_VSPSMaterials
{
    PhongMaterial sys_materials[2];
} VSPSMaterials;

layout(set = 0, binding = 0, std140) uniform type_VSPSSystemCBuffer
{
    mat4 sys_mVP;
    vec4 sys_clippingPlane;
    vec3 sys_cameraPos;
    float padding;
    DirectionalLight dirLight;
    PointLight pointLights[2];
} VSPSSystemCBuffer;

layout(push_constant, scalar) uniform type_PushConstant_
{
    mat4 sys_mWorld;
    uint sys_materialIndex;
    vec3 padding;
} VSPSConsts;

layout(location = 0) in vec4 in_var_POSITION0;
layout(location = 1) in vec2 in_var_TEXCOORD0;
layout(location = 2) in vec3 in_var_NORMAL0;
layout(location = 3) in vec3 in_var_TANGENT0;
layout(location = 4) in vec3 in_var_BINORMAL0;
layout(location = 0) out vec3 varying_NORMAL0;
layout(location = 1) out vec2 varying_TEXCOORD0;
layout(location = 2) out vec3 varying_TOCAM;
layout(location = 3) out mat3 varying_TBN;

mat3 _63;

void main()
{
    vec4 _77 = in_var_POSITION0;
    _77.w = 1.0;
    vec4 _78 = _77 * VSPSConsts.sys_mWorld;
    mat3 _107;
    if (VSPSMaterials.sys_materials[VSPSConsts.sys_materialIndex].normalTexIndex != (-1))
    {
        mat3 _98 = mat3(VSPSConsts.sys_mWorld[0].xyz, VSPSConsts.sys_mWorld[1].xyz, VSPSConsts.sys_mWorld[2].xyz);
        _107 = transpose(mat3(normalize(in_var_TANGENT0) * _98, normalize(in_var_BINORMAL0) * _98, normalize(in_var_NORMAL0) * _98));
    }
    else
    {
        _107 = _63;
    }
    gl_Position = _78 * VSPSSystemCBuffer.sys_mVP;
    varying_NORMAL0 = normalize(in_var_NORMAL0 * mat3(VSPSConsts.sys_mWorld[0].xyz, VSPSConsts.sys_mWorld[1].xyz, VSPSConsts.sys_mWorld[2].xyz));
    varying_TEXCOORD0 = in_var_TEXCOORD0;
    gl_ClipDistance[0u] = dot(_78, VSPSSystemCBuffer.sys_clippingPlane);
    varying_TOCAM = VSPSSystemCBuffer.sys_cameraPos - _78.xyz;
    varying_TBN = _107;
}

