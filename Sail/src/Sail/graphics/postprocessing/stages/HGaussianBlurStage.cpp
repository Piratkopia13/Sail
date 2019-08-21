#include "pch.h"
#include "HGaussianBlurStage.h"

HGaussianBlurStage::HGaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad)
	: PostProcessStage(renderer, "postprocess/GaussianBlurHorizontal.hlsl", width, height, fullscreenQuad)
{
	// Set up constant buffer
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

}

HGaussianBlurStage::~HGaussianBlurStage() {
}

bool HGaussianBlurStage::onResize(WindowResizeEvent& event) {
	PostProcessStage::onResize(event);

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	// Update constant buffer
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));

	return false;
}
