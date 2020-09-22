#version 450
#extension GL_EXT_nonuniform_qualifier : require

out gl_PerVertex
{
    vec4 gl_Position;
};

layout(set = 0, binding = 0, std140) uniform type_VSSystemCBuffer
{
    mat4 sys_mView;
    mat4 sys_mProjection;
} VSSystemCBuffer;

layout(location = 0) in vec4 in_var_POSITION;
layout(location = 0) out vec3 varying_TEXCOORD;

void main()
{
    vec3 _27 = in_var_POSITION.xyz * 10.0;
    vec4 _29 = vec4(_27.x, _27.y, _27.z, in_var_POSITION.w);
    _29.w = 0.0;
    vec4 _33 = _29 * VSSystemCBuffer.sys_mView;
    _33.w = 1.0;
    vec4 _37 = _33 * VSSystemCBuffer.sys_mProjection;
    _37.z = 1.0;
    gl_Position = _37;
    varying_TEXCOORD = in_var_POSITION.xyz;
}

