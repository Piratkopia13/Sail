#pragma once

#include "SailImGuiWindow.h"

class Entity;
class Camera;

class PlayerInfoWindow : public SailImGuiWindow {
public:
	PlayerInfoWindow(bool showWindow = true);
	~PlayerInfoWindow();

	void setPlayerInfo(Entity* player, Camera* cam);

	virtual void renderWindow() override;

private:
	Entity* m_player;
	Camera* m_cam;
};