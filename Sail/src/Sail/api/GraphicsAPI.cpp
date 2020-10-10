#include "pch.h"
#include "GraphicsAPI.h"

GraphicsAPI::GraphicsAPI() {
	EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);
}

GraphicsAPI::~GraphicsAPI() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
}

bool GraphicsAPI::onEvent(Event& event) {
	EventHandler::HandleType<WindowResizeEvent>(event, SAIL_BIND_EVENT(&GraphicsAPI::onResize));
	return true;
}

bool GraphicsAPI::supportsFeature(Feature feature) const {
	return supportedFeatures & feature;
}
