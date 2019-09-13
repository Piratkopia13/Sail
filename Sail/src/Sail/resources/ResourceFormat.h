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
		IXAudio2SourceVoice* m_sourceVoice = nullptr;
		XAUDIO2_BUFFER m_soundBuffer;
	};

}
