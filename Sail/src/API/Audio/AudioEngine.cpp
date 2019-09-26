#include "pch.h"
#include "API/Audio/AudioEngine.h"
#include "MemoryManager/MemoryManager/src/MemoryManager.h"
#include "Sail/Application.h"
#include "Sail/KeyCodes.h"
#include "API/Audio/GeneralFunctions.h"
#include <fstream>
#include <xaudio2.h>
#include "WaveBankReader.h"
#include <math.h>

AudioEngine::AudioEngine() {

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
		m_sourceVoiceSound[i] = nullptr;
	}

	for (int i = 0; i < STREAMED_SOUNDS_COUNT; i++) {
		m_sourceVoiceStream[i] = nullptr;
		m_isStreaming[i] = false;
		m_isFinished[i] = false;
		m_overlapped[i] = { 0 };
	}
}

AudioEngine::~AudioEngine(){
	m_isRunning = false;
}

void AudioEngine::loadSound(const std::string& filename) {
	Application::getInstance()->getResourceManager().loadAudioData(filename, m_xAudio2);
}

int AudioEngine::playSound(const std::string& filename) {
	if (m_masterVoice == nullptr) {
		Logger::Error("'IXAudio2MasterVoice' has not been correctly initialized; audio is unplayable!");
		return -1;
	}
	if (Application::getInstance()->getResourceManager().hasAudioData(filename)) {

		int returnValue = m_currSoundIndex; // Store early

		if (m_sourceVoiceSound[m_currSoundIndex] != nullptr) {
			m_sourceVoiceSound[m_currSoundIndex]->Stop();
		}

		// creating a 'sourceVoice' for WAV file-type
		HRESULT hr = m_xAudio2->CreateSourceVoice(&m_sourceVoiceSound[m_currSoundIndex], (WAVEFORMATEX*)Application::getInstance()->getResourceManager().getAudioData(filename).getFormat());

		// THIS IS THE OTHER VERSION FOR ADPC
				// ... for ADPC-WAV compressed file-type
				//hr = xAudio->CreateSourceVoice(&pSourceVoice, (WAVEFORMATEX*)& adpcwf);

		if (hr != S_OK) {
			Logger::Error("Failed to create the actual 'SourceVoice' for a sound file!");
		}

		hr = m_sourceVoiceSound[m_currSoundIndex]->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());

		if (hr != S_OK) {
			Logger::Error("Failed to submit the 'sourceBuffer' to the 'sourceVoice' for a sound file!");
		}

		hr = m_sourceVoiceSound[m_currSoundIndex]->Start(0);
		if (hr != S_OK) {
			Logger::Error("Failed submit processed audio data to data buffer for a audio file");
		}

		m_currSoundIndex++;
		m_currSoundIndex %= SOUND_COUNT;

		return returnValue;
	}

	else {
		Logger::Error("That audio file has NOT been loaded yet!");
		return (-1);
	}
}

int AudioEngine::streamSound(const std::string& filename, bool loop) {
	int returnValue = m_currStreamIndex; // Store early

	if (m_masterVoice == nullptr) {
		Logger::Error("'IXAudio2MasterVoice' has not been correctly initialized; audio is unplayable!");
		return -1;
	}

	Application::getInstance()->pushJobToThreadPool(
		[this](int id) {
			return this->streamSoundInternal("../Audio/wavebankLong.xwb", m_currStreamIndex, false);
		});

	m_currStreamIndex.store((m_currStreamIndex + 1) % STREAMED_SOUNDS_COUNT);
	return returnValue;
}

void AudioEngine::stopSpecificSound(int index) {

	if (index < 0 || index > SOUND_COUNT) {
		Logger::Error("Tried to STOP a sound from playing with an INVALID INDEX!");
		return;
	}

	if (m_sourceVoiceSound[index] != nullptr) {
		m_sourceVoiceSound[index]->Stop();
	}
}

void AudioEngine::stopSpecificStream(int index) {
	
	if (index < 0 || index > STREAMED_SOUNDS_COUNT) {
		Logger::Error("Tried to STOP a sound being streamed with an INVALID INDEX!");
		return;
	}

	if (m_sourceVoiceStream[index] != nullptr) {
		m_isStreaming[index] = false;
	}
}

