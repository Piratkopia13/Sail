#pragma once
#include "../DXRBase.h"

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
	const std::unique_ptr<DX12RenderableTexture>* m_gbuffers;
	std::unique_ptr<ShaderComponent::DX12ConstantBuffer> m_sceneCB;

};