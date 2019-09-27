#pragma once

class Renderer;
class LightSetup;
class PostProcessPipeline;

class RendererWrapper {
public:
	RendererWrapper();
	~RendererWrapper();

	void initialize();

	void changeRenderer(unsigned int index);
	void togglePostProcessing(const bool flag);
	bool& getDoPostProcessing();
	Renderer* getCurrentRenderer();
	PostProcessPipeline* getPostProcessPipeline();

private:
	void setLightSetup(LightSetup* lights);

	std::unique_ptr<Renderer> m_rendererRaster;
	std::unique_ptr<Renderer> m_rendererRaytrace;
	Renderer* m_currentRenderer;
	PostProcessPipeline* m_postProcessPipeline;

	bool m_doPostProcessing;
};