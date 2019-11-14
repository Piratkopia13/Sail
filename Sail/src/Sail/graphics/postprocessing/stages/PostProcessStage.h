#pragma once

#include "Sail/api/RenderableTexture.h"
#include "Sail/api/shader/ShaderPipeline.h"

class Model;

class PostProcessStage final : public ShaderPipeline, public EventReceiver {
public:
	PostProcessStage(const std::string& filename, UINT width, UINT height);
	virtual ~PostProcessStage();

	virtual void run(RenderableTexture& inputTexture);
	RenderableTexture& getOutput();

	virtual bool onEvent(const Event& event) override;

protected:
	virtual bool onResize(const WindowResizeEvent& event);

protected:
	RenderableTexture* OutputTexture;
	UINT Width, Height;

};