#pragma once

#include "Sail/api/Renderer.h"

class DX12GBufferRenderer;
class DX12RaytracingRenderer;

class DX12HybridRaytracerRenderer : public Renderer {
public:
	DX12HybridRaytracerRenderer();
	~DX12HybridRaytracerRenderer();

	virtual void begin(Camera* camera) override;
	virtual void submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags) override;
	virtual void setLightSetup(LightSetup* lightSetup) override;
	virtual void end() override;
	virtual void present(PostProcessPipeline* postProcessPipeline = nullptr, RenderableTexture* output = nullptr) override;
	virtual bool onEvent(Event& event) override;

private:
	std::unique_ptr<DX12GBufferRenderer> m_rendererGbuffer;
	std::unique_ptr<DX12RaytracingRenderer> m_rendererRaytrace;

};