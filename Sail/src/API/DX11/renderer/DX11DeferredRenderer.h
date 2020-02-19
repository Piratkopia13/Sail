#pragma once

#include "Sail/api/Renderer.h"
#include "../DX11API.h"
#include "Sail/graphics/material/DeferredShadingPassMaterial.h"
#include "Sail/graphics/SSAO.h"

class DX11RenderableTexture;

class DX11DeferredRenderer : public Renderer {
public:
	DX11DeferredRenderer();
	~DX11DeferredRenderer();

	void* present(Renderer::RenderFlag flags, void* skippedPrepCmdList = nullptr) override;
	void* getDepthBuffer() override;

	void runFramePreparation();
	void runGeometryPass();
	void runSSAO();
	void runShadingPass();

private:
	DX11API* m_context;

	static const unsigned int NUM_GBUFFERS = 4;
	std::unique_ptr<DX11RenderableTexture> m_gbufferTextures[NUM_GBUFFERS];
	std::unique_ptr<Model> m_screenQuadModel;
	DeferredShadingPassMaterial m_shadingPassMaterial;

	std::unique_ptr<SSAO> m_ssao;
	DX11RenderableTexture* m_ssaoShadingTexture;

};