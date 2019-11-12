#include "pch.h"
#include "PostProcessPipeline.h"
#include "Sail/graphics/shader/postprocess/RedTintShader.h"
#include "Sail/graphics/shader/postprocess/GaussianBlurHorizontal.h"
#include "Sail/graphics/shader/postprocess/GaussianBlurVertical.h"
#include "Sail/graphics/shader/postprocess/BlendShader.h"
#include "Sail/events/EventDispatcher.h"

PostProcessPipeline::PostProcessPipeline() 
	: m_bloomTexture(nullptr)
{

	m_dispatcher = std::unique_ptr<ComputeShaderDispatcher>(ComputeShaderDispatcher::Create());

	add<GaussianBlurVertical>("BloomBlur1V", 1.0f);
	add<GaussianBlurHorizontal>("BloomBlur1H", 1.0f);
	add<GaussianBlurVertical>("BloomBlur2V", 0.75f, 1.f / 0.75f);
	add<GaussianBlurHorizontal>("BloomBlur2H", 0.75f);
	add<GaussianBlurVertical>("BloomBlur3V", 0.5f, 1.5f);
	add<GaussianBlurHorizontal>("BloomBlur3H", 0.5f);
	add<BlendShader>("BloomBlend", 1.0f, 1.f / 2.f);
	//add<RedTintShader>(0.5f);

	EventDispatcher::Instance().subscribe(Event::Type::WINDOW_RESIZE, this);
}

bool PostProcessPipeline::onResize(const WindowResizeEvent& event) {

	return false;
}

PostProcessPipeline::~PostProcessPipeline() {
	EventDispatcher::Instance().unsubscribe(Event::Type::WINDOW_RESIZE, this);
}

RenderableTexture* PostProcessPipeline::run(RenderableTexture* baseTexture, void* cmdList) {
	PostProcessInput input;
	input.inputRenderableTexture = m_bloomTexture;
	// Blur pass one
	auto* output = runStage(input, m_stages["BloomBlur1V"], cmdList);
	input.inputRenderableTexture = output->outputTexture;
	output = runStage(input, m_stages["BloomBlur1H"], cmdList);
	// Blur pass two
	input.inputRenderableTexture = output->outputTexture;
	output = runStage(input, m_stages["BloomBlur2V"], cmdList);
	input.inputRenderableTexture = output->outputTexture;
	output = runStage(input, m_stages["BloomBlur2H"], cmdList);
	// Blur pass three
	input.inputRenderableTexture = output->outputTexture;
	output = runStage(input, m_stages["BloomBlur3V"], cmdList);
	input.inputRenderableTexture = output->outputTexture;
	output = runStage(input, m_stages["BloomBlur3H"], cmdList);

	input.inputRenderableTexture = baseTexture;
	input.inputRenderableTextureTwo = output->outputTexture;
	output = runStage(input, m_stages["BloomBlend"], cmdList);

	return output->outputTexture;
}

void PostProcessPipeline::setBloomInput(RenderableTexture* bloomTexture) {
	m_bloomTexture = bloomTexture;
}

PostProcessPipeline::PostProcessOutput* PostProcessPipeline::runStage(PostProcessInput& input, StageData& stage, void* cmdList) {
	Application* app = Application::getInstance();
	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	m_dispatcher->begin(cmdList);

	PostProcessOutput* output;
	auto* settings = stage.shader->getComputeSettings();

	input.outputWidth = (unsigned int)(stage.resolutionScale * windowWidth);
	input.outputHeight = (unsigned int)(stage.resolutionScale * windowHeight);
	input.threadGroupCountX = (unsigned int)glm::ceil(input.outputWidth * settings->threadGroupXScale);
	input.threadGroupCountY = (unsigned int)glm::ceil(input.outputHeight * settings->threadGroupYScale);

	stage.shader->getPipeline()->setCBufferVar("textureSizeDifference", &stage.textureSizeDifference, sizeof(float));
	glm::u32vec2 textureSize = glm::u32vec2(input.outputWidth, input.outputHeight);
	stage.shader->getPipeline()->trySetCBufferVar("textureSize", &textureSize, sizeof(float));
	
	output = static_cast<PostProcessOutput*>(&m_dispatcher->dispatch(*stage.shader, input, 0, cmdList));
	return output;
}

bool PostProcessPipeline::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::WINDOW_RESIZE: onResize((const WindowResizeEvent&)event); break;
	default: break;
	}

	return true;
}
