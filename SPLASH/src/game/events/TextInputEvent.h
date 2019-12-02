#pragma once
#include <WinUser.h>
#include "Sail/events/Event.h"

struct TextInputEvent : public Event {
	TextInputEvent(const MSG& _msg)
		: Event(Event::Type::TEXTINPUT) 
		, msg(_msg) { }
	~TextInputEvent() = default;

	const MSG msg;
};


struct ChatSent : public Event {
	ChatSent(const std::string& _msg)
		: Event(Event::Type::CHATSENT)
		,msg(_msg) { }
	~ChatSent() = default;
	std::string msg;
};