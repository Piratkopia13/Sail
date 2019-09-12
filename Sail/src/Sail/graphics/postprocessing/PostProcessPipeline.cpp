#include "pch.h"
#include "PostProcessPipeline.h"

PostProcessPipeline::PostProcessPipeline() {

}

bool PostProcessPipeline::onResize(WindowResizeEvent& event) {

	return false;
}

PostProcessPipeline::~PostProcessPipeline() {
}

void PostProcessPipeline::clear() {
	m_pipelineStages.clear();
}

void PostProcessPipeline::run(RenderableTexture& baseTexture) {
	
	assert(false);

}

bool PostProcessPipeline::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&PostProcessPipeline::onResize));

	// Forward events
	for (StageData& s : m_pipelineStages)
		s.stage->onEvent(event);

	return true;
}
