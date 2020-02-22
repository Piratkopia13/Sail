#pragma once

#include "Sail/api/Renderer.h"
#include "Sail/events/Events.h"
#include <glm/glm.hpp>
#include "../DX12API.h"
#include "../resources/DX12RenderableTexture.h"
#include "Sail/graphics/material/DeferredShadingPassMaterial.h"
#include "Sail/graphics/SSAO.h"

class DX12DeferredRenderer : public Renderer, public IEventListener {
public:
	DX12DeferredRenderer();
	~DX12DeferredRenderer();

	void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;
	void* getDepthBuffer() override;

	ID3D12GraphicsCommandList4* runFramePreparation();
	void runGeometryPass(ID3D12GraphicsCommandList4* cmdList);
	void runShadingPass(ID3D12GraphicsCommandList4* cmdList);
	void runSSAO(ID3D12GraphicsCommandList4* cmdList);
	void runFrameExecution(ID3D12GraphicsCommandList4* cmdList);

	bool onEvent(Event& event) override;

private:
	D3D12_CPU_DESCRIPTOR_HANDLE getGeometryPassDsv();

private:
	DX12API* m_context;
	DX12API::Command m_command;

	static const unsigned int NUM_GBUFFERS = 4;
	std::unique_ptr<DX12RenderableTexture> m_gbufferTextures[NUM_GBUFFERS];
	std::unique_ptr<Model> m_screenQuadModel;
	DeferredShadingPassMaterial m_shadingPassMaterial;

	std::unique_ptr<SSAO> m_ssao;
	DX12RenderableTexture* m_ssaoShadingTexture;
	
};