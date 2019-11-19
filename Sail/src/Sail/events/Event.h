#pragma once

struct Event {
	enum class Type {
		// Engine events
		NEW_FRAME,
		WINDOW_RESIZE,
		WINDOW_FOCUS_CHANGED,
		POTATO,
		TEXTINPUT,
		CHATSENT,
		// Network events
		NETWORK_LAN_HOST_FOUND,
		NETWORK_JOINED,
		NETWORK_DISCONNECT,
		NETWORK_CHAT,
		NETWORK_WELCOME,
		NETWORK_NAME,
		NETWORK_DROPPED,
		NETWORK_CHANGE_STATE,
		NETWORK_PLAYER_CHANGED_TEAM,
		NETWORK_PLAYER_REQUESTED_TEAM_CHANGE,
		NETWORK_UPDATE_STATE_LOAD_STATUS,
		NETWORK_SERIALIZED_DATA_RECIEVED,
		// Game events
		GAME_OVER,
		RESET_WATER,
		WATER_HIT_PLAYER,
		PLAYER_DEATH,
		IGNITE_CANDLE,
		HOLDING_CANDLE_TOGGLE,
		PLAYER_JUMPED,
		PLAYER_LANDED,
		START_SHOOTING,
		LOOP_SHOOTING,
		STOP_SHOOTING,
		CHANGE_WALKING_SOUND,
		STOP_WALKING,
		SETTINGS_UPDATED,
		START_THROWING,
		STOP_THROWING,

		SANITY_SYSTEM_UPDATE_SANITY,
		NR_OF_EVENTS		// Needs to be last, and no type above can set their values manually
	};

	Event(const Type& _type) : type(_type) {}
	virtual ~Event() = default;

	const Type type;
};