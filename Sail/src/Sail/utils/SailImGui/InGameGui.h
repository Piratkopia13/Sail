#pragma once

#include "SailImGuiWindow.h"

#include "Sail/entities/Entity.h"

class InGameGui : public SailImGuiWindow {
public:
	InGameGui(bool showWindow = true);
	~InGameGui();

	virtual void renderWindow() override;
	virtual void setPlayer(Entity* player);
	void setCrosshair(Entity* pCrosshairEntity);

private:
	Entity* m_player = nullptr;
	Entity* m_crosshair = nullptr;
};