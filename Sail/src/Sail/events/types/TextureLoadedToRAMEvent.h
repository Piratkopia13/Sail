#pragma once
#include "Sail/events/Event.h"

class Texture;

struct TextureLoadedToRAMEvent : public Event {
	TextureLoadedToRAMEvent(Texture* _texture)
		: Event(Event::Type::TEXTURE_LOADED_TO_RAM)
		, texture(_texture)
	{}
	Texture* texture;
};