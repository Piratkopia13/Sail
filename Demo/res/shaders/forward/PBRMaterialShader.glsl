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

layout(set = 0, binding = 0, std140) uniform type_VSPSSystemCBuffer
{
    mat4 sys_mVP;
    vec3 sys_cameraPos;
    float padding;
    vec4 sys_clippingPlane;
    DirectionalLight dirLight;
    PointLight pointLights[8];
} VSPSSystemCBuffer;

layout(set = 0, binding = 6) uniform sampler PSss;
layout(set = 0, binding = 3) uniform textureCube texCubeArr[];

layout(location = 0) in vec3 in_var_NORMAL0;
layout(location = 1) in vec2 in_var_TEXCOORD0;
layout(location = 2) in vec3 varying_worldPos;
layout(location = 3) in mat3 in_var_TBN;
layout(location = 0) out vec4 out_var_SV_Target0;

void main()
{
    out_var_SV_Target0 = texture(samplerCube(texCubeArr[0], PSss), normalize(varying_worldPos - VSPSSystemCBuffer.sys_cameraPos));
}

