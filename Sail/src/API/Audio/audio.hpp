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

#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfuuid")

#define SOUND_COUNT 3
#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3

struct SourceReaderCallback : public IMFSourceReaderCallback
{
	Microsoft::WRL::ComPtr<IMFSample>	sample = nullptr;	   // The audio-chunk sample that's read
	HANDLE								hReadSample = 0;	  // 'handle' to read the sample
	bool								endOfStream = false; // 'true' if the end of the file is reached
	HRESULT								status = 0;			// Current status of the stream
	std::mutex							guard;			   // Thread-safety

	STDMETHOD(QueryInterface) (REFIID iid, _COM_Outptr_ void** ppv) override
	{
		if (!ppv) {
			return E_POINTER;
		}

		if (_uuidof(IMFSourceReaderCallback) == iid)
		{
			*ppv = this;
			return S_OK;
		}

		*ppv = nullptr;
		return E_NOINTERFACE;
	}

	STDMETHOD_(ULONG, AddRef)() override { return 1; }
	STDMETHOD_(ULONG, Release)() override { return 1; }
	STDMETHOD(OnReadSample)(_In_ HRESULT hrStatus, _In_ DWORD dwStreamIndex, _In_ DWORD dwStreamFlags, _In_ LONGLONG llTimestamp, _In_opt_ IMFSample* pSample) override {

		UNREFERENCED_PARAMETER(dwStreamIndex);
		UNREFERENCED_PARAMETER(llTimestamp);

		std::lock_guard<std::mutex> lock(guard);
		if (SUCCEEDED(hrStatus)) {
			if (pSample) {
				sample = pSample;
			}
		}

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			endOfStream = true;
		}

		status = hrStatus;
		SetEvent(hReadSample);

		return S_OK;
	}

	STDMETHOD(OnFlush)(_In_ DWORD) override { return S_OK; };
	STDMETHOD(OnEvent)(_In_ DWORD, _In_ IMFMediaEvent*) override { return S_OK; };

	void Restart() { ; }

	SourceReaderCallback() { ; };
	virtual ~SourceReaderCallback() { ; };
};

struct StreamingVoiceCallback : public IXAudio2VoiceCallback
{
	HANDLE hBufferEndEvent;

	STDMETHOD_(void, OnVoiceProcessingPassStart)(UINT32) override { };
	STDMETHOD_(void, OnVoiceProcessingPassEnd)() override { };
	STDMETHOD_(void, OnStreamEnd)() override { };
	STDMETHOD_(void, OnBufferStart)(void*) override { };
	STDMETHOD_(void, OnBufferEnd)(void*) override { SetEvent(hBufferEndEvent); };
	STDMETHOD_(void, OnLoopEnd)(void*) override { };
	STDMETHOD_(void, OnVoiceError)(void*, HRESULT) override { };

	StreamingVoiceCallback() { ; }
	virtual ~StreamingVoiceCallback() { ; }
};

struct StreamEvent {

	std::wstring filename;
	bool loop = false;
	AudioType type = AudioType::MUSIC;

	StreamEvent() : filename(L""), loop(false), type(AudioType::MUSIC) {};
	StreamEvent(const std::wstring& filenameIn, const bool loopIn, const AudioType typeIn) {
		this->filename = filenameIn;
		this->loop = loopIn;
		this->type = typeIn;
	}
	~StreamEvent() {};

	friend class AudioComponent;
};

class Audio
{
public:
	Audio();
	~Audio();

	void loadSound(const std::string &filename);
	int playSound(const std::string& filename);
	int streamSound(const std::string& filename);
	void pauseSound(int index);
	void pauseAllSounds();

	void updateAudio();

private:

	// TEMPORARY *-*-*-*-*-*-*-*-//
	bool m_singlePress1 = true; //
	bool m_singlePress2 = true;//
	// *-*-*-*-*-*-*-*-*-*-*-*//

	// The main 'XAudio2' engine
	IXAudio2* m_xAudio2 = nullptr;
	// Represents the audio output device
	IXAudio2MasteringVoice* m_masterVoice = nullptr;
	// Represents each loaded sound in the form of an 'object'
	IXAudio2SourceVoice* m_sourceVoice[SOUND_COUNT];
	
	int m_currIndex = 0;

	Microsoft::WRL::ComPtr<IMFAttributes> m_sourceReaderConfiguration; // Windows Media Foundation Source Reader Configuration
	// Streaming Variables
	SourceReaderCallback m_sourceReaderCallback;	// Callback class for the source reader
	StreamingVoiceCallback m_streamingVoiceCallback;// callback class for the source voice
	static const int m_maxBufferCount = 3;			// maximum number of buffers used during streaming
	bool m_stopStreaming = false;					// breaks the streaming thread

	StreamEvent* m_streamedMusic = nullptr;
	IMFSourceReader* m_tempSourceReader = nullptr;
	WAVEFORMATEX m_wfx = { 0 };


	// Initialization
	void initialize();
	// Read audio data from the harddrive; load file
	void loadFile(const std::wstring& filename, std::vector<BYTE>& audioData, WAVEFORMATEX** waveFormatEx, unsigned int& waveLength);
	// STREAM AUDIO
	// Streams a file from the harddrive
	void streamFile(const std::wstring& filename, XAUDIO2_VOICE_SENDS& sendList, const bool loop = false);
	// Actual loop of the streaming function
	void loopStream(IMFSourceReader* const sourceReader, IXAudio2SourceVoice* const sourceVoice, const bool loop = false);

	BYTE m_streamBuffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];
	OVERLAPPED m_overlapped = { 0 };
	std::thread* m_streamingThread = nullptr;
	XAUDIO2_VOICE_SENDS m_voiceSends;

	// PRIVATE FUNCTION
	//-----------------
	void initXAudio2();

	// Creates a source reader in asynchronous mode
	void createAsyncReader(const std::wstring& filename, IMFSourceReader** sourceReader, WAVEFORMATEX* wfx, size_t wfxSize);
	void endStream();
	// ----------------
};

#endif
