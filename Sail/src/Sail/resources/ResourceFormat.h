#pragma once

#include "Xaudio2.h"

namespace ResourceFormat {

	struct TextureData {
		unsigned int width;
		unsigned int height;
		unsigned int channels;
		unsigned char* textureData;
	};

	struct AudioData {
		XAUDIO2_BUFFER m_soundBuffer = { 0 };
		WAVEFORMATEXTENSIBLE m_formatWAV = { 0 };
	};

}
