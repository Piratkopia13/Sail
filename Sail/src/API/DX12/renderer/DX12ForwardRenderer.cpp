#include "pch.h"
#include "DX12ForwardRenderer.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12ShaderPipeline.h"
#include "../resources/DescriptorHeap.h"
#include "Sail/graphics/shader/compute/TestComputeShader.h"
#include "../resources/DX12Texture.h"

DX12ForwardRenderer::DX12ForwardRenderer() {
	m_context = Application::getInstance()->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Forward Renderer main command list");
}

DX12ForwardRenderer::~DX12ForwardRenderer() {

}

void DX12ForwardRenderer::present(RenderableTexture* output) {
	assert(!output); // Render to texture is currently not implemented for DX12!

	auto frameIndex = m_context->getFrameIndex();

	auto& allocator = m_command.allocators[frameIndex];
	auto& cmdList = m_command.list;

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

	// Compute shader testing
	auto* computeShader = &Application::getInstance()->getResourceManager().getShaderSet<TestComputeShader>();
	cmdList->SetComputeRootSignature(m_context->getGlobalRootSignature());
	// Set offset in SRV heap for this mesh 
	cmdList->SetComputeRootDescriptorTable(m_context->getRootIndexFromRegister("t0"), m_context->getMainGPUDescriptorHeap()->getGPUDescriptorHandleForIndex(1));
	computeShader->getPipeline()->bind(cmdList.Get());
	// Bind the second mesh's textures
	auto* mat = commandQueue.at(1).mesh->getMaterial();
	mat->bind(cmdList.Get());
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), static_cast<DX12Texture*>(mat->getTexture(0))->getBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_GENERIC_READ);
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), static_cast<DX12Texture*>(mat->getTexture(1))->getBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_GENERIC_READ);
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), static_cast<DX12Texture*>(mat->getTexture(2))->getBuffer(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_GENERIC_READ);
	computeShader->getPipeline()->dispatch(10, 10, 10, cmdList.Get());
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), static_cast<DX12Texture*>(mat->getTexture(0))->getBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), static_cast<DX12Texture*>(mat->getTexture(1))->getBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), static_cast<DX12Texture*>(mat->getTexture(2))->getBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);


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
	m_context->executeCommandLists({ cmdList.Get() });

}
