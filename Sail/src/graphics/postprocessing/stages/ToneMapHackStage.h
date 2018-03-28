#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../api/Application.h"

class ToneMapHackStage : public PostProcessStage {
public:
	ToneMapHackStage(UINT width, UINT height, Model* fullscreenQuad);
	virtual ~ToneMapHackStage();

	void run(RenderableTexture& inputTexture);

private:

	std::unique_ptr<VertexShader> m_VS;
	std::unique_ptr<PixelShader> m_PS;

	// Components
	std::unique_ptr<ShaderComponent::Sampler> m_sampler;

};