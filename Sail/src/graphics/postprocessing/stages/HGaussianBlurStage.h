#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../api/Application.h"

class HGaussianBlurStage : public PostProcessStage {
public:
	HGaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad);
	virtual ~HGaussianBlurStage();

	void run(RenderableTexture& inputTexture) override;
	void resize(UINT width, UINT height) override;

};