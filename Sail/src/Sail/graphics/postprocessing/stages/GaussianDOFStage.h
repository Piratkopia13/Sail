#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderPipeline.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "Sail/Application.h"

class GaussianDOFStage : public PostProcessStage {
public:
	GaussianDOFStage(UINT width, UINT height, Model* fullScreenQuad);
	virtual ~GaussianDOFStage();

	void setDepthSRV(ID3D11ShaderResourceView** depthSRV);
	void setBlurredSRV(ID3D11ShaderResourceView** blurredSRV);
	void run(RenderableTexture& colorTexture);
	void setFocus(float focusDistance, float focusWidth);

private:
	std::unique_ptr<VertexShader> m_VS;
	std::unique_ptr<PixelShader> m_PS;

	struct CBuffer {
		float zNear;
		float zFar;
		float focalDistance;
		float focalWidth;
	};

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

	ID3D11ShaderResourceView** m_depthSRV;
	ID3D11ShaderResourceView** m_blurredSRV;
};
