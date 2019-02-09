#pragma once

#include "Event.h"

class IEventListener{
public:
	virtual void onEvent(Event& event) = 0;
};