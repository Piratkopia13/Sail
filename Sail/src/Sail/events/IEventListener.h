#pragma once

#include "Event.h"

class IEventListener{
public:
	virtual bool onEvent(Event& event) = 0;
};