#pragma once
#include "Sail/events/Event.h"
#include <string>

class Texture;

struct TextureLoadedToRAMEvent : public Event {
	TextureLoadedToRAMEvent(Texture* _texture, const std::string& _fileName)
		: Event(Event::Type::TEXTURE_LOADED_TO_RAM)
		, texture(_texture)
		, fileName(_fileName)
	{}
	Texture* texture;
	const std::string fileName;
};