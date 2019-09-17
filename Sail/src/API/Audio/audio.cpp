#include "pch.h"
#include "API/Audio/audio.hpp"
#include "MemoryManager/MemoryManager/src/MemoryManager.h"
#include "Sail/Application.h"
#include "Sail/KeyCodes.h"
#include "API/Audio/GeneralFunctions.h"
#include <fstream>

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

		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::playSound()", "Failed to create the actual 'SourceVoice' for a sound file!");

		hr = m_sourceVoice[m_currIndex]->SubmitSourceBuffer(Application::getInstance()->getResourceManager().getAudioData(filename).getSoundBuffer());

		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::playSound()", "Failed to submit the 'sourceBuffer' to the 'sourceVoice' for a sound file!");

		hr = m_sourceVoice[m_currIndex]->Start(0);

		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loadSound()", "Failed submit processed audio data to data buffer for a audio file");

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

	//m_tempStreamThread = new std::thread(&Audio::streamingLoop, this, (this->m_sourceVoice[this->m_currIndex]));

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

		m_streamedMusic = new StreamEvent(stringToWString("../Audio/sampleLarge.wav"), true, AudioType::MUSIC);
		this->streamFile(stringToWString("../Audio/sampleLarge.wav"), m_voiceSends, true);

		if (true) {


		}
	}

	else if (!Input::IsKeyPressed(SAIL_KEY_2) && !m_singlePress2) {
		m_singlePress2 = true;
	}
}

void Audio::initialize() {

}

void Audio::loadFile(const std::wstring& filename, std::vector<BYTE>& audioData, WAVEFORMATEX** waveFormatEx, unsigned int& waveLength) {
	// handle errors
	HRESULT hr = S_OK;

	// stream index
	DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

	// create the source reader
	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	hr = MFCreateSourceReaderFromURL(filename.c_str(), m_sourceReaderConfiguration.Get(), sourceReader.GetAddressOf());
	errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to create source reader!");

	// select the first audio stream, and deselect all other streams
	hr = sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
	errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to disable streams!");

	hr = sourceReader->SetStreamSelection(streamIndex, true);
	errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to enable first audio stream!");

	// query information about the media file
	Microsoft::WRL::ComPtr<IMFMediaType> nativeMediaType;
	hr = sourceReader->GetNativeMediaType(streamIndex, 0, nativeMediaType.GetAddressOf());
	errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to query media!");

	// make sure that this is really an audio file
	GUID majorType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
	errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "The requested file was not an audio file!");

	// check whether the audio file is compressed or uncompressed
	GUID subType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
	if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM)
	{
		// the audio file is uncompressed
	}
	else
	{
		// the audio file is compressed; we have to decompress it first
		// to do so, we inform the SourceReader that we want uncompressed data
		// this causes the SourceReader to look for decoders to perform our request
		Microsoft::WRL::ComPtr<IMFMediaType> partialType = nullptr;
		hr = MFCreateMediaType(partialType.GetAddressOf());
		errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to create media type!");

		// set the media type to "audio"
		hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to set media type to audio!");

		// request uncompressed data
		hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
		errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to set GUID of media type to 'uncompressed'!");

		hr = sourceReader->SetCurrentMediaType(streamIndex, NULL, partialType.Get());
		errorCheck(hr, "AUDIO ERROR!", "Audio::loadFile()", "Failed to set current media type!");
	}

	// uncompress the data and load it into an XAudio2 Buffer
	Microsoft::WRL::ComPtr<IMFMediaType> uncompressedAudioType = nullptr;
	hr = sourceReader->GetCurrentMediaType(streamIndex, uncompressedAudioType.GetAddressOf());
	//if (FAILED(hr))
	//	return std::runtime_error("Critical error: Unable to retrieve the current media type!");

	//hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType.Get(), waveFormatEx, &waveFormatLength);
	//if (FAILED(hr))
	//	return std::runtime_error("Critical error: Unable to create the wave format!");

	// ensure the stream is selected
	hr = sourceReader->SetStreamSelection(streamIndex, true);
	//if (FAILED(hr))
	//	return std::runtime_error("Critical error: Unable to select audio stream!");

	// copy data into byte vector
	Microsoft::WRL::ComPtr<IMFSample> sample = nullptr;
	Microsoft::WRL::ComPtr<IMFMediaBuffer> buffer = nullptr;
	BYTE* localAudioData = NULL;
	DWORD localAudioDataLength = 0;

	while (true)
	{
		DWORD flags = 0;
		hr = sourceReader->ReadSample(streamIndex, 0, nullptr, &flags, nullptr, sample.GetAddressOf());
		//if (FAILED(hr))
		//	return std::runtime_error("Critical error: Unable to read audio sample!");

		// check whether the data is still valid
		if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
			break;

		// check for eof
		if (flags & MF_SOURCE_READERF_ENDOFSTREAM)
			break;

		if (sample == nullptr)
			continue;

		// convert data to contiguous buffer
		hr = sample->ConvertToContiguousBuffer(buffer.GetAddressOf());
		//if (FAILED(hr))
		//	return std::runtime_error("Critical error: Unable to convert audio sample to contiguous buffer!");

		// lock buffer and copy data to local memory
		hr = buffer->Lock(&localAudioData, nullptr, &localAudioDataLength);
		//if (FAILED(hr))
		//	return std::runtime_error("Critical error: Unable to lock the audio buffer!");

		for (size_t i = 0; i < localAudioDataLength; i++)
			audioData.push_back(localAudioData[i]);

		// unlock the buffer
		hr = buffer->Unlock();
		localAudioData = nullptr;

		//if (FAILED(hr))
		//	return std::runtime_error("Critical error while unlocking the audio buffer!");
	}

	// return success
	return;
}

