#include "PostProcessStage.h"

PostProcessStage::PostProcessStage(UINT width, UINT height, Model* fullscreenQuad, UINT outputTexBindFlags)
	: OutputTexture(1, width, height, false, false, outputTexBindFlags)
	, FullscreenQuad(fullscreenQuad)
	, Width(width)
	, Height(height)
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