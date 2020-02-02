#pragma once

#include "Event.h"
#include "IEventListener.h"
#include <set>

class EventSystem {
public:
	EventSystem(const EventSystem&) = delete;
	void operator=(const EventSystem&) = delete;

	static EventSystem* EventSystem::getInstance() {
		static EventSystem instance;
		return &instance;
	}

	void dispatchEvent(Event& event);

	void subscribeToEvent(const Event::Type& type, IEventListener* subscriber);
	void unsubscribeFromEvent(const Event::Type& type, IEventListener* subscriber);

private:
	EventSystem() { }

private:
	std::set<IEventListener*> m_subscribers[static_cast<size_t>(Event::NUM_EVENTS)];
};

inline void EventSystem::dispatchEvent(Event& event) {
	auto& subs = m_subscribers[static_cast<size_t>(event.getType())];
	for (auto& sub : subs) {
		sub->onEvent(event);
	}
}

inline void EventSystem::subscribeToEvent(const Event::Type& type, IEventListener* subscriber) {
	m_subscribers[static_cast<size_t>(type)].insert(subscriber);
}

inline void EventSystem::unsubscribeFromEvent(const Event::Type& type, IEventListener* subscriber) {
	m_subscribers[static_cast<size_t>(type)].erase(subscriber);
}
