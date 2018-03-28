#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../api/Application.h"

class FXAAStage : public PostProcessStage {
public:
	FXAAStage(UINT width, UINT height, Model* fullScreenQuad);
	virtual ~FXAAStage();

	void run(RenderableTexture& inputTexture);
	void resize(UINT width, UINT height);

private:
	std::unique_ptr<VertexShader> m_VS;
	std::unique_ptr<PixelShader> m_PS;

	struct CBuffer {
		float texWidth;
		float texHeight;
		float padding[2];
	};

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;
};
