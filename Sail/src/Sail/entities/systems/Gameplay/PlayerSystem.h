#pragma once
#include "../BaseComponentSystem.h"
#include "Sail/events/EventReceiver.h"

// Does nothing yet, but will probably contain player specific logic (insanity etc.) in the future
class PlayerSystem final : public BaseComponentSystem, public EventReceiver {
public:
	PlayerSystem();
	~PlayerSystem();

private:
	bool onEvent(const Event& event) override;
};