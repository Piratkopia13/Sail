#pragma once

#include "Sail/api/Renderer.h"
#include "Sail/events/Event.h"
#include "../SVkAPI.h"
#include "../rt/SVkRTBase.h"
#include "../resources/SVkRenderableTexture.h"

class SVkRaytracingRenderer : public Renderer, public IEventListener {
public:
	SVkRaytracingRenderer();
	~SVkRaytracingRenderer();

	void begin(Camera* camera, Environment* environment) override;
	void* present(Renderer::PresentFlag flags, void* skippedPrepCmdList = nullptr) override;

	static SVkRenderableTexture* GetOutputTexture();
	
	bool onEvent(Event& event) override;

private:
	SVkAPI* m_context;
	SVkRTBase m_rtBase;
	SVkAPI::Command m_command;

	Mesh::SPtr m_testMesh;
	Mesh::SPtr m_testMesh2;

	static std::unique_ptr<SVkRenderableTexture> sRTOutputTexture;

};