void Audio::streamFile(const std::wstring& filename, XAUDIO2_VOICE_SENDS& sendList, const bool loop) {
	HRESULT hr = S_OK;

	DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

	// Create the asynchronous source reader
	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	WAVEFORMATEX waveFormat;
	createAsyncReader(filename, sourceReader.GetAddressOf(), &waveFormat, sizeof(waveFormat));

	// Create the source voice
	IXAudio2SourceVoice* sourceVoice;
	hr = m_xAudio2->CreateSourceVoice(&sourceVoice, &waveFormat, 0, XAUDIO2_DEFAULT_FREQ_RATIO, &m_streamingVoiceCallback, &sendList, nullptr);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::streamFile()", "Failed to create source voice for streaming!");
	sourceVoice->Start();

	// Loop
	loopStream(sourceReader.Get(), sourceVoice, loop);

	sourceReader->Flush(streamIndex);
	sourceVoice->DestroyVoice();
	sourceReader = nullptr;

	return;
}

void Audio::loopStream(IMFSourceReader* const sourceReader, IXAudio2SourceVoice* const sourceVoice, const bool loop) {

	DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;
	DWORD currentStreamBuffer = 0;
	HRESULT hr = S_OK;
	size_t bufferSize[m_maxBufferCount] = { 0 };
	std::unique_ptr<uint8_t[]> buffers[m_maxBufferCount];

	while (true)
	{
		if (m_stopStreaming) {
			break;
		}

		hr = sourceReader->ReadSample(streamIndex, 0, nullptr, nullptr, nullptr, nullptr);
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loopStream()", "Error reading source sample!");

		WaitForSingleObject(m_sourceReaderCallback.hReadSample, INFINITE);

		if (m_sourceReaderCallback.endOfStream) {
			if (loop) {

				// Restart the stream
				m_sourceReaderCallback.Restart();
				PROPVARIANT var = { 0 };
				var.vt = VT_I8;
				hr = sourceReader->SetCurrentPosition(GUID_NULL, var);

				if (SUCCEEDED(hr)) {
					continue;
				}
				else {
					errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loopStream()", "Error when setting source reader's position!");
				}
			}
		}
		else {
			break;
		}

		Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
		hr = m_sourceReaderCallback.sample->ConvertToContiguousBuffer(mediaBuffer.GetAddressOf());
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: loopStream()", "Error converting audio data to contiguous buffer!");

		BYTE* audioData = nullptr;
		DWORD sampleBufferLength = 0;

		hr = mediaBuffer->Lock(&audioData, nullptr, &sampleBufferLength);
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loopStream()", "Error creating the lock for the media buffer!");

		if (bufferSize[currentStreamBuffer] < sampleBufferLength)
		{
			buffers[currentStreamBuffer].reset(new uint8_t[sampleBufferLength]);
			bufferSize[currentStreamBuffer] = sampleBufferLength;
		}

		memcpy_s(buffers[currentStreamBuffer].get(), sampleBufferLength, audioData, sampleBufferLength);

		hr = mediaBuffer->Unlock();
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loopStream()", "Error unklock the media buffer!");

		// Wait until 'xAudio2' source has played enough data.
		// Only ('maxBufferCount' - 1) buffers queued
		// making sure there is always one free buffer for 'Media Foundation' streamer.
		XAUDIO2_VOICE_STATE state;

		while (true) {

			sourceVoice->GetState(&state);
			if (state.BuffersQueued < m_maxBufferCount - 1) {
				break;
			}

			WaitForSingleObject(m_streamingVoiceCallback.hBufferEndEvent, INFINITE);
		}

		XAUDIO2_BUFFER buf = { 0 };
		buf.AudioBytes = sampleBufferLength;
		buf.pAudioData = buffers[currentStreamBuffer].get();
		sourceVoice->SubmitSourceBuffer(&buf);

		currentStreamBuffer++;
		currentStreamBuffer %= m_maxBufferCount;
	}

	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::loopStream()", "Error looping through the media stream!");

	return;
}

