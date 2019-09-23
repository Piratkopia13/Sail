#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"
#include "../states/LobbyState.h"

class NetworkChatEvent : public Event{
public:
	NetworkChatEvent(Message chatMessage)
		: Event(Event::NETWORK_CHAT) 
		, m_chatMessage(chatMessage)
	{

	}
	~NetworkChatEvent() {}

	inline Message getMessage() const { return m_chatMessage; };

	static Type getStaticType() {
		return Event::NETWORK_CHAT;
	}

private:
	Message m_chatMessage;
};