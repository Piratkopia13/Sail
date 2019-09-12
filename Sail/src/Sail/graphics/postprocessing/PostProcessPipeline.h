#pragma once

#include "Sail/api/RenderableTexture.h"
#include "stages/PostProcessStage.h"
#include "../geometry/Model.h"

class PostProcessPipeline : public IEventListener {
public:
	PostProcessPipeline();
	~PostProcessPipeline();

	template <typename T>
	int add(float resolutionScale = 1.0f) {
		Application* app = Application::getInstance();
		UINT windowWidth = app->getWindow()->getWindowWidth();
		UINT windowHeight = app->getWindow()->getWindowHeight();

		// Create a new stage instance with the given scale
		// TODO: look into caching these stages with resourceManager
		m_pipeline.emplace_back(std::make_unique<T>(UINT(windowWidth * resolutionScale), UINT(windowHeight * resolutionScale)), resolutionScale);

		// Return the index of the inserted stage
		return m_pipeline.size() - 1;
	}
	void clear();

	void run(RenderableTexture& baseTexture);

	virtual bool onEvent(Event& event) override;

private:
	bool onResize(WindowResizeEvent& event);

private:
	struct StageData {
		StageData(std::unique_ptr<PostProcessStage> stage, float resolutionScale)
			: stage(std::move(stage))
			, resolutionScale(resolutionScale)
		{}
		std::unique_ptr<PostProcessStage> stage;
		float resolutionScale;
	};

	std::vector<StageData> m_pipelineStages;

};