#pragma once

#include "../../RenderableTexture.h"
#include "../../shader/ShaderSet.h"
#include "../../renderer/Renderer.h"

class Model;

class PostProcessStage : public ShaderSet {
public:
	PostProcessStage(const Renderer& renderer, const std::string& filename, UINT width, UINT height, Mesh* fullscreenQuad, UINT outputTexBindFlags = 0);
	virtual ~PostProcessStage();

	virtual void run(RenderableTexture& inputTexture) = 0;
	virtual void resize(UINT width, UINT height);
	RenderableTexture& getOutput();

protected:
	RenderableTexture OutputTexture;
	Mesh* FullscreenQuad;
	UINT Width, Height;
	const Renderer& RendererRef;

	struct Vertex {
		DirectX::SimpleMath::Vector3 position;
	};

};