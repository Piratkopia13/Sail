#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../../Application.h"
#include "../../light/LightSetup.h"
#include "../../geometry/Material.h"

class DeferredPointLightShader : public ShaderSet {
public:
	DeferredPointLightShader();
	~DeferredPointLightShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);
	
	void setLight(const PointLight& pl);

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

private:
	void updateModelDataBuffer(const DirectX::SimpleMath::Matrix& wv, const DirectX::SimpleMath::Matrix& p) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[1];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	// Transform constant buffer structure
	struct ModelDataBuffer {
		DirectX::SimpleMath::Matrix mWV;
		DirectX::SimpleMath::Matrix mP;
	};
	struct LightDataBuffer {
		DirectX::SimpleMath::Vector3 color;
		float attCostant;
		float attLinear;
		float attQuadratic;
		float padding[2];
		DirectX::SimpleMath::Vector3 positionVS; // View space position of pointlight
		float padding2;
	};
	DirectX::SimpleMath::Matrix m_mV;
	DirectX::SimpleMath::Matrix m_mP;

	// Components

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_lightDataBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};
