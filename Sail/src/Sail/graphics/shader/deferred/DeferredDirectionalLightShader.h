#pragma once

#include <d3d11.h>
#include <glm/glm.hpp>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "Sail/Application.h"
#include "../../light/LightSetup.h"
#include "../../geometry/Material.h"
#include "../../../api/RenderableTexture.h"

class DeferredDirectionalLightShader : public ShaderSet {
public:
	DeferredDirectionalLightShader();
	~DeferredDirectionalLightShader();

	void bind() override;

	void setLight(const DirectionalLight& dl, Camera* camera);

private:
	//// Transform constant buffer structure
	//struct ModelDataBuffer {
	//	glm::mat4 mInvP;
	//};
	struct LightDataBuffer {
		glm::vec3 direction;
		float padding;
		glm::vec3 color;
		float padding2;
	};
	//struct ShadowLightBuffer {
	//	glm::mat4 mInvV;
	//	glm::mat4 mLightV;
	//	glm::mat4 mLightP;
	//};
	//glm::mat4 m_mV;
	//glm::mat4 m_mInvP;

	//// Components

	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_lightDataBuffer;
	//std::unique_ptr<ShaderComponent::ConstantBuffer> m_shadowLightBuffer;
	//std::unique_ptr<ShaderComponent::Sampler> m_sampler;
	//std::unique_ptr<ShaderComponent::Sampler> m_shadowSampler;

};
