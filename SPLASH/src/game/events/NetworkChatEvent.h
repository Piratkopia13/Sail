#pragma once

#include "../../../../Sail/src/Sail/events/Event.h"

class NetworkChatEvent : public Event{
public:
	NetworkChatEvent(std::string chatMessage) 
		: Event(Event::NETWORK_CHAT) 
		, m_chatMessage(chatMessage)
	{

	}
	~NetworkChatEvent() {}

	inline std::string getMessage() const { return m_chatMessage; };

	static Type getStaticType() {
		return Event::NETWORK_CHAT;
	}

private:
	std::string m_chatMessage;
};