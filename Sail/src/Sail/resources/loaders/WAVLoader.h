#ifndef WAV_LOADER_H
#define WAV_LOADER_H

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

#include <string>
#include "../ResourceFormat.h"
#include "../../utils/Utils.h"

namespace Fileloader {

	class WAVLoader {

	public:
		WAVLoader(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData);
		~WAVLoader();

	private:
		bool loadWAV(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData);
		bool loadADPC(std::string filename, IXAudio2* xAudio2, ResourceFormat::AudioData& audioData);

		// PRIVATE FUNCTIONS
		//------------------
		std::string getFileExtension(const std::string& filename);
		std::wstring s2ws(const std::string& s);
		HRESULT findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition);
		HRESULT readChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset);
		//------------------
	};
}
#endif
