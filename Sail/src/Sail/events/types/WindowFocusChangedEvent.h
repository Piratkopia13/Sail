#pragma once
#include "../Event.h"

struct WindowFocusChangedEvent : public Event {
	WindowFocusChangedEvent(const bool _isFocused)
		: Event(Event::Type::WINDOW_FOCUS_CHANGED)
		, isFocused(_isFocused) { }
	~WindowFocusChangedEvent() = default;

	const bool isFocused;
};