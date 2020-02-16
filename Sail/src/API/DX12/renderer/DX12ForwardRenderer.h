#pragma once

#include "Sail/api/Renderer.h"
#include "../DX12API.h"

class DX12ForwardRenderer : public Renderer {
public:
	DX12ForwardRenderer();
	~DX12ForwardRenderer();

	void begin(Camera* camera, Environment* environment) override;
	void* present(Renderer::RenderFlag flags, void* skippedPrepCmdList = nullptr) override;

	ID3D12GraphicsCommandList4* runFramePreparation();
	void runRenderingPass(ID3D12GraphicsCommandList4* cmdList);
	void runFrameExecution(ID3D12GraphicsCommandList4* cmdList);

	void useDepthBuffer(void* buffer, void* cmdList) override;

private:
	DX12API* m_context;
	DX12API::Command m_command;

};