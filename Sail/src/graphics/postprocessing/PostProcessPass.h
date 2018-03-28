#pragma once

#include "../RenderableTexture.h"
#include "../geometry/Model.h"

#include "stages/PostProcessStage.h"
#include "stages/HGaussianBlurStage.h"
#include "stages/VGaussianBlurStage.h"
#include "stages/VGaussianBlurDepthTest.h"
#include "stages/HGaussianBlurDepthTest.h"
#include "stages/BrightnessCutoffStage.h"
#include "stages/ToneMapHackStage.h"
#include "stages/FXAAStage.h"
//#include "stages/DOFStage.h"
#include "stages/GaussianDOFStage.h"
#include "stages/BlendStage.h"
//#include "../shader/postprocess/GaussianBlurCShader.h"
#include "../shader/postprocess/PostProcessFlushShader.h"

class PostProcessPass {
public:
	PostProcessPass(const Camera* cam = nullptr);
	~PostProcessPass();

	void run(RenderableTexture& baseTexture, ID3D11ShaderResourceView** depthTexture, RenderableTexture& bloomInputTexture, RenderableTexture& particlesTexture);

	void resize(UINT width, UINT height);

	void setCamera(const Camera& cam);
private:
	void createFullscreenQuad();

private:
	// Post process stages - TODO: make this a map
	std::unique_ptr<HGaussianBlurStage> m_hGaussStage;
	std::unique_ptr<VGaussianBlurStage> m_vGaussStage;
	std::unique_ptr<HGaussianBlurStage> m_hGaussStage2;
	std::unique_ptr<VGaussianBlurStage> m_vGaussStage2;
	std::unique_ptr<HGaussianBlurStage> m_hGaussStage3;
	std::unique_ptr<VGaussianBlurStage> m_vGaussStage3;

	std::unique_ptr<HGaussianBlurDepthTest> m_hGaussDepthStage;
	std::unique_ptr<VGaussianBlurDepthTest> m_vGaussDepthStage;

	std::unique_ptr<FXAAStage> m_FXAAStage;
	std::unique_ptr<BrightnessCutoffStage> m_brightnessCutoffStage;
	//std::unique_ptr<DOFStage> m_dofStage;
	std::unique_ptr<GaussianDOFStage> m_gaussianDofStage;
	std::unique_ptr<ToneMapHackStage> m_toneMapHackStage;
	std::unique_ptr<BlendStage> m_blendStage;
	std::unique_ptr<BlendStage> m_blendStage2;

	PostProcessFlushShader m_flushShader;

	float m_gaussPass1Scale;
	float m_gaussPass2Scale;
	float m_gaussPass3Scale;
	float m_brightnessCutoffScale;
	float m_gaussDepthPassScale;
	float m_dofFocusWidth;

	bool m_doFXAAPass;
	bool m_doDOFPass;
	bool m_doBloom;
	float m_bloomStrength;

	// TODO make enableEffect(enum) and disableEffect(enum) and a map that binds the enums to enabled/disabled bool

	const Camera* m_cam;
	std::unique_ptr<Model> m_fullscreenQuad;

};