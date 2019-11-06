#pragma once
#include "Event.h"

// Each inheriting class needs to check which event type was sent, and then static_cast the value (if any) to a pointer of the corresponding value type

class EventReceiver {
public:
	virtual ~EventReceiver() = default;
	virtual bool onEvent(const Event& e) = 0;

protected:
	EventReceiver() = default;
};
