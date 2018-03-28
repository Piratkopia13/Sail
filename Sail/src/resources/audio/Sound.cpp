#include "Sound.h"

Sound::Sound() { }
Sound::~Sound() {
	delete m_buffer.pAudioData;
}


HRESULT Sound::Initialize(IXAudio2* audioEngine, wchar_t* file) {

	////////////////////////////////////////////
	//// Populating XAudio2 structures with ////
	////    the contents of RIFF chunks	    ////
	////////////////////////////////////////////

	// Declare WAVEFORMATEXTENSIBLE and XAUDIO2_BUFFER structures.
	m_WFX = { 0 };
	m_buffer = { 0 };

	// Open the audio file with CreateFile.
	TCHAR * strFileName = file;

	// Open the file
	HANDLE hFile = CreateFile(
		strFileName,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (INVALID_HANDLE_VALUE == hFile)
		return HRESULT_FROM_WIN32(GetLastError());

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	// Locate the 'RIFF' chunk in the audio file, and check the file type.
	DWORD dwChunkSize;
	DWORD dwChunkPosition;
	//check the file type, should be fourccWAVE or 'XWMA'
	FindChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);
	DWORD filetype;
	ReadChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);
	if (filetype != fourccWAVE)
		return S_FALSE;



	// Locate the 'fmt ' chunk, and copy its contents into a WAVEFORMATEXTENSIBLE structure.
	FindChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);
	ReadChunkData(hFile, &m_WFX, dwChunkSize, dwChunkPosition);



	// Locate the 'data' chunk, and read its contents into a buffer.
	//fill out the audio data buffer with the contents of the fourccDATA chunk
	FindChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);
	BYTE * pDataBuffer = new BYTE[dwChunkSize];
	ReadChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);



	// Populate an XAUDIO2_BUFFER structure.
	m_buffer.AudioBytes = dwChunkSize;  //size of the audio buffer in bytes
	m_buffer.pAudioData = pDataBuffer;  //buffer containing audio data
	m_buffer.Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer


	// Gets the length in seconds of the audio
	m_playtime = float(m_buffer.AudioBytes) / float(m_WFX.Format.nAvgBytesPerSec);

	return S_OK;
}


HRESULT Sound::FindChunk(HANDLE hFile, DWORD fourcc, DWORD & dwChunkSize, DWORD & dwChunkDataPosition) {

	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());

	DWORD dwChunkType;
	DWORD dwChunkDataSize;
	DWORD dwRIFFDataSize = 0;
	DWORD dwFileType;
	DWORD bytesRead = 0;
	DWORD dwOffset = 0;

	while (hr == S_OK) {
		DWORD dwRead;
		if (0 == ReadFile(hFile, &dwChunkType, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		if (0 == ReadFile(hFile, &dwChunkDataSize, sizeof(DWORD), &dwRead, NULL))
			hr = HRESULT_FROM_WIN32(GetLastError());

		switch (dwChunkType) {
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

		if (dwChunkType == fourcc) {
			dwChunkSize = dwChunkDataSize;
			dwChunkDataPosition = dwOffset;
			return S_OK;
		}

		dwOffset += dwChunkDataSize;

		if (bytesRead >= dwRIFFDataSize) return S_FALSE;

	}

	return S_OK;
}


HRESULT Sound::ReadChunkData(HANDLE hFile, void * buffer, DWORD buffersize, DWORD bufferoffset) {
	HRESULT hr = S_OK;

	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());

	return hr;
}

XAUDIO2_BUFFER Sound::getBuffer() {
	return m_buffer;
}

WAVEFORMATEXTENSIBLE Sound::getWFX() {
	return m_WFX;
}