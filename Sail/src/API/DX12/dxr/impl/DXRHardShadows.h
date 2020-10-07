#pragma once
#include "../DXRBase.h"
#include "../../renderer/DX12DeferredRenderer.h"

class DXRHardShadows : public DXRBase {
public:
	DXRHardShadows();

	void updateSceneData(Camera* cam, LightSetup* lights);

private:
	virtual void addInitialShaderResources(DescriptorHeap* heap) override;

private:
	// DXR shader inputs and outputs
	Resource m_gbufferPositionsResource;
	Resource m_gbufferNormalsResource;
	const DX12DeferredRenderer::GBufferTextures* m_gbuffers;
	std::unique_ptr<ShaderComponent::DX12ConstantBuffer> m_sceneCB;

};