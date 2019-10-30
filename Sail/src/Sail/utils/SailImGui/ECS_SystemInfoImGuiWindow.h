#pragma once

#include "SailImGuiWindow.h"
#include <map>

class ECS_SystemInfoImGuiWindow : public SailImGuiWindow {

public:
	ECS_SystemInfoImGuiWindow(bool showWindow = true);
	~ECS_SystemInfoImGuiWindow();

	void updateNumEntitiesInSystems(std::string systemName, int n);
	void updateNumEntitiesInECS(int n);

	virtual void renderWindow() override;

private:
	std::map<std::string, int> m_nEntitiesInSystems;
	int m_nEntitiesInECS;
};