#include "pch.h"
#include "Sail/graphics/light/LightSetup.h"
#include "../shader/SVkPipelineStateObject.h"
#include "../shader/SVkShader.h"

#include "SVkForwardRenderer.h"
#include "SVkDeferredRenderer.h"
#include "../SVkUtils.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new SVkForwardRenderer();
		break;
	case DEFERRED:
		return new SVkDeferredRenderer();
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

	createRenderPass();
}

SVkForwardRenderer::~SVkForwardRenderer() {
	vkDeviceWaitIdle(m_context->getDevice());

	vkDestroyRenderPass(m_context->getDevice(), m_renderPassClear, nullptr);
	vkDestroyRenderPass(m_context->getDevice(), m_renderPassLoad, nullptr);
}

void SVkForwardRenderer::begin(Camera* camera, Environment* environment) {
	Renderer::begin(camera, environment);
}

void* SVkForwardRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Forward renderer present");

	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if (flags & Renderer::PresentFlag::SkipPreparation) {
		if (skippedPrepCmdList)
			cmd = static_cast<VkCommandBuffer>(skippedPrepCmdList);
		else
			Logger::Error("SVkForwardRenderer present was called with skipPreparation flag but no CmdList was passed");
	}

	// If SkipPreparation flag is set, use a render pass that does not clear any attachment
	auto renderPass = (flags & Renderer::PresentFlag::SkipPreparation) ? m_renderPassLoad : m_renderPassClear;

	if (!(flags & Renderer::PresentFlag::SkipPreparation))
		cmd = runFramePreparation();
	if (!(flags & Renderer::PresentFlag::SkipRendering))
		runRenderingPass(cmd, renderPass);
	if (!(flags & Renderer::PresentFlag::SkipExecution))
		runFrameExecution(cmd);

	return cmd;
}

void SVkForwardRenderer::useDepthBuffer(void* buffer, void* cmdList) {
	// TODO: fix?
	//Logger::Warning("useDepthBuffer not implemented");
	//assert(false);
}

void SVkForwardRenderer::createRenderPass() {
	auto msaaSamples = m_context->getSampleCount();

	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = m_context->getSwapchainImageFormat();
	colorAttachment.samples = msaaSamples;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = m_context->getDepthFormat();
	depthAttachment.samples = msaaSamples;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	// Only used when msaaSamples > 1
	VkAttachmentDescription colorAttachmentResolve{};
	colorAttachmentResolve.format = m_context->getSwapchainImageFormat();
	colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthAttachmentRef = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };
	VkAttachmentReference colorAttachmentResolveRef = { 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	if (msaaSamples > 1)
		subpass.pResolveAttachments = &colorAttachmentResolveRef;

	// Render pass
	std::vector<VkAttachmentDescription> attachments = { colorAttachment, depthAttachment };
	if (msaaSamples > 1)
		attachments.emplace_back(colorAttachmentResolve);

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	// Add dependency to wait for image reading to complete before writing
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	// Create a render pass used when this is the first (or only) pass using the framebuffer this frame
	VK_CHECK_RESULT(vkCreateRenderPass(m_context->getDevice(), &renderPassInfo, nullptr, &m_renderPassClear));

	// Create a render pass used when this is not the first pass using the framebuffer this frame
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VK_CHECK_RESULT(vkCreateRenderPass(m_context->getDevice(), &renderPassInfo, nullptr, &m_renderPassLoad));
}

const VkCommandBuffer& SVkForwardRenderer::runFramePreparation() {
	// Fetch the swap image index to use this frame
	// It may be out of order and has to be passed on to certain bind methods
	auto imageIndex = m_context->getSwapImageIndex();

	auto& cmd = m_command.buffers[imageIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Tell vk we will only submit this commmand buffer once
	beginInfo.pInheritanceInfo = nullptr; // Optional

	SAIL_PROFILE_API_SPECIFIC_SCOPE("Begin command buffer, render pass and set viewport");

	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		Logger::Error("Failed to begin recording command buffer!");
		return nullptr;
	}
	return cmd;
}

void SVkForwardRenderer::runRenderingPass(const VkCommandBuffer& cmd, const VkRenderPass& renderPass) {
	if (commandQueue.empty()) return;

	// Start render pass
	{
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass;
		renderPassInfo.framebuffer = m_context->getCurrentFramebuffer();
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = m_context->getCurrentExtent();

		std::array<VkClearValue, 2> clearValues;
		clearValues[0] = { 0.f, 0.0f, 0.f, 1.f }; // Default clear color
		clearValues[1] = { 1.f, 0.f }; // Default clear depth

		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	// Set dynamic viewport and scissor states
	vkCmdSetViewport(cmd, 0, 1, &m_context->getViewport());
	vkCmdSetScissor(cmd, 0, 1, &m_context->getScissorRect());

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

		// Sort based on distance to draw back to front (for transparency)
		// TODO: Only sort meshes that have transparency
		// TODO: Fix ordering between different PSO's
		std::sort(renderCommands.begin(), renderCommands.end(), [&](Renderer::RenderCommand& a, Renderer::RenderCommand& b) {
			float dstA = glm::distance2(glm::vec3(glm::transpose(a.transform)[3]), camera->getPosition());
			float dstB = glm::distance2(glm::vec3(glm::transpose(b.transform)[3]), camera->getPosition());
			return dstA > dstB;
		});

		// Iterate render commands
		for (auto& command : renderCommands) {
			shader->trySetCBufferVar("sys_materialIndex", &command.materialIndex, sizeof(unsigned int), cmd);
			shader->trySetCBufferVar("sys_mWorld", &command.transform, sizeof(glm::mat4), cmd);

			command.mesh->draw(*this, command.material, shader, environment, cmd);
		}
	}

	vkCmdEndRenderPass(cmd);
}

void SVkForwardRenderer::runFrameExecution(const VkCommandBuffer& cmd) {
	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		Logger::Error("failed to record command buffer!");
		return;
	}

	// Submit the command buffer to the graphics queue
	m_context->submitCommandBuffers({ cmd });
}
