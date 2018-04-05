#pragma once

#include "../ShaderSet.h"
#include "../../light/PointLight.h"

class DeferredPointLightShader : public ShaderSet {
public:
	DeferredPointLightShader();
	~DeferredPointLightShader();

	void bind() override;

	void setLight(const PointLight& pl, Camera* camera);

private:

	//// Transform constant buffer structure
	//struct ModelDataBuffer {
	//	DirectX::SimpleMath::Matrix mWV;
	//	DirectX::SimpleMath::Matrix mP;
	//};
	struct LightDataBuffer {
		DirectX::SimpleMath::Vector3 color;
		float attCostant;
		float attLinear;
		float attQuadratic;
		float padding[2];
		DirectX::SimpleMath::Vector3 positionVS; // View space position of pointlight
		float padding2;
	};
	//DirectX::SimpleMath::Matrix m_mV;
	//DirectX::SimpleMath::Matrix m_mP;

	//// Components

	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_lightDataBuffer;
	//std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};
