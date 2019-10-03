#pragma once

class Event {
public:
	enum Type {
		WINDOW_RESIZE,
		WINDOW_FOCUS_CHANGED,
		POTATO,
		TEXTINPUT,
		NETWORK_JOINED,
		NETWORK_DISCONNECT,
		NETWORK_CHAT,
		NETWORK_WELCOME,
		NETWORK_NAME,
		NETWORK_DROPPED,
		NETWORK_START_GAME,
		NETWORK_SERIALIZED_DATA_RECIEVED,
		PLAYER_CANDLE_HIT,
	};
public:
	Event(Type type);
	~Event();

	Type getType() const;

private:
	Type m_type;

};