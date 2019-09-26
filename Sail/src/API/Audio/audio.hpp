#ifndef AUDIO_HPP
#define AUDIO_HPP

enum AudioType {MUSIC};

#include "Xaudio2.h"
#include <thread>

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl/client.h>
#include <mutex>
#include <utility>
#include <windows.h>
#include <exception>
#include <stdexcept>
#include <atomic>

#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid")

#define SOUND_COUNT 236
#define STREAMED_SOUNDS_COUNT 20
#define STREAMING_BUFFER_SIZE 32768
#define MAX_BUFFER_COUNT 3

struct StreamingVoiceContext : public IXAudio2VoiceCallback
{
	STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override
	{
	}
	STDMETHOD_(void, OnVoiceProcessingPassEnd)() override
	{
	}
	STDMETHOD_(void, OnStreamEnd)() override
	{
	}
	STDMETHOD_(void, OnBufferStart)(void*) override
	{
	}
	STDMETHOD_(void, OnBufferEnd)(void*) override
	{
		SetEvent(hBufferEndEvent);
	}
	STDMETHOD_(void, OnLoopEnd)(void*) override
	{
	}
	STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override
	{
	}

	HANDLE hBufferEndEvent;

#pragma region STREAMING_VOICE_CONTEXT
	StreamingVoiceContext() :
#if (_WIN32_WINNT >= _WIN32_WINNT_VISTA)
		hBufferEndEvent(CreateEventEx(nullptr, nullptr, 0, EVENT_MODIFY_STATE | SYNCHRONIZE))
#else
		hBufferEndEvent(CreateEvent(nullptr, FALSE, FALSE, nullptr))
#endif
	{
	}
	virtual ~StreamingVoiceContext()
	{
		CloseHandle(hBufferEndEvent);
	}
};
#pragma endregion

class Audio
{
public:
	Audio();
	~Audio();

	void loadSound(const std::string &filename);
	int playSound(const std::string& filename);
	int streamSound(const std::string& filename, bool loop = true);
	void pauseSound(int index);
	void stopAllSounds();

	void updateAudio();

private:
	bool m_isRunning = true;

	// 'TESTER' BUTTONS *-*-*-*-*-//
	bool m_singlePress1 = true;  //
	bool m_singlePress2 = true; //
	bool m_singlePress3 = true;//
	// *-*-*-*-*-*-*-*-*-*-*-*//

	// The main 'XAudio2' engine
	IXAudio2* m_xAudio2 = nullptr;
	// Represents the audio output device
	IXAudio2MasteringVoice* m_masterVoice = nullptr;
	// Represents each loaded sound in the form of an 'object'
	IXAudio2SourceVoice* m_sourceVoiceSound[SOUND_COUNT];
	IXAudio2SourceVoice* m_sourceVoiceStream[STREAMED_SOUNDS_COUNT];

	
	int m_currSoundIndex = 0;
	std::atomic<int> m_currStreamIndex = 0;

	// INIT
	void initialize();

	BYTE m_streamBuffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];
	bool m_isStreaming[STREAMED_SOUNDS_COUNT];
	bool m_isFinished[STREAMED_SOUNDS_COUNT];
	OVERLAPPED m_overlapped[STREAMED_SOUNDS_COUNT];

	// PRIVATE FUNCTION
	//-----------------
	void initXAudio2();
	void streamSoundInternal(const std::string& filename, int myIndex, bool loop);
	HRESULT FindMediaFileCch(WCHAR* strDestPath, int cchDest, LPCWSTR strFilename);
	// ----------------
};

#endif
