#pragma once

#include "Sail/api/Renderer.h"
#include <glm/glm.hpp>
#include "../DX12API.h"
#include "../DXR/DXRBase.h"
#include "../resources/DX12RenderableTexture.h"

class DX12RaytracingRenderer : public Renderer {
public:
	DX12RaytracingRenderer(DX12RenderableTexture** inputs);
	~DX12RaytracingRenderer();

	void present(PostProcessPipeline* postProcessPipeline = nullptr, RenderableTexture* output = nullptr) override;
	virtual bool onEvent(Event& event) override;
	virtual void submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags) override;

	void setGBufferInputs(DX12RenderableTexture** inputs);

private:
	bool onResize(WindowResizeEvent& event);

private:
	DX12API* m_context;
	DX12API::Command m_command;
	DXRBase m_dxr;
	std::unique_ptr<DX12RenderableTexture> m_outputTexture;

};