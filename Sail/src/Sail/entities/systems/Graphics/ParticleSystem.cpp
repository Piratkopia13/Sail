#include "ParticleSystem.h"
#include "pch.h"
#include "ParticleSystem.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/Entity.h"
#include "Sail/Application.h"
#include "API/DX12/DX12VertexBuffer.h"

#include "Sail/graphics/shader/compute/ParticleComputeShader.h"

ParticleSystem::ParticleSystem() {
	registerComponent<ParticleEmitterComponent>(true, true, true);

	m_particleShader = &Application::getInstance()->getResourceManager().getShaderSet<ParticleComputeShader>();
	m_dispatcher = std::unique_ptr<ComputeShaderDispatcher>(ComputeShaderDispatcher::Create());
	m_inputLayout = std::unique_ptr<InputLayout>(InputLayout::Create());

	m_numberOfParticles = 0;
}

ParticleSystem::~ParticleSystem() {

}

void ParticleSystem::spawnParticles(int particlesToSpawn, ParticleEmitterComponent* particleEmitterComp) {
	//Not implemented yet
	m_newParticles.emplace_back();
	m_newParticles.back().nrOfNewParticles = particlesToSpawn;
	m_newParticles.back().emitter = particleEmitterComp;

	m_numberOfParticles += particlesToSpawn;
}

void ParticleSystem::update(float dt) {
	for (auto& e : entities) {
		ParticleEmitterComponent* particleEmitterComp = e->getComponent<ParticleEmitterComponent>();

		if (particleEmitterComp->spawnTimer >= particleEmitterComp->spawnRate) {
			//Spawn the correct number of particles
			int particlesToSpawn = (int) glm::floor(particleEmitterComp->spawnTimer / glm::max(particleEmitterComp->spawnRate, 0.00001f));
			spawnParticles(particlesToSpawn, particleEmitterComp);
			//Decrease timer
			particleEmitterComp->spawnTimer -= particleEmitterComp->spawnRate * particlesToSpawn;
		}
		particleEmitterComp->spawnTimer += dt;
	}
}

void ParticleSystem::updateOnGPU(ID3D12GraphicsCommandList4* cmdList) {
	m_dispatcher->begin(cmdList);

	auto* settings = m_particleShader->getComputeSettings();

	ParticleComputeShader::Input input;

	//Input = vad?

	ParticleComputeShader::Output output;
	
	
	output = static_cast<ParticleComputeShader::Output&>(m_dispatcher->dispatch(*m_particleShader, input, 0, cmdList));

	// Initialize output vertex buffer if it has not been done already
	if (!output.outputVB) {
		Mesh::Data data;
		data.resizeVertices(1000);
		output.outputVB = static_cast<DX12VertexBuffer*>(DX12VertexBuffer::Create(*m_inputLayout, data));
	}
	
}
