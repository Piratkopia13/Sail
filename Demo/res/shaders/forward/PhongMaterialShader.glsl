#version 450
#extension GL_EXT_nonuniform_qualifier : require

out gl_PerVertex
{
    vec4 gl_Position;
    float gl_ClipDistance[1];
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

layout(set = 0, binding = 1, std140) uniform type_ConstantBuffer_PhongMaterial
{
    vec4 modelColor;
    float ka;
    float kd;
    float ks;
    float shininess;
    uint hasDiffuseTexture;
    uint hasNormalTexture;
    uint hasSpecularTexture;
    float padding;
} VSPSMaterials[];

layout(push_constant, std430) uniform type_PushConstant_
{
    mat4 sys_mWorld;
    uint materialIndex;
} VSPCTest;

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

float _68;
vec3 _69;

void main()
{
    vec4 _79 = in_var_POSITION0;
    _79.w = 1.0;
    vec4 _80 = _79 * VSPCTest.sys_mWorld;
    vec3 _87 = VSPSSystemCBuffer.sys_cameraPos - _80.xyz;
    vec3 _112;
    if (VSPSMaterials[0].hasNormalTexture != 0u)
    {
        mat3 _102 = mat3(VSPCTest.sys_mWorld[0].xyz, VSPCTest.sys_mWorld[1].xyz, VSPCTest.sys_mWorld[2].xyz);
        _112 = transpose(mat3(normalize(in_var_TANGENT0) * _102, normalize(in_var_BINORMAL0) * _102, normalize(in_var_NORMAL0) * _102)) * _87;
    }
    else
    {
        _112 = _87;
    }
    PointLight _122 = PointLight(_69, _68, _69, _68);
    gl_Position = _80 * VSPSSystemCBuffer.sys_mVP;
    varying_NORMAL0 = normalize(in_var_NORMAL0 * mat3(VSPCTest.sys_mWorld[0].xyz, VSPCTest.sys_mWorld[1].xyz, VSPCTest.sys_mWorld[2].xyz));
    varying_TEXCOORD0 = in_var_TEXCOORD0;
    gl_ClipDistance[0u] = dot(_80, VSPSSystemCBuffer.sys_clippingPlane);
    varying_TOCAM = _112;
    varying_ASD = uint(gl_VertexIndex);
    varying_LIGHTS4 = PointLight[](_122, _122);
}

