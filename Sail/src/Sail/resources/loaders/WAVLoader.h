#ifndef WAV_LOADER_H
#define WAV_LOADER_H

#include <string>
#include "../ResourceFormat.h"
#include "../../utils/Utils.h"
#include "../Sail/src/API/Audio/GeneralFunctions.h"

namespace Fileloader {
		
	class WAVLoader {

	public:
		WAVLoader(const std::string& filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData);
		~WAVLoader();

	private:
		bool loadWAV(const std::string& filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData);
		//bool loadADPC(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData);

		// PRIVATE FUNCTIONS
		//------------------

		//------------------
	};
}
#endif
