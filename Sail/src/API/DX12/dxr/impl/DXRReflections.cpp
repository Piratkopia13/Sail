#include "pch.h"
#include "DXRReflections.h"
#include "../../renderer/DX12DeferredRenderer.h"
#include "../../shader/DX12ConstantBuffer.h"
#include "Sail/KeyCodes.h"
#include "Sail/api/Input.h"
#include "Sail/graphics/camera/Camera.h"
#include "Sail/graphics/light/LightSetup.h"

// Include defines shared with dxr shaders
#include "Sail/../../Demo/res/shaders/dxr/Reflections.shared"

DXRReflections::DXRReflections()
	: DXRBase("Reflections", {
		sizeof(DXRShaderReflections::RayPayload),		// Max payload size
		sizeof(float) * 2,								// Max attribute size
		DXRShaderReflections::MAX_RAY_RECURSION_DEPTH,	// Max recusion depth
		true, true										// Bind vbuffers, textures to hit group
		})
{
	// Temporary, this static getter should probably be removed
	m_gbuffers = DX12DeferredRenderer::GetGBuffers();
	assert(m_gbuffers);

	// Add gbuffer inputs
	{
		DXRBase::DescriptorTableData dtData;
		dtData.numDescriptors = 1;
		dtData.space = 0;
		dtData.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;

		dtData.resource = &m_gbufferPositionsResource;
		dtData.shaderRegister = 1;
		rayGenDescriptorTables.insert({ "gbufferPositionsInputSRV", dtData });

		dtData.resource = &m_gbufferNormalsResource;
		dtData.shaderRegister = 2;
		rayGenDescriptorTables.insert({ "gbufferNormalsInputSRV", dtData });
	}
	//// Add Vertex buffer inputs
	//{
	//	DXRBase::DescriptorTableData dtData;
	//	dtData.numDescriptors = 1;
	//	dtData.space = 0;
	//	dtData.type = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	//	dtData.resource = ??;
	//	dtData.shaderRegister = 3;
	//	rayGenDescriptorTables.insert({ "vertices", dtData });
	//}

	// Create cbuffer holding required scene information
	{
		unsigned int size = sizeof(DXRShaderReflections::SceneCBuffer);
		DXRShaderReflections::SceneCBuffer initData = {};
		m_sceneCB = std::make_unique<ShaderComponent::DX12ConstantBuffer>(&initData, size, ShaderComponent::BIND_SHADER::CS, 0);
	}

	// Add cbuffer input
	{
		DXRBase::ConstantData cData;
		cData.cbuffer = m_sceneCB.get();
		cData.shaderRegister = 0;
		cData.space = 0;
		globalConstants.insert({ "SceneCBuffer", cData });
	}

	init();
}

void DXRReflections::updateSceneData(Camera* cam, LightSetup* lights) {
	if (Input::IsKeyPressed(SAIL_KEY_R))
		reloadShaders();

	DXRShaderReflections::SceneCBuffer newData = {};
	if (cam) {
		newData.cameraPosition = cam->getPosition();
		newData.viewToWorld = glm::inverse(cam->getViewMatrix());
	}

	m_sceneCB->updateData(&newData, sizeof(newData), 0U);
}

void DXRReflections::addInitialShaderResources(DescriptorHeap* heap) {
	// Gbuffer inputs
	for (unsigned int i = 0; i < 2; i++) {
		auto index = heap->getAndStepIndex();
		m_gbufferPositionsResource.cpuHandle[i] = heap->getCPUDescriptorHandleForIndex(index);
		m_gbufferPositionsResource.gpuHandle[i] = heap->getGPUDescriptorHandleForIndex(index);
		context->getDevice()->CopyDescriptorsSimple(1, m_gbufferPositionsResource.cpuHandle[i], m_gbuffers[0]->getSrvCDH(i), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		index = heap->getAndStepIndex();
		m_gbufferNormalsResource.cpuHandle[i] = heap->getCPUDescriptorHandleForIndex(index);
		m_gbufferNormalsResource.gpuHandle[i] = heap->getGPUDescriptorHandleForIndex(index);
		context->getDevice()->CopyDescriptorsSimple(1, m_gbufferNormalsResource.cpuHandle[i], m_gbuffers[1]->getSrvCDH(i), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}
