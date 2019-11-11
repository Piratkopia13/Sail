#include "pch.h"
#include "GraphicsAPI.h"
#include "Sail/events/Events.h"

GraphicsAPI::GraphicsAPI() {
	EventDispatcher::Instance().subscribe(Event::Type::WINDOW_RESIZE, this);
}

GraphicsAPI::~GraphicsAPI() {
	EventDispatcher::Instance().unsubscribe(Event::Type::WINDOW_RESIZE, this);
}

bool GraphicsAPI::onEvent(const Event& event) {
	switch (event.type) {
	case Event::Type::WINDOW_RESIZE: onResize((const WindowResizeEvent&)event); break;
	default: break;
	}
	return true;
}
