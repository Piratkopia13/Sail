#pragma once

#include <d3d11.h>
#include <SimpleMath.h>

#include "../component/ConstantBuffer.h"
#include "../component/Sampler.h"
#include "../../camera/Camera.h"
#include "../../../Application.h"
#include "../ShaderSet.h"
#include "../../RenderableTexture.h"

class GaussianBlurCShaderPoo : public ShaderSet {
public:
	GaussianBlurCShaderPoo();
	~GaussianBlurCShaderPoo();

	void bind() override;

	void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, const void* data) {}

	void createBuffers();

	virtual void draw(bool bindFirst = true);

	void setOutputTexture(RenderableTexture* tex);
 	void setInputSRV(ID3D11ShaderResourceView** srv);
	void setTextureSize(UINT width, UINT height);

	void setFullScreenQuadModel(Model* model);
	void resize(int width, int height);

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

private:
	void updateConstantBuffer() const;
	void setHorPassUAV(ID3D11Texture2D* tex);

private:

	UINT m_texWidth, m_texHeight;

	ID3D11UnorderedAccessView* m_horPassUAV;
	ID3D11UnorderedAccessView* m_vertPassUAV;

	ID3D11ShaderResourceView** m_horInputSRV;
	ID3D11ShaderResourceView** m_vertInputSRV;

	std::unique_ptr<RenderableTexture> m_middleTex;

	Model* m_fullScreenQuadModel;

	RenderableTexture* m_outputTex;

	std::unique_ptr<PixelShader> m_vertPS;

	struct CBuffer {
		float pixelSize;
		float padding[3];
	};

	// Components
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};