#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderPipeline.h"

class FXAAStage : public PostProcessStage {
public:
	FXAAStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullScreenQuad);
	virtual ~FXAAStage();

	bool onResize(WindowResizeEvent& event) override;

};
