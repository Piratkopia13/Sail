#pragma once

#include "Event.h"

class WindowResizeEvent : public Event {
public:
	WindowResizeEvent(unsigned int width, unsigned int height, bool isMinimized = false)
		: Event(Event::WINDOW_RESIZE)
		, m_width(width)
		, m_height(height)
		, m_isMinimized(isMinimized)
	{ }
	~WindowResizeEvent() { };

	inline unsigned int getWidth() const { return m_width; }
	inline unsigned int getHeight() const { return m_height; }
	inline bool isMinimized() const { return m_isMinimized; };

	static Type getStaticType() {
		return Event::WINDOW_RESIZE;
	}
private:
	unsigned int m_width, m_height;
	bool m_isMinimized;

};