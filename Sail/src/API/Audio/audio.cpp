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

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::playSound()\n\nMESSAGE: Failed to create the actual 'SourceVoice' for the sound file '%S'!", filename.c_str());
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = m_sourceVoice[m_currIndex]->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::playSound()\n\nMESSAGE: Failed to submit the 'sourceBuffer' to the 'sourceVoice' for the sound file '%S'!", filename.c_str());
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		hr = m_sourceVoice[m_currIndex]->Start(0);

#pragma region ERROR_CHECKING
		try {
			if (hr != S_OK) {
				throw std::invalid_argument(nullptr);
			}
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::loadSound()\n\nMESSAGE: Failed submit processed audio data to data buffer for the audio file '%S'!", filename.c_str());
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO ERROR!"), MB_ICONERROR);
			std::exit(0);
		}
#pragma endregion

		m_currIndex++;
		m_currIndex %= SOUND_COUNT;

		return (m_currIndex - 1);
	}

	else {

#pragma region ERROR_CHECKING
		try {
				throw std::invalid_argument(nullptr);
		}
		catch (const std::invalid_argument& e) {

			UNREFERENCED_PARAMETER(e);
			wchar_t errorMsgBuffer[256];
			wsprintfW(errorMsgBuffer, L"FUNCTION: Audio::playSound()\n\nMESSAGE: The audio file '%S' has NOT been loaded yet!", filename.c_str());
			MessageBox(NULL, errorMsgBuffer, static_cast<LPCWSTR>(L"AUDIO WARNING!"), MB_ICONWARNING);
		}
#pragma endregion

		return (-1);
	}

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

	if (Input::IsKeyPressed(SAIL_KEY_1) && m_singlePressBool1) {

		m_singlePressBool1 = false;
		this->loadSound("../Audio/sampleLarge.wav");
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_1) && !m_singlePressBool1) {
		m_singlePressBool1 = true;
		this->playSound("../Audio/sampleLarge.wav");
	}

	if (Input::IsKeyPressed(SAIL_KEY_0)) {
		this->pauseAllSounds();
	}

	if (Input::IsKeyPressed(SAIL_KEY_9)) {
		this->pauseSound(0);
	}

	//if (Input::IsKeyPressed(SAIL_KEY_7)) {

	//	if (m_singlePressBool2) {

	//		m_singlePressBool2 = false;
	//		//this->stopSound(1);
	//		//this->loadCompressedSound("../Audio/sampleADPC_1.wav", 1);
	//		//this->playSound(1);
	//	}

	//	else if (!m_singlePressBool2) {
	//		m_singlePressBool2 = true;
	//	}
	//}


	//if (Input::IsKeyPressed(SAIL_KEY_8)) {
	//	//this->stopSound();
	//}
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
