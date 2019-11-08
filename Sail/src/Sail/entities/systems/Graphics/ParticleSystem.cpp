#include "pch.h"
#include "ParticleSystem.h"
#include "Sail/Application.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/Entity.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "API/DX12/shader/DX12ConstantBuffer.h"
#include "API/DX12/resources/DescriptorHeap.h"
#include "API/DX12/DX12Utils.h"
#include "API/DX12/DX12API.h"

#include "Sail/graphics/shader/compute/ParticleComputeShader.h"
#include "Sail/graphics/shader/dxr/GBufferOutShader.h"

ParticleSystem::ParticleSystem() {
	registerComponent<ParticleEmitterComponent>(true, true, true);

	m_particleShader = &Application::getInstance()->getResourceManager().getShaderSet<ParticleComputeShader>();
	m_dispatcher = std::unique_ptr<ComputeShaderDispatcher>(ComputeShaderDispatcher::Create());
	auto& gbufferShader = Application::getInstance()->getResourceManager().getShaderSet<GBufferOutShader>();
	auto& inputLayout = gbufferShader.getPipeline()->getInputLayout();

	//m_outputVertexBufferSize = 9996;
	m_outputVertexBufferSize = 36;

	m_model = std::make_unique<Model>(m_outputVertexBufferSize, &gbufferShader);

	m_outputVertexBuffer = static_cast<DX12VertexBuffer*>(&m_model->getMesh(0)->getVertexBuffer());

	m_numberOfParticles = 0;

	m_prevNumberOfParticles = SAIL_NEW int[DX12API::NUM_GPU_BUFFERS];
	for (unsigned int i = 0; i < DX12API::NUM_GPU_BUFFERS; i++) {
		m_prevNumberOfParticles[i] = 0;
	}

	m_newEmitters = SAIL_NEW std::vector<NewParticleInfo>[DX12API::NUM_GPU_BUFFERS];

}

ParticleSystem::~ParticleSystem() {
	delete[] m_prevNumberOfParticles;
	delete[] m_newEmitters;
}

void ParticleSystem::spawnParticles(int particlesToSpawn, ParticleEmitterComponent* particleEmitterComp) {
	for (unsigned int i = 0; i < DX12API::NUM_GPU_BUFFERS; i++) {
		m_newEmitters[i].emplace_back();
		m_newEmitters[i].back().nrOfNewParticles = particlesToSpawn;
		m_newEmitters[i].back().emitter = particleEmitterComp;
	}

	m_numberOfParticles += particlesToSpawn;
}

void ParticleSystem::update(float dt) {
	for (auto& e : entities) {
		ParticleEmitterComponent* particleEmitterComp = e->getComponent<ParticleEmitterComponent>();

		if (particleEmitterComp->spawnTimer >= particleEmitterComp->spawnRate) {
			//Spawn the correct number of particles
			int particlesToSpawn = (int)glm::floor(particleEmitterComp->spawnTimer / glm::max(particleEmitterComp->spawnRate, 0.0001f));
			spawnParticles(particlesToSpawn, particleEmitterComp);
			//Decrease timer
			particleEmitterComp->spawnTimer -= particleEmitterComp->spawnRate * particlesToSpawn;
		}
		particleEmitterComp->spawnTimer += dt;
	}
}

void ParticleSystem::updateOnGPU(ID3D12GraphicsCommandList4* cmdList) {
	auto* context = Application::getInstance()->getAPI<DX12API>();

	m_outputVertexBuffer->init(cmdList);

	m_dispatcher->begin(cmdList);

	auto* settings = m_particleShader->getComputeSettings();

	ComputeInput inputData;
	inputData.numEmitters = m_newEmitters[context->getSwapIndex()].size();
	inputData.previousNrOfParticles = m_prevNumberOfParticles[context->getSwapIndex()];
	inputData.maxOutputVertices = m_outputVertexBufferSize;

	for (unsigned int i = 0; i < m_newEmitters[context->getSwapIndex()].size(); i++) {
		inputData.emitters[i].position = m_newEmitters[context->getSwapIndex()][i].emitter->position;
		inputData.emitters[i].velocity = m_newEmitters[context->getSwapIndex()][i].emitter->velocity;
		inputData.emitters[i].acceleration = m_newEmitters[context->getSwapIndex()][i].emitter->acceleration;
		inputData.emitters[i].nrOfParticlesToSpawn = m_newEmitters[context->getSwapIndex()][i].nrOfNewParticles;
	}

	m_particleShader->getPipeline()->setCBufferVar("inputBuffer", &inputData, sizeof(ComputeInput));

	auto& cdh = context->getComputeGPUDescriptorHeap()->getCurentCPUDescriptorHandle();
	cdh.ptr += context->getComputeGPUDescriptorHeap()->getDescriptorIncrementSize() * 10;

	// Create a unordered access view for the animated vertex buffer in the correct place in the heap
	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.FirstElement = 0;
	uavDesc.Buffer.NumElements = m_outputVertexBufferSize;
	uavDesc.Buffer.StructureByteStride = 4 * 14; // TODO: replace with sizeof(Vertex)
	context->getDevice()->CreateUnorderedAccessView(m_outputVertexBuffer->getBuffer(), nullptr, &uavDesc, cdh);
	// Transition to UAV access
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_outputVertexBuffer->getBuffer(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	m_dispatcher->dispatch(*m_particleShader, Shader::ComputeShaderInput(), 0, cmdList);

	context->getComputeGPUDescriptorHeap()->getAndStepIndex(10);

	// Transition to Cbuffer usage
	DX12Utils::SetResourceTransitionBarrier(cmdList, m_outputVertexBuffer->getBuffer(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);

	m_prevNumberOfParticles[context->getSwapIndex()] = glm::min(m_numberOfParticles, (int) (m_outputVertexBufferSize / 6));

	Logger::Log(std::to_string(m_prevNumberOfParticles[context->getSwapIndex()]));

	m_newEmitters[context->getSwapIndex()].clear();
}

void ParticleSystem::submitAll() const {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
	Renderer::RenderFlag flags = Renderer::MESH_DYNAMIC;
	flags |= Renderer::IS_VISIBLE_ON_SCREEN;

	renderer->submit(
		m_model.get(),
		glm::identity<glm::mat4>(),
		flags,
		0
	);

}
