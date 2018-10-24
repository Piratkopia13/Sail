#include "HGaussianBlurStage.h"

using namespace DirectX;
using namespace SimpleMath;

HGaussianBlurStage::HGaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad)
	: PostProcessStage(renderer, "postprocess/GaussianBlurPS.hlsl", width, height, fullscreenQuad)
{
	// Set up constant buffer
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

	inputLayout.push<Vector3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.create(VSBlob);

}

HGaussianBlurStage::~HGaussianBlurStage() {
}

void HGaussianBlurStage::run(RenderableTexture& inputTexture) {

	ShaderSet::bind();

	OutputTexture.begin();

	FullscreenQuad->getMaterial()->setDiffuseTexture(*inputTexture.getColorSRV());
	FullscreenQuad->draw(RendererRef);

}

void HGaussianBlurStage::resize(UINT width, UINT height) {
	PostProcessStage::resize(width, height);
	// Update constant buffer
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));
}
