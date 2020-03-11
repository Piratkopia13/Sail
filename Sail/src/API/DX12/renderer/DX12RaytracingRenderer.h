#pragma once

#include "Sail/api/Renderer.h"
#include "../DX12API.h"
#include "Sail/events/Event.h"
#include "../dxr/DXRBase.h"

class DX12RaytracingRenderer : public Renderer, public IEventListener {
public:
	DX12RaytracingRenderer();
	~DX12RaytracingRenderer();

	void begin(Camera* camera, Environment* environment) override;
	void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;

	static const std::vector<DX12RenderableTexture*>& GetOutputTextures();

	bool onEvent(Event& event) override;

private:
	DX12API* m_context;
	DX12API::Command m_command;
	std::unique_ptr<DXRBase> m_dxrBase;
	static std::vector<DX12RenderableTexture*> sRTOutputTextures;

};