#pragma once

#include "SailImGuiWindow.h"

#include <string>
#include <vector>

class KillFeedWindow : public SailImGuiWindow {
public:
	KillFeedWindow(bool showWindow = true);
	~KillFeedWindow();

	void setDeaths(const std::vector<std::string>& kills);

	virtual void renderWindow() override;

private:
	std::vector<std::string> m_kills;
};