void AudioEngine::stopAllSounds() {

	for (int i = 0; i < SOUND_COUNT; i++) {
		if (m_sourceVoiceSound[i] != nullptr) {
			m_sourceVoiceSound[i]->Stop();
			m_sourceVoiceSound[i]->DestroyVoice();
			m_sourceVoiceSound[i] = nullptr;
		}
	}

	for (int i = 0; i < STREAMED_SOUNDS_COUNT; i++) {
		m_isStreaming[i] = false;
	}
}

void AudioEngine::updateAudio() {

	// 'PLAY' Sound
	if (Input::IsKeyPressed(SAIL_KEY_1) && m_singlePress1) {

		m_singlePress1 = false;
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_1) && !m_singlePress1) {
		this->playSound("../Audio/sampleLarge.wav");
		m_singlePress1 = true;
	}

	// 'STREAM' Sound
	if (Input::IsKeyPressed(SAIL_KEY_2) && m_singlePress2) {
		m_singlePress2 = false;
		this->streamSound("../Audio/wavebankLong.xwb", false);
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_2) && !m_singlePress2) {
		m_singlePress2 = true;
	}


	if (Input::IsKeyPressed(SAIL_KEY_3) && m_singlePress3) {
		m_singlePress3 = false;
		this->streamSound("../Audio/wavebankShortFade.xwb", false);
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_3) && !m_singlePress3) {
		m_singlePress3 = true;
	}

	// 'STOPPING' sound
	if (Input::IsKeyPressed(SAIL_KEY_9)) {
		this->stopSpecificSound(0);
	}

	if (Input::IsKeyPressed(SAIL_KEY_0)) {
		this->stopAllSounds();
	}
}

void AudioEngine::initialize() {

}

void AudioEngine::initXAudio2() {

	HRESULT hr;

	hr = XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if (hr != S_OK) {
		Logger::Error("Creating the 'IXAudio2' object failed!");
	}

	hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice);
	if (hr != S_OK) {
		Logger::Error("Creating the 'IXAudio2MasterVoice' failed!");
	}
}

