#define SAIL_BONES_PER_VERTEX 5
struct Vertex { // Size of this type is hardcoded in ShaderPipeline.cpp - change this if struct changes
	float3 position;
	float2 texCoords;
	float3 normal;
	float3 tangent;
	float3 bitangent;
};
struct VertConnections { // Size of this type is hardcoded in ShaderPipeline.cpp - change this if struct changes
	unsigned int count;
	unsigned int transform[SAIL_BONES_PER_VERTEX];
	float weight[SAIL_BONES_PER_VERTEX];
};

StructuredBuffer<float4x4> CSTransforms : register(t0);
StructuredBuffer<Vertex> CSVertices : register(t1);
StructuredBuffer<VertConnections> CSVertConnections : register(t2);

RWStructuredBuffer<Vertex> CSVertexBuffer: register(u10) : SAIL_IGNORE;

struct ComputeShaderInput {
	uint3 GroupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
	uint3 GroupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
	uint3 DispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
	uint  GroupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};

[numthreads(1, 1, 1)]
void CSMain(ComputeShaderInput input) {
    
	CSVertexBuffer[input.DispatchThreadID.x].position.x += CSVertConnections[0].count + CSTransforms[0][0][0] + CSVertices[0].position.x;

}
