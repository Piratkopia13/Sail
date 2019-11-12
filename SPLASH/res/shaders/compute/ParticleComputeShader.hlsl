struct Particle{
	float3 position;
	float padding0;
	float3 velocity;
	float padding1;
	float3 acceleration;
	float spawnTime;
};

struct ParticleInput{
	Particle particles[100];
	uint removeParticles[100];
	uint numParticles;
	uint removalCounter; // How many particles is being removed this frame
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
	
	// Triangle 1
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
	
	// Triangle 2
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

void overwriteParticle(int particle1Index, int particle2Index) {
	CSPhysicsBuffer[particle1Index] = CSPhysicsBuffer[particle2Index];
	
	for (uint j = 0 ; j < 6; j++) {
		CSOutputBuffer[particle1Index * 6 + j] = CSOutputBuffer[particle2Index * 6 + j];
	}
}

void updatePhysics(int particleIndex, float dt) {
	float3 oldVelocity = CSPhysicsBuffer[particleIndex].velocity;
	CSPhysicsBuffer[particleIndex].velocity += CSPhysicsBuffer[particleIndex].acceleration * dt;
	for (uint j = 0 ; j < 6; j++) {
		CSOutputBuffer[particleIndex * 6 + j].position += (oldVelocity + CSPhysicsBuffer[particleIndex].velocity) * 0.5 * dt;
	}
}

#define X_THREADS 8
#define Y_THREADS 8
#define Z_THREADS 1

struct ComputeShaderInput
{
    uint3 GroupID           : SV_GroupID;           // 3D index of the thread group in the dispatch.
    uint3 GroupThreadID     : SV_GroupThreadID;     // 3D index of local thread ID in a thread group.
    uint3 DispatchThreadID  : SV_DispatchThreadID;  // 3D index of global thread ID in the dispatch.
    uint  GroupIndex        : SV_GroupIndex;        // Flattened local index of the thread within a thread group.
};

[numthreads(X_THREADS, Y_THREADS, Z_THREADS)]
void CSMain(ComputeShaderInput IN) {
	// Change these if multiple thread groups are added
	uint totalThreads = X_THREADS * Y_THREADS * Z_THREADS;
	uint thisThread = IN.GroupIndex;
	
	uint stride = totalThreads;
	
	// Reorder particles that is being removed
	for (uint i = thisThread; i < removalCounter; i += stride) {
		overwriteParticle(inputBuffer.removeParticles[i], numPrevParticles - i); // Overwrite particle to be removed with one that is still 
	}
	
	// Find how many particles should be spawned
	int particleBufferSizeLeft = floor(inputBuffer.maxOutputVertices / 6) - (inputBuffer.numPrevParticles);
	int particlesToSpawn = inputBuffer.numParticles;
	if (inputBuffer.numParticles > particleBufferSizeLeft) {
		particlesToSpawn = particleBufferSizeLeft;
	}
	
    for (uint i = thisThread; i < particlesToSpawn; i += stride) {
		float3 v0, v1, v2, v3;
		
		v0 = inputBuffer.particles[i].position + float3(-0.1, 0.1, 0.0);
		v1 = inputBuffer.particles[i].position + float3(-0.1, -0.1, 0.0);
		v2 = inputBuffer.particles[i].position + float3(0.1, 0.1, 0.0);
		v3 = inputBuffer.particles[i].position + float3(0.1, -0.1, 0.0);
		
		CSPhysicsBuffer[inputBuffer.numPrevParticles + i].acceleration = inputBuffer.particles[i].acceleration;
		CSPhysicsBuffer[inputBuffer.numPrevParticles + i].velocity = inputBuffer.particles[i].velocity;
		CSPhysicsBuffer[inputBuffer.numPrevParticles + i].remainingLifeTime = inputBuffer.particles[i].lifeTime;
		
		createParticle(v0, v1, v2, v3, inputBuffer.numPrevParticles + i);
		
		// Physics for new particle (accounts for how far into the frame the particle was created)
		updatePhysics(inputBuffer.numPrevParticles + i, inputBuffer.particles[i].spawnTime);
	}
	
	// Physics for all prevous particles
	for (uint i = thisThread; i < inputBuffer.numPrevParticles; i += stride) {
		updatePhysics(i, inputBuffer.frameTime);
	}
	
	
}
