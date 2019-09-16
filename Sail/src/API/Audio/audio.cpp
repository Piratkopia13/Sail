#include "pch.h"
#include "API/Audio/audio.hpp"
#include "MemoryManager/MemoryManager/src/MemoryManager.h"
#include "Sail/Application.h"
#include "Sail/KeyCodes.h"

Audio::Audio() {

	HRESULT hr;
	hr = CoInitialize(nullptr);

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

	for (int i = 0; i < SOUND_COUNT; i++) {
		m_sourceVoice[i] = nullptr;
	}
}

Audio::~Audio(){
	for (int i = 0; i < SOUND_COUNT; i++) {
		if (m_sourceVoice[i] != nullptr) {
	
			m_sourceVoice[i]->Stop();
			m_sourceVoice[i]->DestroyVoice();
		}
	}
}

void Audio::loadSound(const std::string& filename) {
	Application::getInstance()->getResourceManager().loadAudioData(filename, m_xAudio2);
}

int Audio::playSound(const std::string &filename) {

	if (Application::getInstance()->getResourceManager().hasAudioData(filename)) {

		if (m_sourceVoice[m_currIndex] != nullptr) {
			m_sourceVoice[m_currIndex]->Stop();
		}

		// creating a 'sourceVoice' for WAV file-type
		HRESULT hr = m_xAudio2->CreateSourceVoice(&m_sourceVoice[m_currIndex], (WAVEFORMATEX*)Application::getInstance()->getResourceManager().getAudioData(filename).getFormat());
			
		// THIS IS THE OTHER VERSION FOR ADPC
				// ... for ADPC-WAV compressed file-type
				//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::playSound()", "Failed to create the actual 'SourceVoice' for a sound file!", 0, true);

		hr = m_sourceVoice[m_currIndex]->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());

		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::playSound()", "Failed to submit the 'sourceBuffer' to the 'sourceVoice' for a sound file!", 0, true);

		hr = m_sourceVoice[m_currIndex]->Start(0);

		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loadSound()", "Failed submit processed audio data to data buffer for a audio file", 0, true);

		m_currIndex++;
		m_currIndex %= SOUND_COUNT;

		return (m_currIndex - 1);
	}

	else {

		errorCheck(E_FAIL, "AUDIO WARNING!", "FUNCTION: Audio::playSound()", "That audio file has NOT been loaded yet!", 1, false);
		return (-1);
	}

}

int Audio::streamSound(const std::string& filename) {

	this->loadSound(filename);

	m_tempStreamThread = new std::thread(&Audio::streamingLoop, this, (this->m_sourceVoice[this->m_currIndex]));

	return 0;
}

void Audio::pauseSound(int index) {

	if (m_sourceVoice[index] != nullptr) {
		m_sourceVoice[index]->Stop();
	}
}

void Audio::pauseAllSounds() {

	for (int i = 0; i < SOUND_COUNT; i++) {
		if (m_sourceVoice[i] != nullptr) {
			m_sourceVoice[i]->Stop();
		}
	}
}

void Audio::updateAudio() {

	// 'PLAY' Sound
	if (Input::IsKeyPressed(SAIL_KEY_1) && m_singlePress1) {

		m_singlePress1 = false;
		this->loadSound("../Audio/sampleLarge.wav");
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_1) && !m_singlePress1) {
		m_singlePress1 = true;
		this->playSound("../Audio/sampleLarge.wav");
	}

	if (Input::IsKeyPressed(SAIL_KEY_9)) {
		this->pauseSound(0);
	}

	if (Input::IsKeyPressed(SAIL_KEY_0)) {
		this->pauseAllSounds();
	}



	// 'STREAM' Sound
	if (Input::IsKeyPressed(SAIL_KEY_2) && m_singlePress2) {

		m_singlePress2 = false;
		
		this->streamSound("../Audio/sampleLarge.wav");
		//m_sourceVoice[this->m_currIndex]->Start(0, 0);
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_2) && !m_singlePress2) {
		m_singlePress2 = true;
	}
}

void Audio::initXAudio2() {

	HRESULT hr;

	hr = XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::initXAudio2()", "Creating the 'IXAudio2' object failed!", 0, true);

	hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice);

	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::initXAudio2()", "Creating the 'IXAudio2MasterVoice' failed!", 0, true);

	m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

void Audio::streamingLoop(const std::string& filename, IXAudio2SourceVoice* streamedSourceVoice) {
	
	int currentDiskReadBuffer = 0;
	int currentPos = 0;

	AudioData* tempAudioData = &Application::getInstance()->getResourceManager().getAudioData(filename);
	int samplesPerSec = tempAudioData->getFormat()->Format.nSamplesPerSec;
	int bitsPerSample = tempAudioData->getFormat()->Format.wBitsPerSample;
	double soundDuration = static_cast<double>(tempAudioData->getSoundBuffer()->AudioBytes / static_cast<UINT32>(tempAudioData->getFormat()->Format.nAvgBytesPerSec));

	long cbWaveSize = 0.1;
	DWORD cbValid;
	DWORD dwRead;
	
	std::wstring fileNameWSTR = s2ws(filename);
	LPCWSTR fileNameLPCWSTR = fileNameWSTR.c_str();
	HANDLE hFile = CreateFile(
		fileNameLPCWSTR,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		nullptr);

	while (currentPos < cbWaveSize) {

		if (SUCCEEDED(hr) && 0 == ReadFile(hFile, &tempAudioData->getSoundBuffer()->pAudioData, dwDataSize, &dwRead, &m_overlapped)) {
			hr = HRESULT_FROM_WIN32(GetLastError());
		}
		cbValid = min(STREAMING_BUFFER_SIZE, cbWaveSize - CurrentPosition);
		if (0 == ReadFile(hFile, buffers[CurrentDiskReadBuffer], STREAMING_BUFFER_SIZE, &dwRead, &m_overlapped))
			hr = HRESULT_FROM_WIN32(GetLastError());
		m_overlapped.Offset += cbValid;

		//update the file position to where it will be once the read finishes
		CurrentPosition += cbValid;
	}
}

std::wstring Audio::s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
