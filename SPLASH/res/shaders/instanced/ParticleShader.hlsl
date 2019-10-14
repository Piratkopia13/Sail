struct VSIn {
  float4 position : POSITION0;
  float3 positionOffset : TEXCOORD0;
  float4 color : TEXCOORD1;
  float4 textureOffsets : TEXCOORD2;
  float blendFactor : TEXCOORD3;
};

struct GSIn {
  float4 worldPos : POSITION0;
  float4 color : COLOR0;
  float4 textureOffsets : TEXCOORD0;
  float blendFactor : TEXCOORD1;
};

struct PSIn {
  float4 pos : SV_POSITION;
  float4 color : COLOR0;
  float2 texCoord1 : TEXCOORD0;
  float2 texCoord2 : TEXCOORD1;
  float blendFactor : TEXCOORD2;
};

// =====================
// === VERTEX SHADER === 
// =====================
GSIn VSMain(VSIn input) {
  GSIn output;
    
  output.color = input.color;
  output.worldPos = float4(input.position.xyz + input.positionOffset, 1.f);
  output.blendFactor = input.blendFactor;
  output.textureOffsets = input.textureOffsets;

  return output;
}

cbuffer cameraData : register(b0) {
  matrix mVP;
  float3 camPos;
};

cbuffer SpriteData : register(b1) {
  uint spritesPerRow;
  float scale;
}

float4 format(float3 vec) {
  return mul(float4(vec, 1.f), mVP);
}

// =======================
// === GEOMETRY SHADER === 
// =======================
[maxvertexcount(6)]
void GSMain (point GSIn input[1], inout TriangleStream<PSIn> output) {

  //const float spritesPerRow = 3.f;
  const float texCoord = 1.f / spritesPerRow;

  float halfSize = 0.5f * scale;
  float3 p_pos = input[0].worldPos.xyz;
  float3 p_camPos = camPos;
	
  float3 planeNormal = normalize(p_pos - p_camPos);
  float3 right = cross(planeNormal, float3(0.f, 1.f, 0.f));
  float3 up = cross(right, planeNormal);

  PSIn psin;
  psin.color = input[0].color;
  psin.blendFactor = input[0].blendFactor;
  
  psin.pos = format(p_pos - (right - up) * halfSize);
  psin.texCoord1 = float2(texCoord, 0.f) + input[0].textureOffsets.xy;
  psin.texCoord2 = float2(texCoord, 0.f) + input[0].textureOffsets.zw;
  //psin.color = float4(1.f, 0.f, 0.f, 1.f);
  output.Append(psin);
  psin.pos = format(p_pos - (right + up) * halfSize);
  //psin.color = float4(1.f, 1.f, 0.f, 1.f);
  psin.texCoord1 = float2(texCoord, texCoord) + input[0].textureOffsets.xy;
  psin.texCoord2 = float2(texCoord, texCoord) + input[0].textureOffsets.zw;
  output.Append(psin);
  psin.pos = format(p_pos + (right + up) * halfSize);
  //psin.color = float4(0.f, 1.f, 0.f, 1.f);
  psin.texCoord1 = float2(0.f, 0.f) + input[0].textureOffsets.xy;
  psin.texCoord2 = float2(0.f, 0.f) + input[0].textureOffsets.zw;
  output.Append(psin);
  psin.pos = format(p_pos + (right - up) * halfSize);
  //psin.color = float4(0.f, 0.f, 0.f, 1.f);
  psin.texCoord1 = float2(0.f, texCoord) + input[0].textureOffsets.xy;
  psin.texCoord2 = float2(0.f, texCoord) + input[0].textureOffsets.zw;
  output.Append(psin);

}

Texture2D tex : register(t0);
SamplerState ss : register(s0);

// ====================
// === PIXEL SHADER === 
// ====================
float4 PSMain(PSIn input) : SV_Target0 {
  return lerp(tex.Sample(ss, input.texCoord1), tex.Sample(ss, input.texCoord2), input.blendFactor) * input.color;
  //return float4(1.f, 0.f, 0.f, 0.2f);

}