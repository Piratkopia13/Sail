#pragma once

#include "Sail/api/Renderer.h"
#include "../SVkAPI.h"

class SVkForwardRenderer : public Renderer {
public:
	SVkForwardRenderer();
	~SVkForwardRenderer();

	void begin(Camera* camera, Environment* environment) override;
	void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;

	void useDepthBuffer(void* buffer, void* cmdList) override;

private:
	SVkAPI* m_context;
	SVkAPI::Command m_command;
};