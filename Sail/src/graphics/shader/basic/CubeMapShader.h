#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../../Application.h"
#include "../../Lights.h"
#include "../../geometry/Model.h"
#include "../../geometry/Material.h"

class CubeMapShader : public ShaderSet {
public:
	CubeMapShader();
	~CubeMapShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);

	virtual void setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane);

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

private:
	void updateModelDataBuffer(const Material& material, const DirectX::SimpleMath::Matrix& w, const DirectX::SimpleMath::Matrix& vp) const;
	void updateWorldDataBuffer() const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[1];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	// Transform constant buffer structure
	struct ModelDataBuffer {
		DirectX::SimpleMath::Matrix mWorld;
		DirectX::SimpleMath::Matrix mVP;
	};
	struct WorldDataBuffer {
		DirectX::SimpleMath::Vector4 clippingPlane;
		DirectX::SimpleMath::Vector3 cameraPos;
		float padding;
	};
	DirectX::SimpleMath::Matrix m_vpMatrix;
	DirectX::SimpleMath::Vector4 m_clippingPlane;
	DirectX::SimpleMath::Vector3 m_cameraPos;
	bool m_clippingPlaneHasChanged;
	bool m_cameraPosHasChanged;

	// Components

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_worldDataBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};
