#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "Xaudio2.h"

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

#define SOUND_COUNT 3
#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3

class Audio
{
public:
	Audio();
	~Audio();

	void loadSound(std::string const &fileName, int index);
	void loadCompressedSound(std::string const& fileName, int index);
	void playSound(int index);
	void stopSound(int index);

	void updateAudio();

private:
	bool m_singlePressBool1 = true;
	bool m_singlePressBool2 = true;

	// Main 'interface' object managing all audio engine states
	IXAudio2* m_xAudio2 = nullptr;
	// Represents the audio output device
	IXAudio2MasteringVoice* m_masterVoice = nullptr;
	// Represents one loaded sound in the form of an 'object'
	IXAudio2SourceVoice* m_sourceVoice[SOUND_COUNT];
	
	XAUDIO2_BUFFER m_soundBuffers[SOUND_COUNT] = { 0 };

	/*BYTE m_streamBuffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];*/
	/*OVERLAPPED m_overlapped = { 0 };*/
	/*int m_currIndex = 0;*/

	// PRIVATE FUNCTION
	//-----------------
	void initXAudio2();

	HRESULT findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());

		DWORD dwChunkType;
		DWORD dwChunkDataSize;
		DWORD dwRIFFDataSize = 0;
		DWORD dwFileType;
		DWORD bytesRead = 0;
		DWORD dwOffset = 0;

		while (hr == S_OK)
		{
			DWORD dwRead;
			if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
				hr = HRESULT_FROM_WIN32(GetLastError());

			switch (dwChunkType)
			{
			case fourccRIFF:
				dwRIFFDataSize = dwChunkDataSize;
				dwChunkDataSize = 4;
				if (0 == ReadFile(hFile, &dwFileType, sizeof(DWORD), &dwRead, NULL))
					hr = HRESULT_FROM_WIN32(GetLastError());
				break;

			default:
				if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, dwChunkDataSize, NULL, FILE_CURRENT))
					return HRESULT_FROM_WIN32(GetLastError());
			}

			dwOffset += sizeof(DWORD) * 2;

			if (dwChunkType == fourcc)
			{
				dwChunkSize = dwChunkDataSize;
				dwChunkDataPosition = dwOffset;
				return S_OK;
			}

			dwOffset += dwChunkDataSize;

			if (bytesRead >= dwRIFFDataSize) return S_FALSE;

		}

		return S_OK;
	}

	HRESULT readChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
	{
		HRESULT hr = S_OK;
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
			return HRESULT_FROM_WIN32(GetLastError());
		DWORD dwRead;
		if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}
	// ----------------
};

#endif
