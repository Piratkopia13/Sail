#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../ShaderPipeline.h"
#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../../api/Application.h"
#include "../../geometry/Material.h"

class ParticleShader : public ShaderSet {
public:
	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

	struct InstanceData {
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector4 color;
		DirectX::SimpleMath::Vector2 textureOffset1;
		DirectX::SimpleMath::Vector2 textureOffset2;
		float blendFactor;
		float padding[2];
	};
public:
	ParticleShader();
	~ParticleShader();

	void bind() override;

	void draw(Model& model, bool bindFirst = true, UINT instanceCount = -1);

	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data);

	virtual void updateCamera(Camera& cam);
	void updateSpriteData(UINT spritesPerRow, float scale);

	void updateInstanceData(const void* instanceData, UINT bufferSize, ID3D11Buffer* instanceBuffer);

private:
	void updateCameraBuffer(const DirectX::SimpleMath::Matrix& vp) const;

private:
	// Input element description
	static D3D11_INPUT_ELEMENT_DESC IED[5];
	// Input layout
	ID3D11InputLayout* m_inputLayout;

	// Transform constant buffer structure
	struct CameraBuffer {
		DirectX::SimpleMath::Matrix mVP;
		DirectX::SimpleMath::Vector3 camPos;
		float padding;
	};
	DirectX::SimpleMath::Matrix m_mV;
	DirectX::SimpleMath::Matrix m_mP;
	DirectX::SimpleMath::Vector3 m_camPos;
	struct SpriteData {
		UINT spritesPerRow;
		float scale;
		float padding[2];
	};

	// Components
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cameraDataBuffer;
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_spriteDataBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};
