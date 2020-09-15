#version 450
#extension GL_EXT_nonuniform_qualifier : require

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

struct PointLight
{
    vec3 color;
    float attRadius;
    vec3 fragToLight;
    float intensity;
};

layout(set = 0, binding = 0, std140) uniform type_VSPSSystemCBuffer
{
    mat4 sys_mVP;
    vec4 sys_clippingPlane;
    vec3 sys_cameraPos;
} VSPSSystemCBuffer;

layout(set = 0, binding = 1, std140) uniform type_VSPSMaterials
{
    PhongMaterial sys_materials[1024];
} VSPSMaterials;

layout(push_constant, std430) uniform type_PushConstant_
{
    mat4 sys_mWorld;
    uint sys_materialIndex;
} VSPSTest;

layout(location = 0) in vec4 in_var_POSITION0;
layout(location = 1) in vec2 in_var_TEXCOORD0;
layout(location = 2) in vec3 in_var_NORMAL0;
layout(location = 3) in vec3 in_var_TANGENT0;
layout(location = 4) in vec3 in_var_BINORMAL0;
layout(location = 0) out vec3 varying_NORMAL0;
layout(location = 1) out vec2 varying_TEXCOORD0;
layout(location = 2) out vec3 varying_TOCAM;
layout(location = 3) flat out uint varying_ASD;
layout(location = 4) out vec3 out_var_LIGHTS;
layout(location = 5) out float out_var_LIGHTS1;
layout(location = 6) out vec3 out_var_LIGHTS2;
layout(location = 7) out float out_var_LIGHTS3;
layout(location = 8) out PointLight varying_LIGHTS4[2];

float _71;
vec3 _72;

void main()
{
    vec4 _87 = in_var_POSITION0;
    _87.w = 1.0;
    vec4 _88 = _87 * VSPSTest.sys_mWorld;
    vec3 _95 = VSPSSystemCBuffer.sys_cameraPos - _88.xyz;
    vec3 _118;
    if (VSPSMaterials.sys_materials[VSPSTest.sys_materialIndex].normalTexIndex != (-1))
    {
        mat3 _108 = mat3(VSPSTest.sys_mWorld[0].xyz, VSPSTest.sys_mWorld[1].xyz, VSPSTest.sys_mWorld[2].xyz);
        _118 = transpose(mat3(normalize(in_var_TANGENT0) * _108, normalize(in_var_BINORMAL0) * _108, normalize(in_var_NORMAL0) * _108)) * _95;
    }
    else
    {
        _118 = _95;
    }
    PointLight _128 = PointLight(_72, _71, _72, _71);
    gl_Position = _88 * VSPSSystemCBuffer.sys_mVP;
    varying_NORMAL0 = normalize(in_var_NORMAL0 * mat3(VSPSTest.sys_mWorld[0].xyz, VSPSTest.sys_mWorld[1].xyz, VSPSTest.sys_mWorld[2].xyz));
    varying_TEXCOORD0 = in_var_TEXCOORD0;
    gl_ClipDistance[0u] = dot(_88, VSPSSystemCBuffer.sys_clippingPlane);
    varying_TOCAM = _118;
    varying_ASD = uint(gl_VertexIndex);
    varying_LIGHTS4 = PointLight[](_128, _128);
}

