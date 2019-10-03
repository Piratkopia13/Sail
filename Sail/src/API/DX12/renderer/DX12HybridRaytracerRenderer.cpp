#include "pch.h"
#include "DX12HybridRaytracerRenderer.h"
#include "DX12GBufferRenderer.h"
#include "DX12RaytracingRenderer.h"

DX12HybridRaytracerRenderer::DX12HybridRaytracerRenderer() { 
	m_rendererGbuffer = std::make_unique<DX12GBufferRenderer>();
	m_rendererRaytrace = std::make_unique<DX12RaytracingRenderer>(m_rendererGbuffer->getGBufferOutputs());
}

DX12HybridRaytracerRenderer::~DX12HybridRaytracerRenderer() { 

}

void DX12HybridRaytracerRenderer::begin(Camera* camera) {
	m_rendererGbuffer->begin(camera);
	m_rendererRaytrace->begin(camera);
}

void DX12HybridRaytracerRenderer::submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags) {
	m_rendererGbuffer->submit(mesh, modelMatrix, flags);
	m_rendererRaytrace->submit(mesh, modelMatrix, flags);
}

void DX12HybridRaytracerRenderer::submitNonMesh(RenderCommandType type, Material* material, const glm::mat4& modelMatrix, RenderFlag flags) {
	m_rendererGbuffer->submitNonMesh(type, material, modelMatrix, flags);
	m_rendererRaytrace->submitNonMesh(type, material, modelMatrix, flags);
}

void DX12HybridRaytracerRenderer::setLightSetup(LightSetup* lightSetup) {
	m_rendererGbuffer->setLightSetup(lightSetup);
	m_rendererRaytrace->setLightSetup(lightSetup);
}

void DX12HybridRaytracerRenderer::end() {
	m_rendererGbuffer->end();
	m_rendererRaytrace->end();
}

void DX12HybridRaytracerRenderer::present(PostProcessPipeline* postProcessPipeline, RenderableTexture* output) {
	m_rendererGbuffer->present();
	m_rendererRaytrace->present(postProcessPipeline, output);
}

bool DX12HybridRaytracerRenderer::onEvent(Event& event) {
	m_rendererGbuffer->onEvent(event);
	m_rendererRaytrace->onEvent(event);
	return true;
}
