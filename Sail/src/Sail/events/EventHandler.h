#pragma once

#include "Event.h"
#include <functional>

#define SAIL_BIND_EVENT(x) std::bind(x, this, std::placeholders::_1)

class EventHandler {
public:

	template <typename T>
	static void dispatch(Event& event, std::function<bool(T&)> func) {
		if (event.getType() == T::getStaticType()) {
			func((T&)event); // TODO: do something with the return bool
		}
	}

};

