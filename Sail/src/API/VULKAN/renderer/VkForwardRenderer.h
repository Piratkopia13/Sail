#pragma once

#include "Sail/api/Renderer.h"
#include "../VkAPI.h"

class VkForwardRenderer : public Renderer {
public:
	VkForwardRenderer();
	~VkForwardRenderer();

	void begin(Camera* camera, Environment* environment) override;
	void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;

	void useDepthBuffer(void* buffer, void* cmdList) override;

private:
	VkAPI* m_context;

};