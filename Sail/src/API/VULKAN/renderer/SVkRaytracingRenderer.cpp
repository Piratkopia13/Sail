#include "pch.h"
#include "SVkRaytracingRenderer.h"
#include "Sail/graphics/geometry/factory/Cube.h"
#include "../SVkUtils.h"

std::unique_ptr<SVkRenderableTexture> SVkRaytracingRenderer::sRTOutputTexture;

SVkRaytracingRenderer::SVkRaytracingRenderer() {
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


	//static bool asBuilt = false;
	//if (!asBuilt) {

		std::vector<Renderer::RenderCommand> testList;
		{
			auto& rcmd = testList.emplace_back();
			rcmd.dxrFlags = MESH_STATIC;
			rcmd.mesh = m_testMesh.get();
			rcmd.transform = glm::mat4(1.0f);
		}
		{
			auto& rcmd = testList.emplace_back();
			rcmd.dxrFlags = MESH_STATIC;
			rcmd.mesh = m_testMesh2.get();
			rcmd.transform = glm::translate(glm::vec3(0.f, 1.f, 0.f));
		}

		m_rtBase.update(testList, camera, lightSetup, cmd);
		/*m_rtBase.createBLAS(m_testMesh.get(), VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, cmd);
		m_rtBase.createTLAS(VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR, cmd);*/

		//asBuilt = true;
	//}

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
	return true;
}