void AudioEngine::streamSoundInternal(const std::string& filename, int myIndex, bool loop) {

	if (m_isStreaming[m_currStreamIndex]) {
		while (m_isFinished[m_currStreamIndex] == false) {
			if (m_isStreaming[m_currStreamIndex]) {
				m_isStreaming[m_currStreamIndex] = false;
			}
		}
	}

	if (m_isRunning) {

#pragma region VARIABLES_LIST
		m_isStreaming[myIndex] = true;
		m_isFinished[myIndex] = false;
		WCHAR wavebank[MAX_PATH];
		DirectX::WaveBankReader wbr;
		StreamingVoiceContext voiceContext;
		HRESULT hr;
		char formatBuff[64];
		WAVEFORMATEX* wfx = nullptr;
		DirectX::WaveBankReader::Metadata metadata;
		std::unique_ptr<uint8_t[]> buffers[MAX_BUFFER_COUNT];
		float currentVolume = 0.0f;
		int totalChunks = 0;
		int currentChunk = 0;
#pragma endregion

		hr = FindMediaFileCch(wavebank, MAX_PATH, stringToWString(filename).c_str());
		if (hr != S_OK) {
			Logger::Error("Failed to find the specified '.xwb' file!");
			return;
		}

		hr = wbr.Open(wavebank);
		if (hr != S_OK) {
			Logger::Error("Failed to open wavebank file!");
			return;
		}

		if (!wbr.IsStreamingBank()) {
			Logger::Error("Tried to stream a non-streamable '.xwb' file! Contact Oliver if you've gotten this message!");
			return;
		}

		while (m_isStreaming[myIndex]) {
			// For every piece of audio within the '.xwb' file (preferably only be 1)
			for (DWORD i = 0; i < wbr.Count(); i++) {

				// Get info we need to play this wave (need space fo PCM, ADPCM, and xWMA formats)
				wfx = reinterpret_cast<WAVEFORMATEX*>(&formatBuff);
				hr = wbr.GetFormat(i, wfx, 64);
				if (hr != S_OK) {
					Logger::Error("Failed to get wave format for '.xwb' file!");
				}

				hr = wbr.GetMetadata(i, metadata);
				if (hr != S_OK) {
					Logger::Error("Failed to get meta data for '.xwb' file!");
				}

				hr = m_xAudio2->CreateSourceVoice(&m_sourceVoiceStream[myIndex], wfx, 0, 1.0f, &voiceContext);
				if (hr != S_OK) {
					Logger::Error("Failed to create source voice!");
				}

				m_sourceVoiceStream[myIndex]->SetVolume(0);

				// Create the 'overlapped' structure as well as buffers to handle async I/O
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
				m_overlapped[myIndex].hEvent = CreateEventEx(nullptr, nullptr, CREATE_EVENT_MANUAL_RESET, EVENT_MODIFY_STATE | SYNCHRONIZE);
#else
				m_overlapped.hEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
#endif

				if ((STREAMING_BUFFER_SIZE % wfx->nBlockAlign) != 0)
				{
					//
					// non-PCM data will fail here. ADPCM requires a more complicated streaming mechanism to deal with submission in audio frames that do
					// not necessarily align to the 2K async boundary.
					//
					m_isStreaming[myIndex] = false;
					break;
				}

				for (size_t j = 0; j < MAX_BUFFER_COUNT; ++j)
				{
					buffers[j].reset(SAIL_NEW uint8_t[STREAMING_BUFFER_SIZE]);
				}

				DWORD currentDiskReadBuffer = 0;
				DWORD currentPosition = 0;

				HANDLE async = wbr.GetAsyncHandle();

				// Reading from the file (when time-since-last-read has passed threshold)
				while ((currentPosition < metadata.lengthBytes) && m_isStreaming[myIndex])
				{
					if (GetAsyncKeyState(VK_ESCAPE))
					{
						m_isStreaming[myIndex] = false;
						while (GetAsyncKeyState(VK_ESCAPE) && m_isStreaming[myIndex]) {
							Sleep(10);
						}
						break;
					}

					DWORD cbValid = std::min(STREAMING_BUFFER_SIZE, static_cast<int>(metadata.lengthBytes - static_cast<UINT32>(currentPosition)));
					m_overlapped[myIndex].Offset = metadata.offsetBytes + currentPosition;

					bool wait = false;
					if (!ReadFile(async, buffers[currentDiskReadBuffer].get(), STREAMING_BUFFER_SIZE, nullptr, &m_overlapped[myIndex]))
					{
						DWORD error = GetLastError();
						if (error != ERROR_IO_PENDING)
						{
							m_isStreaming[myIndex] = false;
							break;
						}
						wait = true;
					}

					currentPosition += cbValid;

					//
					// At this point the read is progressing in the background and we are free to do
					// other processing while we wait for it to finish. For the purposes of this sample,
					// however, we'll just go to sleep until the read is done.
					//
					if (wait) {
						WaitForSingleObject(m_overlapped[myIndex].hEvent, INFINITE);
					}

					DWORD cb;
#if (_WIN32_WINNT >= 0x0602 /*_WIN32_WINNT_WIN8*/)
					BOOL result = GetOverlappedResultEx(async, &m_overlapped[myIndex], &cb, 0, FALSE);
#else
					BOOL result = GetOverlappedResult(async, &ovlCurrentRequest, &cb, FALSE);
#endif

					if (!result)
					{
						m_isStreaming[myIndex] = false;
						break;
					}

					//
					// Now that the event has been signaled, we know we have audio available. The next
					// question is whether our XAudio2 source voice has played enough data for us to give
					// it another buffer full of audio. We'd like to keep no more than MAX_BUFFER_COUNT - 1
					// buffers on the queue, so that one buffer is always free for disk I/O.
					//
					XAUDIO2_VOICE_STATE state;
					for (;;)
					{
						m_sourceVoiceStream[myIndex]->GetState(&state);
						if (state.BuffersQueued < MAX_BUFFER_COUNT - 1) {
							break;
						}

						m_sourceVoiceStream[myIndex]->Start();
						if (currentVolume < 0.80f) {
							currentVolume += 0.1f;
							m_sourceVoiceStream[myIndex]->SetVolume(currentVolume);
						}

						currentChunk++;
						WaitForSingleObject(voiceContext.hBufferEndEvent, INFINITE);
					}

					//
					// At this point we have a buffer full of audio and enough room to submit it, so
					// let's submit it and get another read request going.
					//
					XAUDIO2_BUFFER buf = { 0 };
					buf.AudioBytes = cbValid;
					buf.pAudioData = buffers[currentDiskReadBuffer].get();
					if (currentPosition >= metadata.lengthBytes) {
						buf.Flags = XAUDIO2_END_OF_STREAM;
					}

					m_sourceVoiceStream[myIndex]->SubmitSourceBuffer(&buf);

					currentDiskReadBuffer++;
					currentDiskReadBuffer %= MAX_BUFFER_COUNT;
				}
			}

			if (!loop) {
				m_isStreaming[myIndex] = false;
			}

			if (!m_isStreaming[myIndex])
			{
				m_sourceVoiceStream[myIndex]->SetVolume(0);

				XAUDIO2_VOICE_STATE state;
				for (;;)
				{
					m_sourceVoiceStream[myIndex]->GetState(&state);
					if (!state.BuffersQueued)
						break;

					WaitForSingleObject(voiceContext.hBufferEndEvent, INFINITE);
				}
			}

			currentChunk = 0;
			currentVolume = 0;
			m_sourceVoiceStream[myIndex]->Stop();
		}
		//
		// Clean up
		//
		if (m_sourceVoiceStream[myIndex] != nullptr) {
			m_sourceVoiceStream[myIndex]->Stop(0);
			m_sourceVoiceStream[myIndex]->DestroyVoice();
			m_sourceVoiceStream[myIndex] = nullptr;
			CloseHandle(m_overlapped[myIndex].hEvent);
		}

		m_isFinished[myIndex] = true;
	}
	return;
}

