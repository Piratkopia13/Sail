#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderSet.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../../Application.h"
#include "../../Lights.h"
#include "../../geometry/Material.h"

class DynBlockDeferredInstancedGeometryShader : public ShaderSet {
public:
	DynBlockDeferredInstancedGeometryShader();
	~DynBlockDeferredInstancedGeometryShader();

	void bind() override;

	virtual void draw(Model& model, bool bindFirst = true);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);
	void updateInstanceData(const void* instanceData, UINT bufferSize, ID3D11Buffer* instanceBuffer);

	virtual void setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane);

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texCoords;
		DirectX::SimpleMath::Vector3 normal;
		DirectX::SimpleMath::Vector3 tangent;
		DirectX::SimpleMath::Vector3 bitangent;
	};
	struct InstanceData {
		DirectX::SimpleMath::Matrix modelMatrix = DirectX::SimpleMath::Matrix::Identity;
		DirectX::SimpleMath::Vector3 color = DirectX::SimpleMath::Vector3::One;
		float blockVariationOffset = 0.f;
	};

private:
	void updateModelDataBuffer(const Material& material, const DirectX::SimpleMath::Matrix& wv, const DirectX::SimpleMath::Matrix& p) const;
	void updateWorldDataBuffer(const DirectX::SimpleMath::Vector4& clippingPlaneVS) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[11];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	// Transform constant buffer structure
	struct ModelDataBuffer {
		DirectX::SimpleMath::Matrix mV;
		DirectX::SimpleMath::Matrix mP;
		DirectX::SimpleMath::Vector4 modelColor;
		float ka;
		float kd;
		float ks;
		float shininess;
		int textureFlags[3];
		int padding;
	};
	struct WorldDataBuffer {
		DirectX::SimpleMath::Vector4 clippingPlaneVS;
	};
	DirectX::SimpleMath::Matrix m_mView, m_mProjection;
	DirectX::SimpleMath::Vector4 m_clippingPlane;
	bool m_clippingPlaneHasChanged;

	// Components

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_modelDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_worldDataBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};
