#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../../geometry/Model.h"
#include "../../geometry/Material.h"
#include "../../../api/Application.h"


class SimpleColorShader : public ShaderSet {
public:
	SimpleColorShader();
	~SimpleColorShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);

	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};


private:
	void updateBuffer(const DirectX::SimpleMath::Vector4& color, const DirectX::SimpleMath::Matrix& mvp) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[2];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	struct ModelDataBuffer {
		DirectX::SimpleMath::Vector4 modelColor;
		DirectX::SimpleMath::Matrix mModelViewProj;
	};
	DirectX::SimpleMath::Matrix m_vpMatrix;

	// Components

	// Transform constant buffer structure
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_transformBuffer;

};
