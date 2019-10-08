#define SAIL_BONES_PER_VERTEX 5
struct Vertex {
	float3 position;
	float2 texCoords;
	float3 normal;
	float3 tangent;
	float3 bitangent;
};
struct VertConnections {
	unsigned int count;
	unsigned int transform[SAIL_BONES_PER_VERTEX];
	float weight[SAIL_BONES_PER_VERTEX];
};

StructuredBuffer<float4x4> m_transforms : register();
StructuredBuffer<Vertex> m_vertices : register();
StructuredBuffer<VertConnections> m_vertConnections : register();

RWStructuredBuffer<Vertex> m_vertexBuffer: register();

struct ComputeShaderInput {
	uint3 GroupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
	uint3 GroupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
	uint3 DispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
	uint  GroupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};

[numthreads(1, 1, 1)]
void CSMain(ComputeShaderInput input) {
    
	m_vertexBuffer[dispatchThreadID.xy].texCoords = 0.0f;

}
