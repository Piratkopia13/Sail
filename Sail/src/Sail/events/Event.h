#pragma once

class Event {
public:
	enum Type {
		WINDOW_RESIZE,
		WINDOW_FOCUS_CHANGED,
		NEW_FRAME,

		NUM_EVENTS
	};
public:
	Event(Type type);
	~Event();

	Type getType() const;

private:
	Type m_type;

};