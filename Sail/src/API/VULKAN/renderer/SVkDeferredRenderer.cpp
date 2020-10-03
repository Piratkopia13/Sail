#include "pch.h"
#include "SVkDeferredRenderer.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/Environment.h"
#include "Sail/graphics/geometry/factory/ScreenQuadModel.h"
#include "../shader/SVkShader.h"
#include "../SVkUtils.h"

SVkDeferredRenderer::SVkDeferredRenderer() 
	: m_ssaoRenderPass(VK_NULL_HANDLE)
{
	EventSystem::getInstance()->subscribeToEvent(Event::SWAPCHAIN_RECREATED, this);
	auto* app = Application::getInstance();
	m_context = app->getAPI<SVkAPI>();

	assert(m_context->getSampleCount() == 1 && "MSAA is not supported when using a deferred renderer");

	m_context->initCommand(m_command);

	m_width = app->getWindow()->getWindowWidth();
	m_height = app->getWindow()->getWindowHeight();

	glm::vec4 clearColor(0.f);
	m_gbuffers.positions = std::unique_ptr<SVkRenderableTexture>(SAIL_NEW SVkRenderableTexture(m_width, m_height, RenderableTexture::USAGE_SAMPLING_ACCESS, ResourceFormat::R16G16B16A16_FLOAT));
	m_gbuffers.normals	 = std::unique_ptr<SVkRenderableTexture>(SAIL_NEW SVkRenderableTexture(m_width, m_height, RenderableTexture::USAGE_SAMPLING_ACCESS, ResourceFormat::R16G16B16A16_FLOAT));
	m_gbuffers.albedo	 = std::unique_ptr<SVkRenderableTexture>(SAIL_NEW SVkRenderableTexture(m_width, m_height, RenderableTexture::USAGE_SAMPLING_ACCESS, ResourceFormat::R8G8B8A8));
	m_gbuffers.mrao		 = std::unique_ptr<SVkRenderableTexture>(SAIL_NEW SVkRenderableTexture(m_width, m_height, RenderableTexture::USAGE_SAMPLING_ACCESS, ResourceFormat::R8G8B8A8));

	createGeometryRenderPass();
	createShadingRenderPass();
	auto& geometryPassShader = static_cast<SVkShader&>(app->getResourceManager().getShaderSet(Shaders::DeferredGeometryPassShader));
	geometryPassShader.setRenderPass(m_geometryRenderPass);

	createGeometryFramebuffers();

	Application::getInstance()->getResourceManager().loadTexture("pbr/brdfLUT.tga");
	m_brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");

	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create();

	// SSAO
	if (app->getSettings().getBool(Settings::Graphics_SSAO)) {
		m_ssao = std::make_unique<SSAO>();
		createSSAORenderPass();
		createSSAOFramebuffers();
	}
}

SVkDeferredRenderer::~SVkDeferredRenderer() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::SWAPCHAIN_RECREATED, this);
	vkDeviceWaitIdle(m_context->getDevice());

	vkDestroyRenderPass(m_context->getDevice(), m_geometryRenderPass, nullptr);
	vkDestroyRenderPass(m_context->getDevice(), m_shadingRenderPass, nullptr);
	vkDestroyRenderPass(m_context->getDevice(), m_ssaoRenderPass, nullptr);
	for (auto& fb : m_geometryFramebuffers) {
		vkDestroyFramebuffer(m_context->getDevice(), fb, nullptr);
	}
	for (auto& fb : m_ssaoFramebuffers) {
		vkDestroyFramebuffer(m_context->getDevice(), fb, nullptr);
	}
}

void* SVkDeferredRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Deferred renderer present");

	bool doSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);
	if (!m_ssao && doSSAO) {
		// Handle enabling of ssao in runtime
		m_ssao = std::make_unique<SSAO>();
		createSSAORenderPass();
		createSSAOFramebuffers();
	} else if (m_ssao && !doSSAO) {
		// Handle disabling of ssao in runtime
		m_context->waitForGPU();
		m_ssao.reset();
	}

	VkCommandBuffer cmd = nullptr;
	if (flags & Renderer::PresentFlag::SkipPreparation) {
		if (skippedPrepCmdList)
			cmd = static_cast<VkCommandBuffer>(skippedPrepCmdList);
		else
			Logger::Error("SVkDeferredRenderer present was called with skipPreparation flag but no CmdList was passed");
	}

	if (!(flags & Renderer::PresentFlag::SkipPreparation))
		cmd = runFramePreparation();
	if (!(flags & Renderer::PresentFlag::SkipRendering)) {
		runGeometryPass(cmd);
		if (doSSAO)
			runSSAO(cmd);
	}
	if (!(flags & Renderer::PresentFlag::SkipDeferredShading))
		runShadingPass(cmd);
	if (!(flags & Renderer::PresentFlag::SkipExecution))
		runFrameExecution(cmd);

	return cmd;
}

void* SVkDeferredRenderer::getDepthBuffer() {
	return nullptr; // fix
}

bool SVkDeferredRenderer::onEvent(Event& event) {
	auto e = [&](SwapchainRecreatedEvent& event) {
		m_width = Application::getInstance()->getWindow()->getWindowWidth();
		m_height = Application::getInstance()->getWindow()->getWindowHeight();

		m_gbuffers.positions->resize(m_width, m_height);
		m_gbuffers.normals->resize(m_width, m_height);
		m_gbuffers.albedo->resize(m_width, m_height);
		m_gbuffers.mrao->resize(m_width, m_height);

		for (auto& fb : m_geometryFramebuffers) {
			vkDestroyFramebuffer(m_context->getDevice(), fb, nullptr);
		}
		createGeometryFramebuffers();

		m_ssao->resize(m_width, m_height);
		createSSAOFramebuffers();

		return true;
	};


	EventHandler::HandleType<SwapchainRecreatedEvent>(event, e);
	return true;
}

void SVkDeferredRenderer::createGeometryRenderPass() {
	// Set up separate renderpass with references to the color and depth attachments
	std::array<VkAttachmentDescription, NUM_GBUFFERS + 1> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < NUM_GBUFFERS + 1; ++i) {
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == NUM_GBUFFERS) {
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		} else {
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	attachmentDescs[0].format = m_gbuffers.positions->getFormat();
	attachmentDescs[1].format = m_gbuffers.normals->getFormat();
	attachmentDescs[2].format = m_gbuffers.albedo->getFormat();
	attachmentDescs[3].format = m_gbuffers.mrao->getFormat();
	attachmentDescs[4].format = m_context->getDepthFormat();

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = NUM_GBUFFERS; // Last attachment is for depth
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(m_context->getDevice(), &renderPassInfo, nullptr, &m_geometryRenderPass));
}

