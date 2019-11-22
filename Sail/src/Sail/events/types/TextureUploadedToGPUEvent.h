#pragma once
#include "Sail/events/Event.h"
#include <string>

struct TextureUploadedToGPUEvent : public Event {
	TextureUploadedToGPUEvent(const std::string& _fileName)
		: Event(Event::Type::TEXTURE_UPLOADED_TO_GPU)
		, fileName(_fileName)
	{}
	const std::string fileName;
};