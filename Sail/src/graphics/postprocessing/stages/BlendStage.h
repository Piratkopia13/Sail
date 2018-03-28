#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../api/Application.h"

class BlendStage : public PostProcessStage {
public:
	BlendStage(UINT width, UINT height, Model* fullScreenQuad);
	virtual ~BlendStage();

	void setBlendInput(ID3D11ShaderResourceView** blendSRV, float blendFactor = 1.0f);
	void run(RenderableTexture& colorTexture);

private:
	std::unique_ptr<VertexShader> m_VS;
	std::unique_ptr<PixelShader> m_PS;

	struct CBuffer {
		float blendFactor;
		float padding[3];
	};

	std::unique_ptr<ShaderComponent::ConstantBuffer> m_cBuffer;
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

	ID3D11ShaderResourceView** m_blendSRV;
};
