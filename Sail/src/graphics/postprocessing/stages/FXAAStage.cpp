#include "pch.h"
#include "FXAAStage.h"

FXAAStage::FXAAStage(const Renderer& renderer, UINT width, UINT height, Mesh* fullScreenQuad) 
	: PostProcessStage(renderer, "postprocess/FXAAShader.hlsl", width, height, fullScreenQuad) 
{
	// Set up constant buffer
	float w = static_cast<float>(width);
	float h = static_cast<float>(height);
	setCBufferVar("windowWidth", &w, sizeof(float));
	setCBufferVar("windowHeight", &h, sizeof(float));

	//m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

}

FXAAStage::~FXAAStage() {}

bool FXAAStage::onResize(WindowResizeEvent& event) {
	PostProcessStage::onResize(event);

	Logger::Log("FXAA RESIZED!");

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	// Update constant buffer
	float w = static_cast<float>(width);
	float h = static_cast<float>(height);
	setCBufferVar("windowWidth", &w, sizeof(float));
	setCBufferVar("windowHeight", &h, sizeof(float));

	return false;
}