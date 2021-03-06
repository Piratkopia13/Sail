#pragma once

#include "Sail.h"

/*
* This state can be used for testing while implementing a new graphics API
* No textures, models or anything will be loaded or rendered
*/

class EmptyState : public State {
public:
	EmptyState(StateStack& stack);
	~EmptyState();

	// Process input for the state
	virtual bool processInput(float dt) override;
	// Updates the state
	virtual bool update(float dt) override;
	// Renders the state
	virtual bool render(float dt) override;
	// Renders imgui
	virtual bool renderImgui(float dt) override;

private:
	Application* m_app;
	std::unique_ptr<Renderer> m_forwardRenderer;
	std::shared_ptr<Mesh> m_mesh;
	std::shared_ptr<Mesh> m_mesh2;
	
	//PhongMaterial m_material;
	//PhongMaterial m_material2;

	PBRMaterial m_pbrMaterial;

	Scene m_scene;

	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;
	LightSetup m_lightSetup;
	Environment m_environment;

};