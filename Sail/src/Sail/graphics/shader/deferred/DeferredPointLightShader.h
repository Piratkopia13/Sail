#pragma once

#include "../ShaderPipeline.h"
#include "../../light/PointLight.h"

class DeferredPointLightShader : public ShaderPipeline {
public:
	DeferredPointLightShader();
	~DeferredPointLightShader();

	void bind() override;

	void setLight(const PointLight& pl, Camera* camera);

private:

	//// Transform constant buffer structure
	//struct ModelDataBuffer {
	//	glm::mat4 mWV;
	//	glm::mat4 mP;
	//};
	struct LightDataBuffer {
		glm::vec3 color;
		float attCostant;
		float attLinear;
		float attQuadratic;
		float padding[2];
		glm::vec3 positionVS; // View space position of pointlight
		float padding2;
	};
	//glm::mat4 m_mV;
	//glm::mat4 m_mP;

	//// Components

	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_lightDataBuffer;
	//std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};
