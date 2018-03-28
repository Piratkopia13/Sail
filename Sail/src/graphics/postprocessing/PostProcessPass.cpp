#include "PostProcessPass.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

PostProcessPass::PostProcessPass(const Camera* cam)
	: m_cam(cam)
{
	createFullscreenQuad();

	Application* app = Application::getInstance();
	UINT windowWidth = app->getWindow()->getWindowWidth();
	UINT windowHeight = app->getWindow()->getWindowHeight();

	m_gaussPass1Scale = 1.f / 1.f;
	m_gaussPass2Scale = 1.f / 1.5f;
	m_gaussPass3Scale = 1.f / 2.f;
	m_brightnessCutoffScale = 1.f / 1.f;
	m_gaussDepthPassScale = 1.f / 2.f;
	m_dofFocusWidth = 220.f;

	m_doFXAAPass = true;
	m_doDOFPass = true;
	m_doBloom = true;

	if (m_doFXAAPass)
		m_FXAAStage = std::make_unique<FXAAStage>(windowWidth, windowHeight, m_fullscreenQuad.get());

	if (m_doBloom || m_doDOFPass) {
		m_hGaussStage = std::make_unique<HGaussianBlurStage>(UINT(windowWidth * m_gaussPass1Scale), UINT(windowHeight * m_gaussPass1Scale), m_fullscreenQuad.get());
		m_vGaussStage = std::make_unique<VGaussianBlurStage>(UINT(windowWidth * m_gaussPass1Scale), UINT(windowHeight * m_gaussPass1Scale), m_fullscreenQuad.get());
		m_hGaussStage2 = std::make_unique<HGaussianBlurStage>(UINT(windowWidth * m_gaussPass2Scale), UINT(windowHeight * m_gaussPass2Scale), m_fullscreenQuad.get());
		m_vGaussStage2 = std::make_unique<VGaussianBlurStage>(UINT(windowWidth * m_gaussPass2Scale), UINT(windowHeight * m_gaussPass2Scale), m_fullscreenQuad.get());
		m_hGaussStage3 = std::make_unique<HGaussianBlurStage>(UINT(windowWidth * m_gaussPass3Scale), UINT(windowHeight * m_gaussPass3Scale), m_fullscreenQuad.get());
		m_vGaussStage3 = std::make_unique<VGaussianBlurStage>(UINT(windowWidth * m_gaussPass3Scale), UINT(windowHeight * m_gaussPass3Scale), m_fullscreenQuad.get());
	}

	if (m_doDOFPass) {
		m_hGaussDepthStage = std::make_unique<HGaussianBlurDepthTest>(UINT(windowWidth * m_gaussDepthPassScale), UINT(windowHeight * m_gaussDepthPassScale), m_fullscreenQuad.get());
		m_vGaussDepthStage = std::make_unique<VGaussianBlurDepthTest>(UINT(windowWidth * m_gaussDepthPassScale), UINT(windowHeight * m_gaussDepthPassScale), m_fullscreenQuad.get());
		m_gaussianDofStage = std::make_unique<GaussianDOFStage>(windowWidth, windowHeight, m_fullscreenQuad.get());
	}

	if (m_doBloom) {
		m_brightnessCutoffStage = std::make_unique<BrightnessCutoffStage>(UINT(windowWidth * m_brightnessCutoffScale), UINT(windowHeight * m_brightnessCutoffScale), m_fullscreenQuad.get());
		m_blendStage = std::make_unique<BlendStage>(windowWidth, windowHeight, m_fullscreenQuad.get());
	}
	//m_dofStage = std::make_unique<DOFStage>(windowWidth, windowHeight, m_fullscreenQuad.get());
	m_blendStage2 = std::make_unique<BlendStage>(windowWidth, windowHeight, m_fullscreenQuad.get());
	m_toneMapHackStage = std::make_unique<ToneMapHackStage>(windowWidth, windowHeight, m_fullscreenQuad.get());

}

