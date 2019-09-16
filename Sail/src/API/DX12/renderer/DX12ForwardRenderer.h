#pragma once

#include "Sail/api/Renderer.h"
#include <glm/glm.hpp>
#include "../DX12API.h"
#include "Sail/api/ComputeShaderDispatcher.h"

class DX12RenderableTexture;
class PostProcessPipeline;

class DX12ForwardRenderer : public Renderer {
public:
	DX12ForwardRenderer();
	~DX12ForwardRenderer();

	void present(PostProcessPipeline* postProcessPipeline = nullptr, RenderableTexture* output = nullptr) override;

private:
	DX12API* m_context;
	DX12API::Command m_command;
	std::unique_ptr<DX12RenderableTexture> m_outputTexture;

	// Compute shader testing
	//std::unique_ptr<ComputeShaderDispatcher> m_computeShaderDispatcher;

};