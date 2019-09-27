#include "pch.h"
#include "RendererWrapper.h"

#include "api/Renderer.h"
#include "graphics/postprocessing/PostProcessPipeline.h"

RendererWrapper::RendererWrapper() {
}

RendererWrapper::~RendererWrapper() {
}

void RendererWrapper::initialize() {
	m_rendererRaster = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));
	m_rendererRaytrace = std::unique_ptr<Renderer>(Renderer::Create(Renderer::RAYTRACED));
	m_currentRenderer = m_rendererRaster.get();

	m_doPostProcessing = false;
}

/*
	0 : Raster
	1 : Raytrace
*/
void RendererWrapper::changeRenderer(unsigned int index) {

	switch (index) {
	case 0:
		m_currentRenderer = m_rendererRaster.get();
		break;
	case 1:
		m_currentRenderer = m_rendererRaytrace.get();
		break;
	default:
		break;
	}
}

void RendererWrapper::togglePostProcessing(const bool flag) {
	m_doPostProcessing = flag;
}

bool& RendererWrapper::getDoPostProcessing() {
	return m_doPostProcessing;
}

Renderer* RendererWrapper::getCurrentRenderer() {
	return m_currentRenderer;
}

PostProcessPipeline* RendererWrapper::getPostProcessPipeline() {
	return m_postProcessPipeline;
}

void RendererWrapper::setLightSetup(LightSetup* lights) {
	m_currentRenderer->setLightSetup(lights);
}


