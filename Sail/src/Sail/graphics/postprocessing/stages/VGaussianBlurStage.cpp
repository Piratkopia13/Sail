#include "pch.h"
#include "VGaussianBlurStage.h"


using namespace DirectX;
using namespace SimpleMath;

VGaussianBlurStage::VGaussianBlurStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullscreenQuad)
	: PostProcessStage(renderer, "postprocess/GaussianBlurVertical.hlsl", width, height, fullscreenQuad)
{
	// Set up constant buffer
	float invWidth = 1.f / width;
	float invHeight = 1.f / height;
	setCBufferVar("invWindowWidth", &invWidth, sizeof(float));
	setCBufferVar("invWindowHeight", &invHeight, sizeof(float));
}

VGaussianBlurStage::~VGaussianBlurStage() {
}

bool VGaussianBlurStage::onResize(WindowResizeEvent& event) {
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
