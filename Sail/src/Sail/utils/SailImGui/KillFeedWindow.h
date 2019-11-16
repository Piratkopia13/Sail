#pragma once
#include "Sail/events/EventReceiver.h"
#include "SailImGuiWindow.h"

#include <string>
#include <vector>

class GameDataTracker;

class KillFeedWindow final : public SailImGuiWindow, public EventReceiver {
public:
	// used as name1 + " " + type + " " + name2
	struct KillFeedInfo {
		// 0 = not relevant, 1 = name1 relevant, 2 = name2 relevant
		unsigned int relevant = 0;
		std::string name1 = "";
		std::string type = "";
		std::string name2 = "";
	};

public:
	KillFeedWindow(bool showWindow = true);
	~KillFeedWindow();

	virtual void renderWindow() override;
	void updateTiming(float dt);

private:
	bool onEvent(const Event& event) override;

private:
	bool m_doRender;
	GameDataTracker& m_gameDataTracker;
	float m_maxTimeShowed;
	std::vector<std::pair<float, KillFeedWindow::KillFeedInfo>> m_kills;
};