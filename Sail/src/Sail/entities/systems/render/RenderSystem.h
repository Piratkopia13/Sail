#pragma once
#include "..//BaseComponentSystem.h"

class Camera;
class Renderer;

class RenderSystem : public BaseComponentSystem {
public:
	RenderSystem();
	~RenderSystem();
	void update(float dt) override;
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