//--------------------------------------------------------------------------------------
// Helper function to try to find the location of a media file
//--------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT AudioEngine::FindMediaFileCch(WCHAR* strDestPath, int cchDest, LPCWSTR strFilename)
{
	bool bFound = false;

	if (!strFilename || strFilename[0] == 0 || !strDestPath || cchDest < 10)
		return E_INVALIDARG;

	// Get the exe name, and exe path
	WCHAR strExePath[MAX_PATH] = { 0 };
	WCHAR strExeName[MAX_PATH] = { 0 };
	WCHAR* strLastSlash = nullptr;
	GetModuleFileName(nullptr, strExePath, MAX_PATH);
	strExePath[MAX_PATH - 1] = 0;
	strLastSlash = wcsrchr(strExePath, TEXT('\\'));
	if (strLastSlash)
	{
		wcscpy_s(strExeName, MAX_PATH, &strLastSlash[1]);

		// Chop the exe name from the exe path
		*strLastSlash = 0;

		// Chop the .exe from the exe name
		strLastSlash = wcsrchr(strExeName, TEXT('.'));
		if (strLastSlash)
			* strLastSlash = 0;
	}

	wcscpy_s(strDestPath, cchDest, strFilename);
	if (GetFileAttributes(strDestPath) != 0xFFFFFFFF)
		return S_OK;

	// Search all parent directories starting at .\ and using strFilename as the leaf name
	WCHAR strLeafName[MAX_PATH] = { 0 };
	wcscpy_s(strLeafName, MAX_PATH, strFilename);

	WCHAR strFullPath[MAX_PATH] = { 0 };
	WCHAR strFullFileName[MAX_PATH] = { 0 };
	WCHAR strSearch[MAX_PATH] = { 0 };
	WCHAR* strFilePart = nullptr;

	GetFullPathName(L".", MAX_PATH, strFullPath, &strFilePart);
	if (!strFilePart)
		return E_FAIL;

	while (strFilePart && *strFilePart != '\0')
	{
		swprintf_s(strFullFileName, MAX_PATH, L"%s\\%s", strFullPath, strLeafName);
		if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
		{
			wcscpy_s(strDestPath, cchDest, strFullFileName);
			bFound = true;
			break;
		}

		swprintf_s(strFullFileName, MAX_PATH, L"%s\\%s\\%s", strFullPath, strExeName, strLeafName);
		if (GetFileAttributes(strFullFileName) != 0xFFFFFFFF)
		{
			wcscpy_s(strDestPath, cchDest, strFullFileName);
			bFound = true;
			break;
		}

		swprintf_s(strSearch, MAX_PATH, L"%s\\..", strFullPath);
		GetFullPathName(strSearch, MAX_PATH, strFullPath, &strFilePart);
	}
	if (bFound)
		return S_OK;

	// On failure, return the file as the path but also return an error code
	wcscpy_s(strDestPath, cchDest, strFilename);

	return HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
}