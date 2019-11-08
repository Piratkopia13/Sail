#pragma once
#include "Event.h"

struct UpdateInsanityEvent : public Event {
	UpdateInsanityEvent(const int _id, const float _insanityVal)
		: Event(Event::Type::INSANITY_SYSTEM_UPDATE_INSANITY)
		, id(_id) 
		, insanityVal(_insanityVal) 
		{}
	~UpdateInsanityEvent() = default;

	const int id;
	const float insanityVal;
};