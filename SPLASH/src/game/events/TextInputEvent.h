#pragma once

#include "../../../../Sail/src/Sail/events/Event.h" // FOUR
#include <WinUser.h>

class TextInputEvent : public Event {
public:
	TextInputEvent(MSG& msg)
		: Event(Event::TEXTINPUT) 
		, m_msg(msg)
	{ }
	~TextInputEvent() {}

	inline MSG& getMSG() const { return m_msg; };

	static Type getStaticType() {
		return Event::TEXTINPUT;
	}

private:
	MSG& m_msg;
	// Comment
	//
};