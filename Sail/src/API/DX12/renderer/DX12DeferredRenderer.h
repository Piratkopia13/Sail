#pragma once

#include "Sail/api/Renderer.h"
#include <glm/glm.hpp>
#include "../DX12API.h"
#include "../resources/DX12RenderableTexture.h"
#include "Sail/graphics/material/CustomMaterial.h"

class DX12DeferredRenderer : public Renderer {
public:
	DX12DeferredRenderer();
	~DX12DeferredRenderer();

	void present(RenderableTexture* output = nullptr) override;

private:
	DX12API* m_context;
	DX12API::Command m_command;

	static const unsigned int NUM_GBUFFERS = 4;
	std::unique_ptr<DX12RenderableTexture> m_gbufferTextures[NUM_GBUFFERS];
	std::unique_ptr<Model> m_screenQuadModel;
	CustomMaterial m_shadingPassMaterial;

};