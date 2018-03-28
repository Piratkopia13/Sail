#pragma once

#include "../../Sail.h"

class Sound {

public:
	Sound();
	~Sound();

	virtual HRESULT Initialize(IXAudio2* audioEngine, wchar_t* file);

	XAUDIO2_BUFFER getBuffer();
	WAVEFORMATEXTENSIBLE getWFX();

private:
	HRESULT FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition);
	HRESULT ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset);

protected:
	float m_playtime;


private:
	XAUDIO2_BUFFER m_buffer;
	WAVEFORMATEXTENSIBLE m_WFX;


};