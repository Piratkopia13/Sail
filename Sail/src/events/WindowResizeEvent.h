#pragma once

#include "Event.h"

class WindowResizeEvent : public Event {
public:
	WindowResizeEvent(unsigned int width, unsigned int height)
		: Event(Event::WINDOW_RESIZE)
		, m_width(width)
		, m_height(height)
	{ }
	~WindowResizeEvent() { };

	unsigned int getWidth() const { return m_width; }
	unsigned int getHeight() const { return m_height; }

	static Type getStaticType() {
		return Event::WINDOW_RESIZE;
	}
private:
	unsigned int m_width, m_height;

};