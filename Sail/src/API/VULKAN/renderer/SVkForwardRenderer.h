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
	void createRenderPass();

	const VkCommandBuffer& runFramePreparation();
	void runRenderingPass(const VkCommandBuffer& cmd, const VkRenderPass& renderPass);
	void runFrameExecution(const VkCommandBuffer& cmd);

private:
	SVkAPI* m_context;
	SVkAPI::Command m_command;
	VkRenderPass m_renderPassClear;
	VkRenderPass m_renderPassLoad;
};