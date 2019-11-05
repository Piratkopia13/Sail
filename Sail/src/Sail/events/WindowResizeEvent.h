#pragma once
#include "Event.h"

struct WindowResizeEvent : public Event {
	WindowResizeEvent(const unsigned int _width, const unsigned int _height, const bool _isMinimized = false)
		: Event(Event::Type::WINDOW_RESIZE)
		, width(_width)
		, height(_height)
		, isMinimized(_isMinimized) { }
	~WindowResizeEvent() = default;
	
	const unsigned int width;
	const unsigned int height;
	const bool isMinimized;
};