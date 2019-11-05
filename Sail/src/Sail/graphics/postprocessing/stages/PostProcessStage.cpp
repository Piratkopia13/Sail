#include "pch.h"
#include "PostProcessStage.h"

PostProcessStage::PostProcessStage(const std::string& filename, UINT width, UINT height)
	: ShaderPipeline(filename)
	, Width(width)
	, Height(height)
{
}

PostProcessStage::~PostProcessStage() {

}

void PostProcessStage::run(RenderableTexture& inputTexture) {
	ShaderPipeline::bind();

	// TODO: Dispatch n stuff
}

RenderableTexture& PostProcessStage::getOutput() {
	return *OutputTexture;
}

bool PostProcessStage::onResize(const WindowResizeEvent& event) {

	// TODO: fix this. should be set to width*resScale instead of full width - same with height
	unsigned int width = event.width;
	unsigned int height = event.height;

	OutputTexture->resize(width, height);
	Width = width;
	Height = height;

	return false;
}

bool PostProcessStage::onEvent(const Event & event) {
	switch (event.type) {
	case Event::Type::WINDOW_RESIZE: onResize((const WindowResizeEvent&)event); break;
	default: break;
	}
	return true;
}
