#pragma once
#include "Sail/events/EventReceiver.h"
#include "SailImGuiWindow.h"

#include <string>
#include <vector>

class GameDataTracker;

class KillFeedWindow final : public SailImGuiWindow, public EventReceiver {
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
	std::vector<std::pair<float, std::pair<bool, std::string>>> m_kills;
};