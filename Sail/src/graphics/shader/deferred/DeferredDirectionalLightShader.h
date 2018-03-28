#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../../api/Application.h"
#include "../../light/LightSetup.h"
#include "../../geometry/Material.h"
#include "../../RenderableTexture.h"

class DeferredDirectionalLightShader : public ShaderSet {
public:
	DeferredDirectionalLightShader();
	~DeferredDirectionalLightShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);

	void setLight(const DirectionalLight& dl);

	void updateCameraBuffer(Camera& cam, Camera& dlCam);

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

private:
	void updateModelDataBuffer(const DirectX::SimpleMath::Matrix& invP) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[1];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	// Transform constant buffer structure
	struct ModelDataBuffer {
		DirectX::SimpleMath::Matrix mInvP;
	};
	struct LightDataBuffer {
		DirectX::SimpleMath::Vector3 direction;
		float padding;
		DirectX::SimpleMath::Vector3 color;
		float padding2;
	};
	struct ShadowLightBuffer {
		DirectX::SimpleMath::Matrix mInvV;
		DirectX::SimpleMath::Matrix mLightV;
		DirectX::SimpleMath::Matrix mLightP;
	};
	DirectX::SimpleMath::Matrix m_mV;
	DirectX::SimpleMath::Matrix m_mInvP;

	// Components

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_lightDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_shadowLightBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;
	std::unique_ptr<ShaderComponent::Sampler> m_shadowSampler;

};
