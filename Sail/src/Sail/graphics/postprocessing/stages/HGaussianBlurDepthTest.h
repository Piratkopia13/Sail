#pragma once

#include "PostProcessStage.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"

#include <memory>

class VertexShader;
class PixelShader;

class HGaussianBlurDepthTest : public PostProcessStage {
public:
	HGaussianBlurDepthTest(UINT width, UINT height, Model* fullscreenQuad);
	virtual ~HGaussianBlurDepthTest();

	void setDepthSRV(ID3D11ShaderResourceView** depthSRV);
	void run(RenderableTexture& inputTexture);
	void resize(UINT width, UINT height);

private:
	//GaussianBlurCShader m_gaussianShader;

	std::unique_ptr<VertexShader> m_VS;
	std::unique_ptr<PixelShader> m_PS;

	struct CBuffer {
		float invTexWidth;
		float invTexHeight;
		float padding[2];
	};

	// Components
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

	ID3D11ShaderResourceView** m_depthSRV;

};