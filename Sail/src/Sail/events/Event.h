#pragma once

struct Event {
	enum class Type {
		WINDOW_RESIZE,
		WINDOW_FOCUS_CHANGED,
		POTATO,
		TEXTINPUT,
		NETWORK_LAN_HOST_FOUND,
		NETWORK_JOINED,
		NETWORK_DISCONNECT,
		NETWORK_CHAT,
		NETWORK_WELCOME,
		NETWORK_NAME,
		NETWORK_DROPPED,
		NETWORK_START_GAME,
		NETWORK_BACK_TO_LOBBY,
		NETWORK_SERIALIZED_DATA_RECIEVED,
		GAME_OVER,
		RESET_WATER,

		NR_OF_EVENTS		// Needs to be last, and no type above can set their values manually
	};

	Event(const Type& _type) : type(_type) {}
	virtual ~Event() = default;

	const Type type;
};