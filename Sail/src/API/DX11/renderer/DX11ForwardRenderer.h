#pragma once

#include "Sail/api/Renderer.h"
#include "../DX11API.h"

class DX11ForwardRenderer : public Renderer {
public:
	DX11ForwardRenderer();
	~DX11ForwardRenderer();

	void* present(Renderer::RenderFlag flags, void* skippedPrepCmdList = nullptr) override;
	void useDepthBuffer(void* buffer, void* cmdList) override;

private:
	DX11API* m_context;

};