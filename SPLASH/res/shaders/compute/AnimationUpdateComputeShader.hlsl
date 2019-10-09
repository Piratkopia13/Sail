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

StructuredBuffer<float4x4> CSTransforms : register(t0); // animationComponent->transforms
StructuredBuffer<VertConnections> CSVertConnections : register(t1); // connections
StructuredBuffer<Vertex> CSTPoseVertices : register(t2) : SAIL_IGNORE; // T-Pose vertex buffer

RWStructuredBuffer<Vertex> CSOuputVertices: register(u10) : SAIL_IGNORE; // Output animated vertex buffer

struct ComputeShaderInput {
	uint3 GroupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
	uint3 GroupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
	uint3 DispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
	uint  GroupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};

[numthreads(1, 1, 1)]
void CSMain(ComputeShaderInput input) {
    
	const unsigned int index = input.DispatchThreadID.x;

	float4x4 posMat = 0.f;
	float4x4 normalMat = 0.f;

	const unsigned int count = CSVertConnections[index].count;
	for (unsigned int countIndex = 0; countIndex < count; countIndex++) {
		posMat += CSTransforms[CSVertConnections[index].transform[countIndex]] * CSVertConnections[index].weight[countIndex];
	}
	posMat = transpose(posMat); // TODO check if inverse is needed as well

	CSOuputVertices[index].position = mul(posMat, float4(CSTPoseVertices[index].position, 1.0f));
	CSOuputVertices[index].normal = mul(normalMat, float4(CSTPoseVertices[index].normal, 0.0f));
	CSOuputVertices[index].tangent = mul(normalMat, float4(CSTPoseVertices[index].tangent, 0.0f));
	CSOuputVertices[index].bitangent = mul(normalMat, float4(CSTPoseVertices[index].bitangent, 0.0f));

}
