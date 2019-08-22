#pragma once

#include "Event.h"

class IEventDispatcher {
public:
	virtual void dispatchEvent(Event& event) = 0;
};