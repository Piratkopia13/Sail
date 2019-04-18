#pragma once

#include "API/DX11/DX11RenderableTexture.h"
#include "../../shader/ShaderPipeline.h"
#include "../../renderer/Renderer.h"

class Model;

class PostProcessStage : public ShaderSet, public IEventListener {
public:
	PostProcessStage(const Renderer& renderer, const std::string& filename, UINT width, UINT height, Mesh* fullscreenQuad, UINT outputTexBindFlags = 0);
	virtual ~PostProcessStage();

	virtual void run(DX11RenderableTexture& inputTexture);
	DX11RenderableTexture& getOutput();

	virtual void onEvent(Event& event) override;

protected:
	virtual bool onResize(WindowResizeEvent& event);

protected:
	DX11RenderableTexture OutputTexture;
	Mesh* FullscreenQuad;
	UINT Width, Height;
	const Renderer& RendererRef;

	struct Vertex {
		glm::vec3 position;
	};

};