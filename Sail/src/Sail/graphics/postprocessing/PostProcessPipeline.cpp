#include "pch.h"
#include "PostProcessPipeline.h"
#include "Sail/graphics/shader/postprocess/RedTintShader.h"
#include "Sail/graphics/shader/postprocess/GaussianBlurHorizontal.h"

PostProcessPipeline::PostProcessPipeline() {

	m_dispatcher = std::unique_ptr<ComputeShaderDispatcher>(ComputeShaderDispatcher::Create());

	add<GaussianBlurHorizontal>(1.0f);
	//add<RedTintShader>(0.5f);

}

bool PostProcessPipeline::onResize(const WindowResizeEvent& event) {

	return false;
}

PostProcessPipeline::~PostProcessPipeline() {
}

void PostProcessPipeline::clear() {
	m_pipelineStages.clear();
}

RenderableTexture* PostProcessPipeline::run(Texture* baseTexture, void* cmdList) {
	PostProcessInput input;
	input.inputTexture = baseTexture;
	return runInternal(input, cmdList);
}

RenderableTexture* PostProcessPipeline::run(RenderableTexture* baseTexture, void* cmdList) {
	PostProcessInput input;
	input.inputRenderableTexture = baseTexture;
	return runInternal(input, cmdList);
}

RenderableTexture* PostProcessPipeline::runInternal(PostProcessInput& input, void* cmdList) {
	Application* app = Application::getInstance();
	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	m_dispatcher->begin(cmdList);

	PostProcessOutput output;
	for (auto& stage : m_pipelineStages) {
		auto* settings = stage.shader->getComputeSettings();

		input.outputWidth = (unsigned int)(stage.resolutionScale * windowWidth);
		input.outputHeight = (unsigned int)(stage.resolutionScale * windowHeight);
		input.threadGroupCountX = (unsigned int)glm::ceil(input.outputWidth * settings->threadGroupXScale);
		input.threadGroupCountY = (unsigned int)glm::ceil(input.outputHeight * settings->threadGroupYScale);

		output = static_cast<PostProcessOutput&>(m_dispatcher->dispatch(*stage.shader, input, 0, cmdList));
		input.inputTexture = nullptr;
		input.inputRenderableTexture = output.outputTexture;
	}
	return output.outputTexture;
}

bool PostProcessPipeline::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::WINDOW_RESIZE: onResize((const WindowResizeEvent&)event); break;
	default: break;
	}

	return true;
}
