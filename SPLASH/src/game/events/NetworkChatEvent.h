#pragma once

#include "Sail/events/Event.h"

struct NetworkChatEvent : public Event{
	NetworkChatEvent(const Message& _chatMessage)
		: Event(Event::Type::NETWORK_CHAT)
		, chatMessage(_chatMessage) { }
	~NetworkChatEvent() = default;

	const Message chatMessage;
};