void SVkDeferredRenderer::createShadingRenderPass() {
	std::array<VkAttachmentDescription, 2> attachmentDescs = {};

	// Init attachment properties
	attachmentDescs[0].format = m_context->getSwapchainImageFormat();
	attachmentDescs[0].samples = m_context->getSampleCount();
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // There is always a transition to LAYOUT_PRESENT happening right before present

	attachmentDescs[1].format = m_context->getDepthFormat();
	attachmentDescs[1].samples = m_context->getSampleCount();
	attachmentDescs[1].loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
	attachmentDescs[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachmentDescs[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
	VkAttachmentReference depthReference = { 1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = &colorReference;
	subpass.colorAttachmentCount = 1;
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layout transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(m_context->getDevice(), &renderPassInfo, nullptr, &m_shadingRenderPass));
}

void SVkDeferredRenderer::createSSAORenderPass() {
	if (m_ssaoRenderPass)
		vkDestroyRenderPass(m_context->getDevice(), m_ssaoRenderPass, nullptr);

	std::array<VkAttachmentDescription, 1> attachmentDescs = {};

	// Init attachment properties
	attachmentDescs[0].format = static_cast<SVkRenderableTexture*>(m_ssao->getRenderTargetTexture())->getFormat();
	attachmentDescs[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescs[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescs[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescs[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescs[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescs[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDescs[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL; // Will be blurred in a compute shader, general layout required

	VkAttachmentReference colorReference = { 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = &colorReference;
	subpass.colorAttachmentCount = 1;

	// Dependency to make sure color write has finished before blur compute shaders can run
	std::array<VkSubpassDependency, 1> dependencies;
	dependencies[0].srcSubpass = 0;
	dependencies[0].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = dependencies.size();
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(m_context->getDevice(), &renderPassInfo, nullptr, &m_ssaoRenderPass));

	// Make sure ssao shader will be created using this render pass
	auto& ssaoShader = static_cast<SVkShader&>(Application::getInstance()->getResourceManager().getShaderSet(Shaders::SSAOShader));
	ssaoShader.setRenderPass(m_ssaoRenderPass);
}

void SVkDeferredRenderer::createGeometryFramebuffers() {
	m_gbuffers.depthView = m_context->getDepthView();

	//auto numBuffers = m_context->getNumSwapchainImages();
	auto numBuffers = m_gbuffers.positions->getNumBuffers();

	m_geometryFramebuffers.resize(numBuffers);
	for (int i = 0; i < numBuffers; i++) {
		// Create the framebuffer
		VkImageView attachments[] = {
			m_gbuffers.positions->getView(i),
			m_gbuffers.normals->getView(i),
			m_gbuffers.albedo->getView(i),
			m_gbuffers.mrao->getView(i),
			m_gbuffers.depthView
		};

		VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = m_geometryRenderPass;
		framebufferInfo.attachmentCount = ARRAYSIZE(attachments);
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_width;
		framebufferInfo.height = m_height;
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_context->getDevice(), &framebufferInfo, nullptr, &m_geometryFramebuffers[i]));
	}
}

void SVkDeferredRenderer::createSSAOFramebuffers() {
	for (auto& fb : m_ssaoFramebuffers) {
		vkDestroyFramebuffer(m_context->getDevice(), fb, nullptr);
	}

	auto numBuffers = m_geometryFramebuffers.size();

	m_ssaoFramebuffers.resize(numBuffers);
	for (int i = 0; i < numBuffers; i++) {
		// Create the framebuffer
		VkImageView attachments[] = {
			static_cast<SVkRenderableTexture*>(m_ssao->getRenderTargetTexture())->getView(i)
		};

		VkFramebufferCreateInfo framebufferInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		framebufferInfo.renderPass = m_ssaoRenderPass;
		framebufferInfo.attachmentCount = ARRAYSIZE(attachments);
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = m_ssao->getRenderTargetWidth();
		framebufferInfo.height = m_ssao->getRenderTargetHeight();
		framebufferInfo.layers = 1;

		VK_CHECK_RESULT(vkCreateFramebuffer(m_context->getDevice(), &framebufferInfo, nullptr, &m_ssaoFramebuffers[i]));
	}
}



const VkCommandBuffer& SVkDeferredRenderer::runFramePreparation() {
	// Fetch the swap image index to use this frame
	auto imageIndex = m_context->getSwapImageIndex();

	auto& cmd = m_command.buffers[imageIndex];

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // Tell vk we will only submit this commmand buffer once
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS) {
		Logger::Error("Failed to begin recording command buffer!");
		return nullptr;
	}

	return cmd;
}

void SVkDeferredRenderer::runGeometryPass(const VkCommandBuffer& cmd) {
	// Fetch the swap image index to use this frame
	auto imageIndex = m_context->getSwapImageIndex();

	{
		// Clear values for all attachments written in the fragment shader
		std::array<VkClearValue, 5> clearValues;
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[3].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[4].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBeginInfo.renderPass = m_geometryRenderPass;
		renderPassBeginInfo.framebuffer = m_geometryFramebuffers[(m_geometryFramebuffers.size() == 1) ? 0 : imageIndex];
		renderPassBeginInfo.renderArea.extent.width = m_width;
		renderPassBeginInfo.renderArea.extent.height = m_height;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		// Start render pass
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
		// Set dynamic viewport and scissor states
		vkCmdSetViewport(cmd, 0, 1, &m_context->getViewport());
		vkCmdSetScissor(cmd, 0, 1, &m_context->getScissorRect());
	}

	// Iterate unique PSO's
	for (auto it : commandQueue) {
		SVkPipelineStateObject* pso = static_cast<SVkPipelineStateObject*>(it.first);
		auto& renderCommands = it.second;

		SVkShader* shader = static_cast<SVkShader*>(pso->getShader());
		shader->updateDescriptorsAndMaterialIndices(renderCommands, pso);

		if (camera) {
			// Transpose all matrices to convert them to row-major which is required in order for the hlsl->spir-v multiplication order
			shader->trySetCBufferVar("sys_mView", &glm::transpose(camera->getViewMatrix()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_mProjection", &glm::transpose(camera->getProjMatrix()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_mVP", &glm::transpose(camera->getViewProjection()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3), cmd);
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
}

void SVkDeferredRenderer::runSSAO(const VkCommandBuffer& cmd) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("SSAO");
	auto& resman = Application::getInstance()->getResourceManager();
	auto imageIndex = m_context->getSwapImageIndex();

	auto* ssaoRenderTarget = static_cast<SVkRenderableTexture*>(m_ssao->getRenderTargetTexture());
	float ssaoWidth = m_ssao->getRenderTargetWidth();
	float ssaoHeight = m_ssao->getRenderTargetHeight();

	{
		// Clear values for all attachments written in the fragment shader
		std::array<VkClearValue, 1> clearValues;
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };

		VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBeginInfo.renderPass = m_ssaoRenderPass;
		renderPassBeginInfo.framebuffer = m_ssaoFramebuffers[(m_ssaoFramebuffers.size() == 1) ? 0 : imageIndex];
		renderPassBeginInfo.renderArea.extent.width = ssaoWidth;
		renderPassBeginInfo.renderArea.extent.height = ssaoHeight;
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		// Start render pass
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport;
		viewport.x = 0.f;
		viewport.y = ssaoHeight;
		viewport.width = ssaoWidth;
		viewport.height = -ssaoHeight;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissorRect;
		scissorRect.extent.width = ssaoWidth;
		scissorRect.extent.height = ssaoHeight;
		scissorRect.offset = { 0, 0 };

		// Set dynamic viewport and scissor states
		vkCmdSetViewport(cmd, 0, 1, &viewport);
		vkCmdSetScissor(cmd, 0, 1, &scissorRect);
	}

	auto* shader = static_cast<SVkShader*>(&resman.getShaderSet(Shaders::SSAOShader));
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject
	auto& pso = static_cast<SVkPipelineStateObject&>(resman.getPSO(shader, mesh));

	// Bind images to slots in shader with specific names
	{
		SVkShader::Descriptors descriptors;
		{
			auto& desc = descriptors.images.emplace_back();
			desc.name = "def_positions";
			auto& imageInfo = desc.infos.emplace_back();
			imageInfo.imageView = m_gbuffers.positions->getView(imageIndex);
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		{
			auto& desc = descriptors.images.emplace_back();
			desc.name = "def_worldNormals";
			auto& imageInfo = desc.infos.emplace_back();
			imageInfo.imageView = m_gbuffers.normals->getView(imageIndex);
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
		shader->updateDescriptors(descriptors, &pso);
	}

	// Draw
	{
		if (camera) {
			shader->trySetCBufferVar("sys_mView", &glm::transpose(camera->getViewMatrix()), sizeof(glm::mat4), cmd);
			shader->trySetCBufferVar("sys_mProjection", &glm::transpose(camera->getProjMatrix()), sizeof(glm::mat4), cmd);
		}
		auto& [kernelData, kernelDataSize] = m_ssao->getKernel();
		shader->trySetCBufferVar("kernel", kernelData, kernelDataSize, cmd);
		auto& [noiseData, noiseDataSize] = m_ssao->getNoise();
		shader->trySetCBufferVar("noise", noiseData, noiseDataSize, cmd);
		glm::vec2 ssaoSize(ssaoWidth, ssaoHeight);
		shader->trySetCBufferVar("windowSize", &ssaoSize, sizeof(glm::vec2), cmd);

		pso.bind(cmd); // Binds the pipeline and descriptor sets

		mesh->draw(*this, &m_shadingPassMaterial, shader, environment, cmd);
	}

	vkCmdEndRenderPass(cmd);
	
	// There is a render pass dependency that waits for the ssao texture to be created before blur computer shaders run

	// Blur ssao output
	auto& blurHorizontalShader = static_cast<SVkShader&>(resman.getShaderSet(Shaders::GaussianBlurHorizontalComputeShader));
	auto& blurVerticalShader = static_cast<SVkShader&>(resman.getShaderSet(Shaders::GaussianBlurVerticalComputeShader));

	auto& settingsHorizontal = blurHorizontalShader.getSettings().computeShaderSettings;
	auto& settingsVertical = blurVerticalShader.getSettings().computeShaderSettings;
	float textureSizeDiff = 1.f;

	// Dispatch horizontal blur pass
	{
		blurHorizontalShader.setCBufferVar("textureSizeDifference", &textureSizeDiff, sizeof(float), cmd);

		unsigned int x = (unsigned int)glm::ceil(ssaoWidth * settingsHorizontal.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(ssaoHeight * settingsHorizontal.threadGroupYScale);
		unsigned int z = 1;

		auto& pso = static_cast<SVkPipelineStateObject&>(resman.getPSO(&blurHorizontalShader));
		{
			// Bind image to slot in shader with a specific name
			SVkShader::Descriptors descriptors;
			auto& desc = descriptors.images.emplace_back();
			desc.name = "inoutput";
			auto& imageInfo = desc.infos.emplace_back();
			imageInfo.imageView = ssaoRenderTarget->getView(imageIndex);
			imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

			blurHorizontalShader.updateDescriptors(descriptors, &pso);
		}
		pso.bind(cmd);

		vkCmdDispatch(cmd, x, y, z);
	}

	// Memory barrier that waits for first blur stage to finish writing
	VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.image = ssaoRenderTarget->getImage();
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);


	// Dispatch vertical blur pass
	{
		blurVerticalShader.setCBufferVar("textureSizeDifference", &textureSizeDiff, sizeof(float), cmd);

		m_ssaoShadingTexture = static_cast<SVkRenderableTexture*>(blurVerticalShader.getRenderableTexture("output"));
		m_ssaoShadingTexture->resize(ssaoWidth, ssaoHeight);

		// Transition output texture to GENERAL
		VkImageMemoryBarrier imageMemoryBarrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		imageMemoryBarrier.image = m_ssaoShadingTexture->getImage();
		imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);

		unsigned int x = (unsigned int)glm::ceil(ssaoWidth * settingsVertical.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(ssaoHeight * settingsVertical.threadGroupYScale);
		unsigned int z = 1;
		
		auto& pso = static_cast<SVkPipelineStateObject&>(resman.getPSO(&blurVerticalShader));
		{
			// Bind images to slots in shader with specific names
			SVkShader::Descriptors descriptors;
			{
				auto& desc = descriptors.images.emplace_back();
				desc.name = "input";
				auto& imageInfo = desc.infos.emplace_back();
				imageInfo.imageView = ssaoRenderTarget->getView(imageIndex);
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			}
			{
				auto& desc = descriptors.images.emplace_back();
				desc.name = "output";
				auto& imageInfo = desc.infos.emplace_back();
				imageInfo.imageView = m_ssaoShadingTexture->getView(imageIndex);
				imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
			}

			blurVerticalShader.updateDescriptors(descriptors, &pso);
		}
		pso.bind(cmd);

		vkCmdDispatch(cmd, x, y, z);
	}

	// Image memory barrier to make sure that compute shader writes are finished before sampling from the texture
	// It also transitions the image to shader read optimal
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageMemoryBarrier.image = m_ssaoShadingTexture->getImage();
	imageMemoryBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	vkCmdPipelineBarrier( cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
}

void SVkDeferredRenderer::runShadingPass(const VkCommandBuffer& cmd) {
	// Fetch the swap image index to use this frame
	auto imageIndex = m_context->getSwapImageIndex();

	auto& resman = Application::getInstance()->getResourceManager();
	bool useSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);
	
	// TODO:: figure out if a semaphore or barrier is still needed to sync gbuffer pass finishing before shading pass starts

	{
		// Clear values for all attachments written in the fragment shader
		std::array<VkClearValue, 2> clearValues;
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBeginInfo.renderPass = m_shadingRenderPass;
		renderPassBeginInfo.framebuffer = m_context->getCurrentFramebuffer();
		renderPassBeginInfo.renderArea.extent = m_context->getCurrentExtent();
		renderPassBeginInfo.clearValueCount = clearValues.size();
		renderPassBeginInfo.pClearValues = clearValues.data();

		// Start render pass
		vkCmdBeginRenderPass(cmd, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}
	// Set dynamic viewport and scissor states
	vkCmdSetViewport(cmd, 0, 1, &m_context->getViewport());
	vkCmdSetScissor(cmd, 0, 1, &m_context->getScissorRect());


	auto* shader = static_cast<SVkShader*>(&resman.getShaderSet(Shaders::DeferredShadingPassShader));
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject
	auto& pso = static_cast<SVkPipelineStateObject&>(resman.getPSO(shader, mesh));

	// Set up material
	{
		m_shadingPassMaterial.clearTextures();
		// This order needs to match the indexing used in the shader
		m_shadingPassMaterial.addTexture(m_gbuffers.positions.get());
		m_shadingPassMaterial.addTexture(m_gbuffers.normals.get());
		m_shadingPassMaterial.addTexture(m_gbuffers.albedo.get());
		m_shadingPassMaterial.addTexture(m_gbuffers.mrao.get());

		if (m_ssao)
			m_shadingPassMaterial.addTexture(m_ssaoShadingTexture);
		//m_shadingPassMaterial.addTexture(shadows);

		m_shadingPassMaterial.addTexture(m_brdfLutTexture);
		m_shadingPassMaterial.addTexture(environment->getRadianceTexture());
		m_shadingPassMaterial.addTexture(environment->getIrradianceTexture());
	}
	
	Renderer::RenderCommand fakeRenderCmd;
	fakeRenderCmd.material = &m_shadingPassMaterial;
	std::vector< Renderer::RenderCommand> fakeVec = { fakeRenderCmd };
	// TODO: Replace this with a call to updateDescriptors()
	shader->updateDescriptorsAndMaterialIndices(fakeVec, &pso);

	int useSSAOInt = (int)useSSAO;
	shader->trySetCBufferVar("useSSAO", &useSSAOInt, sizeof(int), cmd);
	int useShadowTextureInt = 0;
	shader->trySetCBufferVar("useShadowTexture", &useShadowTextureInt, sizeof(int), cmd);

	if (camera) {
		// Transpose all matrices to convert them to row-major which is required in order for the hlsl->spir-v multiplication order
		shader->trySetCBufferVar("sys_mViewInv", &glm::transpose(glm::inverse(camera->getViewMatrix())), sizeof(glm::mat4), cmd);
		shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3), cmd);
	}
	if (lightSetup) {
		auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
		auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
		shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize, cmd);
		shader->trySetCBufferVar("pointLights", plData, plDataByteSize, cmd);
	}

	pso.bind(cmd); // Binds the pipeline and descriptor sets

	// Draw 
	mesh->draw(*this, &m_shadingPassMaterial, shader, environment, cmd);

	vkCmdEndRenderPass(cmd);
}

void SVkDeferredRenderer::runFrameExecution(const VkCommandBuffer& cmd) {
	if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
		Logger::Error("failed to record command buffer!");
		return;
	}

	// Submit the command buffer to the graphics queue
	m_context->submitCommandBuffers({ cmd });
}

