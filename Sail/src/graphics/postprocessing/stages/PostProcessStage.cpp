#include "PostProcessStage.h"

PostProcessStage::PostProcessStage(const Renderer& renderer, const std::string& filename, UINT width, UINT height, Mesh* fullscreenQuad, UINT outputTexBindFlags)
	: ShaderSet(filename)
	, OutputTexture(1, width, height, false, false, outputTexBindFlags)
	, FullscreenQuad(fullscreenQuad)
	, Width(width)
	, Height(height)
	, RendererRef(renderer)
{
	// Set up the input layout
	// Will be the same for all pp stages
	inputLayout.push<DirectX::SimpleMath::Vector3>(InputLayout::POSITION, "POSITION", 0);
	inputLayout.create(VSBlob);
}

PostProcessStage::~PostProcessStage() {

}

void PostProcessStage::run(RenderableTexture & inputTexture) {
	ShaderSet::bind();

	OutputTexture.begin();

	FullscreenQuad->getMaterial()->setDiffuseTexture(*inputTexture.getColorSRV());
	FullscreenQuad->draw(RendererRef);
}

bool PostProcessStage::onResize(WindowResizeEvent& event) {

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	OutputTexture.resize(width, height);
	Width = width;
	Height = height;

	return false;
}

RenderableTexture& PostProcessStage::getOutput() {
	return OutputTexture;
}

void PostProcessStage::onEvent(Event & event) {
	EventHandler::dispatch<WindowResizeEvent>(event, FUNC(&PostProcessStage::onResize));
}
