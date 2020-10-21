#include "pch.h"
#include "SVkRaytracingRenderer.h"

SVkRaytracingRenderer::SVkRaytracingRenderer() {

}

SVkRaytracingRenderer::~SVkRaytracingRenderer() {

}

void SVkRaytracingRenderer::begin(Camera* camera, Environment* environment) {

}

void* SVkRaytracingRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList /*= nullptr*/) {
	return nullptr;
}

bool SVkRaytracingRenderer::onEvent(Event& event) {
	return true;
}
