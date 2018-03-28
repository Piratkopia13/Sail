#pragma once

class Event {
public:
	enum Type {
		WINDOW_RESIZE
	};
public:
	Event(Type type);
	~Event();

	Type getType() const;

private:
	Type m_type;

};