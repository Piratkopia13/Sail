#pragma once

#include "Event.h"

class WindowFocusChangedEvent : public Event {
public:
	WindowFocusChangedEvent(bool isFocused)
		: Event(Event::WINDOW_FOCUS_CHANGED)
		, m_isFocused(isFocused) {}
	~WindowFocusChangedEvent() {};

	inline bool isFocused() const { return m_isFocused; };

	static Type GetStaticType() {
		return Event::WINDOW_FOCUS_CHANGED;
	}
private:
	bool m_isFocused;

};