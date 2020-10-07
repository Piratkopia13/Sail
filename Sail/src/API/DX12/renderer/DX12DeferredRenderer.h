#pragma once

#include "Sail/api/Renderer.h"
#include "Sail/events/Events.h"
#include <glm/glm.hpp>
#include "../DX12API.h"
#include "../resources/DX12RenderableTexture.h"
#include "Sail/graphics/SSAO.h"
#include "Sail/graphics/material/TexturesMaterial.h"

class DX12DeferredRenderer : public Renderer, public IEventListener {
public:
	static const unsigned int NUM_GBUFFERS = 4; // depth not accounted for

public:
	struct GBufferTextures {
		std::unique_ptr<DX12RenderableTexture> positions;
		std::unique_ptr<DX12RenderableTexture> normals;
		std::unique_ptr<DX12RenderableTexture> albedo;
		std::unique_ptr<DX12RenderableTexture> mrao;
		std::unique_ptr<DX12RenderableTexture> depth;
	};

public:
	DX12DeferredRenderer();
	~DX12DeferredRenderer();

	void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;
	void* getDepthBuffer() override;

	bool onEvent(Event& event) override;

	static const GBufferTextures& GetGBuffers();

private:

	ID3D12GraphicsCommandList4* runFramePreparation();
	void runGeometryPass(ID3D12GraphicsCommandList4* cmdList);
	void runShadingPass(ID3D12GraphicsCommandList4* cmdList);
	void runSSAO(ID3D12GraphicsCommandList4* cmdList);
	void runFrameExecution(ID3D12GraphicsCommandList4* cmdList);

	D3D12_CPU_DESCRIPTOR_HANDLE getGeometryPassDsv();

private:
	DX12API* m_context;
	DX12API::Command m_command;

	glm::vec4 m_clearColor;
	static GBufferTextures sGBufferTextures;
	std::unique_ptr<Model> m_screenQuadModel;
	TexturesMaterial m_shadingPassMaterial;

	std::unique_ptr<SSAO> m_ssao;
	DX12RenderableTexture* m_ssaoShadingTexture;
	
};