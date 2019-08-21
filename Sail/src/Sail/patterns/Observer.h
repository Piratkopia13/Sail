#pragma once

enum EventType {
	EVENT_WINDOW_RESIZE,
};

class Event {
public:
	EventType type;
};

class Observer {

public:
	virtual ~Observer() { }
	virtual void onNotify(Event event) = 0;

};