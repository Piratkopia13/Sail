#pragma once
#include "..//BaseComponentSystem.h"
#include <d3d12.h>
#include "Sail/api/ComputeShaderDispatcher.h"

class ParticleEmitterComponent;
class ParticleComputeShader;
class DX12VertexBuffer;

class ParticleSystem final : public BaseComponentSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	void spawnParticles(int particlesToSpawn, ParticleEmitterComponent* particleEmitterComp);

	void update(float dt);
	void updateOnGPU(ID3D12GraphicsCommandList4* cmdList);

	void submitAll() const;

private:
	std::unique_ptr<ComputeShaderDispatcher> m_dispatcher;
	ParticleComputeShader* m_particleShader;

	DX12VertexBuffer* m_outputVertexBuffer;
	unsigned int m_outputVertexBufferSize;

	std::unique_ptr<Model> m_model;

	int m_numberOfParticles;
	int m_prevNumberOfParticles;

	struct NewParticleInfo {
		int nrOfNewParticles;
		ParticleEmitterComponent* emitter;
	};

	std::vector<NewParticleInfo> m_newEmitters;

	struct EmitterData {
		glm::vec3 position;
		float padding0;
		glm::vec3 velocity;
		float padding1;
		glm::vec3 acceleration;
		int nrOfParticlesToSpawn;
	};

	struct ComputeInput {
		EmitterData emitters[100];
		unsigned int numEmitters;
		unsigned int previousNrOfParticles;
		unsigned int maxOutputVertices;
	};
};