void PostProcessPass::resize(UINT width, UINT height) {
	if (m_doBloom || m_doDOFPass) {
		m_hGaussStage->resize(UINT(width * m_gaussPass1Scale), UINT(height * m_gaussPass1Scale));
		m_vGaussStage->resize(UINT(width * m_gaussPass1Scale), UINT(height * m_gaussPass1Scale));
		m_hGaussStage2->resize(UINT(width * m_gaussPass2Scale), UINT(height * m_gaussPass2Scale));
		m_vGaussStage2->resize(UINT(width * m_gaussPass2Scale), UINT(height * m_gaussPass2Scale));
		m_hGaussStage3->resize(UINT(width * m_gaussPass3Scale), UINT(height * m_gaussPass3Scale));
		m_vGaussStage3->resize(UINT(width * m_gaussPass3Scale), UINT(height * m_gaussPass3Scale));
	}
	if (m_doDOFPass) {
		m_hGaussDepthStage->resize(UINT(width * m_gaussPass1Scale), UINT(height * m_gaussPass1Scale));
		m_vGaussDepthStage->resize(UINT(width * m_gaussPass1Scale), UINT(height * m_gaussPass1Scale));
		m_gaussianDofStage->resize(width, height);
	}
	if (m_doFXAAPass)
		m_FXAAStage->resize(width, height);
	if (m_doBloom) {
		m_brightnessCutoffStage->resize(UINT(width * m_brightnessCutoffScale), UINT(height * m_brightnessCutoffScale));
		m_blendStage->resize(width, height);
	}
	m_toneMapHackStage->resize(width, height);
	m_blendStage2->resize(width, height);
	//m_dofStage->resize(width, height);
}

void PostProcessPass::setCamera(const Camera& cam) {
	m_cam = &cam;
}

PostProcessPass::~PostProcessPass() {
}

