#pragma once
#include "..//BaseComponentSystem.h"
#include <d3d12.h>
#include "Sail/api/ComputeShaderDispatcher.h"

class ParticleEmitterComponent;

class ParticleSystem final : public BaseComponentSystem {
public:
	ParticleSystem();
	~ParticleSystem();

	void spawnParticles(int particlesToSpawn, ParticleEmitterComponent* particleEmitterComp);

	void update(float dt);
	void updateOnGPU(ID3D12GraphicsCommandList4* cmdList);

private:
	std::unique_ptr<ComputeShaderDispatcher> m_dispatcher;
	std::unique_ptr<InputLayout> m_inputLayout;
	Shader* m_particleShader;

	int m_numberOfParticles;

	struct newParticleInfo {
		int nrOfNewParticles;
		ParticleEmitterComponent* emitter;
	};

	std::vector<newParticleInfo> m_newParticles;
};