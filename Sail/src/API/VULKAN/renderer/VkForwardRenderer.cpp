#include "pch.h"
#include "VkForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
//#include "../VkUtils.h"
#include "../shader/VkPipelineStateObject.h"
//#include "../resources/DescriptorHeap.h"
#include <unordered_set>
#include "../shader/VkShader.h"
//#include "VkDeferredRenderer.h"
//#include "VkRaytracingRenderer.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new VkForwardRenderer();
		break;
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
		break;
	}
	return nullptr;
}

VkForwardRenderer::VkForwardRenderer() {
	m_context = Application::getInstance()->getAPI<VkAPI>();
	assert(false);
}

VkForwardRenderer::~VkForwardRenderer() { }

void VkForwardRenderer::begin(Camera* camera, Environment* environment) {
	Renderer::begin(camera, environment);
}

void* VkForwardRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	assert(false);
	return nullptr;
}

void VkForwardRenderer::useDepthBuffer(void* buffer, void* cmdList) {
	assert(false);
}
