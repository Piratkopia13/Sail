#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../../geometry/Model.h"
#include "../../geometry/Material.h"
#include "../../../Application.h"


class DepthShader : public ShaderSet {
public:
	DepthShader();
	~DepthShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);
	//virtual void setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane);

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texCoords;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 bitangent;
	};


private:
	void updateBuffer(const DirectX::SimpleMath::Matrix& w, const DirectX::SimpleMath::Matrix& vp) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[5];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	struct ModelDataBuffer {
		DirectX::SimpleMath::Matrix mW;
		DirectX::SimpleMath::Matrix mVP;
	};
	DirectX::SimpleMath::Matrix m_vpMatrix;

	// Components

	// Transform constant buffer structure
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_transformBuffer;

};
