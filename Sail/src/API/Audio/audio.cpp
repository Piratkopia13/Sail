#include "pch.h"
#include "API/Audio/audio.hpp"
#include "MemoryManager/MemoryManager/src/MemoryManager.h"
#include "Sail/Application.h"
#include "Sail/KeyCodes.h"

Audio::Audio() {

	HRESULT hr;
	hr = CoInitialize(nullptr);

	//for (int i = 0; i < SOUND_COUNT; i++) {
	//	m_sourceVoice[i] = nullptr;
	//}

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
	//for (int i = 0; i < SOUND_COUNT; i++) {
	//	if (m_sourceVoice[i] != nullptr) {
	//
	//		m_sourceVoice[i]->Stop();
	//		m_sourceVoice[i]->DestroyVoice();
	//	}
	//}
}

void Audio::loadSound(std::string const &filename) {
	Application::getInstance()->getResourceManager().loadAudioData(filename, m_xAudio2);
}

void Audio::loadCompressedSound(std::string const& filename, int index)
{

}

void Audio::playSound(int index) {

	HRESULT hr;

	//hr = m_sourceVoice[index]->Start(0);

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
	//if (m_soundBuffers[index].pAudioData != nullptr) {
	//	m_sourceVoice[index]->Stop();
	//}
}

void Audio::updateAudio() {
	if (Input::IsKeyPressed(SAIL_KEY_9) && m_singlePressBool1) {

		m_singlePressBool1 = false;
		this->stopSound(0);
		this->loadSound("../Audio/sampleLarge.wav");
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
