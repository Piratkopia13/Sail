#include "PostProcessStage.h"

PostProcessStage::PostProcessStage(const Renderer& renderer, const std::string& filename, UINT width, UINT height, Mesh* fullscreenQuad, UINT outputTexBindFlags)
	: ShaderSet(filename)
	, OutputTexture(1, width, height, false, false, outputTexBindFlags)
	, FullscreenQuad(fullscreenQuad)
	, Width(width)
	, Height(height)
	, RendererRef(renderer)
{
	
}

PostProcessStage::~PostProcessStage() {

}

void PostProcessStage::resize(UINT width, UINT height) {
	OutputTexture.resize(width, height);
	Width = width;
	Height = height;
}

RenderableTexture& PostProcessStage::getOutput() {
	return OutputTexture;
}