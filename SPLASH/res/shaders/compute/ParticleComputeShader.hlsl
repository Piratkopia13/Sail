struct Emitter{
	float3 position;
	float padding0;
	float3 velocity;
	float padding1;
	float3 acceleration;
	int nrOfParticlesToSpawn;
};

struct ParticleInput{
	Emitter emitters[100];
	uint numEmitters;
	uint numPrevParticles;
	uint maxOutputVertices;
	float frameTime;
};

cbuffer CSInputBuffer : register(b0) {
	ParticleInput inputBuffer;
}

struct Vertex { // Size of this type is hardcoded in ShaderPipeline.cpp - change this if struct changes
	float3 position;
	float2 texCoords;
	float3 normal;
	float3 tangent;
	float3 bitangent;
};

RWStructuredBuffer<Vertex> CSOutputBuffer: register(u10);

void createTriangle(float3 v0, float3 v1, float3 v2, int v0Index) {
	CSOutputBuffer[v0Index].position = v0;
	CSOutputBuffer[v0Index].texCoords = 0.f;
	CSOutputBuffer[v0Index].normal = 0.f;
	CSOutputBuffer[v0Index].tangent = 0.f;
	CSOutputBuffer[v0Index].bitangent = 0.f;
	
	CSOutputBuffer[v0Index + 1].position = v1;
	CSOutputBuffer[v0Index + 1].texCoords = 0.f;
	CSOutputBuffer[v0Index + 1].normal = 0.f;
	CSOutputBuffer[v0Index + 1].tangent = 0.f;
	CSOutputBuffer[v0Index + 1].bitangent = 0.f;
	
	CSOutputBuffer[v0Index + 2].position = v2;
	CSOutputBuffer[v0Index + 2].texCoords = 0.f;
	CSOutputBuffer[v0Index + 2].normal = 0.f;
	CSOutputBuffer[v0Index + 2].tangent = 0.f;
	CSOutputBuffer[v0Index + 2].bitangent = 0.f;
}

[numthreads(1, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID) {
	int counter = 0;
    for (uint i = 0; i < inputBuffer.numEmitters; i++) {
		for (uint j = 0; j < min(inputBuffer.emitters[i].nrOfParticlesToSpawn, inputBuffer.maxOutputVertices - (inputBuffer.numPrevParticles * 6 + counter)); j++) {
			float3 v0, v1, v2, v3;
			
			v0 = inputBuffer.emitters[i].position + float3(-0.1 + 0.3 * (inputBuffer.numPrevParticles + (counter / 6)), 0.1, 0.0);
			v1 = inputBuffer.emitters[i].position + float3(-0.1 + 0.3 * (inputBuffer.numPrevParticles + (counter / 6)), -0.1, 0.0);
			v2 = inputBuffer.emitters[i].position + float3(0.1 + 0.3 * (inputBuffer.numPrevParticles + (counter / 6)), 0.1, 0.0);
			v3 = inputBuffer.emitters[i].position + float3(0.1 + 0.3 * (inputBuffer.numPrevParticles + (counter / 6)), -0.1, 0.0);
			
			createTriangle(v0, v1, v2, inputBuffer.numPrevParticles * 6 + counter);
			counter += 3;
			createTriangle(v2, v1, v3, inputBuffer.numPrevParticles * 6 + counter);
			counter += 3;
		}
	}
	
	for (uint i = 0; i < inputBuffer.numPrevParticles * 6 + counter; i++) {
		CSOutputBuffer[i].position.x += 1.0f * inputBuffer.frameTime;
	}
}
