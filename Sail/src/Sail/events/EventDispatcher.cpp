#include "pch.h"
#include "EventDispatcher.h"
#include "EventReceiver.h"

void EventDispatcher::emit(const Event& e) {
	// Emit to each subscriber of this event type
	auto& subs = m_subscribers[static_cast<size_t>(e.type)];
	size_t size = subs.size();
	for (size_t i = 0; i < size; i++) {
		subs[i]->onEvent(e);
	}
}

void EventDispatcher::subscribe(const Event::Type& type, EventReceiver* subscriber) {
	// Prevent receivers from having multiple subscriptions to one event type
	if (subscribed(type, subscriber)) {
		return;
	}

	// Add subscriber to the list
	m_subscribers[static_cast<size_t>(type)].push_back(subscriber);
}

void EventDispatcher::unsubscribe(const Event::Type& type, EventReceiver* subscriber) {
	auto& receivers = m_subscribers[static_cast<size_t>(type)];
	receivers.erase(std::remove(receivers.begin(), receivers.end(), subscriber), receivers.end());
}

bool EventDispatcher::subscribed(const Event::Type& type, EventReceiver* subscriber) {
	auto& receivers = m_subscribers[static_cast<size_t>(type)];
	return std::find(receivers.begin(), receivers.end(), subscriber) != receivers.end();
}
