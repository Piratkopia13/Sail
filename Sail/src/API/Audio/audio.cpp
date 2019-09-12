#include "pch.h"
#include "API/Audio/audio.hpp"
#include "MemoryManager/MemoryManager/src/MemoryManager.h"
#include "Sail/Application.h"
#include "Sail/KeyCodes.h"

Audio::Audio() {

	HRESULT hr;
	hr = CoInitialize(nullptr);

	for (int i = 0; i < SOUND_COUNT; i++) {
		m_sourceVoice[i] = nullptr;
	}

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {
		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::Audio()\n\nMESSAGE: The 'CoInitialize' function failed!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	this->initXAudio2();
}

Audio::~Audio(){
	for (int i = 0; i < SOUND_COUNT; i++) {
		if (m_sourceVoice[i] != nullptr) {

			m_sourceVoice[i]->Stop();
			m_sourceVoice[i]->DestroyVoice();
		}
	}
}

void Audio::loadSound(std::string const &fileName, int index)
{
	WAVEFORMATEXTENSIBLE m_formatWAV = { 0 };

	if (m_soundBuffers[index].pAudioData != nullptr) {
		delete m_soundBuffers[index].pAudioData;
	}

	HRESULT hr = 0;

	std::wstring fileNameWSTR = s2ws(fileName);
	LPCWSTR fileNameLPCWSTR = fileNameWSTR.c_str();

	HANDLE hFile = CreateFile(
		fileNameLPCWSTR,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		nullptr);

	DWORD dwChunkSize = 0;
	DWORD dwChunkPosition = 0;

	DWORD filetype = 0;

#pragma region ERROR_CHECKING
	try {
		if (INVALID_HANDLE_VALUE == hFile) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {
		
		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to create the internal audio file!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}

	try {
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {
	
		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: 'SetFilePointer' failed!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = findChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to find the audio file's 'RIFF' data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = readChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to read data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}

	try {
		if (filetype != fourccWAVE) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: The file type was NOT of the 'WAV' type!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = findChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to find the desired data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	// reading data from WAV files
	hr = readChunkData(hFile, &m_formatWAV, dwChunkSize, dwChunkPosition);
	// reading data from ADPC-WAV files (compressed)
	/*hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);*/

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to read data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = findChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to find the desired data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	BYTE* pDataBuffer = new BYTE[dwChunkSize];

	hr = readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to read data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion ERROR_CHECKING

	m_soundBuffers[index].AudioBytes = dwChunkSize;  //buffer containing audio data
	m_soundBuffers[index].pAudioData = pDataBuffer;  //size of the audio buffer in bytes
	m_soundBuffers[index].Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	// creating a 'sourceVoice' for WAV file-type
	hr = m_xAudio2->CreateSourceVoice(&m_sourceVoice[index], (WAVEFORMATEX*)&m_formatWAV);
	// creating a 'sourceVoice' for ADPC-WAV compressed file-type
	//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed to create the actual 'SourceVoice' for the sound!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = m_sourceVoice[index]->SubmitSourceBuffer(&m_soundBuffers[index]);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed submit processed audio data to data buffer!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	//m_currIndex++;
}

void Audio::loadCompressedSound(std::string const& fileName, int index)
{
	ADPCMWAVEFORMAT m_formatADPCM = { 0 };

	if (m_soundBuffers[index].pAudioData != nullptr) {
		delete m_soundBuffers[index].pAudioData;
	}

	HRESULT hr = 0;

	std::wstring fileNameWSTR = s2ws(fileName);
	LPCWSTR fileNameLPCWSTR = fileNameWSTR.c_str();

	HANDLE hFile = CreateFile(
		fileNameLPCWSTR,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		nullptr);

	DWORD dwChunkSize = 0;
	DWORD dwChunkPosition = 0;
	DWORD filetype = 0;

#pragma region ERROR_CHECKING
	try {
		if (INVALID_HANDLE_VALUE == hFile) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to create the internal audio file!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}

	try {
		if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, 0, NULL, FILE_BEGIN)) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: 'SetFilePointer' failed!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = findChunk(hFile, fourccRIFF, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the audio file's 'RIFF' data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = readChunkData(hFile, &filetype, sizeof(DWORD), dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}

	try {
		if (filetype != fourccWAVE) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: The file type was NOT of the 'WAV' type!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = findChunk(hFile, fourccFMT, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the desired data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	// reading data from WAV files
	hr = readChunkData(hFile, &m_formatADPCM, dwChunkSize, dwChunkPosition);
	// reading data from ADPC-WAV files (compressed)
	/*hr = ReadChunkData(hFile, &adpcwf, dwChunkSize, dwChunkPosition);*/

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = findChunk(hFile, fourccDATA, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to find the desired data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	BYTE* pDataBuffer = new BYTE[dwChunkSize];

	hr = readChunkData(hFile, pDataBuffer, dwChunkSize, dwChunkPosition);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to read data chunk!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion ERROR_CHECKING

	m_soundBuffers[index].AudioBytes = dwChunkSize;  //buffer containing audio data
	m_soundBuffers[index].pAudioData = pDataBuffer;  //size of the audio buffer in bytes
	m_soundBuffers[index].Flags = XAUDIO2_END_OF_STREAM; // tell the source voice not to expect any data after this buffer

	// creating a 'sourceVoice' for WAV file-type
	hr = m_xAudio2->CreateSourceVoice(&m_sourceVoice[index], (WAVEFORMATEX*)&m_formatADPCM);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed to create the actual 'SourceVoice' for the sound!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = m_sourceVoice[index]->SubmitSourceBuffer(&m_soundBuffers[index]);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadCompressedSound()\n\nMESSAGE: Failed submit processed audio data to data buffer!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion
}

void Audio::playSound(int index) {

	HRESULT hr;

	hr = m_sourceVoice[index]->Start(0);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::playSound()\n\nMESSAGE: Failed to play the loaded audio sample!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion
}

void Audio::stopSound(int index) {
	if (m_soundBuffers[index].pAudioData != nullptr) {
		m_sourceVoice[index]->Stop();
	}
}

void Audio::updateAudio() {
	if (Input::IsKeyPressed(SAIL_KEY_9) && m_singlePressBool1) {

		m_singlePressBool1 = false;
		this->stopSound(0);
		this->loadSound("../Audio/sampleLarge.wav", 0);
		this->playSound(0);
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_9) && !m_singlePressBool1) {
		m_singlePressBool1 = true;
	}

	if (Input::IsKeyPressed(SAIL_KEY_0)) {
		this->stopSound(0);
	}

	if (Input::IsKeyPressed(SAIL_KEY_7)) {

		if (m_singlePressBool2) {

			m_singlePressBool2 = false;
			this->stopSound(1);
			this->loadCompressedSound("../Audio/sampleADPC_1.wav", 1);
			this->playSound(1);
		}

		else if (!m_singlePressBool2) {
			m_singlePressBool2 = true;
		}
	}


	if (Input::IsKeyPressed(SAIL_KEY_8)) {
		this->stopSound(1);
	}
}

void Audio::initXAudio2() {

	HRESULT hr;

	hr = XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {

		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::initXAudio2()\n\nMESSAGE: Creating the 'IXAudio2' object failed!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice);

#pragma region ERROR_CHECKING
	try {
		if (hr != S_OK) {
			throw std::invalid_argument(nullptr);
		}
	}
	catch (const std::invalid_argument& e) {
		UNREFERENCED_PARAMETER(e);
		wchar_t errorMsgBuffer[256];
		wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::initXAudio2()\n\nMESSAGE: Creating the 'IXAudio2MasterVoice' failed!");
		MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
		std::exit(0);
	}
#pragma endregion

	//m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

HRESULT Audio::findChunk(HANDLE hFile, DWORD fourcc, DWORD& dwChunkSize, DWORD& dwChunkDataPosition)
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

HRESULT Audio::readChunkData(HANDLE hFile, void* buffer, DWORD buffersize, DWORD bufferoffset)
{
	HRESULT hr = S_OK;
	if (INVALID_SET_FILE_POINTER == SetFilePointer(hFile, bufferoffset, NULL, FILE_BEGIN))
		return HRESULT_FROM_WIN32(GetLastError());
	DWORD dwRead;
	if (0 == ReadFile(hFile, buffer, buffersize, &dwRead, NULL))
		hr = HRESULT_FROM_WIN32(GetLastError());
	return hr;
}