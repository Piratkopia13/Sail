#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../api/Application.h"
//#include "../../shader/postprocess/GaussianBlurCShader.h"

class HGaussianBlurStage : public PostProcessStage {
public:
	HGaussianBlurStage(UINT width, UINT height, Model* fullscreenQuad);
	virtual ~HGaussianBlurStage();

	void run(RenderableTexture& inputTexture);
	void resize(UINT width, UINT height);

private:
	//GaussianBlurCShader m_gaussianShader;

	std::unique_ptr<VertexShader> m_VS;
	std::unique_ptr<PixelShader> m_PS;

	struct CBuffer {
		float texWidth;
		float texHeight;
		float padding[2];
	};

	// Components
	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;


};