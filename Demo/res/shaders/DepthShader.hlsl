struct VSIn
{
    float4 position : POSITION0;
    float2 texCoords : TEXCOORD0;
    float3 normal : NORMAL0;
    float3 tangent : TANGENT0;
    float3 bitangent : BINORMAL0;
};

struct PSIn
{
    float4 pos : SV_POSITION;
};

cbuffer modelData
{
    matrix mWorld;
    matrix mVP;
};

PSIn VSMain( VSIn input )
{
    PSIn output;
    
    output.pos = mul(float4(input.position.xyz, 1.f), mWorld);
    output.pos = mul(output.pos, mVP);

    return output;
}

float4 PSMain( PSIn input ) : SV_Target0
{
    return input.pos;
}