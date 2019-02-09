#pragma once

#include "../../RenderableTexture.h"
#include "../../shader/ShaderSet.h"
#include "../../renderer/Renderer.h"

class Model;

class PostProcessStage : public ShaderSet, public IEventListener {
public:
	PostProcessStage(const Renderer& renderer, const std::string& filename, UINT width, UINT height, Mesh* fullscreenQuad, UINT outputTexBindFlags = 0);
	virtual ~PostProcessStage();

	virtual void run(RenderableTexture& inputTexture);
	RenderableTexture& getOutput();

	virtual void onEvent(Event& event) override;

protected:
	virtual bool onResize(WindowResizeEvent& event);

protected:
	RenderableTexture OutputTexture;
	Mesh* FullscreenQuad;
	UINT Width, Height;
	const Renderer& RendererRef;

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

};