void PostProcessPass::run(RenderableTexture& baseTexture, ID3D11ShaderResourceView** depthTexture, RenderableTexture& bloomInputTexture, RenderableTexture& particlesTexture) {

	auto* dxm = Application::getInstance()->getAPI();

#ifdef _DEBUG
	auto& kbState = Application::getInstance()->getInput().getKbStateTracker();

	if (kbState.pressed.I)
		m_dofFocusWidth += 10.f;
	if (kbState.pressed.K)
		m_dofFocusWidth -= 10.f;
	if (kbState.pressed.Up)
		m_dofFocusWidth += 1.f;
	if (kbState.pressed.Down)
		m_dofFocusWidth -= 1.f;
#endif

	if (m_cam && m_doDOFPass)
		m_gaussianDofStage->setFocus(fabsf(m_cam->getPosition().z), m_dofFocusWidth);
	//m_gaussianDofStage->setFocus(10.f, m_dofFocusWidth);

	dxm->setDepthMask(GraphicsAPI::BUFFER_DISABLED);


	dxm->getPerfProfiler()->BeginEvent(L"FXAA");
	// FXAA pass
	if (m_doFXAAPass)
		m_FXAAStage->run(baseTexture);
	dxm->getPerfProfiler()->EndEvent();
	RenderableTexture& processedBaseTexture = (m_doFXAAPass) ? m_FXAAStage->getOutput() : baseTexture;

 	dxm->getPerfProfiler()->BeginEvent(L"Tonemap");
 	// Tone map pass - clamp color values to 1 without changing the color (too much)
 	m_toneMapHackStage->run(processedBaseTexture);
 	dxm->getPerfProfiler()->EndEvent();
 
 	// DOF pass
 	if (m_doDOFPass) {
 		dxm->getPerfProfiler()->BeginEvent(L"Depth of field");
 
 		dxm->getPerfProfiler()->BeginEvent(L"GaussDepth");
 		m_hGaussDepthStage->setDepthSRV(depthTexture);
 		m_hGaussDepthStage->run(m_toneMapHackStage->getOutput());
 		m_vGaussDepthStage->setDepthSRV(depthTexture);
 		m_vGaussDepthStage->run(m_hGaussDepthStage->getOutput());
 		dxm->getPerfProfiler()->EndEvent();
 
 		
 		dxm->getPerfProfiler()->BeginEvent(L"Gauss");
 		m_hGaussStage->run(m_vGaussDepthStage->getOutput());
 		m_vGaussStage->run(m_hGaussStage->getOutput());
 		dxm->getPerfProfiler()->EndEvent();
 
 
 		dxm->getPerfProfiler()->BeginEvent(L"Gaussian DOF");
 		m_gaussianDofStage->setBlurredSRV(m_vGaussStage->getOutput().getColorSRV());
 		m_gaussianDofStage->setDepthSRV(depthTexture);
 		m_gaussianDofStage->run(processedBaseTexture);
 		dxm->getPerfProfiler()->EndEvent();
 
 		dxm->getPerfProfiler()->EndEvent();
 	}
 
 	if (m_doBloom) {
 		dxm->getPerfProfiler()->BeginEvent(L"Bloom");
 		// Blend particles with the bloom input texture
 		m_blendStage->setBlendInput(particlesTexture.getColorSRV());
 		m_blendStage->run(bloomInputTexture);
 
 		// Bloom pass
 		m_brightnessCutoffStage->run(m_blendStage->getOutput());
 
 		m_hGaussStage->run(m_brightnessCutoffStage->getOutput());
 		m_vGaussStage->run(m_hGaussStage->getOutput());
 
 		m_hGaussStage2->run(m_vGaussStage->getOutput());
 		m_vGaussStage2->run(m_hGaussStage2->getOutput());
 
 		m_hGaussStage3->run(m_vGaussStage2->getOutput());
 		m_vGaussStage3->run(m_hGaussStage3->getOutput());
 
 		// Blend particles with the bloom output
 		m_blendStage->setBlendInput(particlesTexture.getColorSRV());
 		m_blendStage->run(m_vGaussStage3->getOutput());
 
 		// Blend bloom with processBaseTexture
 		m_blendStage2->setBlendInput(m_blendStage->getOutput().getColorSRV(), m_bloomStrength);
 		m_blendStage2->run((m_doDOFPass) ? m_gaussianDofStage->getOutput() : m_toneMapHackStage->getOutput());
 
 		dxm->getPerfProfiler()->EndEvent();
 	} else {
 		m_blendStage2->setBlendInput(particlesTexture.getColorSRV());
 		m_blendStage2->run((m_doDOFPass) ? m_gaussianDofStage->getOutput() : m_toneMapHackStage->getOutput());
 	}

// 	m_hGaussStage->run(baseTexture);
// 	m_vGaussStage->run(m_hGaussStage->getOutput());

	// Blend last output together with the baseTexture to produce the final image
 	dxm->renderToBackBuffer();
	m_fullscreenQuad->getMaterial()->setTextures(m_blendStage2->getOutput().getColorSRV(), 1);

	/*m_fullscreenQuad->draw();*/ // TODO: fix
	dxm->setDepthMask(GraphicsAPI::NO_MASK);
}

void PostProcessPass::createFullscreenQuad() {

	Vector2 halfSizes(1.f, 1.f);

	const int numVerts = 4;
	Vector3* positions = new Vector3[numVerts]{
		Vector3(-halfSizes.x, -halfSizes.y, 0.f),
		Vector3(-halfSizes.x, halfSizes.y, 0.f),
		Vector3(halfSizes.x, -halfSizes.y, 0.f),
		Vector3(halfSizes.x, halfSizes.y, 0.f),
	};

	const int numIndices = 6;
	ULONG* indices = new ULONG[numIndices]{
		0, 1, 2, 2, 1, 3
	};

	// Tex coords not used in shader, only set to get rid of warning
	Vector2* texCoords = new Vector2[numVerts]{
		Vector2(0.f, 1.f),
		Vector2(0.f, 0.f),
		Vector2(1.f, 1.f),
		Vector2(1.f, 0.f)
	};

	Model::Data data;
	data.numVertices = numVerts;
	data.numIndices = numIndices;
	data.positions = positions;
	data.indices = indices;
	data.texCoords = texCoords;

	m_fullscreenQuad = std::make_unique<Model>(data, &m_flushShader);
	//m_fullscreenQuad.buildBufferForShader(&m_flushShader);
}