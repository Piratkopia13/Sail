#pragma once
#include "..//BaseComponentSystem.h"
#include <d3d12.h>
#include "Sail/api/ComputeShaderDispatcher.h"
#include "Sail/utils/Timer.h"
#include "API/DX12/DX12API.h"

class ParticleEmitterComponent;
class ParticleComputeShader;
class DX12VertexBuffer;
namespace ShaderComponent {
	class DX12StructuredBuffer;
}

class ParticleSystem final : public BaseComponentSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	void spawnParticles(int particlesToSpawn, ParticleEmitterComponent* particleEmitterComp);

	void update(float dt);
	void updateOnGPU(ID3D12GraphicsCommandList4* cmdList, const glm::vec3& cameraPos);

	void submitAll() const;

private:
	void syncWithGPUUpdate(unsigned int swapIndex);

private:
	std::unique_ptr<ComputeShaderDispatcher> m_dispatcher;
	ParticleComputeShader* m_particleShader;

	DX12VertexBuffer* m_outputVertexBuffer;
	unsigned int m_outputVertexBufferSize;
	wComPtr<ID3D12Resource>* m_physicsBufferDefaultHeap;
	int m_particlePhysicsSize;

	std::unique_ptr<Model> m_model;

	Timer m_timer;
	INT64 m_startTime;

	int m_gpuUpdates;

	std::vector<float>* m_particleLife;

	struct NewParticleInfo {
		ParticleEmitterComponent* emitter;
		glm::vec3 spread;
		float spawnTime;
	};

	struct CPUOutput {
		std::vector<NewParticleInfo> newParticles;
		std::vector<unsigned int> toRemove;
		unsigned int previousNrOfParticles;
		float lastFrameTime;
	};

	CPUOutput* m_cpuOutput;

	struct ParticleData {
		glm::vec3 position;
		float padding0;
		glm::vec3 velocity;
		float padding1;
		glm::vec3 acceleration;
		float spawnTime;
	};

	static const unsigned int m_maxParticlesSpawnPerFrame = 312;

	struct ComputeInput {
		ParticleData particles[m_maxParticlesSpawnPerFrame];
		unsigned int particlesToRemove[m_maxParticlesSpawnPerFrame];
		glm::vec3 cameraPos;
		unsigned int numParticles;
		unsigned int numParticlesToRemove;
		unsigned int previousNrOfParticles;
		unsigned int maxOutputVertices;
		float frameTime;
	};

	ComputeInput m_inputData;
};