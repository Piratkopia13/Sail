#include "pch.h"
#include "GaussianBlurStage.h"

using namespace DirectX;
using namespace SimpleMath;

GaussianBlurStage::GaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad)
	: PostProcessStage(renderer, "postprocess/GaussianBlurHorizontal.hlsl", width, height, fullscreenQuad)
	, m_firstOutputTexture(1, width, height, false, false)
{
	m_verticalPassShader = std::make_unique<ShaderSet>("postprocess/GaussianBlurVertical.hlsl");

	// Set up the input layout for second pass
	m_verticalPassShader->getInputLayout().push<DirectX::SimpleMath::Vector3>(InputLayout::POSITION, "POSITION", 0);
	m_verticalPassShader->getInputLayout().create(VSBlob);

	// Set up constant buffer for horizontal pass
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

	// Set up constant buffer for vertical pass
	m_verticalPassShader->setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	m_verticalPassShader->setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

}

GaussianBlurStage::~GaussianBlurStage() {
}

void GaussianBlurStage::run(DX11RenderableTexture& inputTexture) {
	// Overriden run method, dont call parent

	// First pass - horizontal
	ShaderSet::bind();
	m_firstOutputTexture.begin();

	FullscreenQuad->getMaterial()->setDiffuseTexture(*inputTexture.getColorSRV());
	FullscreenQuad->draw(RendererRef);

	// Second pass vertical
	m_verticalPassShader->bind();
	OutputTexture.begin();

	FullscreenQuad->getMaterial()->setDiffuseTexture(*m_firstOutputTexture.getColorSRV());
	FullscreenQuad->draw(RendererRef);

}

bool GaussianBlurStage::onResize(WindowResizeEvent& event) {
	PostProcessStage::onResize(event);

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	m_firstOutputTexture.resize(width, height);

	// Update constant buffer for horizontal pass
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

	// Update constant buffer for vertical pass
	m_verticalPassShader->setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	m_verticalPassShader->setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

	return false;
}
