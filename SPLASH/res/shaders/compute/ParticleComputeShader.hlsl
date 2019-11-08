struct Emitter{
	float3 position;
	float padding0;
	float3 velocity;
	float padding1;
	float3 acceleration;
	int nrOfParticlesToSpawn;
	float spawnTime;
	float3 padding2;
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

struct ParticlePhysics {
	float3 velocity;
	float3 acceleration;
};

RWStructuredBuffer<ParticlePhysics> CSPhysicsBuffer: register(u11) : SAIL_IGNORE;

void createParticle(float3 v0, float3 v1, float3 v2, float3 v3, int particleIndex) {
	int v0Index = particleIndex * 6;
	
	//Triangle 1
	CSOutputBuffer[v0Index].position = v0;
	CSOutputBuffer[v0Index].texCoords = float2(0.f, 0.f);
	CSOutputBuffer[v0Index].normal = 0.f;
	CSOutputBuffer[v0Index].tangent = 0.f;
	CSOutputBuffer[v0Index].bitangent = 0.f;
	
	CSOutputBuffer[v0Index + 1].position = v1;
	CSOutputBuffer[v0Index + 1].texCoords = float2(0.f, 1.f);
	CSOutputBuffer[v0Index + 1].normal = 0.f;
	CSOutputBuffer[v0Index + 1].tangent = 0.f;
	CSOutputBuffer[v0Index + 1].bitangent = 0.f;
	
	CSOutputBuffer[v0Index + 2].position = v2;
	CSOutputBuffer[v0Index + 2].texCoords = float2(1.f, 0.f);
	CSOutputBuffer[v0Index + 2].normal = 0.f;
	CSOutputBuffer[v0Index + 2].tangent = 0.f;
	CSOutputBuffer[v0Index + 2].bitangent = 0.f;
	
	//Triangle 2
	CSOutputBuffer[v0Index + 3].position = v2;
	CSOutputBuffer[v0Index + 3].texCoords = float2(1.f, 0.f);
	CSOutputBuffer[v0Index + 3].normal = 0.f;
	CSOutputBuffer[v0Index + 3].tangent = 0.f;
	CSOutputBuffer[v0Index + 3].bitangent = 0.f;
	
	CSOutputBuffer[v0Index + 4].position = v1;
	CSOutputBuffer[v0Index + 4].texCoords = float2(0.f, 1.f);
	CSOutputBuffer[v0Index + 4].normal = 0.f;
	CSOutputBuffer[v0Index + 4].tangent = 0.f;
	CSOutputBuffer[v0Index + 4].bitangent = 0.f;
	
	CSOutputBuffer[v0Index + 5].position = v3;
	CSOutputBuffer[v0Index + 5].texCoords = float2(1.f, 1.f);
	CSOutputBuffer[v0Index + 5].normal = 0.f;
	CSOutputBuffer[v0Index + 5].tangent = 0.f;
	CSOutputBuffer[v0Index + 5].bitangent = 0.f;
}

void updatePhysics(int particleIndex, float dt) {
	float3 oldVelocity = CSPhysicsBuffer[particleIndex].velocity;
	CSPhysicsBuffer[particleIndex].velocity += CSPhysicsBuffer[particleIndex].acceleration * dt;
	for (uint j = 0 ; j < 6; j++) {
		CSOutputBuffer[particleIndex * 6 + j].position += (oldVelocity + CSPhysicsBuffer[particleIndex].velocity) * 0.5 * dt;
	}
}

[numthreads(1, 1, 1)]
void CSMain(int3 groupThreadID : SV_GroupThreadID, int3 dispatchThreadID : SV_DispatchThreadID) {
	int counter = 0; // New particle counter
    for (uint i = 0; i < inputBuffer.numEmitters; i++) {
		int particleBufferSizeLeft = (inputBuffer.maxOutputVertices / 6) - (inputBuffer.numPrevParticles + counter);
		int particlesToSpawn = inputBuffer.emitters[i].nrOfParticlesToSpawn;
		if (inputBuffer.emitters[i].nrOfParticlesToSpawn > particleBufferSizeLeft) {
			particlesToSpawn = particleBufferSizeLeft;
		}
		for (uint j = 0; j < particlesToSpawn; j++) {
			float3 v0, v1, v2, v3;
			
			v0 = inputBuffer.emitters[i].position + float3(-0.1, 0.1, 0.0);
			v1 = inputBuffer.emitters[i].position + float3(-0.1, -0.1, 0.0);
			v2 = inputBuffer.emitters[i].position + float3(0.1, 0.1, 0.0);
			v3 = inputBuffer.emitters[i].position + float3(0.1, -0.1, 0.0);
			
			CSPhysicsBuffer[inputBuffer.numPrevParticles + counter].acceleration = inputBuffer.emitters[i].acceleration;
			CSPhysicsBuffer[inputBuffer.numPrevParticles + counter].velocity = inputBuffer.emitters[i].velocity;
			
			createParticle(v0, v1, v2, v3, inputBuffer.numPrevParticles + counter);
			
			//Physics for new particle (accounts for how far into the frame the particle was created)
			updatePhysics(inputBuffer.numPrevParticles + counter, inputBuffer.emitters[i].spawnTime);
			
			counter++;
		}
		
		if (particlesToSpawn == particleBufferSizeLeft) {
			i = inputBuffer.numEmitters; // Break because there is no more space for particles in the buffers
		}
	}
	
	//Physics for all prevous particles
	for (uint i = 0; i < inputBuffer.numPrevParticles; i++) {
		updatePhysics(i, inputBuffer.frameTime);
	}
}
