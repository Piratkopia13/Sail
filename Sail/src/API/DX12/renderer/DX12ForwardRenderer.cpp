#include "pch.h"
#include "DX12ForwardRenderer.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12ShaderPipeline.h"
#include "../resources/DescriptorHeap.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new DX12ForwardRenderer();
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
	}
	return nullptr;
}

DX12ForwardRenderer::DX12ForwardRenderer() {
	m_context = Application::getInstance()->getAPI<DX12API>();

	for (size_t i = 0; i < nRecordThreads; i++)
	{
		m_context->initCommand(m_command[i]);
		std::wstring name = L"Forward Renderer main command list for render thread: " + std::to_wstring(i);
		m_command[i].list->SetName(name.c_str());
	}

}

DX12ForwardRenderer::~DX12ForwardRenderer() {

}

void test(int id) {

}

void DX12ForwardRenderer::present(RenderableTexture* output) {
	//assert(!output); // Render to texture is currently not implemented for DX12!

	//auto frameIndex = m_context->getFrameIndex();

	//int a = 0;
	//Application::getInstance()->pushJobToThreadPool([this, a](int id) {
	//	test(a);
	//});

	//RecordCommands(0, frameIndex);

	//m_context->executeCommandLists({ cmdList.Get() });

}

void DX12ForwardRenderer::RecordCommands(const int threadID, const int frameIndex)
{
#ifdef MULTI_THREADED_COMMAND_RECORDING

#else
	auto& allocator = m_command[0].allocators[frameIndex];
	auto& cmdList = m_command[0].list;

	// Reset allocators and lists for this frame
	allocator->Reset();
	cmdList->Reset(allocator.Get(), nullptr);

	// Transition back buffer to render target
	m_context->prepareToRender(cmdList.Get());

	m_context->renderToBackBuffer(cmdList.Get());
	cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind the descriptor heap that will contain all SRVs for this frame
	m_context->getMainGPUDescriptorHeap()->bind(cmdList.Get());

	// Bind mesh-common constant buffers (camera)
	// TODO: bind camera cbuffer here
	//cmdList->SetGraphicsRootConstantBufferView(GlobalRootParam::CBV_CAMERA, asdf);

	// TODO: Sort meshes according to material
	unsigned int meshIndex = 0;
	for (RenderCommand& command : commandQueue) {
		DX12ShaderPipeline* shaderPipeline = static_cast<DX12ShaderPipeline*>(command.mesh->getMaterial()->getShader()->getPipeline());

		// Set mesh index which is used to bind the correct cbuffers from the resource heap
		// The index order does not matter, as long as the same index is used for bind and setCBuffer
		shaderPipeline->setResourceHeapMeshIndex(meshIndex);

		shaderPipeline->bind(cmdList.Get());

		shaderPipeline->setCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
		shaderPipeline->setCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
		shaderPipeline->setCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

		if (lightSetup) {
			auto& dlData = lightSetup->getDirLightData();
			auto& plData = lightSetup->getPointLightsData();
			shaderPipeline->setCBufferVar("dirLight", &dlData, sizeof(dlData));
			shaderPipeline->setCBufferVar("pointLights", &plData, sizeof(plData));
		}

		command.mesh->draw(*this, cmdList.Get());
		meshIndex++;
	}

	// Lastly - transition back buffer to present
	m_context->prepareToPresent(cmdList.Get());
	// Execute command list
	cmdList->Close();
#endif
}