#include "pch.h"
#include "SVkForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
//#include "../VkUtils.h"
#include "../shader/SVkPipelineStateObject.h"
//#include "../resources/DescriptorHeap.h"
#include <unordered_set>
#include "../shader/SVkShader.h"
//#include "VkDeferredRenderer.h"
//#include "VkRaytracingRenderer.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new SVkForwardRenderer();
		break;
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
		break;
	}
	return nullptr;
}

SVkForwardRenderer::SVkForwardRenderer() {
	m_context = Application::getInstance()->getAPI<SVkAPI>();
	m_context->initCommand(m_command);
}

SVkForwardRenderer::~SVkForwardRenderer() { }

void SVkForwardRenderer::begin(Camera* camera, Environment* environment) {
	Renderer::begin(camera, environment);
}

void* SVkForwardRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Forward renderer present");

	// Fetch the swap image index to use this frame
	// It may be out of order and has to be passed on to certain bind methods
	auto imageIndex = m_context->beginPresent();
	
	auto& resman = Application::getInstance()->getResourceManager();

	auto& cmd = m_command.buffers[imageIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Tell vk we will only submit this commmand buffer once
	beginInfo.pInheritanceInfo = nullptr; // Optional

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Begin command buffer, render pass and set viewport");

		if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
			Logger::Error("Failed to begin recording command buffer!");
			return false;
		}

		// Start render pass
		vkCmdBeginRenderPass(cmd, &m_context->getRenderPassInfo(), VK_SUBPASS_CONTENTS_INLINE);
		// Set dynamic viewport and scissor states
		vkCmdSetViewport(cmd, 0, 1, &m_context->getViewport());
		vkCmdSetScissor(cmd, 0, 1, &m_context->getScissorRect());
	}

	// Iterate unique PSO's
	for (auto it : commandQueue) {
		SVkPipelineStateObject* pso = static_cast<SVkPipelineStateObject*>(it.first);
		auto& renderCommands = it.second;

		SVkShader* shader = static_cast<SVkShader*>(pso->getShader());
		shader->prepareToRender(renderCommands, pso);

		if (camera) {
			// Transpose all matrices to convert them to row-major which is required in order for the hlsl->spir-v multiplication order
			shader->trySetCBufferVar("sys_mView", &glm::transpose(camera->getViewMatrix()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_mProjection", &glm::transpose(camera->getProjMatrix()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_mVP", &glm::transpose(camera->getViewProjection()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3), cmd);
		}
		if (lightSetup) {
			auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
			auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
			shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize, cmd);
			shader->trySetCBufferVar("pointLights", plData, plDataByteSize, cmd);
		}

		pso->bind(cmd); // Binds the pipeline and descriptor sets

		// Iterate render commands
		for (auto& command : renderCommands) {
			shader->trySetCBufferVar("sys_materialIndex", &command.materialIndex, sizeof(unsigned int), cmd);
			shader->trySetCBufferVar("sys_mWorld", &command.transform, sizeof(glm::mat4), cmd);

			command.mesh->draw(*this, command.material, shader, environment, cmd);
		}
	}

	vkCmdEndRenderPass(cmd);

	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		Logger::Error("failed to record command buffer!");
		return false;
	}

	// Submit the command buffer to the graphics queue
	m_context->submitCommandBuffers({ cmd });

	return nullptr;
}

void SVkForwardRenderer::useDepthBuffer(void* buffer, void* cmdList) {
	assert(false);
}
