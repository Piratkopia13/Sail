#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderPipeline.h"
#include "../../shader/component/ConstantBuffer.h"
#include "../../shader/component/Sampler.h"
#include "Sail/Application.h"

class VGaussianBlurStage : public PostProcessStage {
public:
	VGaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad);
	virtual ~VGaussianBlurStage();

	bool onResize(WindowResizeEvent& event) override;

};