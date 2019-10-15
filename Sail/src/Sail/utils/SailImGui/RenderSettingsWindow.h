#pragma once

#include "SailImGuiWindow.h"

class Octree;
class Camera;

class RenderSettingsWindow : public SailImGuiWindow {

public:
	RenderSettingsWindow(bool showWindow = true);
	~RenderSettingsWindow();
	void activateMaterialPicking(Camera* camera, Octree* octree);

	virtual void renderWindow() override;

private:
	Octree* m_octree;
	Camera* m_cam;

};