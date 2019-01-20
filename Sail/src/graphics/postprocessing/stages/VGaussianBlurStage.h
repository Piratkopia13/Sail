#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "../../../api/Application.h"

class VGaussianBlurStage : public PostProcessStage {
public:
	VGaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad);
	virtual ~VGaussianBlurStage();

	bool onResize(WindowResizeEvent& event) override;

};