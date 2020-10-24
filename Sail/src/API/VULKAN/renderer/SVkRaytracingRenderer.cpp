#include "pch.h"
#include "SVkRaytracingRenderer.h"
#include "Sail/graphics/geometry/factory/Cube.h"
#include "../SVkUtils.h"

std::unique_ptr<SVkRenderableTexture> SVkRaytracingRenderer::sRTOutputTexture;

SVkRaytracingRenderer::SVkRaytracingRenderer() {
	EventSystem::getInstance()->subscribeToEvent(Event::SWAPCHAIN_RECREATED, this);

	m_context = Application::getInstance()->getAPI<SVkAPI>();
	m_context->initCommand(m_command);

	auto width = Application::getInstance()->getWindow()->getWindowWidth();
	auto height = Application::getInstance()->getWindow()->getWindowHeight();
	sRTOutputTexture = std::unique_ptr<SVkRenderableTexture>(static_cast<SVkRenderableTexture*>(RenderableTexture::Create(width, height, RenderableTexture::USAGE_GENERAL, "Raytracing output texture", ResourceFormat::R16G16B16A16_FLOAT)));

	//m_testMesh = MeshFactory::Cube::Create({ .5f, .5f, .5f });
	m_testMesh = Application::getInstance()->getResourceManager().loadMesh("sphere.fbx");
	m_testMesh2 = Application::getInstance()->getResourceManager().loadMesh("Cerberus_LP.fbx");
}

SVkRaytracingRenderer::~SVkRaytracingRenderer() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::SWAPCHAIN_RECREATED, this);
	vkDeviceWaitIdle(m_context->getDevice());
	sRTOutputTexture.reset();
}

void SVkRaytracingRenderer::begin(Camera* camera, Environment* environment) {
	Renderer::begin(camera, environment);
}

void* SVkRaytracingRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {

	auto swapIndex = m_context->getSwapIndex();
	//auto& cmd = m_command.buffers[swapIndex];
	auto cmd = static_cast<VkCommandBuffer>(skippedPrepCmdList);

	/*VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;*/

	//VkResult a = vkBeginCommandBuffer(cmd, &beginInfo);
	//VK_CHECK_RESULT(a);



	m_rtBase.update(commandQueueCustom, camera, lightSetup, cmd);

	SVkUtils::TransitionImageLayout(cmd, sRTOutputTexture->getImage(), sRTOutputTexture->getFormat(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);
	m_rtBase.dispatch(sRTOutputTexture.get(), camera, lightSetup, cmd);
	SVkUtils::TransitionImageLayout(cmd, sRTOutputTexture->getImage(), sRTOutputTexture->getFormat(), VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	//a = vkEndCommandBuffer(cmd);
	//VK_CHECK_RESULT(a);

	// Submit the command buffer to the graphics queue
	//m_context->submitCommandBuffers({ cmd });

	return nullptr;
}

SVkRenderableTexture* SVkRaytracingRenderer::GetOutputTexture() {
	return sRTOutputTexture.get();
}

bool SVkRaytracingRenderer::onEvent(Event& event) {
	auto e = [&](SwapchainRecreatedEvent& event) {
		auto width = Application::getInstance()->getWindow()->getWindowWidth();
		auto height = Application::getInstance()->getWindow()->getWindowHeight();

		sRTOutputTexture->resize(width, height);

		return true;
	};


	EventHandler::HandleType<SwapchainRecreatedEvent>(event, e);
	return true;
}
