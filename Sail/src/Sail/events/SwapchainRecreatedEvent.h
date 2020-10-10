#pragma once

#include "Event.h"

class SwapchainRecreatedEvent : public Event {
public:
	SwapchainRecreatedEvent()
		: Event(Event::SWAPCHAIN_RECREATED) { }
	~SwapchainRecreatedEvent() { };

	static Type getStaticType() {
		return Event::SWAPCHAIN_RECREATED;
	}

};