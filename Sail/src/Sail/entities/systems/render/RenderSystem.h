#pragma once
#include "..//BaseComponentSystem.h"
#include "..//..//..//graphics/camera/Camera.h"

class Renderer;

class RenderSystem : public BaseComponentSystem {
public:
	RenderSystem();
	~RenderSystem();
	void toggleHitboxes();
	void refreshRenderer();

	// Model component and Transform component is required to be drawn.
	void draw(Camera& camera, const float alpha);

	/*
		Used to clear menu background
	*/
	void draw(void);

private:
	Renderer* m_renderer;

	// Temporary solution to be able to render hitboxes
	bool m_renderHitboxes;
};