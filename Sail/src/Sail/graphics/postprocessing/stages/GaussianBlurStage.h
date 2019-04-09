#pragma once

#include "PostProcessStage.h"
#include "../../shader/ShaderSet.h"

/**
 * Two pass gaussian blur
 */
class GaussianBlurStage : public PostProcessStage {
public:
	GaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad);
	virtual ~GaussianBlurStage();

	virtual void run(DX11RenderableTexture& inputTexture) override;
	bool onResize(WindowResizeEvent& event) override;

private:
	std::unique_ptr<ShaderSet> m_verticalPassShader;
	DX11RenderableTexture m_firstOutputTexture;

};