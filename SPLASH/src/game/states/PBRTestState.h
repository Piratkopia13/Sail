#pragma once

#include "Sail.h"

class RenderSystem;
class OctreeAddRemoverSystem;
class UpdateBoundingBoxSystem;

class PBRTestState : public State {
public:
	PBRTestState(StateStack& stack);
	~PBRTestState();

	// Process input for the state
	virtual bool processInput(float dt) override;
	// Sends events to the state
	virtual bool onEvent(Event& event) override;
	// Updates the state - Runs every frame
	virtual bool update(float dt, float alpha = 1.0f) override;
	// Updates the state - Runs every tick
	virtual bool fixedUpdate(float dt) override;
	// Renders the state
	virtual bool render(float dt, float alpha = 1.0f) override;
	// Renders imgui
	virtual bool renderImgui(float dt) override;



private:
	bool onResize(WindowResizeEvent& event);
	bool renderImGuiLightDebug(float dt);

private:
	struct Systems {
		RenderSystem* renderSystem = nullptr;
		EntityAdderSystem* entityAdderSystem = nullptr;
		EntityRemovalSystem* entityRemovalSystem = nullptr;
		OctreeAddRemoverSystem* octreeAddRemoverSystem = nullptr;
		UpdateBoundingBoxSystem* updateBoundingBoxSystem = nullptr;
	};

	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;
	Octree* m_octree;
	RenderSettingsWindow m_renderSettingsWindow;

	const std::string createCube(const glm::vec3& position);

	Systems m_componentSystems;
	LightSetup m_lights;
	Profiler m_profiler;

	size_t m_currLightIndex;
	// For use by non-deterministic entities
	const float* pAlpha = nullptr;

	std::unique_ptr<Model> m_cubeModel;
	std::unique_ptr<Model> m_planeModel;
	std::unique_ptr<Model> m_sphereModel;

};