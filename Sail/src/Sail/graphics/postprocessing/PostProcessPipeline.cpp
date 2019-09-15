#include "pch.h"
#include "PostProcessPipeline.h"
#include "Sail/graphics/shader/postprocess/RedTintShader.h"

PostProcessPipeline::PostProcessPipeline() {

	m_dispatcher = std::unique_ptr<ComputeShaderDispatcher>(ComputeShaderDispatcher::Create());

	add<RedTintShader>(1.0f);
	add<RedTintShader>(0.5f);

}

bool PostProcessPipeline::onResize(WindowResizeEvent& event) {

	return false;
}

PostProcessPipeline::~PostProcessPipeline() {
}

void PostProcessPipeline::clear() {
	m_pipelineStages.clear();
}

void PostProcessPipeline::run(Texture* baseTexture, void* cmdList) {
	Application* app = Application::getInstance();
	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	m_dispatcher->begin(cmdList);

	PostProcessInput input;
	input.inputTexture = baseTexture;

	for (auto& stage : m_pipelineStages) {
		input.outputWidth = stage.resolutionScale * windowWidth;
		input.outputHeight = stage.resolutionScale * windowHeight;
		input.threadGroupCountX = input.outputWidth;
		input.threadGroupCountY = input.outputHeight;

		PostProcessOutput& output = static_cast<PostProcessOutput&>(m_dispatcher->dispatch(*stage.shader, input, cmdList));
		input.inputTexture = nullptr;
		input.inputRenderableTexture = output.outputTexture;
	}

}

bool PostProcessPipeline::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&PostProcessPipeline::onResize));

	return true;
}