void Audio::initXAudio2() {

	HRESULT hr;

	hr = XAudio2Create(&m_xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);

	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::initXAudio2()", "Creating the 'IXAudio2' object failed!");

	hr = m_xAudio2->CreateMasteringVoice(&m_masterVoice);

	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::initXAudio2()", "Creating the 'IXAudio2MasterVoice' failed!");

	m_overlapped.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
}

void Audio::createAsyncReader(const std::wstring& filename, IMFSourceReader** sourceReader, WAVEFORMATEX* wfx, size_t wfxSize) {

	HRESULT hr = S_OK;

	// Set source reader to 'asynchronous mode'
	hr = m_sourceReaderConfiguration->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, &m_sourceReaderCallback);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to setup source reader for asynchronous reading!");

	// Create the source reader
	hr = MFCreateSourceReaderFromURL(filename.c_str(), m_sourceReaderConfiguration.Get(), sourceReader);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to create source reader!");

	// Stream index
	DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

	// Deselect all streams
	hr = (*sourceReader)->SetStreamSelection((DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to disable streams!");

	// Select chosen stream
	hr = (*sourceReader)->SetStreamSelection(streamIndex, true);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to select audio stream!");

	// Query Information about the media file
	Microsoft::WRL::ComPtr<IMFMediaType> nativeMediaType;
	hr = (*sourceReader)->GetNativeMediaType(streamIndex, 0, nativeMediaType.GetAddressOf());
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Request to get media information failed!");

	// Confirm that it's an audio file
	GUID majorType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &majorType);
	if (majorType != MFMediaType_Audio) {
		hr = E_FAIL;
	}
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "The fetched file was not an audio file!");

	GUID subType{};
	hr = nativeMediaType->GetGUID(MF_MT_MAJOR_TYPE, &subType);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "'GetGUID' fetch-method failed!");

	// UNCOMPRESSED
	if (subType == MFAudioFormat_Float || subType == MFAudioFormat_PCM) {
		//  ----------
		/*	DO NOTHING	*/
		//  ----------  //
	}
	else {

		// COMPRESSED; must uncompress first

		Microsoft::WRL::ComPtr<IMFMediaType> partialType = nullptr;
		hr = MFCreateMediaType(partialType.GetAddressOf());
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to create media type!");

		// Set the media type to 'audio'
		hr = partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to set media type to audio!");

		// Request uncompressed data
		hr = partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to set GUID of media type!");

		hr = (*sourceReader)->SetCurrentMediaType(streamIndex, NULL, partialType.Get());
		errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to set current media type!");
	}

	// Uncompress (Extract?) the data
	Microsoft::WRL::ComPtr<IMFMediaType> uncompressedAudioType = nullptr;
	hr = (*sourceReader)->GetCurrentMediaType(streamIndex, uncompressedAudioType.GetAddressOf());
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to fetch current media type!");

	UINT32 waveFormatSize = 0;
	WAVEFORMATEX* waveFormat = nullptr;
	hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType.Get(), &waveFormat, &waveFormatSize);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to create 'wave' format!");

	// Confirm that the stream is currently selected
	hr = (*sourceReader)->SetStreamSelection(streamIndex, true);
	errorCheck(hr, "AUDIO ERROR!", "FUNCTION: Audio::createAsyncReader", "Failed to select audio stream!");

	// Copy Data
	memcpy_s(wfx, wfxSize, waveFormat, waveFormatSize);
	CoTaskMemFree(waveFormat);

	// SUCCESS!
	return;
}

void Audio::endStream() {

	m_stopStreaming = true;

	if (m_streamingThread->joinable()) {
		m_streamingThread->join();
	}
}



/////////////////////////////////////
// BACK-UP FROM THE FIRST ATTEMPT //
///////////////////////////////////
/*
int currentDiskReadBuffer = 0;
	int currentPos = 0;

	AudioData* tempAudioData = &Application::getInstance()->getResourceManager().getAudioData(filename);
	int samplesPerSec = tempAudioData->getFormat()->Format.nSamplesPerSec;
	int bitsPerSample = tempAudioData->getFormat()->Format.wBitsPerSample;
	double soundDuration = static_cast<double>(tempAudioData->getSoundBuffer()->AudioBytes / static_cast<UINT32>(tempAudioData->getFormat()->Format.nAvgBytesPerSec));

	long cbWaveSize = 0.1;
	DWORD cbValid;
	DWORD dwRead;
	
	std::wstring fileNameWSTR = stringToWString(filename);
	LPCWSTR fileNameLPCWSTR = fileNameWSTR.c_str();
	HANDLE hFile = CreateFile(
		fileNameLPCWSTR,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		0,
		nullptr);
*/